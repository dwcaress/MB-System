/*--------------------------------------------------------------------
 *    The MB-system:  mbne_core.cc  9/11/2026
 *
 *    Copyright (c) 2026 by
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

#include "mbne_core.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <utility>

#include "mb_aux.h"
#include "mb_format.h"
#include "mb_io.h"

namespace fs = std::filesystem;

namespace mbne {

namespace {

bool file_exists(const std::string &path) {
  std::error_code code;
  return fs::is_regular_file(path, code);
}

}  // namespace

Session::Session() {
  mb_uselockfiles(verbose, &uselockfiles_);
  recountPlots();
}

Session::~Session() {
  if (file_open_)
    closeFile(false);
}

void Session::error3(const std::string &a, const std::string &b, const std::string &c) {
  if (on_error)
    on_error(a, b, c);
  else
    fprintf(stderr, "%s\n%s\n%s\n", a.c_str(), b.c_str(), c.c_str());
}

void Session::progress(const std::string &message) {
  if (on_progress)
    on_progress(message);
}

/*--------------------------------------------------------------------*/
bool Session::lockInfo(const std::string &path, std::string &description, bool &mine) {
  bool locked = false;
  int purpose = MBP_LOCK_NONE, error = MB_ERROR_NO_ERROR;
  mb_path program = "", user = "", cpu = "";
  char date[MB_PATH_MAXLINE] = "";
  mb_pr_lockinfo(0, path.c_str(), &locked, &purpose, program, user, cpu, date, &error);
  mine = false;
  if (!locked)
    return false;
  description = std::string("<") + user + "> running <" + program + "> on cpu <" + cpu + "> at <" + date + ">";
  char this_user[256] = "", this_host[256] = "", this_date[32] = "";
  mb_user_host_date(0, this_user, this_host, this_date, &error);
  mine = strcmp(user, this_user) == 0 && strcmp(cpu, this_host) == 0;
  return true;
}

bool Session::removeLock(const std::string &path) { return std::remove((path + ".lck").c_str()) == 0; }

bool Session::hasSavedEdits(const std::string &path) { return file_exists(path + ".nve"); }

/*--------------------------------------------------------------------*/
int Session::open(const std::string &path, int format, bool use_previous, OutputMode output_mode) {
  if (file_open_)
    return MB_FAILURE;
  output_mode_ = output_mode;
  clearUndo();
  saved_depth_ = 0;
  dumped_changes_ = false;
  saved_in_session_ = false;

  int status = openFile(path, format, use_previous);
  if (status != MB_SUCCESS)
    return MB_FAILURE;
  status = loadData();

  if (nbuff() > 0) {
    /* the view starts with all of the buffer */
    data_show_size = 0;
    status = plotAll();
  }
  else {
    error3("No data were read from the input", "file. You may have specified an", "incorrect MB-System format id!");
    closeFile(false);
    status = MB_FAILURE;
  }
  return status;
}

int Session::openFile(const std::string &path, int format, bool use_previous) {
  if (path.size() >= sizeof(mb_path)) {
    error3("Unable to open input file:", "the path is too long", "");
    return MB_FAILURE;
  }
  progress("Opening a data file...");
  snprintf(ifile_, sizeof(ifile_), "%s", path.c_str());
  format_ = format;
  if (format_ == 0) {
    int form, format_error;
    if (mb_get_format(verbose, ifile_, nullptr, &form, &format_error) == MB_SUCCESS)
      format_ = form;
  }
  snprintf(nfile_, sizeof(nfile_), "%s.nve", ifile_);

  int status = MB_SUCCESS;
  error_ = MB_ERROR_NO_ERROR;
  if (output_mode_ == OUTPUT_EDIT && uselockfiles_) {
    status = mb_pr_lockswathfile(verbose, ifile_, MBP_LOCK_EDITNAV, PROGRAM_NAME, &error_);
  }
  else {
    std::string description;
    bool mine;
    if (lockInfo(ifile_, description, mine))
      fprintf(stderr, "\nFile %s locked by %s but lock ignored\n", ifile_, description.c_str());
    error_ = MB_ERROR_NO_ERROR;
  }
  if (status == MB_FAILURE) {
    if (error_ == MB_ERROR_FILE_LOCKED) {
      std::string description;
      bool mine;
      lockInfo(ifile_, description, mine);
      error3("Unable to open input file:", "File locked by " + description, path);
    }
    else {
      error3("Unable to create lock file", "for intended input file:", "-Likely permissions issue");
    }
    return MB_FAILURE;
  }
  const bool locked = output_mode_ == OUTPUT_EDIT && uselockfiles_;
  const auto unlock = [this, locked] {
    if (locked) {
      int unlock_error = MB_ERROR_NO_ERROR;
      mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITNAV, PROGRAM_NAME, &unlock_error);
    }
  };

  /* earlier edits are read from the .nve itself, which stays as it is until a save */
  mb_pathplus ifile_use;
  int format_use = format_;
  if (use_previous) {
    snprintf(ifile_use, sizeof(ifile_use), "%s", nfile_);
    format_use = MBF_MBPRONAV;
  }
  else {
    snprintf(ifile_use, sizeof(ifile_use), "%s", ifile_);
  }

  mb_format_source(verbose, &format_use, &platform_source_, &nav_source_, &sensordepth_source_, &heading_source_,
                   &attitude_source_, &svp_source_, &error_);
  int beams_bath = 0, beams_amp = 0, pixels_ss = 0;
  double bounds[4] = {-360.0, 360.0, -90.0, 90.0};
  double btime_d, etime_d;
  if (mb_read_init(verbose, ifile_use, format_use, 1, 0, bounds, btime_i, etime_i, 0.0, 1000000000.0, &imbio_ptr_, &btime_d, &etime_d,
                   &beams_bath, &beams_amp, &pixels_ss, &error_) != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, error_, &message);
    error3("Unable to open input file:", ifile_use, message != nullptr ? message : "");
    unlock();
    return MB_FAILURE;
  }

  beamflag_ = nullptr;
  bath_ = amp_ = bathacrosstrack_ = bathalongtrack_ = ss_ = ssacrosstrack_ = ssalongtrack_ = nullptr;
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack_, &error_);
  if (error_ != MB_ERROR_NO_ERROR) {
    error3("Unable to allocate memory", "for the data arrays of", ifile_);
    closeReader();
    unlock();
    return MB_FAILURE;
  }

  rec_.clear();
  first_read_ = false;
  at_eof_ = false;
  nload_total_ = 0;
  ndump_total_ = 0;
  plot_start_time_ = 0.0;
  plot_end_time_ = data_show_size;

  /* the new navigation goes to a file of its own */
  replacing_ = false;
  write_failed_ = false;
  new_file_.clear();
  if (output_mode_ == OUTPUT_EDIT) {
    new_file_ = std::string(nfile_) + ".new";
    nfp_ = fopen(new_file_.c_str(), "w");
    if (nfp_ == nullptr) {
      error3("Unable to open new nav edit save file.", "You may not have write", "permission in this directory!");
      closeReader();
      unlock();
      return MB_FAILURE;
    }
    replacing_ = !use_previous && file_exists(nfile_);
  }
  file_open_ = true;
  return MB_SUCCESS;
}

void Session::closeReader() {
  mb_close(verbose, &imbio_ptr_, &error_);
  imbio_ptr_ = nullptr;
  beamflag_ = nullptr;
  bath_ = amp_ = bathacrosstrack_ = bathalongtrack_ = ss_ = ssacrosstrack_ = ssalongtrack_ = nullptr;
}

/*--------------------------------------------------------------------*/
bool Session::commitNve(const std::string &source) {
  std::error_code code;
  fs::rename(source, nfile_, code);
  if (code) {
    error3("Unable to replace the navigation edit file", nfile_, code.message());
    return false;
  }
  return true;
}

void Session::updatePar() {
  int error = MB_ERROR_NO_ERROR;
  mb_pr_update_format(verbose, ifile_, true, format_, &error);
  mb_pr_update_nav(verbose, ifile_, MBP_NAV_ON, nfile_, 9, MBP_NAV_ON, MBP_NAV_ON, MBP_NAV_ON, MBP_NAV_ON, MBP_NAV_LINEAR, 0.0, &error);
}

int Session::closeFile(bool commit) {
  progress("Closing a data file...");
  clearUndo();
  rec_.clear();
  closeReader();

  bool write_ok = !write_failed_;
  if (nfp_ != nullptr) {
    if (fclose(nfp_) != 0)
      write_ok = false;
    nfp_ = nullptr;
  }

  int status = MB_SUCCESS;
  bool saved = false;
  if (output_mode_ == OUTPUT_EDIT) {
    if (commit && write_ok && commitNve(new_file_)) {
      updatePar();
      saved = true;
    }
    else {
      if (commit) {
        if (!write_ok)
          error3("Unable to write the navigation edit file", new_file_, "The earlier navigation edit file is unchanged");
        status = MB_FAILURE;
      }
      if (write_ok || !commit)
        std::remove(new_file_.c_str());
    }
    if (uselockfiles_) {
      int unlock_error = MB_ERROR_NO_ERROR;
      mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITNAV, PROGRAM_NAME, &unlock_error);
    }
  }

  file_open_ = false;
  nload_total_ = 0;
  ndump_total_ = 0;
  offset_lon_ = 0.0;
  offset_lat_ = 0.0;
  offset_lon_applied_ = 0.0;
  offset_lat_applied_ = 0.0;
  if ((saved || saved_in_session_) && on_saved)
    on_saved(ifile_);
  saved_in_session_ = false;
  return status;
}

