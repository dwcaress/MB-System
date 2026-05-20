/**
 * SlopeColorApp.cxx
 *
 * VTK 9 example: colors a terrain surface by elevation (via a lookup table),
 * then darkens each vertex in proportion to local surface slope.
 *
 * Two implementations are selectable at runtime:
 *
 *   (1) CPU path  — vtkProgrammableFilter merges elevation color + slope
 *                   darkening per vertex on the CPU, writes an RGBA array,
 *                   and the mapper uses it via DirectScalars.
 *
 *   (2) GPU path  — mapper does its normal elevation→LUT mapping; a GLSL
 *                   shader replacement modulates the fragment color by a
 *                   brightness factor derived from the world-space normal.
 *                   Two custom uniforms drive the gamma and floor live.
 *
 * Two on-screen sliders (Gamma, Floor) work in both modes.  In CPU mode
 * the slider callback marks the programmable filter dirty so it re-runs
 * the per-vertex math.  In GPU mode it just pokes two uniforms.
 *
 * Usage:
 *   SlopeColorApp [--shader] [dataFile]
 *
 *     --shader      use the GLSL shader-replacement path
 *     (no flag)     use the CPU programmable-filter path
 *     dataFile      optional input; otherwise a sphere is used
 */

#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// VTK core
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>

// Filters
#include <vtkElevationFilter.h>

// Source
#include <vtkSphereSource.h>
#include "TopoDataReader.h"

// Interactor style
#include <vtkInteractorStyleTrackballCamera.h>

// Sliders
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkTextProperty.h>

// Shader replacement (GPU path)
#include <vtkShader.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>

// ═════════════════════════════════════════════════════════════════════════════
//  CPU path: programmable-filter callback data and execute function
// ═════════════════════════════════════════════════════════════════════════════
struct CallbackData
{
  vtkProgrammableFilter* filter;        // owns this callback
  vtkPolyDataNormals*    normalsFilter; // parallel branch with normals
  vtkLookupTable*        lut;           // elevation LUT
  double                 elevMin;
  double                 elevMax;
  double                 slopeGamma;    // mutable — slider updates this
  double                 minBrightness; // mutable — slider updates this
};

static void SlopeColorExecute(void* userData)
{
  auto* d = static_cast<CallbackData*>(userData);

  auto* elevPD = vtkPolyData::SafeDownCast(d->filter->GetInput());
  if (!elevPD)
  {
    std::cerr << "[SlopeColorExecute] primary input is not vtkPolyData\n";
    return;
  }

  auto* normalsPD = d->normalsFilter->GetOutput();
  if (!normalsPD)
  {
    std::cerr << "[SlopeColorExecute] normals output is null\n";
    return;
  }

  const vtkIdType nPts = elevPD->GetNumberOfPoints();

  auto* elevScalars = vtkFloatArray::SafeDownCast(
      elevPD->GetPointData()->GetArray("Elevation"));
  if (!elevScalars)
  {
    elevScalars = vtkFloatArray::SafeDownCast(
        elevPD->GetPointData()->GetScalars());
  }

  auto* normalsArray = vtkFloatArray::SafeDownCast(
      normalsPD->GetPointData()->GetNormals());

  auto* outPD = vtkPolyData::SafeDownCast(d->filter->GetOutput());
  outPD->ShallowCopy(elevPD);

  auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetNumberOfComponents(4);
  colors->SetNumberOfTuples(nPts);
  colors->SetName("Colors");

  for (vtkIdType i = 0; i < nPts; ++i)
  {
    double elev = elevScalars
        ? static_cast<double>(elevScalars->GetValue(i))
        : d->elevMin;

    double rgba[4] = {0.5, 0.5, 0.5, 1.0};
    d->lut->GetColor(elev, rgba);
    rgba[3] = 1.0;

    double normal[3] = {0.0, 0.0, 1.0};
    if (normalsArray)
      normalsArray->GetTuple(i, normal);

    const double Nz          = std::abs(normal[2]);
    const double slopeFactor = 1.0 - Nz;
    const double darkening   = std::pow(slopeFactor, d->slopeGamma);
    const double brightness  = 1.0 - darkening * (1.0 - d->minBrightness);

    unsigned char pix[4];
    pix[0] = static_cast<unsigned char>(std::min(rgba[0] * brightness * 255.0, 255.0));
    pix[1] = static_cast<unsigned char>(std::min(rgba[1] * brightness * 255.0, 255.0));
    pix[2] = static_cast<unsigned char>(std::min(rgba[2] * brightness * 255.0, 255.0));
    pix[3] = 255;
    colors->SetTypedTuple(i, pix);
  }

  outPD->GetPointData()->SetScalars(colors);
}

