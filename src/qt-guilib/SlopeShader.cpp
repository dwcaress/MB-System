
#include <cmath>
#include <iostream>
#include <memory>

#include "SlopeShader.h"

using namespace mb_system;


void SlopeShader::execute(void* userData) {
  auto* d = static_cast<CallbackData*>(userData);

  // ── Inputs ────────────────────────────────────────────────────────────────
  // Primary input: polydata carrying the "Elevation" scalar
  auto* elevPD = vtkPolyData::SafeDownCast(d->filter->GetInput());
  if (!elevPD) {
    std::cerr << "SlopeShader::Execute(): ERROR: primary input is not vtkPolyData\n";
      return;
  }

  // Secondary branch: polydata carrying per-vertex normals
  auto* normalsPD = d->normalsFilter->GetOutput();
  if (!normalsPD) {
    std::cerr << "SlopeShader::Execute ERROR: normals output is null\n";
    return;
  }

  const vtkIdType nPts = elevPD->GetNumberOfPoints();

  // Retrieve the "Elevation" scalar array produced by vtkElevationFilter
  auto* elevScalars = vtkFloatArray::SafeDownCast(
						  elevPD->GetPointData()->GetArray("Elevation"));
  if (!elevScalars) {
      // vtkElevationFilter may produce a generic float array set as scalars
      elevScalars = vtkFloatArray::SafeDownCast(
						elevPD->GetPointData()->GetScalars());
    }
  if (!elevScalars)
    std::cerr << "[SlopeColorExecute] WARNING: no elevation scalar found; "
      "colors will be slope-only\n";

  auto* normalsArray =
    vtkFloatArray::SafeDownCast(normalsPD->GetPointData()->GetNormals());

  if (!normalsArray) {
    std::cerr << "[SlopeColorExecute] WARNING: no normals found; "
      "slope darkening disabled\n";
  }
  
  // ── Output ────────────────────────────────────────────────────────────────
  auto* outPD = vtkPolyData::SafeDownCast(d->filter->GetOutput());
  outPD->ShallowCopy(elevPD); // copy topology + all arrays

  auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetNumberOfComponents(4); // RGBA
  colors->SetNumberOfTuples(nPts);
  colors->SetName("Colors");

  // ── Per-vertex loop ───────────────────────────────────────────────────────
  for (vtkIdType i = 0; i < nPts; ++i) {
      // ── 1. Elevation → base RGBA via lookup table ─────────────────────────
      double elev = elevScalars ? static_cast<double>(elevScalars->GetValue(i))
	: d->elevMin;
      double rgba[4] = {0.5, 0.5, 0.5, 1.0};
      d->lut->GetColor(elev, rgba); // writes r,g,b into rgba[0..2], range 0–1
      rgba[3] = 1.0;

      // ── 2. Slope from the surface normal's Z component ────────────────────
      //
      //   Normal is a unit vector.
      //   |Nz| = cos(θ)  where θ is the angle from vertical (Z axis).
      //
      //   Flat  surface → θ = 0  → |Nz| = 1 → slopeFactor = 0 (no darkening)
      //   Cliff surface → θ = 90 → |Nz| = 0 → slopeFactor = 1 (max darkening)
      //
      double normal[3] = {0.0, 0.0, 1.0}; // default: flat
      if (normalsArray)
	normalsArray->GetTuple(i, normal);

      const double Nz          = std::abs(normal[2]);        // 0 … 1
      const double slopeFactor = 1.0 - Nz;                   // 0 (flat) … 1 (steep)

      // Gamma compresses the low end: only genuinely steep slopes get dark.
      const double darkening   = std::pow(slopeFactor, d->slopeGamma);
      const double brightness  = 1.0 - darkening * (1.0 - d->minBrightness);
      //  flat  → brightness ≈ 1.0
      //  steep → brightness ≈ minBrightness

      // ── 3. Modulate and store as unsigned char RGBA ───────────────────────
      unsigned char pix[4];
      pix[0] = static_cast<unsigned char>(
					  std::min(rgba[0] * brightness * 255.0, 255.0));
      pix[1] = static_cast<unsigned char>(
					  std::min(rgba[1] * brightness * 255.0, 255.0));
      pix[2] = static_cast<unsigned char>(
					  std::min(rgba[2] * brightness * 255.0, 255.0));
      pix[3] = 255;

      colors->SetTypedTuple(i, pix);
    }

  // Replace the scalars on the output with our RGBA array.
  // SetScalars() also marks the array as the active scalar,
  // which is what vtkPolyDataMapper with DirectScalars mode reads.
  outPD->GetPointData()->SetScalars(colors);
}