int Session::actionClose(bool commit) {
  int status = MB_SUCCESS;
  if (file_open_ && output_mode_ == OUTPUT_BROWSE) {
    dumpData(0);
    status = closeFile(false);
  }
  else if (file_open_ && commit) {
    /* every record reaches the new file, read or not */
    int save_ndumped = 0, save_nloaded = 0;
    do {
      dumpData(0);
      save_ndumped += ndump_;
      status = loadData();
      save_nloaded += nload_;
    } while (nload_ > 0);
    ndump_ = save_ndumped;
    nload_ = save_nloaded;
    status = closeFile(true);
  }
  else if (file_open_) {
    status = closeFile(false);
  }
  else {
    ndump_ = 0;
    nload_ = 0;
    rec_.clear();
    current_id_ = 0;
    status = MB_FAILURE;
  }
  return status;
}

bool Session::canSave() const { return file_open_ && output_mode_ == OUTPUT_EDIT && nfp_ != nullptr && at_eof_; }

int Session::save() {
  if (!canSave())
    return MB_FAILURE;
  fflush(nfp_);

  /* what has left the buffer already, and then the buffer */
  const std::string saving = std::string(nfile_) + ".save";
  std::error_code code;
  fs::copy_file(new_file_, saving, fs::copy_options::overwrite_existing, code);
  FILE *fp = code ? nullptr : fopen(saving.c_str(), "a");
  bool ok = fp != nullptr && !write_failed_;
  if (ok)
    for (const Record &r : rec_)
      writeRecord(fp, r);
  ok = ok && !write_failed_;
  if (fp != nullptr && fclose(fp) != 0)
    ok = false;
  if (!ok || !commitNve(saving)) {
    if (!ok)
      error3("Unable to write the navigation edit file", saving, "The navigation has not been saved");
    std::remove(saving.c_str());
    write_failed_ = false;
    return MB_FAILURE;
  }
  updatePar();
  saved_depth_ = (long)undo_.size();
  dumped_changes_ = false;
  replacing_ = false;
  saved_in_session_ = true;
  return MB_SUCCESS;
}

bool Session::unsaved() const {
  return file_open_ && output_mode_ == OUTPUT_EDIT && (replacing_ || dumped_changes_ || (long)undo_.size() != saved_depth_);
}

int Session::closeSave() {
  if (file_open_ && output_mode_ == OUTPUT_EDIT && !compatibility && !unsaved())
    return actionClose(false);
  return actionClose(true);
}

int Session::closeDiscard() {
  if (!file_open_)
    return MB_FAILURE;
  return closeFile(false);
}

int Session::done(bool *quit) {
  *quit = gui_mode;
  return file_open_ ? closeSave() : MB_SUCCESS;
}

int Session::nextBuffer(bool *quit) {
  *quit = false;
  int status = MB_SUCCESS;
  if (file_open_) {
    if ((long)undo_.size() != saved_depth_)
      dumped_changes_ = true;
    clearUndo();
    saved_depth_ = 0;
    dumpData(hold_size);
    status = loadData();
    if (nload_ <= 0) {
      const int save_dumped = ndump_;
      dumpData(0);
      status = closeFile(compatibility || unsaved());
      ndump_ += save_dumped;
      *quit = gui_mode;
    }
    else {
      status = plotAll();
    }
  }
  else {
    status = MB_FAILURE;
    ndump_ = 0;
    nload_ = 0;
    current_id_ = 0;
  }
  return status;
}

/*--------------------------------------------------------------------*/
void Session::writeRecord(FILE *fp, const Record &r) {
  if (fprintf(fp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.3f %.3f %.4f %.3f %.3f %.4f\r\n", r.time_i[0], r.time_i[1],
              r.time_i[2], r.time_i[3], r.time_i[4], r.time_i[5], r.time_i[6], r.time_d, r.lon, r.lat, r.heading, r.speed, r.draft, r.roll,
              r.pitch, r.heave) < 0)
    write_failed_ = true;
}

void Session::dumpData(int hold) {
  if (hold > nbuff())
    hold = nbuff();
  if (hold < 0)
    hold = 0;

  if (nfp_ != nullptr)
    for (int i = 0; i < nbuff() - hold; i++)
      writeRecord(nfp_, rec_[i]);

  ndump_ = 0;
  if (nbuff() > 0) {
    ndump_ = nbuff() - hold;
    rec_.erase(rec_.begin(), rec_.begin() + ndump_);
  }
  ndump_total_ += ndump_;

  if (ndump_ > 0)
    current_id_ -= ndump_;
  if (current_id_ < 0)
    current_id_ = 0;
  if (current_id_ > nbuff() - 1)
    current_id_ = nbuff() - 1;
}

int Session::loadData() {
  nload_ = 0;
  timestamp_problem_ = false;
  progress("0 records loaded so far...");
  const int first_new = nbuff();

  int kind = 0, nbath = 0, namp = 0, nss = 0;
  double distance = 0.0, altitude = 0.0, sensordepth = 0.0;
  char comment[MB_COMMENT_MAXLINE];
  int status = MB_SUCCESS;
  Record r;
  do {
    status = mb_get_all(verbose, imbio_ptr_, &store_ptr_, &kind, r.time_i, &r.time_d, &r.lon, &r.lat, &r.speed, &r.heading, &distance, &altitude,
                        &sensordepth, &nbath, &namp, &nss, beamflag_, bath_, amp_, bathacrosstrack_, bathalongtrack_, ss_, ssacrosstrack_,
                        ssalongtrack_, comment, &error_);
    const bool wanted = kind == nav_source_ || (kind == MB_DATA_DATA && use_ping_data);
    if (error_ <= MB_ERROR_NO_ERROR && wanted &&
        (error_ == MB_ERROR_NO_ERROR || error_ == MB_ERROR_TIME_GAP || error_ == MB_ERROR_OUT_BOUNDS || error_ == MB_ERROR_OUT_TIME ||
         error_ == MB_ERROR_SPEED_TOO_SMALL)) {
      status = MB_SUCCESS;
      error_ = MB_ERROR_NO_ERROR;
    }
    else if (error_ <= MB_ERROR_NO_ERROR) {
      status = MB_FAILURE;
      error_ = MB_ERROR_OTHER;
    }
    if (error_ == MB_ERROR_NO_ERROR && (kind == nav_source_ || (kind == MB_DATA_DATA && use_ping_data))) {
      status = mb_extract_nav(verbose, imbio_ptr_, store_ptr_, &kind, r.time_i, &r.time_d, &r.lon, &r.lat, &r.speed, &r.heading, &r.draft, &r.roll,
                              &r.pitch, &r.heave, &error_);
    }
    if (status == MB_SUCCESS) {
      if (!first_read_) {
        file_start_time_d_ = r.time_d;
        first_read_ = true;
      }
      r.id = nload_;
      r.record = r.id + ndump_total_;
      r.lon_org = r.lon;
      r.lat_org = r.lat;
      r.speed_org = r.speed;
      r.heading_org = r.heading;
      r.draft_org = r.draft;
      r.file_time_d = r.time_d - file_start_time_d_;
      r.lon += offset_lon_;
      r.lat += offset_lat_;
      r.mean_ok = false;
      r.lon_dr = r.lon;
      r.lat_dr = r.lat;
      r.selected = 0;
      r.lonlat_flag = !rec_.empty() && r.lon == rec_.back().lon && r.lat == rec_.back().lat;
      rec_.push_back(r);
      nload_++;
      if (nload_ % 250 == 0)
        progress(std::to_string(nload_) + " records loaded so far...");
    }
  } while (error_ <= MB_ERROR_NO_ERROR && nbuff() < buffer_size);
  at_eof_ = error_ > MB_ERROR_NO_ERROR;
  nload_total_ += nload_;

  if (nbuff() > 0) {
    status = MB_SUCCESS;
    error_ = MB_ERROR_NO_ERROR;
  }

  timestamp_problem_ = false;
  for (int i = 0; i < nbuff() - 1; i++)
    if (rec_[i + 1].time_d <= rec_[i].time_d)
      timestamp_problem_ = true;

  /* the Motif program also resets the original times of the records kept from the last buffer */
  const int first_original = compatibility ? 0 : first_new;
  const int n = nbuff();
  if (n > 1) {
    for (int i = 1; i < n; i++) {
      rec_[i].tint = rec_[i].time_d - rec_[i - 1].time_d;
      if (i >= first_original) {
        rec_[i].tint_org = rec_[i].tint;
        rec_[i].time_d_org = rec_[i].time_d;
      }
    }
    rec_[0].tint = rec_[1].tint;
    if (first_original == 0) {
      rec_[0].tint_org = rec_[1].tint_org;
      rec_[0].time_d_org = rec_[0].time_d;
    }
  }
  else if (n == 1) {
    rec_[0].tint = 0.0;
    if (first_original == 0) {
      rec_[0].tint_org = 0.0;
      rec_[0].time_d_org = rec_[0].time_d;
    }
  }

  current_id_ = 0;
  if (n > 0) {
    data_show_size = 0;
    plot_start_time_ = rec_[0].file_time_d;
    plot_end_time_ = rec_[n - 1].file_time_d;
    nplot_ = n;
  }

  for (int i = 0; i < n; i++)
    getSmgCmg(i);
  getModel();

  if (timestamp_problem_)
    error3("Duplicate or reverse order time", "stamps detected!! Time interpolation", "available under Controls menu.");
  return status;
}

