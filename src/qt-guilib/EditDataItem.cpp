#include "EditDataItem.h"
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkIdList.h>
#include <vtkShaderProperty.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkMatrix4x4.h>
// GL_PROGRAM_POINT_SIZE may not be exposed by VTK's transitive headers on all
// platforms; define the raw enum value directly (OpenGL 3.2 core, §14.4).
#ifndef GL_PROGRAM_POINT_SIZE
#  define GL_PROGRAM_POINT_SIZE 0x8642
#endif
#include <QDebug>
#include <QMetaObject>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QEvent>
#include <QQuickWindow>

using namespace mb_system;

// ─────────────────────────────────────────────────────────────────────────────
//  EditPickInteractorStyle — screen-space point-flagging interactor
//
//  Left-click            flag the single frontmost point under the cursor.
//  Left-drag             camera rotation (trackball, unchanged).
//  Alt+left-click        flag the single frontmost point under the cursor.
//  Alt+left-drag         paint — flag every point whose disc the cursor passes
//                        over; camera does NOT rotate.
//  Right-drag / scroll   zoom  (superclass, unchanged).
//  Middle-drag           pan   (superclass, unchanged).
//
//  Among candidate points inside the pick radius the FRONTMOST (smallest NDC
//  depth) is chosen, matching what is visually rendered on top.
// ─────────────────────────────────────────────────────────────────────────────

namespace mb_system {

class EditPickInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    vtkTypeMacro(EditPickInteractorStyle, vtkInteractorStyleTrackballCamera);

    static EditPickInteractorStyle *New() {
        return new EditPickInteractorStyle;
    }

    void setItem(EditDataItem *item) { item_ = item; }

    /// Set the rendered point size in physical (device) pixels so the pick
    /// radius matches the visible disc radius exactly.  Call from
    /// assemblePipeline() after computing physPtSize.
    void setPhysicalPointSize(float s) { pickRadius_ = s * 0.5f; }

    void OnLeftButtonDown() override {
        startPos_[0]    = Interactor->GetEventPosition()[0];
        startPos_[1]    = Interactor->GetEventPosition()[1];
        leftButtonDown_ = true;
        isDragging_     = false;
        paintMode_      = (Interactor->GetAltKey() != 0);

        if (paintMode_) {
            // Alt is held: capture this gesture for point painting.
            // Do NOT start the trackball rotation state.
        } else {
            // No modifier: normal left-drag rotates the camera.
            Superclass::OnLeftButtonDown();
        }
    }

    void OnMouseMove() override {
        if (leftButtonDown_ && paintMode_ && item_) {
            const int ex = Interactor->GetEventPosition()[0];
            const int ey = Interactor->GetEventPosition()[1];
            const int dx = ex - startPos_[0];
            const int dy = ey - startPos_[1];
            if (dx*dx + dy*dy > kDragThreshold * kDragThreshold) {
                isDragging_ = true;
                pickNearest(ex, ey);
                return;  // don't pass to superclass during alt-paint drag
            }
        }
        Superclass::OnMouseMove();  // handles rotation, pan, zoom
    }

    void OnLeftButtonUp() override {
        const int ex = Interactor->GetEventPosition()[0];
        const int ey = Interactor->GetEventPosition()[1];
        const int dx = ex - startPos_[0];
        const int dy = ey - startPos_[1];

        if (paintMode_) {
            // Alt was held at button-down.
            if (!isDragging_ && dx*dx + dy*dy <= kDragThreshold * kDragThreshold)
                pickNearest(ex, ey);  // alt+click: flag one point
            // Don't call superclass: rotation was never started.
        } else {
            // Normal left gesture.
            if (dx*dx + dy*dy <= kDragThreshold * kDragThreshold)
                pickNearest(ex, ey);  // plain click: flag one point
            Superclass::OnLeftButtonUp();  // end rotation state
        }

        leftButtonDown_ = false;
        isDragging_     = false;
        paintMode_      = false;
    }

private:
    EditDataItem *item_ = nullptr;
    int  startPos_[2]    = {0, 0};
    bool leftButtonDown_ = false;
    bool isDragging_     = false;
    bool paintMode_      = false;  // true when Alt was held at button-down