static void DeleteCallbackData(void* userData)
{
  delete static_cast<CallbackData*>(userData);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Shared application state — slider callbacks read/write this
// ═════════════════════════════════════════════════════════════════════════════
struct AppState
{
  bool   useShader     = false;
  double slopeGamma    = 1.5;
  double minBrightness = 0.15;

  // CPU-path handles
  vtkProgrammableFilter* slopeColorFilter = nullptr;
  CallbackData*          callbackData     = nullptr;

  // GPU-path handles
  vtkActor* surfaceActor = nullptr;

  vtkRenderWindow* renderWindow = nullptr;
};

// ═════════════════════════════════════════════════════════════════════════════
//  Slider callback — same observer drives both sliders, distinguished by `which`
// ═════════════════════════════════════════════════════════════════════════════
class SlopeParamCallback : public vtkCommand
{
public:
  enum Param { Gamma, Floor };

  static SlopeParamCallback* New() { return new SlopeParamCallback; }

  AppState* state = nullptr;
  Param     which = Gamma;

  void Execute(vtkObject* caller, unsigned long, void*) override
  {
    auto* widget = static_cast<vtkSliderWidget*>(caller);
    auto* rep    = static_cast<vtkSliderRepresentation*>(widget->GetRepresentation());
    const double value = rep->GetValue();

    if (which == Gamma)
      state->slopeGamma = value;
    else
      state->minBrightness = value;

    if (state->useShader)
    {
      // GPU path: poke uniforms — no shader recompile, no CPU pass.
      auto* uniforms = state->surfaceActor->GetShaderProperty()
                                          ->GetFragmentCustomUniforms();
      uniforms->SetUniformf("slopeGamma",
                            static_cast<float>(state->slopeGamma));
      uniforms->SetUniformf("minBrightness",
                            static_cast<float>(state->minBrightness));
    }
    else if (state->callbackData)
    {
      // CPU path: update params and force the programmable filter to re-run.
      state->callbackData->slopeGamma    = state->slopeGamma;
      state->callbackData->minBrightness = state->minBrightness;
      state->slopeColorFilter->Modified();
    }

    state->renderWindow->Render();
  }
};

// ═════════════════════════════════════════════════════════════════════════════
//  Small helper to build a 2D slider widget
// ═════════════════════════════════════════════════════════════════════════════
static vtkSmartPointer<vtkSliderWidget>
makeSlider(vtkRenderWindowInteractor* interactor,
           const char*                title,
           double                     minVal,
           double                     maxVal,
           double                     initVal,
           double                     yNorm,
           AppState*                  state,
           SlopeParamCallback::Param  which)
{
  auto rep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
  rep->SetMinimumValue(minVal);
  rep->SetMaximumValue(maxVal);
  rep->SetValue(initVal);
  rep->SetTitleText(title);

  rep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  rep->GetPoint1Coordinate()->SetValue(0.05, yNorm);
  rep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  rep->GetPoint2Coordinate()->SetValue(0.35, yNorm);

  rep->SetSliderLength(0.025);
  rep->SetSliderWidth(0.035);
  rep->SetEndCapLength(0.01);
  rep->SetTubeWidth(0.006);
  rep->SetLabelFormat("%0.2f");

  rep->GetTitleProperty()->SetColor(0.95, 0.95, 0.95);
  rep->GetLabelProperty()->SetColor(0.95, 0.95, 0.95);

  auto widget = vtkSmartPointer<vtkSliderWidget>::New();
  widget->SetInteractor(interactor);
  widget->SetRepresentation(rep);
  widget->SetAnimationModeToAnimate();
  widget->EnabledOn();

  auto cb = vtkSmartPointer<SlopeParamCallback>::New();
  cb->state = state;
  cb->which = which;
  widget->AddObserver(vtkCommand::InteractionEvent, cb);

  return widget;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Build a hand-crafted terrain LUT (deep-blue → green → brown → snow)
// ═════════════════════════════════════════════════════════════════════════════
static void buildTerrainLUT(vtkLookupTable* lut, double elevMin, double elevMax)
{
  const int N = 256;
  lut->SetNumberOfTableValues(N);
  lut->SetRange(elevMin, elevMax);
  lut->Build();

  for (int i = 0; i < N; ++i)
  {
    const double t = static_cast<double>(i) / (N - 1);
    double r, g, b;
    if (t < 0.25)
    {
      double s = t / 0.25;
      r = 0.0;
      g = 0.0 + s * 0.3;
      b = 0.4 + s * 0.4;
    }
    else if (t < 0.45)
    {
      double s = (t - 0.25) / 0.20;
      r = 0.0  + s * 0.40;
      g = 0.30 + s * 0.35;
      b = 0.80 - s * 0.60;
    }
    else if (t < 0.70)
    {
      double s = (t - 0.45) / 0.25;
      r = 0.40 + s * 0.35;
      g = 0.65 - s * 0.35;
      b = 0.20 - s * 0.15;
    }
    else
    {
      double s = (t - 0.70) / 0.30;
      r = 0.75 + s * 0.25;
      g = 0.30 + s * 0.70;
      b = 0.05 + s * 0.95;
    }
    lut->SetTableValue(i, r, g, b, 1.0);
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  GPU path: install shader replacements on the actor
// ═════════════════════════════════════════════════════════════════════════════
static void installSlopeShader(vtkActor* actor,
                               double slopeGamma,
                               double minBrightness)
{
  auto* sp = actor->GetShaderProperty();

  // Vertex shader: declare the normal attribute ourselves (with LightingOff,
  // vtkOpenGLPolyDataMapper does not emit `in vec3 normalMC;` — the VBO is
  // still uploaded, but the shader source has no declaration to bind it to).
  // Then forward the model-space normal to the fragment shader.
  sp->AddVertexShaderReplacement(
      "//VTK::Normal::Dec",
      true,
      "//VTK::Normal::Dec\n"
      "in  vec3 normalMC;\n"
      "out vec3 slopeNormalMC;\n",
      false);

  sp->AddVertexShaderReplacement(
      "//VTK::Normal::Impl",
      true,
      "//VTK::Normal::Impl\n"
      "slopeNormalMC = normalMC;\n",
      false);

  // Fragment shader: matching declaration, then darken by |Nz| after the LUT
  // has populated ambientColor and diffuseColor.
  sp->AddFragmentShaderReplacement(
      "//VTK::Normal::Dec",
      true,
      "//VTK::Normal::Dec\n"
      "in vec3 slopeNormalMC;\n",
      false);

  sp->AddFragmentShaderReplacement(
      "//VTK::Color::Impl",
      true,
      "//VTK::Color::Impl\n"
      "{\n"
      "  float Nz          = abs(normalize(slopeNormalMC).z);\n"
      "  float slopeFactor = 1.0 - Nz;\n"
      "  float darkening   = pow(slopeFactor, slopeGamma);\n"
      "  float brightness  = 1.0 - darkening * (1.0 - minBrightness);\n"
      "  ambientColor *= brightness;\n"
      "  diffuseColor *= brightness;\n"
      "}\n",
      false);

  // Initial uniform values; sliders update these later.
  sp->GetFragmentCustomUniforms()
      ->SetUniformf("slopeGamma",    static_cast<float>(slopeGamma));
  sp->GetFragmentCustomUniforms()
      ->SetUniformf("minBrightness", static_cast<float>(minBrightness));
}

// ═════════════════════════════════════════════════════════════════════════════
int main(int argc, char* argv[])
{
  AppState state;

  // ── Argument parsing ──────────────────────────────────────────────────────
  const char* dataFile = nullptr;
  for (int i = 1; i < argc; ++i)
  {
    std::string a = argv[i];
    if (a == "--shader" || a == "--glsl" || a == "--gpu")
    {
      state.useShader = true;
    }
    else if (a == "-h" || a == "--help")
    {
      std::cout << "usage: " << argv[0] << " [--shader] [dataFile]\n";
      return 0;
    }
    else if (!a.empty() && a[0] != '-')
    {
      dataFile = argv[i];
    }
    else
    {
      std::cerr << "Unknown argument: " << a << "\n";
      std::cerr << "usage: " << argv[0] << " [--shader] [dataFile]\n";
      return 1;
    }
  }
  std::cout << "Mode: "
            << (state.useShader ? "GLSL shader replacement"
                                : "vtkProgrammableFilter (CPU)")
            << "\n";

  // ── 0. Source (reader or synthetic sphere) ────────────────────────────────
  vtkSmartPointer<vtkPolyDataAlgorithm> source;
  if (dataFile)
  {
    auto reader = vtkSmartPointer<mb_system::TopoDataReader>::New();
    reader->SetFileName(dataFile);
    if (reader->GetErrorCode() != 0)
    {
      std::cerr << "Failed to read data file: " << dataFile << "\n";
      return 1;
    }
    source = reader;
  }
  else
  {
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetPhiResolution(80);
    sphere->SetThetaResolution(80);
    sphere->SetRadius(1.0);
    source = sphere;
  }
  source->Update();

  double bounds[6];
  source->GetOutput()->GetBounds(bounds);
  const double elevMin = bounds[4];
  const double elevMax = bounds[5];

  // ── 1. Elevation filter ───────────────────────────────────────────────────
  auto elevationFilter = vtkSmartPointer<vtkElevationFilter>::New();
  elevationFilter->SetInputConnection(source->GetOutputPort());
  elevationFilter->SetLowPoint (0.0, 0.0, elevMin);
  elevationFilter->SetHighPoint(0.0, 0.0, elevMax);
  elevationFilter->SetScalarRange(elevMin, elevMax);
  elevationFilter->Update();   // force exec so the sanity check below is meaningful

  // ── 2. Normals filter ─────────────────────────────────────────────────────
  //
  // In CPU mode the normals branch is parallel to the elevation branch and
  // the programmable filter reads both.  In GPU mode the normals filter sits
  // in series after the elevation filter so the mapper sees both the
  // "Elevation" scalar and the point normals on a single polydata.
  //
  auto normalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
  if (state.useShader)
    normalsFilter->SetInputConnection(elevationFilter->GetOutputPort());
  else
    normalsFilter->SetInputConnection(source->GetOutputPort());

  normalsFilter->ComputePointNormalsOn();
  normalsFilter->ComputeCellNormalsOff();
  normalsFilter->SplittingOff();                   // 1:1 vertex correspondence
  normalsFilter->ConsistencyOn();
  normalsFilter->AutoOrientNormalsOn();
  normalsFilter->Update();

  // Sanity check (CPU mode needs identical vertex counts in both branches)
  if (!state.useShader)
  {
    const vtkIdType nElev = elevationFilter->GetOutput()->GetNumberOfPoints();
    const vtkIdType nNorm = normalsFilter->GetOutput()->GetNumberOfPoints();
    if (nElev != nNorm)
    {
      std::cerr << "ERROR: vertex count mismatch — elev=" << nElev
                << "  norm=" << nNorm << "\n";
      return EXIT_FAILURE;
    }
  }

  // ── 3. Lookup table ───────────────────────────────────────────────────────
  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  buildTerrainLUT(lut, elevMin, elevMax);

  // ── 4. Mapper & actor (mode-independent shells) ───────────────────────────
  auto surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  auto surfaceActor  = vtkSmartPointer<vtkActor>::New();
  surfaceActor->SetMapper(surfaceMapper);
  state.surfaceActor = surfaceActor;

  // ── 5. Mode-specific pipeline tail ────────────────────────────────────────
  vtkSmartPointer<vtkProgrammableFilter> slopeColorFilter;

  if (state.useShader)
  {
    // GPU path: mapper does the LUT lookup; shader replacement darkens.
    surfaceMapper->SetInputConnection(normalsFilter->GetOutputPort());
    surfaceMapper->ScalarVisibilityOn();
    surfaceMapper->SetLookupTable(lut);
    surfaceMapper->SetScalarRange(elevMin, elevMax);
    surfaceMapper->SetColorModeToMapScalars();
    surfaceMapper->SetScalarModeToUsePointData();

    // Turn off VTK lighting so we don't double-shade.
    surfaceActor->GetProperty()->LightingOff();

    installSlopeShader(surfaceActor, state.slopeGamma, state.minBrightness);
  }
  else
  {
    // CPU path: programmable filter writes RGBA, mapper uses it verbatim.
    slopeColorFilter = vtkSmartPointer<vtkProgrammableFilter>::New();
    slopeColorFilter->SetInputConnection(elevationFilter->GetOutputPort());

    auto* cbd = new CallbackData{
        slopeColorFilter.Get(),
        normalsFilter.Get(),
        lut.Get(),
        elevMin, elevMax,
        state.slopeGamma,
        state.minBrightness};

    slopeColorFilter->SetExecuteMethod(SlopeColorExecute, cbd);
    slopeColorFilter->SetExecuteMethodArgDelete(DeleteCallbackData);

    state.slopeColorFilter = slopeColorFilter.Get();
    state.callbackData     = cbd;

    surfaceMapper->SetInputConnection(slopeColorFilter->GetOutputPort());
    surfaceMapper->SetColorModeToDirectScalars();
    surfaceMapper->ScalarVisibilityOn();

    surfaceActor->GetProperty()->SetAmbient(0.1);
    surfaceActor->GetProperty()->SetDiffuse(0.9);
  }

  // ── 6. Renderer / window / interactor ─────────────────────────────────────
  auto renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(surfaceActor);
  renderer->SetBackground(0.15, 0.15, 0.20);

  auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(900, 700);
  renderWindow->SetWindowName(state.useShader
      ? "Elevation + Slope Darkening (GLSL)"
      : "Elevation + Slope Darkening (CPU)");
  state.renderWindow = renderWindow;

  auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactor->SetInteractorStyle(style);
  interactor->Initialize();   // required before enabling 3D widgets

  // ── 7. Sliders ────────────────────────────────────────────────────────────
  auto gammaSlider = makeSlider(interactor,
                                "Gamma",
                                0.5, 5.0, state.slopeGamma,
                                0.10,
                                &state, SlopeParamCallback::Gamma);

  auto floorSlider = makeSlider(interactor,
                                "Floor (min brightness)",
                                0.0, 1.0, state.minBrightness,
                                0.20,
                                &state, SlopeParamCallback::Floor);

  // ── 8. Camera & go ────────────────────────────────────────────────────────
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Elevation(30.0);
  renderer->GetActiveCamera()->Azimuth(45.0);
  renderer->ResetCameraClippingRange();

  renderWindow->Render();
  interactor->Start();

  return EXIT_SUCCESS;
}