/*--------------------------------------------------------------------*/
void Session::recountPlots() {
  number_plots_ = (show.tint ? 1 : 0) + (show.lon ? 1 : 0) + (show.lat ? 1 : 0) + (show.speed ? 1 : 0) + (show.heading ? 1 : 0) +
                  (show.draft ? 1 : 0) + (show.roll ? 1 : 0) + (show.pitch ? 1 : 0) + (show.heave ? 1 : 0);
}

void Session::setPlotSize(int width, int height) {
  plot_width_ = width;
  plot_height_ = height;
}

void Session::setPlotMargins(int left, int right, int top, int bottom) {
  margin_left_ = left;
  margin_right_ = right;
  margin_top_ = top;
  margin_bottom_ = bottom;
}

int Session::plotAll() {
  const int n = nbuff();
  nplot_ = 0;
  if (data_show_size > 0 && n > 0) {
    plot_start_time_ = rec_[current_id_].file_time_d;
    plot_end_time_ = plot_start_time_ + data_show_size;
    for (int i = current_id_; i < n; i++)
      if (rec_[i].file_time_d <= plot_end_time_)
        nplot_++;
  }
  else if (n > 0) {
    plot_start_time_ = rec_[0].file_time_d;
    plot_end_time_ = rec_[n - 1].file_time_d;
    data_show_size = plot_end_time_ - plot_start_time_ + 1;
    if (data_show_max < data_show_size)
      data_show_max = data_show_size;
    nplot_ = n;
  }

  /* nothing stays selected outside of the view */
  for (int i = 0; i < current_id_; i++)
    rec_[i].selected = 0;
  for (int i = current_id_ + nplot_; i < n; i++)
    rec_[i].selected = 0;

  if (nplot_ <= 0)
    return MB_FAILURE;

  const Record &f = rec_[current_id_];
  double time_min = plot_start_time_, time_max = plot_end_time_;
  double tint_min = f.tint, tint_max = f.tint;
  double lon_min = f.lon, lon_max = f.lon;
  double lat_min = f.lat, lat_max = f.lat;
  double speed_min = 0.0, speed_max = f.speed;
  double heading_min = f.heading, heading_max = f.heading;
  double draft_min = f.draft, draft_max = f.draft;
  double roll_min = f.roll, roll_max = f.roll;
  double pitch_min = f.pitch, pitch_max = f.pitch;
  double heave_min = f.heave, heave_max = f.heave;
  for (int i = current_id_ + 1; i < current_id_ + nplot_; i++) {
    const Record &r = rec_[i];
    tint_min = std::min(r.tint, tint_min);
    tint_max = std::max(r.tint, tint_max);
    if (show.tint_org) {
      tint_min = std::min(r.tint_org, tint_min);
      tint_max = std::max(r.tint_org, tint_max);
    }
    lon_min = std::min(r.lon, lon_min);
    lon_max = std::max(r.lon, lon_max);
    if (show.lon_org) {
      lon_min = std::min(r.lon_org, lon_min);
      lon_max = std::max(r.lon_org, lon_max);
    }
    if (model_mode != MODEL_OFF && show.lon_dr) {
      lon_min = std::min(r.lon_dr, lon_min);
      lon_max = std::max(r.lon_dr, lon_max);
    }
    lat_min = std::min(r.lat, lat_min);
    lat_max = std::max(r.lat, lat_max);
    if (show.lat_org) {
      lat_min = std::min(r.lat_org, lat_min);
      lat_max = std::max(r.lat_org, lat_max);
    }
    if (model_mode != MODEL_OFF && show.lat_dr) {
      lat_min = std::min(r.lat_dr, lat_min);
      lat_max = std::max(r.lat_dr, lat_max);
    }
    speed_min = std::min(r.speed, speed_min);
    speed_max = std::max(r.speed, speed_max);
    if (show.speed_org) {
      speed_min = std::min(r.speed_org, speed_min);
      speed_max = std::max(r.speed_org, speed_max);
    }
    if (show.smg) {
      speed_min = std::min(r.speed_made_good, speed_min);
      speed_max = std::max(r.speed_made_good, speed_max);
    }
    heading_min = std::min(r.heading, heading_min);
    heading_max = std::max(r.heading, heading_max);
    if (show.heading_org) {
      heading_min = std::min(r.heading_org, heading_min);
      heading_max = std::max(r.heading_org, heading_max);
    }
    if (show.cmg) {
      heading_min = std::min(r.course_made_good, heading_min);
      heading_max = std::max(r.course_made_good, heading_max);
    }
    draft_min = std::min(r.draft, draft_min);
    draft_max = std::max(r.draft, draft_max);
    if (show.draft_org) {
      draft_min = std::min(r.draft_org, draft_min);
      draft_max = std::max(r.draft_org, draft_max);
    }
    roll_min = std::min(r.roll, roll_min);
    roll_max = std::max(r.roll, roll_max);
    pitch_min = std::min(r.pitch, pitch_min);
    pitch_max = std::max(r.pitch, pitch_max);
    heave_min = std::min(r.heave, heave_min);
    heave_max = std::max(r.heave, heave_max);
  }

  /* a little larger, so that all of the points fit */
  const auto widen = [](double &lo, double &hi, double factor) {
    const double center = 0.5 * (lo + hi);
    const double range = factor * (hi - lo);
    lo = center - range;
    hi = center + range;
  };
  widen(time_min, time_max, 0.51);
  widen(tint_min, tint_max, 0.55);
  widen(lon_min, lon_max, 0.55);
  widen(lat_min, lat_max, 0.55);
  if (speed_min < 0.0)
    widen(speed_min, speed_max, 0.55);
  else
    speed_max = 1.05 * speed_max;
  widen(heading_min, heading_max, 0.55);
  widen(draft_min, draft_max, 0.55);
  roll_max = 1.1 * std::max(std::fabs(roll_min), std::fabs(roll_max));
  roll_min = -roll_max;
  pitch_max = 1.1 * std::max(std::fabs(pitch_min), std::fabs(pitch_max));
  pitch_min = -pitch_max;
  heave_max = 1.1 * std::max(std::fabs(heave_min), std::fabs(heave_max));
  heave_min = -heave_max;

  /* longitude and latitude on the same scale when both are shown */
  if (show.lon && show.lat) {
    if ((lon_max - lon_min) > (lat_max - lat_min)) {
      const double center = 0.5 * (lat_min + lat_max);
      lat_min = center - 0.5 * (lon_max - lon_min);
      lat_max = center + 0.5 * (lon_max - lon_min);
    }
    else {
      const double center = 0.5 * (lon_min + lon_max);
      lon_min = center - 0.5 * (lat_max - lat_min);
      lon_max = center + 0.5 * (lat_max - lat_min);
    }
  }

  /* no range too small to see */
  const auto at_least = [](double &lo, double &hi, double smallest, double half) {
    if ((hi - lo) < smallest) {
      const double center = 0.5 * (lo + hi);
      lo = center - half;
      hi = center + half;
    }
  };
  at_least(time_min, time_max, 1.0e-6, 0.5);
  at_least(tint_min, tint_max, 0.01, 0.005);
  at_least(lon_min, lon_max, 0.001, 0.0005);
  at_least(lat_min, lat_max, 0.001, 0.0005);
  if (speed_max < 10.0)
    speed_max = 10.0;
  at_least(heading_min, heading_max, 10.0, 5.0);
  at_least(draft_min, draft_max, 0.1, 0.05);
  at_least(roll_min, roll_max, 2.0, 1.0);
  at_least(pitch_min, pitch_max, 2.0, 1.0);
  at_least(heave_min, heave_max, 0.02, 0.01);

  const int margin_x = plot_width_ / 10;
  const int margin_y = plot_height_ / 6;

  int xtime_i[7];
  mb_get_date(verbose, file_start_time_d_ + plot_start_time_, xtime_i);
  char xlabel[128];
  snprintf(xlabel, sizeof(xlabel), "Time (HH:MM:SS.SSS) beginning on %2.2d/%2.2d/%4.4d", xtime_i[1], xtime_i[2], xtime_i[0]);

  plots_.clear();
  const auto add_plot = [&](int type, double ymin, double ymax, const char *label1, const char *label2) {
    const int k = (int)plots_.size();
    Plot p;
    p.type = type;
    if (margin_left_ >= 0) {
      p.ixmin = margin_left_;
      p.ixmax = plot_width_ - margin_right_;
      p.iymin = plot_height_ - margin_bottom_ + k * plot_height_;
      p.iymax = k * plot_height_ + margin_top_;
    }
    else {
      p.ixmin = 1.25 * margin_x;
      p.ixmax = plot_width_ - margin_x / 2;
      p.iymin = plot_height_ - margin_y + k * plot_height_;
      p.iymax = k * plot_height_ + margin_y;
    }
    p.xmin = time_min;
    p.xmax = time_max;
    p.ymin = ymin;
    p.ymax = ymax;
    p.xscale = (p.ixmax - p.ixmin) / (p.xmax - p.xmin);
    p.yscale = (p.iymax - p.iymin) / (p.ymax - p.ymin);
    p.xlabel = xlabel;
    p.ylabel1 = label1;
    p.ylabel2 = label2;
    plots_.push_back(p);
  };
  if (show.tint)
    add_plot(PLOT_TINT, tint_min, tint_max, "dT", "(seconds)");
  if (show.lon)
    add_plot(PLOT_LONGITUDE, lon_min, lon_max, "Longitude", "(degrees)");
  if (show.lat)
    add_plot(PLOT_LATITUDE, lat_min, lat_max, "Latitude", "(degrees)");
  if (show.speed)
    add_plot(PLOT_SPEED, speed_min, speed_max, "Speed", "(km/hr)");
  if (show.heading)
    add_plot(PLOT_HEADING, heading_min, heading_max, "Heading", "(degrees)");
  if (show.draft)
    add_plot(PLOT_DRAFT, draft_max, draft_min, "Sonar Depth", "(meters)");
  if (show.roll)
    add_plot(PLOT_ROLL, roll_min, roll_max, "Roll", "(degrees)");
  if (show.pitch)
    add_plot(PLOT_PITCH, pitch_min, pitch_max, "Pitch", "(degrees)");
  if (show.heave)
    add_plot(PLOT_HEAVE, heave_min, heave_max, "Heave", "(meters)");
  number_plots_ = (int)plots_.size();

  /* where each point is */
  for (const Plot &p : plots_) {
    if (p.type >= EDITABLE_PLOTS)
      continue;
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      Record &r = rec_[i];
      const double value = p.type == PLOT_TINT        ? r.tint
                           : p.type == PLOT_LONGITUDE ? r.lon
                           : p.type == PLOT_LATITUDE  ? r.lat
                           : p.type == PLOT_SPEED     ? r.speed
                           : p.type == PLOT_HEADING   ? r.heading
                                                      : r.draft;
      r.pixel[2 * p.type] = p.ixmin + p.xscale * (r.file_time_d - p.xmin);
      r.pixel[2 * p.type + 1] = p.iymin + p.yscale * (value - p.ymin);
    }
  }
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::step(int seconds) {
  int status = MB_SUCCESS;
  if (file_open_ && nbuff() > 0) {
    if (seconds >= 0 && plot_end_time_ < rec_.back().file_time_d) {
      plot_start_time_ += seconds;
      plot_end_time_ = plot_start_time_ + data_show_size;
    }
    else if (seconds < 0 && plot_start_time_ > rec_.front().file_time_d) {
      plot_start_time_ += seconds;
      plot_end_time_ = plot_start_time_ + data_show_size;
    }

    bool set = false;
    const int old_id = current_id_;
    int new_id = nbuff() - 1;
    for (int i = 0; i < nbuff() && !set; i++) {
      if (rec_[i].file_time_d >= plot_start_time_) {
        new_id = i;
        set = true;
      }
    }
    if (new_id < 0)
      new_id = 0;
    if (new_id >= nbuff())
      new_id = nbuff() - 1;
    if (seconds < 0 && new_id > 0 && new_id == old_id)
      new_id--;
    if (seconds > 0 && new_id < nbuff() - 1 && new_id == old_id)
      new_id++;
    current_id_ = new_id;

    status = plotAll();
    if (new_id == old_id)
      status = MB_FAILURE;
  }
  else {
    status = MB_FAILURE;
    current_id_ = 0;
  }
  return status;
}