    // Travel threshold (physical px) distinguishing a click from a drag.
    static constexpr int kDragThreshold = 4;

    // Pick radius in physical pixels.  Initialised to a reasonable default;
    // overridden by setPhysicalPointSize() to equal the rendered disc radius
    // (physPtSize/2).  A click selects a point only when it lands inside the
    // point's rendered circle — not merely "near" its centre.
    float pickRadius_ = 5.0f;

    void pickNearest(int px, int py) {
        if (!item_) return;

        // Use the mapper's actual input — not clipOutput() — so the point
        // indices found here match the indices the mapper uses to read the
        // quality scalar array.  The base-class pipeline may insert one or
        // more intermediate filters (normals, glyph, etc.) between the clip
        // filter and the mapper, causing clipOutput() index N to map to a
        // different rendered position than mapper-input index N.
        vtkPolyData *pts = item_->mapperInputData();
        if (!pts || pts->GetNumberOfPoints() == 0) {
            qDebug() << "EditPickInteractorStyle: mapper input empty, nothing to pick";
            return;
        }

        vtkRenderer *ren = GetDefaultRenderer();
        if (!ren) return;

        // GetEventPosition() in QQuickVTKItem returns physical-pixel coordinates
        // with y already in VTK display convention (y=0 at BOTTOM).  The Qt→VTK
        // flip is applied inside QQuickVTKItem before the interactor sees the
        // event.  Do NOT apply sz1-py here: that would double-flip and mirror
        // every click around the screen centre (error = 0 at centre, grows
        // linearly away from it — exactly the symptom we see).
        const int sz1    = ren->GetSize()[1];
        const int vtk_py = py;   // already VTK y (y=0 bottom)

        // The actor may have a non-identity model transform (e.g. vertical
        // exaggeration: SetScale(1,1,ve) around SetOrigin(0,0,zCenter)).
        // WorldToDisplay() expects WORLD-space coordinates (after that transform).
        // pts->GetPoint() returns OBJECT-space coordinates.  Fetch the actor's
        // composite matrix once and apply it per-point so the projected screen
        // position matches the OpenGL-rendered position.
        vtkMatrix4x4 *actorMat = item_->surfaceActorMatrix();

        const double maxDist2 = double(pickRadius_) * double(pickRadius_);
        vtkIdType bestId    = -1;
        double    bestDist2 = maxDist2 + 1.0;
        double    bestDepth = 2.0;  // NDC depth ∈ [0,1]; 2.0 = no candidate yet

        for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i) {
            double w[4];
            pts->GetPoint(i, w);
            w[3] = 1.0;

            // Object → world space
            if (actorMat) {
                double ww[4];
                actorMat->MultiplyPoint(w, ww);
                // affine matrix keeps ww[3]==1, but divide for safety
                const double inv = (ww[3] != 0.0) ? 1.0 / ww[3] : 1.0;
                w[0] = ww[0] * inv;
                w[1] = ww[1] * inv;
                w[2] = ww[2] * inv;
            }

            ren->SetWorldPoint(w[0], w[1], w[2], 1.0);
            ren->WorldToDisplay();
            double d[3];
            ren->GetDisplayPoint(d);

            if (d[2] < 0.0 || d[2] > 1.0) continue;  // clipped / behind camera

            const double ddx = d[0] - px;
            const double ddy = d[1] - vtk_py;
            const double dist2 = ddx*ddx + ddy*ddy;

            if (dist2 > maxDist2) continue;  // outside pick radius

            // Among all candidates within the pick radius, prefer the frontmost
            // (smallest NDC depth) — that is the point the GPU renders on top
            // and is therefore the one visually under the cursor.  Break depth
            // ties by nearest 2-D centre.
            if (d[2] < bestDepth || (d[2] == bestDepth && dist2 < bestDist2)) {
                bestDist2 = dist2;
                bestDepth = d[2];
                bestId    = i;
            }
        }

