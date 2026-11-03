BUILD_ID = 'v0.8.1_mbio'
print(f"[WCD Viewer] BUILD_ID={BUILD_ID}")

"""
/*--------------------------------------------------------------------
 *    The MB-system:  wcd_viewer.py  31/10/2026
 *
 *    Copyright (c) 2025-2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

Water column viewer reading the data through MB-System's MBIO library (mbio_py),
successor of kmwcd_viewer.py (which used KMALL.py). Opens a *.kmwcd, *.kmall, *.wcd or *.all and
finds the other file of the line; any of the records may be missing.
"""
import math
import os
import sys
import warnings
from datetime import datetime, timezone

import numpy as np

# mbio_py: environment variable MBIO_PY_PATH, else the directory above this one
# (src/python in the source tree, share/mbsystem/python when installed)
_here = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, os.environ.get("MBIO_PY_PATH", os.path.join(_here, "..")))
import mbio_py as mb  # noqa: E402
from mbio_py import constants as K  # noqa: E402

from PySide6.QtCore import Qt, QTimer, Signal  # noqa: E402
from PySide6.QtWidgets import (  # noqa: E402
    QAbstractItemView, QApplication, QCheckBox, QDialog, QDoubleSpinBox, QFileDialog, QHBoxLayout, QHeaderView,
    QLabel, QMainWindow, QPushButton, QSizePolicy, QSlider, QTableWidget, QTableWidgetItem, QVBoxLayout, QWidget)
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas  # noqa: E402
import matplotlib.pyplot as plt  # noqa: E402

# Suppress the pcolormesh cell-center warning to keep the terminal output clean.
warnings.filterwarnings("ignore", message="The input coordinates to pcolormesh are interpreted as cell centers")


class OneStepWheelSlider(QSlider):
    """Slider that moves exactly one step per mouse wheel notch."""

    def wheelEvent(self, event):
        delta = event.angleDelta().y()
        if delta > 0 and self.value() < self.maximum():
            self.setValue(self.value() + 1)
        elif delta < 0 and self.value() > self.minimum():
            self.setValue(self.value() - 1)
        event.accept()


def _fmt(v, fmt, na="N/A"):
    return na if v is None or not np.isfinite(v) else format(v, fmt)


def _utc(time_d):
    return datetime.fromtimestamp(time_d, tz=timezone.utc).strftime("%Y-%m-%d %H:%M:%S.%f")[:-3] + " UTC"