int Session::end() {
  int status = MB_SUCCESS;
  if (file_open_ && nbuff() > 0) {
    plot_end_time_ = rec_.back().file_time_d;
    plot_start_time_ = plot_end_time_ - data_show_size;
    const int old_id = current_id_;
    for (int i = 0; i < nbuff(); i++) {
      if (rec_[i].file_time_d >= plot_start_time_) {
        current_id_ = i;
        break;
      }
    }
    status = plotAll();
    if (current_id_ == old_id)
      status = MB_FAILURE;
  }
  else {
    status = MB_FAILURE;
    current_id_ = 0;
  }
  return status;
}

int Session::start() {
  int status = MB_SUCCESS;
  if (file_open_ && nbuff() > 0) {
    const int old_id = current_id_;
    current_id_ = 0;
    plot_start_time_ = rec_[0].file_time_d;
    plot_end_time_ = plot_start_time_ + data_show_size;
    status = plotAll();
    if (current_id_ == old_id)
      status = MB_FAILURE;
  }
  else {
    status = MB_FAILURE;
    current_id_ = 0;
  }
  return status;
}

int Session::showAll() {
  if (nbuff() > 0) {
    plot_start_time_ = rec_.front().file_time_d;
    plot_end_time_ = rec_.back().file_time_d;
    data_show_size = 0;
    current_id_ = 0;
  }
  return plotAll();
}

int Session::goToTime(double time_d) {
  if (!file_open_ || nbuff() == 0)
    return MB_FAILURE;
  if (time_d < rec_.front().time_d || time_d > rec_.back().time_d)
    return MB_FAILURE;
  for (int i = 0; i < nbuff(); i++) {
    if (rec_[i].time_d >= time_d) {
      current_id_ = i;
      break;
    }
  }
  return plotAll();
}

/*--------------------------------------------------------------------*/
int Session::activePlotAt(int x, int y) const {
  int active = -1;
  if (nplot_ > 0) {
    for (int k = 0; k < number_plots_ && k < (int)plots_.size(); k++)
      if (x >= plots_[k].ixmin && x <= plots_[k].ixmax && y <= plots_[k].iymin && y >= plots_[k].iymax)
        active = k;
  }
  return active;
}

bool Session::deselectOthers(int active) {
  bool any = false;
  for (int k = 0; k < number_plots_ && k < (int)plots_.size(); k++)
    if (k != active && deselectAll(plots_[k].type) == MB_SUCCESS)
      any = true;
  return any;
}

int Session::mousePick(int x, int y) {
  const int active = activePlotAt(x, y);
  if (nplot_ <= 0 || active < 0)
    return MB_FAILURE;
  if (deselectOthers(active))
    plotAll();
  const int type = plots_[active].type;
  if (type >= EDITABLE_PLOTS)
    return MB_SUCCESS;

  int range_min = 100000;
  int iping = 0;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const int ix = x - rec_[i].pixel[2 * type];
    const int iy = y - rec_[i].pixel[2 * type + 1];
    const int range = (int)std::sqrt((double)(ix * ix + iy * iy));
    if (range < range_min) {
      range_min = range;
      iping = i;
    }
  }
  if (range_min <= PICK_DISTANCE)
    rec_[iping].setSelected(type, !rec_[iping].isSelected(type));
  return MB_SUCCESS;
}

int Session::mouseSelect(int x, int y) {
  const int active = activePlotAt(x, y);
  if (nplot_ <= 0 || active < 0)
    return MB_FAILURE;
  if (deselectOthers(active))
    plotAll();
  const int type = plots_[active].type;
  if (type >= EDITABLE_PLOTS)
    return MB_SUCCESS;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const int ix = x - rec_[i].pixel[2 * type];
    const int iy = y - rec_[i].pixel[2 * type + 1];
    if ((int)std::sqrt((double)(ix * ix + iy * iy)) <= SELECT_DISTANCE)
      rec_[i].setSelected(type, true);
  }
  return MB_SUCCESS;
}

int Session::mouseDeselect(int x, int y) {
  const int active = activePlotAt(x, y);
  if (nplot_ <= 0 || active < 0)
    return MB_FAILURE;
  if (deselectOthers(active))
    plotAll();
  const int type = plots_[active].type;
  if (type >= EDITABLE_PLOTS)
    return MB_SUCCESS;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const int ix = x - rec_[i].pixel[2 * type];
    const int iy = y - rec_[i].pixel[2 * type + 1];
    if ((int)std::sqrt((double)(ix * ix + iy * iy)) <= SELECT_DISTANCE)
      rec_[i].setSelected(type, false);
  }
  return MB_SUCCESS;
}

int Session::mouseSelectAll(int x, int y) {
  const int active = activePlotAt(x, y);
  if (nplot_ <= 0 || active < 0)
    return MB_FAILURE;
  deselectOthers(active);
  const int type = plots_[active].type;
  if (type < EDITABLE_PLOTS)
    for (int i = current_id_; i < current_id_ + nplot_; i++)
      rec_[i].setSelected(type, true);
  return plotAll();
}

int Session::mouseDeselectAll() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  for (int i = current_id_; i < current_id_ + nplot_; i++)
    rec_[i].selected = 0;
  return plotAll();
}