        if (bestId >= 0) {
            // Re-project bestId to confirm the screen position we matched.
            // click_qt  = click in Qt physical pixels (y=0 top)
            // best_qt   = selected point projected to Qt physical pixels
            // These should agree within bestDist px; if they don't, the
            // actor matrix or y-flip is wrong.
            double bw[4];
            pts->GetPoint(bestId, bw); bw[3] = 1.0;
            if (actorMat) {
                double ww[4];
                actorMat->MultiplyPoint(bw, ww);
                const double inv = (ww[3] != 0.0) ? 1.0/ww[3] : 1.0;
                bw[0] = ww[0]*inv; bw[1] = ww[1]*inv; bw[2] = ww[2]*inv;
            }
            ren->SetWorldPoint(bw[0], bw[1], bw[2], 1.0);
            ren->WorldToDisplay();
            double bd[3];
            ren->GetDisplayPoint(bd);
            // bd[1] is VTK y (0=bottom); convert to Qt y (0=top)
            const double best_qt_y = sz1 - bd[1];

            // click_qt shows the click in true Qt screen coords (y=0 top) by
            // un-doing the VTK flip from GetEventPosition.  best_qt is already
            // in Qt screen coords (sz1 - VTK_y).  They should be within dist px.
            qDebug() << "PICK DIAG:"
                     << " click_qt=(" << px << "," << (sz1 - py) << ")"
                     << " best_qt=(" << bd[0] << "," << best_qt_y << ")"
                     << " dist=" << std::sqrt(bestDist2) << "px"
                     << " depth=" << bestDepth
                     << " actorMat=" << (actorMat != nullptr)
                     << " sz1=" << sz1
                     << " pointId=" << bestId;
            item_->setPickedLocalId(bestId);
        } else {
            qDebug() << "EditPickInteractorStyle: no point disc covers click ("
                     << px << py << ") radius=" << pickRadius_ << "px";
        }
    }
};

} // namespace mb_system

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

EditDataItem::EditDataItem()
    : TopoDataItem()
{
    // Edit view defaults
    surfaceRenderType_ = SurfaceRenderType::PointCloud;
    coloredScalar_     = ColoredScalar::DataQuality;

    // Box starts collapsed; expanded by the first setEditBounds() call
    clipBox_->SetBounds(0, 0, 0, 0, 0, 0);
    clipFilter_->SetImplicitFunction(clipBox_);
    clipFilter_->ExtractInsideOn();
    clipFilter_->ExtractBoundaryCellsOn();

    // Purpose-built picking interactor for this view (see class comment above)
    editPickStyle_ = EditPickInteractorStyle::New();
    editPickStyle_->setItem(this);

    buildQualityLut();
}