class MainWindow(QMainWindow):
    """UI layout, data loading through mbio_py, plotting, picking and export."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("WCD Viewer - water column through MB-System MBIO")
        self.line = None          # mb.KongsbergLine
        self.steps = []           # (water column ping index, swath) per slider step
        self.current = 0
        self._ping_cache = (None, None)
        self._last_grid = None
        self.setFocusPolicy(Qt.StrongFocus)

        w = QWidget()
        self.setCentralWidget(w)
        layout = QVBoxLayout(w)

        # header
        hh = QHBoxLayout()
        btn_open = QPushButton("Open .kmwcd/.kmall/.wcd/.all")
        btn_open.clicked.connect(self.open_dialog)
        hh.addWidget(btn_open)
        self.lbl_file = QLabel("No file loaded")
        self.lbl_file.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        hh.addWidget(self.lbl_file)
        btn_save = QPushButton("Save Image")
        btn_save.clicked.connect(self.save_image)
        hh.addWidget(btn_save)
        self.chk_bottom = QCheckBox("Bottom")
        self.chk_bottom.stateChanged.connect(lambda _: self.redraw())
        hh.addWidget(self.chk_bottom)
        self.chk_wcbottom = QCheckBox("WC bottom")
        self.chk_wcbottom.stateChanged.connect(lambda _: self.redraw())
        hh.addWidget(self.chk_wcbottom)
        self.chk_raytrace = QCheckBox("Ray trace (SVP)")
        self.chk_raytrace.setToolTip("Trace the water column samples through the sound velocity profile "
                                     "instead of straight rays with the sound speed at the transducer")
        self.chk_raytrace.stateChanged.connect(lambda _: self.redraw())
        hh.addWidget(self.chk_raytrace)
        self.chk_pick = QCheckBox("Pick")
        self.chk_pick.clicked.connect(self._show_pick_dialog)
        self.chk_pick.toggled.connect(self._toggle_pick)
        hh.addWidget(self.chk_pick)
        self.meta_label = QLabel("")
        self.meta_label.setWordWrap(True)
        hh.addWidget(self.meta_label)
        layout.addLayout(hh)

        # depth controls
        dh = QHBoxLayout()
        dh.addWidget(QLabel("Depth Min (m):"))
        self.dmin = QDoubleSpinBox()
        self.dmin.setRange(0, 1e5)
        self.dmin.valueChanged.connect(self.redraw)
        dh.addWidget(self.dmin)
        dh.addWidget(QLabel("Depth Max (m):"))
        self.dmax = QDoubleSpinBox()
        self.dmax.setRange(0, 1e5)
        self.dmax.valueChanged.connect(self.redraw)
        dh.addWidget(self.dmax)
        layout.addLayout(dh)

        # amplitude controls (dB)
        ah = QHBoxLayout()
        ah.addWidget(QLabel("Amp Min (dB):"))
        self.amin = QDoubleSpinBox()
        self.amin.setRange(-200, 200)
        self.amin.setSingleStep(0.5)
        self.amin.valueChanged.connect(self.redraw)
        ah.addWidget(self.amin)
        ah.addWidget(QLabel("Amp Max (dB):"))
        self.amax = QDoubleSpinBox()
        self.amax.setRange(-200, 200)
        self.amax.setSingleStep(0.5)
        self.amax.valueChanged.connect(self.redraw)
        ah.addWidget(self.amax)
        layout.addLayout(ah)

        # across-track X scale and replay
        xh = QHBoxLayout()
        xh.addWidget(QLabel("Across-track Max (m):"))
        self.xmax = QDoubleSpinBox()
        self.xmax.setRange(0, 1e6)
        self.xmax.valueChanged.connect(self.redraw)
        xh.addWidget(self.xmax)
        xh.addWidget(QLabel("Replay (pings/s):"))
        self.replay_rate = QDoubleSpinBox()
        self.replay_rate.setRange(0.1, 10.0)
        self.replay_rate.setSingleStep(0.1)
        self.replay_rate.setDecimals(1)
        self.replay_rate.setValue(2.0)
        xh.addWidget(self.replay_rate)
        self.btn_replay = QPushButton("Replay")
        self.btn_replay.setCheckable(True)
        self.btn_replay.toggled.connect(self._toggle_replay)
        xh.addWidget(self.btn_replay)
        self.replay_rate.valueChanged.connect(self._update_replay_interval)
        layout.addLayout(xh)
        self.replay_timer = QTimer(self)
        self.replay_timer.timeout.connect(self._replay_tick)
        self._is_replaying = False

        # canvas with a fixed colorbar axes (no layout creep on redraw)
        self.fig, self.ax = plt.subplots()
        self.fig.subplots_adjust(right=0.85)
        self.cax = self.fig.add_axes([0.87, 0.15, 0.03, 0.7])
        self.canvas = FigureCanvas(self.fig)
        layout.addWidget(self.canvas, stretch=1)
        self.canvas.mpl_connect("button_press_event", self._on_plot_click)

        # ping slider
        ph = QHBoxLayout()
        ph.addWidget(QLabel("Ping:"))
        self.slider = OneStepWheelSlider(Qt.Horizontal)
        self.slider.valueChanged.connect(self.on_slide)
        ph.addWidget(self.slider)
        self.lbl_ping = QLabel("0")
        ph.addWidget(self.lbl_ping)
        layout.addLayout(ph)

        self.statusBar().showMessage("Ready")
        self._pick_dialog = None
        self._pick_dialog_shown_once = False
        self._pick_active = False

    # ------------------------------------------------------------------ loading
    def open_dialog(self):
        fname, _ = QFileDialog.getOpenFileName(self, "Open .kmwcd/.kmall/.wcd/.all", "", "*.kmwcd *.kmall *.wcd *.all")
        if fname:
            self.open_file(fname)

    def open_file(self, fname):
        """Load a line (either file of the pair) and set up the controls."""
        self._toggle_replay(False)
        self.btn_replay.setChecked(False)
        self.lbl_file.setText(os.path.basename(fname))
        print("[diag] open_file:", fname)
        self.statusBar().showMessage("Reading survey file (navigation, bathymetry)...")
        QApplication.processEvents()
        last = [-1]

        def progress(npings, fraction):
            pct = int(100 * fraction)
            if pct != last[0]:
                last[0] = pct
                self.statusBar().showMessage(f"Loading pings... {pct}% ({npings} pings)")
                QApplication.processEvents()

        try:
            self.line = mb.KongsbergLine(fname, progress=progress)
        except mb.MBIOError as ex:
            self.line = None
            self.steps = []
            self.statusBar().showMessage(f"Cannot read {os.path.basename(fname)}: {ex}")
            self.redraw()
            return
        line = self.line
        print(f"[diag] survey={line.kmall} water column={line.kmwcd}")
        self._ping_cache = (None, None)
        self._geom_cache = (None, None)
        has_svp = line.svp(0.0) is not None
        self.chk_raytrace.blockSignals(True)
        self.chk_raytrace.setEnabled(has_svp)
        self.chk_raytrace.setChecked(has_svp)
        self.chk_raytrace.blockSignals(False)
        self.steps = []
        if line.wc is not None:
            for i in range(len(line.wc)):
                for s in line.wc.swaths(i):
                    self.steps.append((i, int(s)))
        if not self.steps:
            self.statusBar().showMessage("No water column data found.")
            self.redraw()
            return

        # file-wide ranges, as the original pre-scan
        wcf = line.wc
        if np.isfinite(wcf.amp_min) and wcf.amp_min < wcf.amp_max:
            for sb in (self.amin, self.amax):
                sb.blockSignals(True)
                sb.setRange(wcf.amp_min, wcf.amp_max)
            self.amin.setValue(wcf.amp_min)
            self.amax.setValue(wcf.amp_max)
            for sb in (self.amin, self.amax):
                sb.blockSignals(False)
        depth_max = wcf.range_max_m + (float(np.nanmax(line.survey.sensordepth)) if line.survey is not None and
                                       len(line.survey) else 0.0)
        for sb in (self.dmin, self.dmax):
            sb.blockSignals(True)
            sb.setRange(0, depth_max)
        self.dmin.setValue(0)
        self.dmax.setValue(depth_max)
        for sb in (self.dmin, self.dmax):
            sb.blockSignals(False)
        self.xmax.blockSignals(True)
        self.xmax.setRange(0, 2 * wcf.range_max_m if wcf.range_max_m > 0 else 1e5)
        self.xmax.setValue(0)
        self.xmax.blockSignals(False)

        self.slider.blockSignals(True)
        self.slider.setRange(0, len(self.steps) - 1)
        self.slider.setSingleStep(1)
        self.slider.setValue(0)
        self.slider.blockSignals(False)
        self.current = 0
        self.statusBar().showMessage("Load complete")
        self.redraw()

        s = line.survey
        summary = (f"Nav summary: survey pings={len(s) if s is not None else 0}, "
                   f"position records={len(s.nav_time_d) if s is not None else len(wcf.nav_time_d)}"
                   f"{'' if s is not None else ' (from water column file)'}, WC pings={len(wcf)}, matched={line.matched}, "
                   f"fill value={wcf.fill_value}")
        print("[diag]", summary)
        QTimer.singleShot(900, lambda: self.statusBar().showMessage(summary, 6000))

    # ------------------------------------------------------------------ navigation
    def on_slide(self, val):
        self.current = int(val)
        self.redraw()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Left and self.current > 0:
            self.slider.setValue(self.current - 1)
        elif event.key() == Qt.Key_Right and self.current < self.slider.maximum():
            self.slider.setValue(self.current + 1)
        else:
            super().keyPressEvent(event)

    def _toggle_replay(self, checked):
        if checked and self.steps:
            self._is_replaying = True
            self._update_replay_interval()
            self.replay_timer.start()
        else:
            self._is_replaying = False
            self.replay_timer.stop()
            self.btn_replay.setText("Replay")

    def _update_replay_interval(self):
        pps = max(float(self.replay_rate.value()), 0.1)
        self.replay_timer.setInterval(max(1, int(1000.0 / pps)))
        if self._is_replaying:
            self.btn_replay.setText(f"Stop ({pps:.1f} p/s)")

    def _replay_tick(self):
        if not self.steps:
            self._toggle_replay(False)
            return
        self.slider.setValue((self.current + 1) % len(self.steps))

    # ------------------------------------------------------------------ data of a step
    def _swath_ping(self, step):
        i, swath = self.steps[step]
        if self._ping_cache[0] != i:
            self._ping_cache = (i, self.line.wc.ping(i))
        wc = self._ping_cache[1]
        return i, swath, wc, wc.select(wc.swath == swath)

    def _soundings(self, i, swath, wc):
        """Soundings of the matched survey ping for this swath: (across, depth, flag, detects, j) or None."""
        j = self.line.survey_index(i)
        if j is None:
            return None
        s = self.line.survey
        x, z, f, d = s.bathacrosstrack[j], s.bath[j], s.beamflag[j], s.detects[j]
        swaths = np.unique(wc.swath)
        # MBIO keeps the receive fans of a ping in order; with equal fans the soundings of
        # swath k are the k-th block (dual swath, single receiver)
        if len(swaths) > 1 and len(z) % len(swaths) == 0:
            n = len(z) // len(swaths)
            k = int(np.searchsorted(swaths, swath))
            sel = slice(k * n, (k + 1) * n)
            x, z, f, d = x[sel], z[sel], f[sel], d[sel]
        return x, z, f, d, j

    def _geometry(self, step, i, wc, draft):
        """(across, depth, detect across, detect depth, ray traced) of a swath; ray traced results
        are cached for the current step."""
        svp = self.line.svp(wc.time_d) if self.chk_raytrace.isChecked() else None
        if svp is None:
            x, z, _ = wc.xyz()
            dx, dz, _ = wc.detect_xyz()
            return x, z + draft, dx, dz + draft, False
        if self._geom_cache[0] != step:
            cells, det = mb.raytrace(wc, svp[0], svp[1], draft)
            self._geom_cache = (step, (cells[0], cells[1], det[0], det[1]))
        x, depth, dx, dd = self._geom_cache[1]
        return x, depth, dx, dd, True

    # ------------------------------------------------------------------ drawing
    def redraw(self):
        self.ax.clear()
        if self.line is None or not self.steps:
            self.cax.clear()
            self.canvas.draw_idle()
            return
        step = min(self.current, len(self.steps) - 1)
        i, swath, wc_all, wc = self._swath_ping(step)
        j = self.line.survey_index(i)
        draft = float(self.line.survey.sensordepth[j]) if j is not None else 0.0
        zlabel = "Depth below waterline (m)" if j is not None else "Depth below transducer (m)"

        # metadata line
        lon, lat, sog_kn, cog = self.line.nav_at(wc.time_d)
        heading = self.line.heading_at(wc.time_d)
        self.lbl_ping.setText(f"{step} / {len(self.steps) - 1}")
        self.meta_label.setText(
            f"Ping {wc_all.ping_number} swath {swath}: DateTime={_utc(wc.time_d)}, SOG={_fmt(sog_kn, '.2f')} kn,"
            f" COG={_fmt(cog, '.1f')}, Hdg={_fmt(heading, '.1f')}, Pos={_fmt(lat, '.6f')},{_fmt(lon, '.6f')}")

        # geometry: ray traced through the sound velocity profile, or straight rays
        x, depth, det_x, det_depth, traced = self._geometry(step, i, wc, draft)
        if traced:
            zlabel += " - ray traced" + ("" if j is not None else ", draft 0")
        amp = wc.amplitude.copy()
        if self.line.wc.fill_value is not None:
            amp[amp <= self.line.wc.fill_value] = np.nan
        mask = (depth >= self.dmin.value()) & (depth <= self.dmax.value())
        amp[~mask] = np.nan
        valid = np.isfinite(amp)
        xp = np.where(np.isfinite(x), x, 0.0)
        # rays that turned back have no position: their cells are already masked
        zp = np.where(np.isfinite(depth), depth, np.nanmax(depth) if np.isfinite(depth).any() else 0.0)
        mesh = self.ax.pcolormesh(xp, zp, amp, shading="nearest", cmap="viridis", vmin=self.amin.value(),
                                  vmax=self.amax.value())
        self._last_grid = {"x": x, "depth": depth, "amp": amp, "time_d": wc.time_d,
                           "ping": wc_all.ping_number, "swath": swath, "step": step}

        status = []
        if self.chk_bottom.isChecked():
            snd = self._soundings(i, swath, wc_all)
            if snd is None:
                status.append("no survey ping for this water column ping")
            else:
                sx, sz, sf, sd, _ = snd
                shown = 0
                for code, colour in ((K.MB_DETECT_AMPLITUDE, "red"), (K.MB_DETECT_PHASE, "darkblue"),
                                     (K.MB_DETECT_UNKNOWN, "gray")):
                    good = (sd == code) & K.beam_ok(sf)
                    flagged = (sd == code) & K.beam_flagged(sf) & ~K.beam_null(sf)
                    self.ax.scatter(sx[good], sz[good], s=12, marker="o", edgecolors="none", c=colour)
                    self.ax.scatter(sx[flagged], sz[flagged], s=12, marker="o", facecolors="none",
                                    edgecolors=colour, linewidths=0.8)
                    shown += int(good.sum() + flagged.sum())
                status.append(f"soundings: {shown}")
        if self.chk_wcbottom.isChecked():
            self.ax.plot(det_x, det_depth, "-", color="orange", lw=0.8)

        self.cax.clear()
        self.fig.colorbar(mesh, cax=self.cax, label="Amplitude (dB)")
        d0, d1 = self.dmin.value(), self.dmax.value()
        if valid.any():
            d1 = min(d1, float(depth[valid].max()))
        if abs(d1 - d0) < 1e-6:
            d1 = d0 + 1e-3
        self.ax.set_ylim(d0, d1)
        xmax = float(self.xmax.value())
        if xmax > 0:
            self.ax.set_xlim(-xmax, xmax)
        elif valid.any():
            self.ax.set_xlim(float(x[valid].min()), float(x[valid].max()))
        self.ax.invert_yaxis()
        self.ax.set_xlabel("Across-track (m, starboard +)")
        self.ax.set_ylabel(zlabel)
        self.canvas.draw()
        if status:
            self.statusBar().showMessage(" | ".join(status))

    # ------------------------------------------------------------------ export
    def save_image(self, path=None):
        """Save the figure at 2x DPI with file name and metadata at the bottom."""
        filename = self.lbl_file.text() or ""
        overlay = self.fig.text(0.01, 0.01, f"{filename} | {self.meta_label.text()}", va="bottom", ha="left",
                                fontsize=8, color="white", bbox=dict(facecolor="black", alpha=0.5, pad=2))
        if not path:
            path, _ = QFileDialog.getSaveFileName(self, "Save Image", filename, "PNG (*.png);;JPEG (*.jpg)")
        if path:
            fmt = "png" if path.lower().endswith(".png") else "jpg"
            self.fig.savefig(path, format=fmt, dpi=self.fig.get_dpi() * 2, bbox_inches="tight")
        overlay.remove()
        self.canvas.draw_idle()

    # ------------------------------------------------------------------ picking
    def _toggle_pick(self, on):
        self._pick_active = bool(on)
        self.canvas.setCursor(Qt.CrossCursor if self._pick_active else Qt.ArrowCursor)

    def _show_pick_dialog(self, checked=False):
        if self._pick_dialog is None:
            self._pick_dialog = _PickDialog(self)
            self._pick_dialog.finished.connect(lambda _=None: self.chk_pick.setChecked(False))
            self._pick_dialog.resize(max(600, self.width()), max(240, self.height() // 3))
            self._pick_dialog.setMinimumSize(600, 240)
        self._pick_dialog.show()
        if not self._pick_dialog_shown_once:
            self._pick_dialog.raise_()
            self._pick_dialog.activateWindow()
            self._pick_dialog_shown_once = True

    def pick(self, xdata, ydata):
        """Add the water column cell nearest to (xdata across, ydata depth) to the pick table."""
        lg = self._last_grid
        if not lg:
            self.statusBar().showMessage("Nothing to pick (no grid).", 3000)
            return None
        d2 = (lg["x"] - xdata) ** 2 + (lg["depth"] - ydata) ** 2
        d2 = np.where(np.isfinite(lg["amp"]), d2, np.nan)
        if not np.isfinite(d2).any():
            return None
        ib, ks = np.unravel_index(np.nanargmin(d2), d2.shape)
        across, depth = float(lg["x"][ib, ks]), float(lg["depth"][ib, ks])

        lat, lon, sog_kn, cog = (lambda v: (v[1], v[0], v[2], v[3]))(self.line.nav_at(lg["time_d"]))
        heading = self.line.heading_at(lg["time_d"])
        az, az_status = (heading, "Heading") if np.isfinite(heading) else ((cog, "COG") if np.isfinite(cog)
                                                                           else (np.nan, "None"))
        pt_lat = pt_lon = np.nan
        if np.isfinite(lat) and np.isfinite(lon):
            if np.isfinite(az):
                rad = math.radians(az)
                east_m, north_m = across * math.cos(rad), -across * math.sin(rad)
            else:
                east_m, north_m = across, 0.0
            pt_lat = lat + north_m / 111320.0
            pt_lon = lon + east_m / (111320.0 * max(1e-6, math.cos(math.radians(lat))))
        row = [f"{lg['step']} ({lg['ping']}/{lg['swath']})", _utc(lg["time_d"]), f"{across:.2f}", f"{depth:.2f}",
               _fmt(lat, ".6f", ""), _fmt(lon, ".6f", ""), _fmt(pt_lat, ".6f", ""), _fmt(pt_lon, ".6f", ""),
               az_status]
        if self._pick_dialog is None:
            self._show_pick_dialog()
        self._pick_dialog.add_row(row)
        return row

    def _on_plot_click(self, event):
        if event.button != 1 or event.inaxes is not self.ax or not self._pick_active:
            return
        try:
            self.pick(float(event.xdata), float(event.ydata))
        except Exception as e:  # keep the GUI alive, report in the status bar
            self.statusBar().showMessage(f"Pick click error: {e}", 5000)


class _PickDialog(QDialog):
    COLS = ["Ping", "UTC", "AcrossTrack_m", "Depth_m", "ShipLat", "ShipLon", "PointLat", "PointLon", "Azimuth"]
    closed = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Picked Positions")
        self.table = QTableWidget(0, len(self.COLS), self)
        self.table.setHorizontalHeaderLabels(self.COLS)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.table.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.table.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.btn_save = QPushButton("Save as TXT")
        self.btn_remove = QPushButton("Remove selected")
        self.btn_close = QPushButton("Close")
        self.btn_save.clicked.connect(lambda: self._save_txt())
        self.btn_remove.clicked.connect(self._remove_selected)
        self.btn_close.clicked.connect(self._clear_and_close)
        hl = QHBoxLayout()
        hl.addWidget(self.btn_save)
        hl.addWidget(self.btn_remove)
        hl.addStretch(1)
        hl.addWidget(self.btn_close)
        vl = QVBoxLayout(self)
        vl.addWidget(self.table)
        vl.addLayout(hl)

    def add_row(self, values):
        r = self.table.rowCount()
        self.table.insertRow(r)
        for c, v in enumerate(values):
            self.table.setItem(r, c, QTableWidgetItem("" if v is None else str(v)))

    def clear_rows(self):
        self.table.setRowCount(0)

    def _remove_selected(self):
        for r in sorted({i.row() for i in self.table.selectedIndexes()}, reverse=True):
            self.table.removeRow(r)

    def _save_txt(self, path=None):
        if not path:
            path, _ = QFileDialog.getSaveFileName(self, "Save Picks", "picks.txt", "Text Files (*.txt);;All Files (*)")
        if not path:
            return
        with open(path, "w") as f:
            f.write("\t".join(self.COLS) + "\n")
            for r in range(self.table.rowCount()):
                vals = [self.table.item(r, c).text() if self.table.item(r, c) else "" for c in
                        range(self.table.columnCount())]
                f.write("\t".join(vals) + "\n")

    def _clear_and_close(self):
        self.clear_rows()
        self.closed.emit()
        self.close()


def _bootstrap():
    app = QApplication.instance() or QApplication(sys.argv)
    win = MainWindow()
    win.show()
    if len(sys.argv) > 1:
        QTimer.singleShot(0, lambda: win.open_file(sys.argv[1]))
    return app.exec()


if __name__ == "__main__":
    print("[WCD Viewer] Entering Qt event loop...")
    sys.exit(_bootstrap())