int Session::deselectAll(int type) {
  if (nplot_ <= 0 || type >= EDITABLE_PLOTS)
    return MB_FAILURE;
  int ndeselect = 0;
  for (Record &r : rec_) {
    if (r.isSelected(type)) {
      r.setSelected(type, false);
      ndeselect++;
    }
  }
  return ndeselect > 0 ? MB_SUCCESS : MB_FAILURE;
}

int Session::setInterval(int x, int y, int which) {
  (void)y;
  if (nplot_ <= 0 || number_plots_ <= 0 || plots_.empty())
    return MB_FAILURE;
  const Plot &p = plots_[0];
  if (which == 0 || which == 1) {
    if (x < p.ixmin)
      x = p.ixmin;
    if (x > p.ixmax)
      x = p.ixmax;
    const double time = p.xmin + (x - p.ixmin) / p.xscale;
    if (which == 0) {
      interval_bound1_ = x;
      interval_time1_ = time;
      interval_set1_ = true;
    }
    else {
      interval_bound2_ = x;
      interval_time2_ = time;
      interval_set2_ = true;
    }
    return MB_SUCCESS;
  }
  if (which == 2 && interval_set1_ && interval_set2_ && interval_bound1_ != interval_bound2_) {
    if (interval_bound1_ > interval_bound2_) {
      std::swap(interval_bound1_, interval_bound2_);
      std::swap(interval_time1_, interval_time2_);
    }
    plot_start_time_ = interval_time1_;
    plot_end_time_ = interval_time2_;
    data_show_size = plot_end_time_ - plot_start_time_;
    data_step_size = data_show_size / 4;
    if (data_step_size > data_step_max)
      data_step_max = 2 * data_step_size;
    for (int i = 0; i < nbuff(); i++) {
      if (rec_[i].file_time_d >= plot_start_time_) {
        current_id_ = i;
        break;
      }
    }
    if (current_id_ < 0)
      current_id_ = 0;
    if (current_id_ >= nbuff())
      current_id_ = nbuff() - 1;
    plotAll();
    return MB_SUCCESS;
  }
  if (which == 3) {
    interval_set1_ = false;
    interval_set2_ = false;
    return MB_SUCCESS;
  }
  return MB_FAILURE;
}

/*--------------------------------------------------------------------*/
Session::RecordState Session::stateOf(const Record &r) {
  RecordState s;
  std::memcpy(s.time_i, r.time_i, sizeof(s.time_i));
  s.time_d = r.time_d;
  s.file_time_d = r.file_time_d;
  s.tint = r.tint;
  s.lon = r.lon;
  s.lat = r.lat;
  s.speed = r.speed;
  s.heading = r.heading;
  s.draft = r.draft;
  s.speed_made_good = r.speed_made_good;
  s.course_made_good = r.course_made_good;
  s.lonlat_flag = r.lonlat_flag;
  return s;
}

void Session::setState(Record &r, const RecordState &s) {
  std::memcpy(r.time_i, s.time_i, sizeof(s.time_i));
  r.time_d = s.time_d;
  r.file_time_d = s.file_time_d;
  r.tint = s.tint;
  r.lon = s.lon;
  r.lat = s.lat;
  r.speed = s.speed;
  r.heading = s.heading;
  r.draft = s.draft;
  r.speed_made_good = s.speed_made_good;
  r.course_made_good = s.course_made_good;
  r.lonlat_flag = s.lonlat_flag;
}

bool Session::sameState(const RecordState &a, const RecordState &b) {
  return std::memcmp(a.time_i, b.time_i, sizeof(a.time_i)) == 0 && a.time_d == b.time_d && a.file_time_d == b.file_time_d && a.tint == b.tint &&
         a.lon == b.lon && a.lat == b.lat && a.speed == b.speed && a.heading == b.heading && a.draft == b.draft &&
         a.speed_made_good == b.speed_made_good && a.course_made_good == b.course_made_good && a.lonlat_flag == b.lonlat_flag;
}

Session::Scalars Session::scalars() const {
  return Scalars{current_id_, timestamp_problem_, offset_lon_, offset_lat_, offset_lon_applied_, offset_lat_applied_};
}

void Session::setScalars(const Scalars &s) {
  current_id_ = s.current_id;
  timestamp_problem_ = s.timestamp_problem;
  offset_lon_ = s.offset_lon;
  offset_lat_ = s.offset_lat;
  offset_lon_applied_ = s.offset_lon_applied;
  offset_lat_applied_ = s.offset_lat_applied;
}

long Session::UndoStep::bytes() const {
  return (long)(changes.size() * sizeof(Change) + (whole_before.size() + whole_after.size()) * sizeof(Record));
}

void Session::beginEdit(bool whole) {
  snapshot_scalars_ = scalars();
  whole_edit_ = whole;
  if (whole) {
    whole_snapshot_ = rec_;
  }
  else {
    snapshot_.resize(rec_.size());
    for (size_t i = 0; i < rec_.size(); i++)
      snapshot_[i] = stateOf(rec_[i]);
  }
}

void Session::finishEdit(const char *label) {
  UndoStep step;
  step.label = label;
  step.before = snapshot_scalars_;
  step.after = scalars();
  if (whole_edit_ && whole_snapshot_.size() != rec_.size()) {
    step.whole_before = std::move(whole_snapshot_);
    step.whole_after = rec_;
    std::vector<Record>().swap(whole_snapshot_);
  }
  else if (whole_edit_) {
    for (size_t i = 0; i < rec_.size(); i++) {
      const RecordState before = stateOf(whole_snapshot_[i]);
      const RecordState now = stateOf(rec_[i]);
      if (!sameState(before, now))
        step.changes.push_back({(int)i, before, now});
    }
    std::vector<Record>().swap(whole_snapshot_);
  }
  else {
    for (size_t i = 0; i < rec_.size(); i++) {
      const RecordState now = stateOf(rec_[i]);
      if (!sameState(snapshot_[i], now))
        step.changes.push_back({(int)i, snapshot_[i], now});
    }
    std::vector<RecordState>().swap(snapshot_);
  }
  const bool same_scalars = step.before.timestamp_problem == step.after.timestamp_problem &&
                            step.before.offset_lon == step.after.offset_lon && step.before.offset_lat == step.after.offset_lat &&
                            step.before.offset_lon_applied == step.after.offset_lon_applied &&
                            step.before.offset_lat_applied == step.after.offset_lat_applied;
  if (step.changes.empty() && step.whole_before.empty() && step.whole_after.empty() && same_scalars)
    return;

  if (saved_depth_ > (long)undo_.size())
    saved_depth_ = -1;  // the saved state was undone, and now it is out of reach
  undo_bytes_ += step.bytes();
  undo_.push_back(std::move(step));
  redo_.clear();
  while (undo_bytes_ > UNDO_LIMIT && undo_.size() > 1) {
    undo_bytes_ -= undo_.front().bytes();
    undo_.erase(undo_.begin());
    saved_depth_ = saved_depth_ > 0 ? saved_depth_ - 1 : -1;
  }
}

void Session::clearUndo() {
  undo_.clear();
  redo_.clear();
  undo_bytes_ = 0;
}

const std::string &Session::undoLabel() const {
  static const std::string none;
  return undo_.empty() ? none : undo_.back().label;
}

const std::string &Session::redoLabel() const {
  static const std::string none;
  return redo_.empty() ? none : redo_.back().label;
}

int Session::applyStep(const UndoStep &step, bool forward) {
  const int view = current_id_;
  const bool whole = !step.whole_before.empty() || !step.whole_after.empty();
  if (whole) {
    rec_ = forward ? step.whole_after : step.whole_before;
  }
  else {
    for (const Change &c : step.changes)
      setState(rec_[c.index], forward ? c.after : c.before);
  }
  setScalars(forward ? step.after : step.before);
  /* undo does not move the view, unless the records themselves come or go */
  if (!whole)
    current_id_ = view;
  getModel();
  return plotAll();
}

int Session::undo() {
  if (!file_open_ || undo_.empty())
    return MB_FAILURE;
  UndoStep step = std::move(undo_.back());
  undo_.pop_back();
  undo_bytes_ -= step.bytes();
  const int status = applyStep(step, false);
  redo_.push_back(std::move(step));
  return status;
}

int Session::redo() {
  if (!file_open_ || redo_.empty())
    return MB_FAILURE;
  UndoStep step = std::move(redo_.back());
  redo_.pop_back();
  const int status = applyStep(step, true);
  undo_bytes_ += step.bytes();
  undo_.push_back(std::move(step));
  return status;
}

/*--------------------------------------------------------------------*/
int Session::useDr() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  int active = -1;
  for (const Plot &p : plots_)
    if (p.type == PLOT_LONGITUDE || p.type == PLOT_LATITUDE)
      active = p.type;
  if (active < 0)
    return MB_FAILURE;
  beginEdit();
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    if (rec_[i].isSelected(PLOT_LONGITUDE) || rec_[i].isSelected(PLOT_LATITUDE)) {
      rec_[i].lon = rec_[i].lon_dr;
      rec_[i].lat = rec_[i].lat_dr;
    }
  }
  for (int i = 0; i < nbuff(); i++)
    getSmgCmg(i);
  finishEdit("Use Solution");
  return plotAll();
}