EditDataItem::~EditDataItem() {
    if (editPickStyle_) {
        editPickStyle_->Delete();
        editPickStyle_ = nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

vtkPolyData *EditDataItem::clipOutput() {
    return vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
}

vtkMatrix4x4 *EditDataItem::surfaceActorMatrix() {
    if (!pipeline_ || !pipeline_->surfaceActor_) return nullptr;
    return pipeline_->surfaceActor_->GetMatrix();
}

vtkPolyData *EditDataItem::mapperInputData() {
    if (!pipeline_ || !pipeline_->surfaceMapper_) return nullptr;
    return pipeline_->surfaceMapper_->GetInput();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Quality LUT — two entries: BAD_DATA (red/transparent) and GOOD_DATA (blue)
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::buildQualityLut() {
    qualityLut_->SetNumberOfTableValues(2);
    qualityLut_->SetTableRange(BAD_DATA, GOOD_DATA);

    // GOOD_DATA: bright yellow — high luminance, very visible on dark background
    qualityLut_->SetTableValue(GOOD_DATA, 1.0, 1.0, 0.0, 1.0);

    // BAD_DATA: bright red; alpha is controlled by showBadPoints_
    const double badAlpha = showBadPoints_ ? 1.0 : 0.0;
    qualityLut_->SetTableValue(BAD_DATA, 1.0, 0.1, 0.1, badAlpha);

    qualityLut_->Build();
}

// ─────────────────────────────────────────────────────────────────────────────
//  dataOutputPort override — clip filter interposes before render stages
// ─────────────────────────────────────────────────────────────────────────────

vtkAlgorithmOutput *EditDataItem::dataOutputPort(Pipeline * /*pipeline*/) {
    // All base-class apply*() methods will now read from the extracted subset.
    return clipFilter_->GetOutputPort();
}

// ─────────────────────────────────────────────────────────────────────────────
//  connectDataset override — wire clip filter, set mapper to quality LUT
// ─────────────────────────────────────────────────────────────────────────────

bool EditDataItem::connectDataset(Pipeline *pipeline) {
    if (!TopoDataItem::connectDataset(pipeline)) return false;

    vtkPolyData *src = dataset_->polyData();

    // Ensure ORIGINAL_IDS exists so setPickedPoint() can map clip-output
    // indices back to the shared quality array.
    // vtkExtractPolyDataGeometry copies existing point-data arrays through to
    // its output, so a subset of these values will appear after Update().
    if (!src->GetPointData()->GetArray(ORIGINAL_IDS)) {
        vtkNew<vtkIdTypeArray> ids;
        ids->SetName(ORIGINAL_IDS);
        ids->SetNumberOfTuples(src->GetNumberOfPoints());
        for (vtkIdType i = 0; i < src->GetNumberOfPoints(); i++)
            ids->SetValue(i, i);
        src->GetPointData()->AddArray(ids);
    }

    auto *srcQA = vtkIntArray::SafeDownCast(
        src->GetPointData()->GetArray(DATA_QUALITY_NAME));
    qDebug() << "source quality before clip:"
             << (srcQA ? srcQA->GetValue(0) : -999);

    // Feed the clip filter from the shared polyData (not from source_, because
    // vtkExtractPolyDataGeometry needs SetInputData for a stable pointer).
    clipFilter_->SetInputData(src);
    if (boundsSet_) {
        clipBox_->SetBounds(editBounds_);
    }
    clipFilter_->Update();

    // Workaround: vtkExtractPolyDataGeometry does not correctly copy
    // vtkIntArray values into its output (values arrive as 0).  Manually sync.
    syncQualityArray();

    auto *qa = vtkIntArray::SafeDownCast(
        clipFilter_->GetOutput()->GetPointData()->GetArray(DATA_QUALITY_NAME));
    qDebug() << "quality in clip output:" << (qa != nullptr);
    if (qa && qa->GetNumberOfTuples() > 0)
        qDebug() << "clip quality[0]:" << qa->GetValue(0);

    // Point locator runs on the extracted result for fast pick → ID mapping
    rebuildLocator();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  assemblePipeline override — apply edit defaults and install quality LUT
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::assemblePipeline(Pipeline *pipeline) {
    qDebug() << "EditDataItem::assemblePipeline"
             << "dataset=" << (bool)dataset_
             << "loaded=" << (dataset_ && dataset_->isLoaded())
             << "boundsSet=" << boundsSet_;

    // Defaults for edit view
    surfaceRenderType_ = SurfaceRenderType::PointCloud;
    coloredScalar_     = ColoredScalar::DataQuality;

    // Base class builds the pipeline (calls connectDataset, applyColoredScalar,
    // applyRenderType, etc.)
    TopoDataItem::assemblePipeline(pipeline);

    // Background and interactor style are edit-view constants — set them
    // regardless of whether the dataset is loaded yet, so the window looks
    // correct even before the first reassembly after load.
    pipeline->renderer_->SetBackground(0.08, 0.08, 0.12);   // very dark blue-gray
    // SetBackground() sets RGB but leaves alpha at 0 (transparent) by default.
    // On Ubuntu + Mesa, Qt Quick composites VTK's FBO over the window background
    // (white), so a transparent VTK background shows as white.  macOS defaults to
    // opaque and is not affected.  Force alpha=1 so the colour is actually visible.
    pipeline->renderer_->SetBackgroundAlpha(1.0);
    // Install the edit-specific interactor (screen-space picking, robust drag
    // detection) in place of the generic PickInteractorStyle.
    if (renderWindow_ && renderWindow_->GetInteractor()) {
        editPickStyle_->SetDefaultRenderer(pipeline->renderer_);
        renderWindow_->GetInteractor()->SetInteractorStyle(editPickStyle_);
    }

    if (!dataset_ || !dataset_->isLoaded()) return;

    // Override: install the quality LUT on the mapper in place of the
    // elevation LUT that applyColoredScalar / applyShadowSource installed.
    pipeline->surfaceMapper_->SetLookupTable(qualityLut_);
    pipeline->surfaceMapper_->SetScalarRange(BAD_DATA, GOOD_DATA);
    pipeline->surfaceMapper_->ScalarVisibilityOn();
    pipeline->surfaceMapper_->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
    pipeline->surfaceMapper_->SelectColorArray(DATA_QUALITY_NAME);
    pipeline->surfaceMapper_->SetScalarModeToUsePointFieldData();
    pipeline->surfaceMapper_->SetColorModeToMapScalars();

    // Point cloud rendering.
    //
    // Strategy for cross-platform point size on OpenGL core profile:
    //
    // VTK sets a "pointSize" uniform in its built-in vertex shader
    // (//VTK::PointSizeUniform::Impl) from vtkProperty::GetPointSize(), and
    // writes that value to gl_PointSize — but only when GL_PROGRAM_POINT_SIZE is
    // enabled.  glPointSize() is a no-op in core profile.
    //
    // On macOS, Qt Quick / VTK enables GL_PROGRAM_POINT_SIZE automatically.
    // On some Mesa (Ubuntu) configurations it is not enabled by default, so we
    // do it explicitly here via the VTK state tracker so VTK's own context
    // management does not accidentally undo it.
    //
    // gl_PointSize / SetPointSize() is in *physical* (framebuffer) pixels.
    // QQuickVTKItem renders into a HiDPI FBO at device-pixel resolution, so on
    // a 2× Retina display SetPointSize(5) gives 5 device pixels ≈ 2.5 logical
    // pixels.  Multiply by devicePixelRatio so the apparent size matches on
    // every platform.
    {
        auto *primaryScreen = QGuiApplication::primaryScreen();
        const float dpr = primaryScreen
                              ? static_cast<float>(primaryScreen->devicePixelRatio())
                              : 1.0f;
        const float physPtSize = 5.0f * dpr;

        pipeline->surfaceActor_->GetProperty()->SetRepresentationToPoints();
        pipeline->surfaceActor_->GetProperty()->SetPointSize(physPtSize);

        // Match the pick radius to the rendered disc radius so a click only
        // registers when it lands inside a point's visible circle.
        editPickStyle_->setPhysicalPointSize(physPtSize);

        // Belt-and-suspenders: also enable GL_PROGRAM_POINT_SIZE via the VTK
        // state wrapper so the GL capability is on even if VTK would otherwise
        // skip it (e.g. after applyShadowSource() clears the shader property).
        auto *oglrw = vtkOpenGLRenderWindow::SafeDownCast(renderWindow_);
        if (oglrw) {
            oglrw->GetState()->vtkglEnable(GL_PROGRAM_POINT_SIZE);
        }

        // Inject gl_PointSize into the vertex shader explicitly.  Relying on
        // VTK's built-in //VTK::PointSizeUniform::Impl is unreliable here
        // because applyShadowSource() clears all user shader replacements
        // earlier in the same assembly pass, and VTK's own injection may not
        // fire after that.  applyShadowSource() (Illumination case) adds no
        // replacements of its own, so there is nothing to conflict with.
        //
        // Use snprintf("%.1f") not std::to_string(): the latter produces
        // "70.000000" which some GLSL compilers (Mesa) reject as a double
        // literal when assigned to the float gl_PointSize built-in.
        char ptSizeLiteral[32];
        snprintf(ptSizeLiteral, sizeof(ptSizeLiteral), "%.1f", physPtSize);
        const std::string glsl =
            "//VTK::PositionVC::Impl\n"
            "  gl_PointSize = " + std::string(ptSizeLiteral) + ";\n";

        auto *sp = pipeline->surfaceActor_->GetShaderProperty();
        sp->AddVertexShaderReplacement(
            "//VTK::PositionVC::Impl",
            true,
            glsl,
            false);

        qDebug() << "EditDataItem: dpr=" << dpr << " physPtSize=" << physPtSize
                 << " gl_PointSize literal=" << ptSizeLiteral;
    }

    // No lighting needed for quality-colour point cloud
    pipeline->surfaceActor_->GetProperty()->LightingOff();
    pipeline->surfaceActor_->GetProperty()->SetAmbient(1.0);
    pipeline->surfaceActor_->GetProperty()->SetDiffuse(0.0);
    pipeline->surfaceActor_->GetProperty()->SetSpecular(0.0);

    qDebug() << "EditDataItem::assemblePipeline done: actors="
             << pipeline->renderer_->GetActors()->GetNumberOfItems()
             << " clipOutputPts="
             << clipFilter_->GetOutput()->GetNumberOfPoints();
}

// ─────────────────────────────────────────────────────────────────────────────
//  setEditBounds — update clip box and re-render
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setEditBounds(double xMin, double xMax,
                                  double yMin, double yMax,
                                  double zMin, double zMax) {
    editBounds_[0] = xMin;  editBounds_[1] = xMax;
    editBounds_[2] = yMin;  editBounds_[3] = yMax;
    editBounds_[4] = zMin;  editBounds_[5] = zMax;
    boundsSet_ = true;

    qDebug() << "EditDataItem::setEditBounds()"
             << "x[" << xMin << "," << xMax << "]"
             << "y[" << yMin << "," << yMax << "]"
             << "z[" << zMin << "," << zMax << "]"
             << "pipelineReady=" << (pipeline_ != nullptr);

    if (!pipeline_) {
        // Pipeline not yet assembled (first-render path).
        // connectDataset() already checks boundsSet_ and will apply editBounds_
        // when the first frame renders, so nothing else is needed here.
        // Calling dispatch_async() without a render window would be a no-op at
        // best and a crash at worst.
        return;
    }

    // Pipeline is already assembled: push a clip-update job to the render thread.
    // Capture by value: the lambda runs on the render thread.
    dispatch_async([this, xMin, xMax, yMin, yMax, zMin, zMax]
                   (vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "setEditBounds() dispatched";
        clipBox_->SetBounds(xMin, xMax, yMin, yMax, zMin, zMax);
        clipFilter_->Modified();
        clipFilter_->Update();
        qDebug() << "clip output points:"
                 << clipFilter_->GetOutput()->GetNumberOfPoints();
        syncQualityArray();
        rebuildLocator();
        rw->GetRenderers()->GetFirstRenderer()->ResetCamera();
        rw->Render();
    });

    // setEditBounds() is called via DirectConnection from SurfaceDataItem's
    // render thread (inside SurfaceDataItem's updatePaintNode()).  dispatch_async
    // calls QQuickItem::update() internally, but update() from a foreign item's
    // updatePaintNode() is rejected ("Updates can only be scheduled from GUI
    // thread or from QQuickItem::updatePaintNode()"), so the callback sits queued
    // and EditDataItem only refreshes when the user next interacts with it.
    //
    // Fix: post QEvent::UpdateRequest directly to EditDataItem's window.
    // QCoreApplication::postEvent is always thread-safe and routes through
    // Qt Quick's render-scheduling machinery, which accepts UpdateRequest events
    // regardless of which thread posts them.
    if (QQuickWindow *w = window()) {
        QCoreApplication::postEvent(w,
            new QEvent(QEvent::UpdateRequest),
            Qt::HighEventPriority);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  onQualityChanged override — re-sync quality array and re-render
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::onQualityChanged() {
    qDebug() << "EditDataItem::onQualityChanged() — dispatching sync+render";
    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "EditDataItem::onQualityChanged dispatch running on render thread";
        syncQualityArray();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setFlagValue — toggle between BAD_DATA and GOOD_DATA
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setFlagValue(int quality) {
    flagValue_ = quality;
    qDebug() << "EditDataItem::setFlagValue()" << flagValue_;
}

// ─────────────────────────────────────────────────────────────────────────────
//  resetCamera — fit camera to current clip output and re-render
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::resetCamera() {
    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        if (pipeline_ && pipeline_->renderer_)
            pipeline_->renderer_->ResetCamera();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setShowBadPoints — adjust quality LUT alpha without pipeline rebuild
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setShowBadPoints(bool show) {
    showBadPoints_ = show;
    qDebug() << "EditDataItem::setShowBadPoints()" << show;

    dispatch_async([this, show](vtkRenderWindow *rw, vtkUserData) {
        // Adjust only the alpha of the BAD_DATA entry in the quality LUT
        double rgba[4];
        qualityLut_->GetTableValue(BAD_DATA, rgba);
        rgba[3] = show ? 1.0 : 0.0;
        qualityLut_->SetTableValue(BAD_DATA, rgba);
        qualityLut_->Modified();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setPickedLocalId — localId (clip output) → originalId → setPointQuality
//
//  Preferred entry point called by EditPickInteractorStyle::pickNearest().
//  Uses the localId determined by screen-space projection directly, bypassing
//  the 3D point locator.  This avoids selecting a different point when vertical
//  exaggeration causes the locator's 3D nearest-neighbour to differ from the
//  visually frontmost point under the cursor.
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setPickedLocalId(vtkIdType localId) {
    if (!dataset_ || !dataset_->isLoaded()) return;

    // Look up ORIGINAL_IDS in the mapper's actual input — the same polyData
    // that pickNearest() iterated.  An intermediate pipeline filter may have
    // changed point ordering relative to clipFilter_->GetOutput(), so we must
    // use the same source for both the index search and the ID lookup.
    vtkPolyData *mapperIn = mapperInputData();
    if (!mapperIn || localId < 0 ||
        localId >= mapperIn->GetNumberOfPoints()) return;

    vtkDataArray *origIds =
        mapperIn->GetPointData()->GetArray(ORIGINAL_IDS);
    if (!origIds) {
        qWarning() << "EditDataItem::setPickedLocalId(): "
                      "ORIGINAL_IDS array missing from mapper input — "
                      "falling back to clip output";
        // Fall back: try clip output (works when there is no intermediate filter)
        vtkPolyData *extracted =
            vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
        if (!extracted) return;
        origIds = extracted->GetPointData()->GetArray(ORIGINAL_IDS);
        if (!origIds) return;
    }

    vtkIdType originalId =
        static_cast<vtkIdType>(origIds->GetTuple1(localId));
    qDebug() << "EditDataItem::setPickedLocalId(): localId=" << localId
             << " originalId=" << originalId
             << " flagValue=" << flagValue_;

    dataset_->setPointQuality(originalId, flagValue_);

    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "EditDataItem::setPickedLocalId dispatch: syncing and rendering";
        syncQualityArray();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  setPickedPoint override — world coords → originalId → setPointQuality
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::setPickedPoint(double *worldCoords) {
    if (!dataset_ || !dataset_->isLoaded() || !pipeline_) return;

    // Find the nearest point in the *extracted* polyData.
    // clipFilter_->GetOutput() is the same vtkPolyData that pointLocator_ was
    // built on; FindClosestPoint() runs in O(log n) via the KD-tree.
    vtkPolyData *extracted =
        vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
    if (!extracted || extracted->GetNumberOfPoints() == 0) return;

    vtkIdType localId =
        pointLocator_->FindClosestPoint(worldCoords);
    if (localId < 0) return;

    // Map localId (index in the extracted subset) back to the stable
    // ORIGINAL_IDS index used by the shared quality array.
    vtkDataArray *origIds =
        extracted->GetPointData()->GetArray(ORIGINAL_IDS);
    if (!origIds) {
        qWarning() << "EditDataItem::setPickedPoint(): "
                      "ORIGINAL_IDS array not found in extracted polyData";
        return;
    }

    vtkIdType originalId = static_cast<vtkIdType>(origIds->GetTuple1(localId));
    qDebug() << "EditDataItem::setPickedPoint(): localId=" << localId
             << " originalId=" << originalId
             << " flagValue=" << flagValue_;

    // Update the shared quality value directly.  setPickedPoint() is called
    // from the VTK render thread (OnLeftButtonUp → pickNearest); using a
    // QueuedConnection here posts the lambda to the main-thread queue which
    // is not processed until after the render cycle — so the color never
    // updates.  Calling setPointQuality() directly is safe: the render thread
    // is between frames at this point and VTK is not reading the array.
    dataset_->setPointQuality(originalId, flagValue_);

    // Explicitly dispatch clip-output sync + render on the render thread.
    // onQualityChanged() also dispatches this (via the qualityChanged signal),
    // but that signal is delivered on the main thread and may be delayed.
    // Dispatching here ensures the colour change appears immediately.
    dispatch_async([this](vtkRenderWindow *rw, vtkUserData) {
        qDebug() << "EditDataItem::setPickedPoint dispatch: syncing and rendering";
        syncQualityArray();
        rw->Render();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  rebuildLocator (private, render thread)
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::rebuildLocator() {
    vtkPolyData *extracted =
        vtkPolyData::SafeDownCast(clipFilter_->GetOutput());
    if (!extracted || extracted->GetNumberOfPoints() == 0) return;

    pointLocator_->SetDataSet(extracted);
    pointLocator_->BuildLocator();
    qDebug() << "EditDataItem::rebuildLocator():"
             << extracted->GetNumberOfPoints() << "points indexed";
}

// ─────────────────────────────────────────────────────────────────────────────
//  syncQualityArray (private, render thread)
//  Workaround: vtkExtractPolyDataGeometry does not correctly copy vtkIntArray
//  values into its output (all values arrive as 0).  Manually copy from the
//  shared quality array using ORIGINAL_IDS for the index mapping.
// ─────────────────────────────────────────────────────────────────────────────

void EditDataItem::syncQualityArray() {
    vtkIntArray *srcQA = dataset_ ? dataset_->qualityArray() : nullptr;
    if (!srcQA) return;

    // Sync the clip-filter output first (the canonical store).
    vtkPolyData *clipOut = clipFilter_->GetOutput();
    if (clipOut && clipOut->GetNumberOfPoints() > 0) {
        auto *origIds = vtkIdTypeArray::SafeDownCast(
            clipOut->GetPointData()->GetArray(ORIGINAL_IDS));
        auto *outQA   = vtkIntArray::SafeDownCast(
            clipOut->GetPointData()->GetArray(DATA_QUALITY_NAME));
        if (origIds && outQA) {
            for (vtkIdType i = 0; i < clipOut->GetNumberOfPoints(); i++)
                outQA->SetValue(i, srcQA->GetValue(origIds->GetValue(i)));
            outQA->Modified();
            clipOut->Modified();
        }
    }

    // Also sync the mapper's actual input directly.  If there is an
    // intermediate filter between clipFilter and the mapper (normals, glyph,
    // etc.), that filter will re-run on the next Render() and overwrite its
    // output — but syncing here ensures the colour is correct even if the
    // filter decides to skip re-execution based on MTime.
    vtkPolyData *mapperIn = mapperInputData();
    if (mapperIn && mapperIn != clipOut && mapperIn->GetNumberOfPoints() > 0) {
        auto *origIds = vtkIdTypeArray::SafeDownCast(
            mapperIn->GetPointData()->GetArray(ORIGINAL_IDS));
        auto *outQA   = vtkIntArray::SafeDownCast(
            mapperIn->GetPointData()->GetArray(DATA_QUALITY_NAME));
        if (origIds && outQA) {
            for (vtkIdType i = 0; i < mapperIn->GetNumberOfPoints(); i++)
                outQA->SetValue(i, srcQA->GetValue(origIds->GetValue(i)));
            outQA->Modified();
            mapperIn->Modified();
        }
    }

    qDebug() << "syncQualityArray(): clipPts="
             << (clipOut ? clipOut->GetNumberOfPoints() : 0)
             << " mapperPts="
             << (mapperIn ? mapperIn->GetNumberOfPoints() : 0)
             << " sample srcQA[0]=" << srcQA->GetValue(0);
}