void SlopeShader::deleteCallbackData(void* userData) {
  delete static_cast<CallbackData*>(userData);
}


#if 0
// ═════════════════════════════════════════════════════════════════════════════
int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkPolyDataAlgorithm> source;
  vtkSmartPointer<mb_system::TopoDataReader> reader =
    vtkSmartPointer<mb_system::TopoDataReader>::New();

  // Elevation extents (world-space Z range of the source)
  double elevMin = -1.0;
  double elevMax =  1.0;

  
  if (argc == 2) {
    // Data file is second arg
    char *dataFile = argv[1];
    reader->SetFileName(dataFile);
    unsigned long error;
    if ((error = reader->GetErrorCode()) != 0) {
      std::cerr << "Failed to read data file\n";
      exit(1);
    }

    source = reader;
  }
  else if (argc == 1) {
    // ── 0. Synthetic terrain source ───────────────────────────────────────────
    //
    //  Replace this with your own reader, e.g.:
    //    auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    //    reader->SetFileName("terrain.vtp");
    //
    //  The rest of the pipeline is identical.
    //
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();

    sphere->SetPhiResolution(80);
    sphere->SetThetaResolution(80);
    sphere->SetRadius(1.0);
    source = sphere;
  }
  else {
    std::cerr << "Invalid arguments\n";
    std::cerr << "usage: " << argv[0] << " [dataFile]\n";
    exit(1);
  }

  source->Update();
  double bounds[6];
  source->GetOutput()->GetBounds(bounds);
  elevMin = bounds[4];
  elevMax = bounds[5];
  
  // ── 1. Elevation filter ───────────────────────────────────────────────────
  auto elevationFilter = vtkSmartPointer<vtkElevationFilter>::New();
  elevationFilter->SetInputConnection(source->GetOutputPort());
  elevationFilter->SetLowPoint (0.0, 0.0, elevMin);
  elevationFilter->SetHighPoint(0.0, 0.0, elevMax);
  elevationFilter->SetScalarRange(elevMin, elevMax);
  elevationFilter->Update();

  // ── 2. Normals filter (parallel branch from the same source) ──────────────
  auto normalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsFilter->SetInputConnection(source->GetOutputPort());
  normalsFilter->ComputePointNormalsOn();
  normalsFilter->ComputeCellNormalsOff();
  //
  // CRITICAL: SplittingOff() prevents vertex duplication at sharp edges.
  // If splitting is on, the normals array has more tuples than the elevation
  // scalar array → index mismatch → wrong colors.
  //
  normalsFilter->SplittingOff();
  normalsFilter->ConsistencyOn();
  normalsFilter->AutoOrientNormalsOn();
  normalsFilter->Update();

  // Sanity check: both branches must have the same vertex count
  const vtkIdType nElev  = elevationFilter->GetOutput()->GetNumberOfPoints();
  const vtkIdType nNorm  = normalsFilter->GetOutput()->GetNumberOfPoints();
  if (nElev != nNorm)
    {
      std::cerr << "ERROR: point count mismatch – elevation=" << nElev
		<< "  normals=" << nNorm << "\n"
		<< "       Set SplittingOff() on vtkPolyDataNormals.\n";
      return EXIT_FAILURE;
    }
  std::cout << "Vertex count: " << nElev << "\n";

  // ── 3. Lookup table (elevation → color) ───────────────────────────────────
  //
  //  Classic terrain palette: deep-blue (low) → green → brown → white (high).
  //  Swap for any other vtkLookupTable / vtkColorTransferFunction you like.
  //
  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(256);
  lut->SetRange(elevMin, elevMax);
  lut->Build();

  // Override with a hand-crafted terrain ramp
  const int N = 256;
  for (int i = 0; i < N; ++i)
    {
      const double t = static_cast<double>(i) / (N - 1); // 0 … 1

      double r, g, b;
      if (t < 0.25)
        {   // deep water: dark blue → mid blue
	  double s = t / 0.25;
	  r = 0.0;
	  g = 0.0 + s * 0.3;
	  b = 0.4 + s * 0.4;
        }
      else if (t < 0.45)
        {   // shallow / lowland: blue → sandy green
	  double s = (t - 0.25) / 0.20;
	  r = 0.0  + s * 0.40;
	  g = 0.30 + s * 0.35;
	  b = 0.80 - s * 0.60;
        }
      else if (t < 0.70)
        {   // mid-elevation: green → brown
	  double s = (t - 0.45) / 0.25;
	  r = 0.40 + s * 0.35;
	  g = 0.65 - s * 0.35;
	  b = 0.20 - s * 0.15;
        }
      else
        {   // high: brown → snow white
	  double s = (t - 0.70) / 0.30;
	  r = 0.75 + s * 0.25;
	  g = 0.30 + s * 0.70;
	  b = 0.05 + s * 0.95;
        }
      lut->SetTableValue(i, r, g, b, 1.0);
    }

  // ── 4. Programmable filter: merge elevation color + slope darkening ────────
  auto slopeColorFilter = vtkSmartPointer<vtkProgrammableFilter>::New();
  slopeColorFilter->SetInputConnection(elevationFilter->GetOutputPort());

  // Heap-allocate the callback data; VTK will free it via DeleteCallbackData.
  auto* cbd = new CallbackData{
    slopeColorFilter.Get(),
    normalsFilter.Get(),
    lut.Get(),
    elevMin, elevMax,
    /*slopeGamma=*/    1.5,   // raise to limit darkening to very steep areas
    /*minBrightness=*/ 0.15   // 0 = pure black cliffs; 1 = no darkening at all
  };

  slopeColorFilter->SetExecuteMethod(SlopeColorExecute, cbd);
  slopeColorFilter->SetExecuteMethodArgDelete(DeleteCallbackData);
  slopeColorFilter->Update();

  // ── 5. Mapper ─────────────────────────────────────────────────────────────
  auto surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  surfaceMapper->SetInputConnection(slopeColorFilter->GetOutputPort());
  //
  // SetColorModeToDirectScalars():
  //   Treats the unsigned-char RGBA array as final pixel colors.
  //   No LUT pass-through — the mapper uses the values verbatim.
  //
  surfaceMapper->SetColorModeToDirectScalars();
  surfaceMapper->ScalarVisibilityOn();
  // Do NOT call SetLookupTable / SetScalarRange on the mapper;
  // those only apply when the mapper does its own LUT mapping.

  // ── 6. Actor ──────────────────────────────────────────────────────────────
  auto surfaceActor = vtkSmartPointer<vtkActor>::New();
  surfaceActor->SetMapper(surfaceMapper);
  surfaceActor->GetProperty()->SetAmbient(0.1);
  surfaceActor->GetProperty()->SetDiffuse(0.9);

  // ── 7. Renderer / window / interactor ─────────────────────────────────────
  auto renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(surfaceActor);
  renderer->SetBackground(0.15, 0.15, 0.20);
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Elevation(30.0);
  renderer->GetActiveCamera()->Azimuth(45.0);
  renderer->ResetCameraClippingRange();

  auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(900, 700);
  renderWindow->SetWindowName("Elevation + Slope Darkening");

  auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactor->SetInteractorStyle(style);

  renderWindow->Render();
  interactor->Start();

  return EXIT_SUCCESS;
}
#endif   // main() test 