int Session::useSmg() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  bool shown = false;
  for (const Plot &p : plots_)
    shown = shown || p.type == PLOT_SPEED;
  if (!shown)
    return MB_FAILURE;
  beginEdit();
  bool changed = false;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    if (rec_[i].isSelected(PLOT_SPEED)) {
      rec_[i].speed = rec_[i].speed_made_good;
      changed = true;
    }
  }
  if (changed && model_mode == MODEL_DR)
    getModel();
  finishEdit("Use Speed-Made-Good");
  return plotAll();
}

int Session::useCmg() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  bool shown = false;
  for (const Plot &p : plots_)
    shown = shown || p.type == PLOT_HEADING;
  if (!shown)
    return MB_FAILURE;
  beginEdit();
  bool changed = false;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    if (rec_[i].isSelected(PLOT_HEADING)) {
      rec_[i].heading = rec_[i].course_made_good;
      changed = true;
    }
  }
  if (changed && model_mode == MODEL_DR)
    getModel();
  finishEdit("Use Course-Made-Good");
  return plotAll();
}

/* the nearest records on each side that are not selected; iping itself when there is none */
static void neighbours(const std::vector<Record> &rec, int iping, int type, int &ibefore, int &iafter) {
  ibefore = iping;
  for (int i = iping - 1; i >= 0; i--) {
    if (!rec[i].isSelected(type)) {
      ibefore = i;
      break;
    }
  }
  iafter = iping;
  for (int i = iping + 1; i < (int)rec.size(); i++) {
    if (!rec[i].isSelected(type)) {
      iafter = i;
      break;
    }
  }
}

double Session::headingBetween(double a, double b, double fraction) const {
  /* the short way round north */
  double delta = b - a;
  while (delta > 180.0)
    delta -= 360.0;
  while (delta < -180.0)
    delta += 360.0;
  double value = a + delta * fraction;
  if (value < 0.0)
    value += 360.0;
  else if (value >= 360.0)
    value -= 360.0;
  return value;
}

void Session::interpolateTrack(double Record::*field, int type, bool &changed) {
  const int n = nbuff();
  for (int iping = 0; iping < n; iping++) {
    Record &r = rec_[iping];
    if (!r.isSelected(type))
      continue;
    int ibefore, iafter;
    neighbours(rec_, iping, type, ibefore, iafter);
    if (ibefore < iping && iafter > iping) {
      const Record &b = rec_[ibefore], &a = rec_[iafter];
      if (a.time_d - b.time_d > 0.0)
        r.*field = b.*field + (a.*field - b.*field) * (r.time_d - b.time_d) / (a.time_d - b.time_d);
      else
        r.*field = b.*field + 0.5 * (a.*field - b.*field);
    }
    else if (ibefore < iping && ibefore > 0) {
      const Record &b = rec_[ibefore], &bb = rec_[ibefore - 1];
      if (b.time_d - bb.time_d > 0.0)
        r.*field = b.*field + (b.*field - bb.*field) * (r.time_d - b.time_d) / (b.time_d - bb.time_d);
      else
        r.*field = b.*field;
    }
    else if (ibefore < iping) {
      r.*field = rec_[ibefore].*field;
    }
    else if (iafter > iping && iafter < n - 1) {
      const Record &a = rec_[iafter], &aa = rec_[iafter + 1];
      if (aa.time_d - a.time_d > 0.0)
        r.*field = a.*field + (aa.*field - a.*field) * (r.time_d - a.time_d) / (aa.time_d - a.time_d);
      else
        r.*field = a.*field;
    }
    else if (iafter > iping) {
      r.*field = rec_[iafter].*field;
    }
    else {
      continue;
    }
    r.lonlat_flag = true;
    changed = true;
  }
}

void Session::interpolateHold(double Record::*field, int type, bool marks_position, bool &timelonlat_change, bool &speedheading_change) {
  const int n = nbuff();
  for (int iping = 0; iping < n; iping++) {
    Record &r = rec_[iping];
    if (!r.isSelected(type))
      continue;
    int ibefore, iafter;
    neighbours(rec_, iping, type, ibefore, iafter);
    if (ibefore < iping && iafter > iping) {
      const Record &b = rec_[ibefore], &a = rec_[iafter];
      const double dtime = a.time_d - b.time_d;
      const double fraction = dtime > 0.0 ? (r.time_d - b.time_d) / dtime : 0.5;
      if (field == &Record::heading && !compatibility) {
        r.*field = headingBetween(b.*field, a.*field, fraction);
      }
      else if (dtime > 0.0) {
        r.*field = b.*field + (a.*field - b.*field) * (r.time_d - b.time_d) / dtime;
      }
      else {
        r.*field = b.*field + 0.5 * (a.*field - b.*field);
      }
    }
    else if (ibefore < iping) {
      r.*field = rec_[ibefore].*field;
    }
    else if (iafter > iping) {
      r.*field = rec_[iafter].*field;
    }
    else {
      continue;
    }
    if (marks_position) {
      r.lonlat_flag = true;
      timelonlat_change = true;
    }
    else {
      speedheading_change = true;
    }
  }
}

int Session::interpolate() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  beginEdit();
  bool timelonlat_change = false;
  bool speedheading_change = false;
  const int n = nbuff();

  /* time */
  for (int iping = 0; iping < n; iping++) {
    Record &r = rec_[iping];
    if (!r.isSelected(PLOT_TINT))
      continue;
    int ibefore, iafter;
    neighbours(rec_, iping, PLOT_TINT, ibefore, iafter);
    bool done = true;
    if (ibefore < iping && iafter > iping) {
      r.time_d = rec_[ibefore].time_d + (rec_[iafter].time_d - rec_[ibefore].time_d) * ((double)(iping - ibefore)) / ((double)(iafter - ibefore));
      r.tint = r.time_d - rec_[iping - 1].time_d;
    }
    else if (ibefore < iping && ibefore > 0) {
      r.time_d = rec_[ibefore].time_d + (rec_[ibefore].time_d - rec_[ibefore - 1].time_d) * (iping - ibefore);
      r.tint = r.time_d - rec_[iping - 1].time_d;
    }
    else if (ibefore < iping) {
      r.time_d = rec_[ibefore].time_d;
      r.tint = r.time_d - rec_[iping - 1].time_d;
    }
    else if (iafter > iping && iafter < n - 1) {
      r.time_d = rec_[iafter].time_d + (rec_[iafter + 1].time_d - rec_[iafter].time_d) * (iping - iafter);
      r.tint = 0.0;
    }
    else if (iafter > iping) {
      r.time_d = rec_[iafter].time_d;
      r.tint = iping > 0 ? r.time_d - rec_[iping - 1].time_d : 0.0;
    }
    else {
      done = false;
    }
    if (done) {
      r.lonlat_flag = true;
      timelonlat_change = true;
    }
    r.file_time_d = r.time_d - file_start_time_d_;
    mb_get_date(verbose, r.time_d, r.time_i);
    if (iping < n - 1 && !rec_[iping + 1].isSelected(PLOT_TINT))
      rec_[iping + 1].tint = rec_[iping + 1].time_d - r.time_d;
  }

  interpolateTrack(&Record::lon, PLOT_LONGITUDE, timelonlat_change);
  interpolateTrack(&Record::lat, PLOT_LATITUDE, timelonlat_change);
  interpolateHold(&Record::speed, PLOT_SPEED, false, timelonlat_change, speedheading_change);
  interpolateHold(&Record::heading, PLOT_HEADING, false, timelonlat_change, speedheading_change);
  interpolateHold(&Record::draft, PLOT_DRAFT, true, timelonlat_change, speedheading_change);

  if (timelonlat_change)
    for (int i = 0; i < n; i++)
      getSmgCmg(i);
  if (speedheading_change && model_mode == MODEL_DR)
    getModel();
  finishEdit("Interpolate");
  return plotAll();
}

void Session::repeatField(double Record::*field, int type, bool &changed) {
  const int n = nbuff();
  for (int iping = 1; iping < n - 1; iping++) {
    if (!(rec_[iping].isSelected(type) && rec_[iping].*field == rec_[iping - 1].*field))
      continue;
    int iafter = iping;
    for (int j = iping + 1; j < n; j++) {
      if (rec_[iping].*field != rec_[j].*field) {
        iafter = j;
        break;
      }
    }
    const int ibefore = iping - 1;
    for (int j = iping; j < iafter; j++) {
      if (rec_[j].isSelected(type) && rec_[iafter].time_d != rec_[ibefore].time_d) {
        const double fraction = (rec_[j].time_d - rec_[ibefore].time_d) / (rec_[iafter].time_d - rec_[ibefore].time_d);
        if (field == &Record::heading && !compatibility)
          rec_[j].*field = headingBetween(rec_[ibefore].*field, rec_[iafter].*field, fraction);
        else
          rec_[j].*field = rec_[ibefore].*field + (rec_[iafter].*field - rec_[ibefore].*field) * (rec_[j].time_d - rec_[ibefore].time_d) /
                                                      (rec_[iafter].time_d - rec_[ibefore].time_d);
        changed = true;
      }
    }
  }
}

int Session::interpolateRepeats() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  beginEdit();
  bool timelonlat_change = false;
  bool speedheading_change = false;
  const int n = nbuff();

  for (int iping = 1; iping < n - 1; iping++) {
    if (!(rec_[iping].isSelected(PLOT_TINT) && rec_[iping].time_d == rec_[iping - 1].time_d))
      continue;
    int iafter = iping;
    for (int j = iping + 1; j < n; j++) {
      if (rec_[iping].time_d != rec_[j].time_d) {
        iafter = j;
        break;
      }
    }
    const int ibefore = iping - 1;
    for (int j = iping; j < iafter; j++) {
      if (rec_[j].isSelected(PLOT_TINT)) {
        rec_[j].time_d = rec_[ibefore].time_d + (rec_[iafter].time_d - rec_[ibefore].time_d) * ((double)(j - ibefore)) / ((double)(iafter - ibefore));
        rec_[j].file_time_d = rec_[j].time_d - file_start_time_d_;
        mb_get_date(verbose, rec_[j].time_d, rec_[j].time_i);
        rec_[j].tint = rec_[j].time_d - rec_[j - 1].time_d;
        rec_[j + 1].tint = rec_[j + 1].time_d - rec_[j].time_d;
        timelonlat_change = true;
      }
    }
  }
  repeatField(&Record::lon, PLOT_LONGITUDE, timelonlat_change);
  repeatField(&Record::lat, PLOT_LATITUDE, timelonlat_change);
  repeatField(&Record::speed, PLOT_SPEED, speedheading_change);
  repeatField(&Record::heading, PLOT_HEADING, speedheading_change);
  repeatField(&Record::draft, PLOT_DRAFT, timelonlat_change);

  if (timelonlat_change)
    for (int i = 0; i < n; i++)
      getSmgCmg(i);
  if (speedheading_change && model_mode == MODEL_DR)
    getModel();
  finishEdit("Interpolate Repeats");
  return plotAll();
}

int Session::revert() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  beginEdit();
  bool timelonlat_change = false;
  bool speedheading_change = false;
  for (const Plot &p : plots_) {
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      Record &r = rec_[i];
      if (p.type >= EDITABLE_PLOTS || !r.isSelected(p.type))
        continue;
      switch (p.type) {
      case PLOT_TINT:
        r.time_d = r.time_d_org;
        r.file_time_d = r.time_d - file_start_time_d_;
        r.tint = i > 0 ? r.time_d - rec_[i - 1].time_d : 0.0;
        timelonlat_change = true;
        if (i < nbuff() - 1)
          rec_[i + 1].tint = rec_[i + 1].time_d - r.time_d;
        mb_get_date(verbose, r.time_d, r.time_i);
        break;
      case PLOT_LONGITUDE:
        r.lon = r.lon_org;
        timelonlat_change = true;
        break;
      case PLOT_LATITUDE:
        r.lat = r.lat_org;
        timelonlat_change = true;
        break;
      case PLOT_SPEED:
        r.speed = r.speed_org;
        speedheading_change = true;
        break;
      case PLOT_HEADING:
        r.heading = r.heading_org;
        speedheading_change = true;
        break;
      default:
        r.draft = r.draft_org;
        break;
      }
    }
  }
  if (timelonlat_change)
    for (int i = 0; i < nbuff(); i++)
      getSmgCmg(i);
  if (speedheading_change && model_mode == MODEL_DR)
    getModel();
  finishEdit("Revert");
  return plotAll();
}

int Session::flag() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  beginEdit();
  for (const Plot &p : plots_) {
    if (p.type != PLOT_LONGITUDE && p.type != PLOT_LATITUDE)
      continue;
    for (int i = current_id_; i < current_id_ + nplot_; i++)
      if (rec_[i].isSelected(p.type))
        rec_[i].lonlat_flag = true;
  }
  finishEdit("Flag");
  return plotAll();
}

int Session::unflag() {
  if (nplot_ <= 0)
    return MB_FAILURE;
  beginEdit();
  for (const Plot &p : plots_) {
    if (p.type != PLOT_LONGITUDE && p.type != PLOT_LATITUDE)
      continue;
    for (int i = current_id_; i < current_id_ + nplot_; i++)
      if (rec_[i].isSelected(p.type))
        rec_[i].lonlat_flag = false;
  }
  finishEdit("Unflag");
  return plotAll();
}

int Session::fixTime() {
  beginEdit();
  const int n = nbuff();
  int istart = 0;
  double start_time_d = 0.0;
  for (int i = 0; i < n; i++) {
    if (i == 0) {
      istart = i;
      start_time_d = rec_[i].time_d;
    }
    else if (rec_[i].time_d > start_time_d) {
      const int iend = i;
      const double end_time_d = rec_[i].time_d;
      for (int j = istart + 1; j < iend; j++) {
        rec_[j].time_d = start_time_d + (j - istart) * (end_time_d - start_time_d) / (iend - istart);
        mb_get_date(verbose, rec_[j].time_d, rec_[j].time_i);
        rec_[j].file_time_d = rec_[j].time_d - file_start_time_d_;
        if (j > 0)
          rec_[j - 1].tint = rec_[j].time_d - rec_[j - 1].time_d;
        if (j < n - 1)
          rec_[j].tint = rec_[j + 1].time_d - rec_[j].time_d;
      }
      istart = i;
      start_time_d = rec_[i].time_d;
    }
  }
  if (!compatibility) {
    for (int i = 1; i < n; i++)
      rec_[i].tint = rec_[i].time_d - rec_[i - 1].time_d;
    if (n > 1)
      rec_[0].tint = rec_[1].tint;
    for (int i = 0; i < n; i++)
      getSmgCmg(i);
    getModel();
  }
  timestamp_problem_ = false;
  finishEdit("Time Interpolation");
  return plotAll();
}

int Session::deleteBadTime() {
  if (rec_.empty())
    return MB_FAILURE;
  beginEdit(true);
  const int n = nbuff();
  double lastgood_time_d = rec_[0].time_d;
  for (int i = 1; i < n; i++) {
    if ((rec_[i].time_d - lastgood_time_d) <= 0.0) {
      rec_[i].id = -1;
    }
    else if ((rec_[i].time_d - lastgood_time_d) > 60.0) {
      if (i == n - 1)
        rec_[i].id = -1;
      else if (rec_[i + 1].time_d - rec_[i].time_d <= 0.0)
        rec_[i].id = -1;
      else
        lastgood_time_d = rec_[i].time_d;
    }
    else if (rec_[i].time_d > rec_[n - 1].time_d) {
      rec_[i].id = -1;
    }
    else {
      lastgood_time_d = rec_[i].time_d;
    }
  }
  rec_.erase(std::remove_if(rec_.begin(), rec_.end(), [](const Record &r) { return r.id == -1; }), rec_.end());
  if (current_id_ > nbuff() - 1)
    current_id_ = nbuff() - 1;
  if (current_id_ < 0)
    current_id_ = 0;
  for (int i = 1; i < nbuff(); i++)
    rec_[i].tint = rec_[i].time_d - rec_[i - 1].time_d;
  if (nbuff() > 1)
    rec_[0].tint = rec_[1].tint;
  else if (nbuff() == 1)
    rec_[0].tint = 0.0;
  if (!compatibility) {
    for (int i = 0; i < nbuff(); i++)
      getSmgCmg(i);
    getModel();
  }
  timestamp_problem_ = false;
  finishEdit("Delete Bad Times");
  return plotAll();
}

int Session::applyOffset(double lon, double lat) {
  beginEdit();
  offset_lon_ = lon;
  offset_lat_ = lat;
  if (file_open_) {
    for (Record &r : rec_) {
      r.lon += offset_lon_ - offset_lon_applied_;
      r.lat += offset_lat_ - offset_lat_applied_;
    }
  }
  offset_lon_applied_ = offset_lon_;
  offset_lat_applied_ = offset_lat_;
  finishEdit("Position Offset");
  return plotAll();
}

/*--------------------------------------------------------------------*/
void Session::getSmgCmg(int i) {
  const int n = nbuff();
  if (i >= n)
    return;
  Record &r = rec_[i];
  if (n == 1) {
    r.speed_made_good = 0.0;
    r.course_made_good = r.heading;
    return;
  }
  const Record &one = i == 0 ? rec_[i] : rec_[i - 1];
  const Record &two = i == 0 ? rec_[i + 1] : rec_[i];
  double mtodeglon, mtodeglat;
  mb_coor_scale(verbose, one.lat, &mtodeglon, &mtodeglat);
  const double del_time = two.time_d - one.time_d;
  const double dx = (two.lon - one.lon) / mtodeglon;
  const double dy = (two.lat - one.lat) / mtodeglat;
  const double dist = std::sqrt(dx * dx + dy * dy);
  if (del_time > 0.0)
    r.speed_made_good = 3.6 * dist / del_time;
  else
    r.speed_made_good = 0.0;
  if (dist > 0.0)
    r.course_made_good = RTD * std::atan2(dx / dist, dy / dist);
  else
    r.course_made_good = r.heading;
  if (r.course_made_good < 0.0)
    r.course_made_good = r.course_made_good + 360.0;
}

void Session::getModel() {
  if (nbuff() > 0) {
    if (model_mode == MODEL_MEAN)
      getGaussianMean();
    else if (model_mode == MODEL_DR)
      getDeadReckoning();
    else if (model_mode == MODEL_INVERT)
      getInversion();
  }
}

void Session::getGaussianMean() {
  const int n = nbuff();
  const double timewindow = 0.1 * mean_time_window;
  const double a = -4.0 / (timewindow * timewindow);
  int jstart = 0;
  for (int i = 0; i < n; i++) {
    double dt = 0.0, weight = 0.0, sumlon = 0.0, sumlat = 0.0;
    int nsum = 0, npos = 0, nneg = 0;
    for (int j = jstart; j < n && dt <= timewindow; j++) {
      dt = rec_[j].time_d - rec_[i].time_d;
      if (!rec_[j].lonlat_flag && std::fabs(dt) <= timewindow) {
        const double w = std::exp(a * dt * dt);
        nsum++;
        if (dt < 0.0)
          nneg++;
        if (dt >= 0.0)
          npos++;
        weight += w;
        sumlon += w * rec_[j].lon;
        sumlat += w * rec_[j].lat;
        if (nsum == 1)
          jstart = j;
      }
    }
    if (npos > 0 && nneg > 0) {
      rec_[i].mean_ok = true;
      rec_[i].lon_dr = sumlon / weight;
      rec_[i].lat_dr = sumlat / weight;
    }
    else {
      rec_[i].mean_ok = false;
      rec_[i].lon_dr = rec_[i].lon;
      rec_[i].lat_dr = rec_[i].lat;
    }
  }

  /* fill the gaps with lines between valid points */
  int jbefore = -1;
  for (int i = 0; i < n; i++) {
    if (rec_[i].mean_ok)
      continue;
    int jafter = i;
    for (int j = (jbefore >= 0 ? jbefore : 0); j < n && jafter == i; j++) {
      if (j < i && !rec_[j].lonlat_flag)
        jbefore = j;
      if (j > i && !rec_[j].lonlat_flag)
        jafter = j;
    }
    if (jbefore >= 0 && jafter > i) {
      const double dtime = rec_[jafter].time_d - rec_[jbefore].time_d;
      const double dt = dtime > 0.0 ? (rec_[i].time_d - rec_[jbefore].time_d) / dtime : 0.5;
      rec_[i].lon_dr = rec_[jbefore].lon + dt * (rec_[jafter].lon - rec_[jbefore].lon);
      rec_[i].lat_dr = rec_[jbefore].lat + dt * (rec_[jafter].lat - rec_[jbefore].lat);
    }
    else if (jbefore >= 0) {
      rec_[i].lon_dr = rec_[jbefore].lon;
      rec_[i].lat_dr = rec_[jbefore].lat;
    }
    else if (jafter > i) {
      rec_[i].lon_dr = rec_[jafter].lon;
      rec_[i].lat_dr = rec_[jafter].lat;
    }
    else {
      rec_[i].lon_dr = rec_[i].lon;
      rec_[i].lat_dr = rec_[i].lat;
    }
  }
}

void Session::getDeadReckoning() {
  const double driftlon = 0.00001 * drift_lon;
  const double driftlat = 0.00001 * drift_lat;
  for (int i = 0; i < nbuff(); i++) {
    Record &r = rec_[i];
    if (i == 0) {
      r.lon_dr = r.lon;
      r.lat_dr = r.lat;
      continue;
    }
    const double del_time = r.time_d - rec_[i - 1].time_d;
    if (del_time < 300.0) {
      double mtodeglon, mtodeglat;
      mb_coor_scale(verbose, r.lat, &mtodeglon, &mtodeglat);
      const double dx = std::sin(DTR * r.heading) * r.speed * del_time / 3.6;
      const double dy = std::cos(DTR * r.heading) * r.speed * del_time / 3.6;
      r.lon_dr = rec_[i - 1].lon_dr + dx * mtodeglon + del_time * driftlon / 3600.0;
      r.lat_dr = rec_[i - 1].lat_dr + dy * mtodeglat + del_time * driftlat / 3600.0;
    }
    else {
      r.lon_dr = r.lon;
      r.lat_dr = r.lat;
    }
  }
}

void Session::getInversion() {
  if (nplot_ < 1)
    return;
  const int nrows = nplot_ + std::max(nplot_ - 1, 0) + std::max(nplot_ - 2, 0);
  const int ncols = nplot_;
  const int nnz = 3;
  const int ncycle = 512;
  const double bandwidth = 10000.0;

  double lon_avg = 0.0, lat_avg = 0.0;
  int navg = 0;
  int first = current_id_;
  int last = current_id_;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    if (!rec_[i].lonlat_flag) {
      lon_avg += rec_[i].lon;
      lat_avg += rec_[i].lat;
      navg++;
      last = i;
    }
    else if (first == i && i < current_id_ + nplot_ - 1) {
      first = i + 1;
    }
  }
  if (navg > 0) {
    lon_avg /= navg;
    lat_avg /= navg;
  }
  double mtodeglon, mtodeglat;
  mb_coor_scale(verbose, lat_avg, &mtodeglon, &mtodeglat);

  std::vector<double> a(nnz * nrows), d(nrows), x(ncols), dx(ncols), sigma(ncycle), work(ncycle);
  std::vector<int> ia(nnz * nrows), nia(nrows), nx(ncols);

  /* the same problem for longitude and latitude */
  const auto solve = [&](double Record::*org, double avg, double scale, double Record::*out) {
    std::fill(a.begin(), a.end(), 0.0);
    std::fill(ia.begin(), ia.end(), 0);
    std::fill(nia.begin(), nia.end(), 0);
    std::fill(d.begin(), d.end(), 0.0);
    std::fill(x.begin(), x.end(), 0.0);
    std::fill(dx.begin(), dx.end(), 0.0);
    std::fill(nx.begin(), nx.end(), 0);
    std::fill(sigma.begin(), sigma.end(), 0.0);
    std::fill(work.begin(), work.end(), 0.0);

    int nr = 0;
    const int nc = nplot_;
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const int ii = i - current_id_;
      if (!rec_[i].lonlat_flag) {
        const int k = nnz * nr;
        d[nr] = (rec_[i].*org - avg) / scale;
        nia[nr] = 1;
        ia[k] = ii;
        a[k] = 1.0;
        nr++;
      }
      if (weight_speed > 0.0 && ii > 0 && rec_[i].time_d > rec_[i - 1].time_d) {
        const double dtime_d = rec_[i].time_d - rec_[i - 1].time_d;
        const int k = nnz * nr;
        d[nr] = 0.0;
        nia[nr] = 2;
        ia[k] = ii - 1;
        a[k] = -weight_speed / dtime_d;
        ia[k + 1] = ii;
        a[k + 1] = weight_speed / dtime_d;
        nr++;
      }
      if (weight_acceleration > 0.0 && ii > 0 && ii < nplot_ - 1 && rec_[i + 1].time_d > rec_[i - 1].time_d) {
        const double dtime_d = rec_[i + 1].time_d - rec_[i - 1].time_d;
        const double dtime_d_sq = dtime_d * dtime_d;
        const int k = nnz * nr;
        d[nr] = 0.0;
        nia[nr] = 3;
        ia[k] = ii - 1;
        a[k] = weight_acceleration / dtime_d_sq;
        ia[k + 1] = ii;
        a[k + 1] = -2.0 * weight_acceleration / dtime_d_sq;
        ia[k + 2] = ii + 1;
        a[k + 2] = weight_acceleration / dtime_d_sq;
        nr++;
      }
    }

    int ncyc = 0, nsig = 0;
    double smax, sup, err;
    lspeig(a.data(), ia.data(), nia.data(), nnz, nc, nr, ncyc, &nsig, x.data(), dx.data(), sigma.data(), work.data(), &smax, &err, &sup);
    double supt = smax + err;
    if (sup > supt)
      supt = sup;
    ncyc = 16;
    for (int i = 0; i < 4; i++) {
      lspeig(a.data(), ia.data(), nia.data(), nnz, nc, nr, ncyc, &nsig, x.data(), dx.data(), sigma.data(), work.data(), &smax, &err, &sup);
      supt = smax + err;
      if (sup > supt)
        supt = sup;
    }
    const double slo = supt / bandwidth;
    chebyu(sigma.data(), ncycle, supt, slo, work.data());
    std::fill(x.begin(), x.end(), 0.0);
    lsqup(a.data(), ia.data(), nia.data(), nnz, nc, nr, x.data(), dx.data(), d.data(), 0, nullptr, nullptr, ncycle, sigma.data());

    for (int i = current_id_; i < current_id_ + nplot_; i++)
      rec_[i].*out = avg + scale * x[i - current_id_];
    /* the flagged ends stay flat */
    for (int i = current_id_; i < first; i++)
      rec_[i].*out = avg + scale * x[first - current_id_];
    for (int i = last + 1; i < current_id_ + nplot_; i++)
      rec_[i].*out = avg + scale * x[last - current_id_];
  };
  solve(&Record::lon_org, lon_avg, mtodeglon, &Record::lon_dr);
  solve(&Record::lat_org, lat_avg, mtodeglat, &Record::lat_dr);
}

}  // namespace mbne
