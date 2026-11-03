/*--------------------------------------------------------------------
 *    The MB-system:  mbe_core.cc  2/11/2026
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

#include "mbe_core.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>

#include "mb_aux.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"

namespace mbe {

namespace fs = std::filesystem;

namespace {

constexpr size_t ESF_HEADER_SIZE = MB_PATH_MAXLINE;
constexpr long UNDO_LIMIT = 20000000;  // beam changes kept, 16 bytes each

bool file_exists(const std::string &path) {
  struct stat file_status;
  return stat(path.c_str(), &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR;
}

long file_size(const std::string &path) {
  struct stat file_status;
  return stat(path.c_str(), &file_status) == 0 ? (long)file_status.st_size : -1;
}

void set_environment(const char *name, const char *value) {
#ifdef _WIN32
  _putenv_s(name, value != nullptr ? value : "");
#else
  if (value != nullptr)
    setenv(name, value, 1);
  else
    unsetenv(name);
#endif
}

bool history_on() {
  const char *history_env = getenv(MB_HISTORY_ENV);
  return history_env == nullptr || (strcmp(history_env, "off") != 0 && strcmp(history_env, "0") != 0);
}

/* no history snapshots while the temporary edit save file is opened: the
   history is about the real file, and is handled when that is replaced */
class HistoryOff {
 public:
  HistoryOff() {
    const char *value = getenv(MB_HISTORY_ENV);
    had_value_ = value != nullptr;
    if (had_value_)
      value_ = value;
    set_environment(MB_HISTORY_ENV, "off");
  }
  ~HistoryOff() { set_environment(MB_HISTORY_ENV, had_value_ ? value_.c_str() : nullptr); }

 private:
  bool had_value_ = false;
  std::string value_;
};

/* append a file to an open one, skipping the first bytes */
bool append_file(FILE *out, const std::string &path, size_t skip) {
  FILE *in = fopen(path.c_str(), "rb");
  if (in == nullptr)
    return false;
  if (skip > 0 && fseek(in, (long)skip, SEEK_SET) != 0) {
    fclose(in);
    return false;
  }
  bool ok = true;
  char buffer[65536];
  size_t nread;
  while ((nread = fread(buffer, 1, sizeof(buffer), in)) > 0) {
    if (fwrite(buffer, 1, nread, out) != nread) {
      ok = false;
      break;
    }
  }
  if (ferror(in))
    ok = false;
  fclose(in);
  return ok;
}

/* the size of the header of an edit save file, or of its stream */
size_t esf_header_size(const std::string &path) {
  char start[12];
  FILE *in = fopen(path.c_str(), "rb");
  if (in == nullptr)
    return 0;
  const size_t nread = fread(start, 1, sizeof(start), in);
  fclose(in);
  return nread == sizeof(start) && (strncmp(start, "ESFVERSION03", 12) == 0 || strncmp(start, "ESFVERSION02", 12) == 0)
             ? ESF_HEADER_SIZE
             : 0;
}

int esf_action(char flag) {
  if (mb_beam_ok(flag))
    return MBP_EDIT_UNFLAG;
  if (mb_beam_check_flag_filter2(flag))
    return MBP_EDIT_FILTER;
  if (mb_beam_check_flag_filter(flag))
    return MBP_EDIT_FILTER;
  if (!mb_beam_check_flag_unusable2(flag))
    return MBP_EDIT_FLAG;
  return MBP_EDIT_ZERO;
}

}  // namespace

/*--------------------------------------------------------------------*/
bool parse_input(const std::string &path, int form, std::vector<InputFile> &files, std::string &message) {
  mb_path file;
  if (path.size() >= sizeof(mb_path)) {
    message = "File name too long: " + path;
    return false;
  }
  snprintf(file, sizeof(file), "%s", path.c_str());

  int format = form;
  int error = MB_ERROR_NO_ERROR;
  if (form == 0) {
    mb_path root;
    if (mb_get_format(0, file, root, &format, &error) != MB_SUCCESS)
      format = 0;
  }
  if (format > 0) {
    files.push_back({path, format});
    return true;
  }
  if (format != -1) {
    message = "Unable to tell the MBIO format of " + path;
    return false;
  }

  void *datalist = nullptr;
  error = MB_ERROR_NO_ERROR;
  if (mb_datalist_open(0, &datalist, file, MB_DATALIST_LOOK_NO, &error) != MB_SUCCESS) {
    message = "Unable to open datalist " + path;
    return false;
  }
  int filestatus, fileformat;
  double weight;
  mb_path fileraw, fileprocessed, dfile;
  while (mb_datalist_read2(0, datalist, &filestatus, fileraw, fileprocessed, dfile, &fileformat, &weight, &error) == MB_SUCCESS)
    files.push_back({fileraw, fileformat});
  mb_datalist_close(0, &datalist, &error);
  return true;
}

/*--------------------------------------------------------------------*/
Session::Session() {
  memset(&esf_, 0, sizeof(esf_));
  int format, pings, lon;
  double bnd[4], smin, gap;
  int bt[7], et[7];
  mb_defaults(verbose, &format, &pings, &lon, bnd, bt, et, &smin, &gap);
  mb_uselockfiles(verbose, &uselockfiles_);
  text_width = [](const char *text) { return 7 * (int)strlen(text); };
  setCanvas(box_.width, box_.height);
}

Session::~Session() {
  if (file_open_)
    closeDiscard();
  clearReference();
}

void Session::setBuffer(int size, int hold) {
  buff_size_ = size;
  hold_size_ = hold;
}

void Session::error3(const std::string &a, const std::string &b, const std::string &c) {
  fprintf(stderr, "%s %s %s\n", a.c_str(), b.c_str(), c.c_str());
  if (on_error)
    on_error(a, b, c);
}

void Session::progress(const std::string &text) {
  if (on_progress)
    on_progress(text);
}

/* written after each action, so that a crash loses as little as possible */
void Session::streamFlush() {
  if (esf_.essfp != nullptr)
    fflush(esf_.essfp);
}

/*--------------------------------------------------------------------*/
std::string Session::esfName(const std::string &path) {
  mb_path file, esffile;
  if (path.size() >= sizeof(mb_path))
    return path + ".esf";
  snprintf(file, sizeof(file), "%s", path.c_str());
  int found = 0, error = MB_ERROR_NO_ERROR;
  mb_esf_check(0, file, esffile, &found, &error);
  return esffile;
}

bool Session::hasInterruptedSession(const std::string &path) { return file_exists(esfName(path) + ".new.stream"); }

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

/*--------------------------------------------------------------------*/
int Session::open(const std::string &path, int format, bool use_saved_edits, int output_mode, bool recover) {
  resetInfo();
  output_mode_ = output_mode;
  int status = openFile(path, format, use_saved_edits, recover);
  if (status == MB_SUCCESS) {
    if (hold_size_ > buff_size_)
      hold_size_ = buff_size_ / 2;
    int nloaded = 0;
    status = loadData(buff_size_, &nloaded);
    if (nloaded == 0)
      error3("No data were loaded from the input", "file. You may have specified an", "incorrect MB-System format id!");
    if (status == MB_SUCCESS && nbuff() > 0)
      status = plot(true);
  }
  beam_save_ = false;
  return status;
}

int Session::openFile(const std::string &path, int format, bool use_saved_edits, bool recover) {
  pings_before_ = 0;
  if (path.size() >= sizeof(mb_path)) {
    error3("Unable to open input file:", "the path is too long", "");
    return MB_FAILURE;
  }
  progress("Opening a data file...");
  snprintf(ifile_, sizeof(ifile_), "%s", path.c_str());
  format_ = format;

  int status = MB_SUCCESS;
  if (uselockfiles_) {
    status = mb_pr_lockswathfile(verbose, ifile_, MBP_LOCK_EDITBATHY, PROGRAM_NAME, &error_);
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
    else if (error_ == MB_ERROR_OPEN_FAIL) {
      error3("Unable to create lock file", "for intended input file:", "-Likely permissions issue");
    }
    file_open_ = false;
    return status;
  }

  if (mb_read_init(verbose, ifile_, format_, 1, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr_, &btime_d_, &etime_d_,
                   &beams_bath_, &beams_amp_, &pixels_ss_, &error_) != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, error_, &message);
    error3("Unable to open input file:", path, message != nullptr ? message : "");
    if (uselockfiles_) {
      int unlock_error = MB_ERROR_NO_ERROR;
      mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITBATHY, PROGRAM_NAME, &unlock_error);
    }
    file_open_ = false;
    return MB_FAILURE;
  }

  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&detect_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&priority_, &error_);
  if (error_ == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr_, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&pulses_, &error_);
  if (error_ != MB_ERROR_NO_ERROR) {
    int close_error = MB_ERROR_NO_ERROR;
    mb_close(verbose, &imbio_ptr_, &close_error);
    if (uselockfiles_)
      mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITBATHY, PROGRAM_NAME, &close_error);
    error3("Unable to allocate memory", "for the data arrays", "");
    file_open_ = false;
    return MB_FAILURE;
  }

  ping_.clear();
  clearUndo();
  pan_across_ = pan_depth_ = 0.0;
  zoomed_ = false;
  current_id_ = 0;
  nload_total_ = 0;
  ndump_total_ = 0;
  laid_first_ = laid_last_ = 0;
  dumped_changes_ = 0;
  replacing_ = false;
  saved_in_session_ = false;
  write_failed_ = false;
  esffile_open_ = false;
  esf_new_.clear();
  memset(&esf_, 0, sizeof(esf_));

  status = openEsf(use_saved_edits, recover);
  if (status == MB_FAILURE) {
    int close_error = MB_ERROR_NO_ERROR;
    mb_close(verbose, &imbio_ptr_, &close_error);
    if (esf_.edit != nullptr || esf_.esffp != nullptr || esf_.essfp != nullptr)
      mb_esf_close(verbose, &esf_, &close_error);
    if (uselockfiles_)
      mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITBATHY, PROGRAM_NAME, &close_error);
    file_open_ = false;
    fprintf(stderr, "\nERROR: Multibeam File <%s> NOT initialized for reading\n", ifile_);
    return status;
  }

  file_open_ = true;
  fprintf(stderr, "\nMultibeam File <%s> initialized for reading\n", ifile_);
  fprintf(stderr, "Multibeam Data Format ID: %d\n", format_);
  return MB_SUCCESS;
}

/* The edit save file itself is not opened for writing here, so that it stays
   as it was until the edits are saved. The new edits go to <esf>.new and
   <esf>.new.stream, which replace <esf> and <esf>.stream at the save. */
int Session::openEsf(bool use_saved_edits, bool recover) {
  int status = MB_SUCCESS;

  if (output_mode_ == OUTPUT_BROWSE) {
    if (use_saved_edits) {
      status = mb_esf_load(verbose, PROGRAM_NAME, ifile_, true, MBP_ESF_NOWRITE, esffile_, &esf_, &error_);
      /* no existing edits to load is not an error */
      if (status == MB_FAILURE && error_ == MB_ERROR_NO_DATA_LOADED) {
        status = MB_SUCCESS;
        error_ = MB_ERROR_NO_ERROR;
      }
    }
    return status;
  }

  int found = 0;
  mb_esf_check(verbose, ifile_, esffile_, &found, &error_);
  esf_new_ = std::string(esffile_) + ".new";
  if (esf_new_.size() + 8 >= sizeof(mb_path)) {
    error3("Unable to initialize", "the edit save file:", "the path is too long");
    return MB_FAILURE;
  }

  /* read the old edits, or after a crash the old edits and those of the interrupted session */
  if (recover) {
    const std::string merged = std::string(esffile_) + ".recover";
    const std::string stream = esf_new_ + ".stream";
    FILE *out = fopen(merged.c_str(), "wb");
    bool ok = out != nullptr;
    if (ok && use_saved_edits && file_exists(esffile_) && file_size(esffile_) >= (long)ESF_HEADER_SIZE) {
      ok = append_file(out, esffile_, 0);
      ok = ok && append_file(out, stream, esf_header_size(stream));
    }
    else if (ok) {
      ok = append_file(out, stream, 0);
    }
    if (out != nullptr && fclose(out) != 0)
      ok = false;
    if (ok) {
      mb_path name;
      snprintf(name, sizeof(name), "%s", merged.c_str());
      status = mb_esf_open(verbose, PROGRAM_NAME, name, true, MBP_ESF_NOWRITE, &esf_, &error_);
      snprintf(esf_.esffile, sizeof(esf_.esffile), "%s", esffile_);
    }
    else {
      status = MB_FAILURE;
      error_ = MB_ERROR_OPEN_FAIL;
    }
    std::remove(merged.c_str());
    replacing_ = true;
  }
  else {
    status = mb_esf_open(verbose, PROGRAM_NAME, esffile_, use_saved_edits, MBP_ESF_NOWRITE, &esf_, &error_);
    if (!use_saved_edits && file_size(esffile_) > (long)ESF_HEADER_SIZE)
      replacing_ = true;
  }

  /* open the new files; the library writes their headers */
  if (status == MB_SUCCESS) {
    mb_path name;
    snprintf(name, sizeof(name), "%s", esf_new_.c_str());
    struct mb_esf_struct out;
    memset(&out, 0, sizeof(out));
    {
      HistoryOff history_off;
      status = mb_esf_open(verbose, PROGRAM_NAME, name, false, MBP_ESF_WRITE, &out, &error_);
    }
    if (status == MB_SUCCESS) {
      esf_.esffp = out.esffp;
      esf_.essfp = out.essfp;
      fflush(esf_.esffp);
      fflush(esf_.essfp);
      esffile_open_ = true;
    }
  }

  if (status == MB_FAILURE && error_ == MB_ERROR_OPEN_FAIL) {
    error3("Unable to open new edit save file.", "You may not have write", "permission in this directory!");
  }
  else if (status == MB_FAILURE && error_ == MB_ERROR_MEMORY_FAIL) {
    error3("Unable to allocate memory for.", "edits in existing edit", "save file!");
  }
  else if (status == MB_FAILURE) {
    error3("Unable to initialize", "the edit save file", "for this swath file!");
  }
  return status;
}

/*--------------------------------------------------------------------*/
/* Replace the edit save file, keeping its old content in the history. */
bool Session::commitEsf(const std::string &source) {
  int history_error = MB_ERROR_NO_ERROR;
  int history_version = 0;
  if (history_on()) {
    if (file_exists(esffile_)) {
      if (mb_history_snapshot(verbose, esffile_, PROGRAM_NAME, nullptr, MB_HISTORY_MODE_DEFAULT, &history_version, &history_error) !=
          MB_SUCCESS)
        fprintf(stderr, "\nWarning: unable to save a history snapshot of edit save file %s\n", esffile_);
    }
    else {
      /* a new edit save file: the first history version holds no edits, and records when it was created */
      FILE *in = fopen(source.c_str(), "rb");
      FILE *out = fopen(esffile_, "wb");
      if (in != nullptr && out != nullptr) {
        char header[ESF_HEADER_SIZE];
        if (fread(header, ESF_HEADER_SIZE, 1, in) == 1)
          fwrite(header, ESF_HEADER_SIZE, 1, out);
      }
      if (in != nullptr)
        fclose(in);
      if (out != nullptr)
        fclose(out);
      if (mb_history_snapshot(verbose, esffile_, PROGRAM_NAME, MB_HISTORY_NOTE_CREATED, MB_HISTORY_MODE_DEFAULT, &history_version,
                              &history_error) != MB_SUCCESS)
        fprintf(stderr, "\nWarning: unable to save a history snapshot of edit save file %s\n", esffile_);
    }
  }
  std::error_code code;
  fs::rename(source, esffile_, code);
  if (code) {
    error3("Unable to replace the edit save file", esffile_, "The edits remain in " + source);
    return false;
  }
  return true;
}

bool Session::writeEdits(FILE *fp) const {
  bool ok = true;
  struct mb_esf_struct out = esf_;
  out.esffp = fp;
  out.byteswapped = mb_swap_check();
  int error = MB_ERROR_NO_ERROR;
  for (const Ping &p : ping_) {
    for (int j = 0; j < p.beams_bath; j++) {
      if (p.beamflag[j] != p.beamflagorg[j] &&
          mb_esf_save(verbose, &out, p.time_d, j + p.multiplicity * MB_ESF_MULTIPLICITY_FACTOR, esf_action(p.beamflag[j]), &error) != MB_SUCCESS)
        ok = false;
    }
  }
  for (int i = 0; i < esf_.nedit; i++) {
    if (esf_.edit[i].use == 0 && mb_esf_save(verbose, &out, esf_.edit[i].time_d, esf_.edit[i].beam, esf_.edit[i].action, &error) != MB_SUCCESS)
      ok = false;
  }
  return ok;
}

/* the edits as they are now, for another program to start from */
bool Session::writeCurrentEdits(const std::string &path) const {
  if (!file_open_)
    return false;
  if (esf_.esffp != nullptr)
    fflush(esf_.esffp);
  FILE *fp = fopen(path.c_str(), "wb");
  if (fp == nullptr)
    return false;
  bool ok = true;
  if (!esf_new_.empty() && file_exists(esf_new_)) {
    ok = append_file(fp, esf_new_, 0);
  }
  else {
    char user[256] = "", host[256] = "", date[32] = "";
    int error = MB_ERROR_NO_ERROR;
    mb_user_host_date(0, user, host, date, &error);
    char header[ESF_HEADER_SIZE];
    memset(header, 0, sizeof(header));
    snprintf(header, sizeof(header), "ESFVERSION03\nESF Mode: %d\nMB-System Version %s\nProgram: %s\nUser: %s\nCPU: %s\nDate: %s\n", MB_ESF_MODE_EXPLICIT,
             MB_VERSION, PROGRAM_NAME, user, host, date);
    ok = fwrite(header, ESF_HEADER_SIZE, 1, fp) == 1;
  }
  ok = ok && writeEdits(fp);
  if (fclose(fp) != 0)
    ok = false;
  return ok;
}

int Session::save() {
  if (!file_open_ || output_mode_ != OUTPUT_EDIT || esf_.esffp == nullptr)
    return MB_FAILURE;
  fflush(esf_.esffp);
  streamFlush();

  /* the edits of the pings already dumped, those of the buffer, and the old edits of the pings not read yet */
  const std::string saving = std::string(esffile_) + ".save";
  FILE *fp = fopen(saving.c_str(), "wb");
  bool ok = fp != nullptr && append_file(fp, esf_new_, 0);
  if (ok)
    ok = writeEdits(fp);
  if (fp != nullptr && fclose(fp) != 0)
    ok = false;
  if (!ok || write_failed_) {
    error3("Unable to write the edit save file", saving, "The edits have not been saved");
    std::remove(saving.c_str());
    return MB_FAILURE;
  }
  if (!commitEsf(saving))
    return MB_FAILURE;

  for (Ping &p : ping_)
    p.beamflagsession = p.beamflag;
  dumped_changes_ = 0;
  replacing_ = false;
  saved_in_session_ = true;
  int error = MB_ERROR_NO_ERROR;
  mb_pr_update_format(verbose, ifile_, true, format_, &error);
  mb_pr_update_edit(verbose, ifile_, MBP_EDIT_ON, esffile_, &error);
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::closeFile(bool commit) {
  progress("Closing a data file...");
  clearUndo();
  ping_.clear();
  ping_.shrink_to_fit();
  laid_first_ = laid_last_ = 0;

  int status = mb_close(verbose, &imbio_ptr_, &error_);
  beamflag_ = nullptr;
  bath_ = amp_ = bathacrosstrack_ = bathalongtrack_ = ss_ = ssacrosstrack_ = ssalongtrack_ = nullptr;
  detect_ = priority_ = pulses_ = nullptr;

  bool saved = false;
  bool write_ok = !write_failed_;
  if (esf_.esffp != nullptr && fflush(esf_.esffp) != 0)
    write_ok = false;
  if (esf_.edit != nullptr || esf_.esffp != nullptr || esf_.essfp != nullptr)
    mb_esf_close(verbose, &esf_, &error_);

  if (output_mode_ == OUTPUT_EDIT && !esf_new_.empty()) {
    const std::string stream = esf_new_ + ".stream";
    if (commit && write_ok) {
      saved = commitEsf(esf_new_);
      if (saved) {
        std::error_code code;
        fs::rename(stream, std::string(esffile_) + ".stream", code);
        mb_pr_update_format(verbose, ifile_, true, format_, &error_);
        mb_pr_update_edit(verbose, ifile_, MBP_EDIT_ON, esffile_, &error_);
      }
      else {
        status = MB_FAILURE;
      }
    }
    else if (commit) {
      error3("Unable to write the edit save file", esf_new_, "The earlier edit save file is unchanged");
      status = MB_FAILURE;
    }
    else {
      std::remove(esf_new_.c_str());
      std::remove(stream.c_str());
    }
  }

  if (uselockfiles_) {
    int unlock_error = MB_ERROR_NO_ERROR;
    mb_pr_unlockswathfile(verbose, ifile_, MBP_LOCK_EDITBATHY, PROGRAM_NAME, &unlock_error);
  }

  fprintf(stderr, "\nMultibeam Input File <%s> closed\n", ifile_);
  fprintf(stderr, "%d data records loaded\n", nload_total_);
  fprintf(stderr, "%d data records dumped\n", ndump_total_);
  file_open_ = false;
  esffile_open_ = false;
  nload_total_ = 0;
  ndump_total_ = 0;
  if ((saved || saved_in_session_) && on_saved)
    on_saved(ifile_);
  return status == MB_SUCCESS ? MB_SUCCESS : MB_FAILURE;
}

long Session::changes() const {
  long count = dumped_changes_;
  for (const Ping &p : ping_) {
    for (int j = 0; j < p.beams_bath; j++) {
      if (p.beamflag[j] != p.beamflagsession[j])
        count++;
    }
  }
  return count;
}

bool Session::unsaved() const { return file_open_ && output_mode_ == OUTPUT_EDIT && (replacing_ || changes() > 0); }

int Session::closeSave() {
  resetInfo();
  int status = MB_SUCCESS;
  int ndumped = 0, nloaded = 0;
  if (!file_open_) {
    status = MB_FAILURE;
  }
  else if (output_mode_ == OUTPUT_EDIT && !compatibility && !unsaved()) {
    /* nothing to save: the edit save file stays as it was */
    status = closeFile(false);
  }
  else if (output_mode_ == OUTPUT_BROWSE || esf_.nedit == 0) {
    dumpData(0, &ndumped);
    status = closeFile(true);
  }
  else {
    /* the old edits of the pings not read yet must reach the new file */
    do {
      dumpData(0, &ndumped);
      loadData(buff_size_, &nloaded);
    } while (nloaded > 0);
    status = closeFile(true);
  }
  beam_save_ = false;
  return status;
}

int Session::closeDiscard() {
  resetInfo();
  if (!file_open_)
    return MB_FAILURE;
  const int status = closeFile(false);
  beam_save_ = false;
  return status;
}

int Session::done(bool *quit) {
  resetInfo();
  *quit = gui_mode;
  return file_open_ ? closeSave() : MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::nextBuffer(bool *quit) {
  resetInfo();
  *quit = false;
  int status = MB_SUCCESS;
  if (file_open_) {
    int ndumped = 0, nloaded = 0;
    do {
      dumpData(hold_size_, &ndumped);
      status = loadData(buff_size_, &nloaded);
    } while (nloaded > 0 && nbuff() == 0);

    if (nloaded <= 0) {
      /* the end of the file: write the last pings and close */
      dumpData(0, &ndumped);
      status = (compatibility || unsaved()) ? closeFile(true) : closeFile(false);
      *quit = gui_mode;
    }
    else {
      status = plot(true);
    }
  }
  else {
    status = MB_FAILURE;
    current_id_ = 0;
  }
  beam_save_ = false;
  return status;
}

int Session::gotoTime(int ttime_i[7]) {
  double ttime_d;
  mb_get_time(verbose, ttime_i, &ttime_d);
  resetInfo();

  bool found = false;
  int status = MB_SUCCESS;
  if (!file_open_) {
    status = MB_FAILURE;
    current_id_ = 0;
    error3("No data file has", "been opened...", "  ");
  }
  else if (nbuff() > 0) {
    if (ping_[0].time_d > ttime_d) {
      status = MB_FAILURE;
      error3("Beginning of loaded data", "is later than the", "specified target time...");
    }
    else if (ping_[nbuff() - 1].time_d < ttime_d && nbuff() < buff_size_) {
      status = MB_FAILURE;
      error3("Target time is", "beyond the end", "of the data file...");
    }
  }

  int ndumped = 0, nloaded = 0;
  while (!found && status == MB_SUCCESS) {
    for (int i = 0; i < nbuff(); i++) {
      if (ping_[i].time_d > ttime_d && !found) {
        found = true;
        current_id_ = i;
      }
    }

    if (!found && nbuff() >= buff_size_) {
      dumpData(hold_size_, &ndumped);
      status = loadData(buff_size_, &nloaded);
      if (status == MB_FAILURE) {
        dumpData(0, &ndumped);
        if (compatibility || unsaved())
          closeFile(true);
        else
          closeFile(false);
        status = MB_FAILURE;
        error3("Target time is beyond the", "end of the data file!", "The file has been closed...");
      }
    }
    else if (!found && nbuff() < buff_size_) {
      status = MB_FAILURE;
      error3("Target time is", "beyond the end", "of the data file...");
    }
  }

  status = MB_SUCCESS;
  if (nbuff() > 0)
    status = plot(false);
  beam_save_ = false;
  return status;
}

/*--------------------------------------------------------------------*/
int Session::dumpData(int hold_size, int *ndumped) {
  const int nb = nbuff();
  hold_size = std::max(0, std::min(hold_size, nb));
  int ndump = 0;
  if (nb > 0) {
    progress("Clearing data...");
    for (int iping = 0; iping < nb - hold_size; iping++) {
      const Ping &p = ping_[iping];
      for (int jbeam = 0; jbeam < p.beams_bath; jbeam++) {
        if (p.beamflag[jbeam] != p.beamflagorg[jbeam]) {
          if (mb_esf_save(verbose, &esf_, p.time_d, jbeam + p.multiplicity * MB_ESF_MULTIPLICITY_FACTOR, esf_action(p.beamflag[jbeam]),
                          &error_) != MB_SUCCESS)
            write_failed_ = true;
        }
        if (p.beamflag[jbeam] != p.beamflagsession[jbeam])
          dumped_changes_++;
      }
    }
    for (int i = laid_first_; i < std::min(laid_last_, nb); i++) {
      ping_[i].bath_x.clear();
      ping_[i].bath_x.shrink_to_fit();
      ping_[i].bath_y.clear();
      ping_[i].bath_y.shrink_to_fit();
    }
    laid_first_ = laid_last_ = 0;
    if (nb - hold_size > 0)
      clearUndo();
    ping_.erase(ping_.begin(), ping_.begin() + (nb - hold_size));
    ndump = nb - hold_size;
  }
  pings_before_ += ndump;
  *ndumped = ndump;
  ndump_total_ += ndump;

  if (ndump > 0)
    current_id_ -= ndump;
  if (current_id_ < 0)
    current_id_ = 0;
  if (current_id_ > nbuff() - 1)
    current_id_ = nbuff() - 1;
  return MB_SUCCESS;
}

int Session::loadData(int buffer_size, int *nloaded) {
  int status = MB_SUCCESS;
  int nload = 0;
  int namp, nss, kind = MB_DATA_NONE;
  int sensorhead = 0, sensorhead_error = MB_ERROR_NO_ERROR;
  int detect_error = MB_ERROR_NO_ERROR;
  double distance, draft, speed_nav;
  char comment[MB_COMMENT_MAXLINE];
  Ping p;

  progress("0 records loaded so far...");
  do {
    error_ = MB_ERROR_NO_ERROR;
    status = mb_get_all(verbose, imbio_ptr_, &store_ptr_, &kind, p.time_i, &p.time_d, &p.navlon, &p.navlat, &p.speed, &p.heading, &distance,
                        &p.altitude, &p.sensordepth, &p.beams_bath, &namp, &nss, beamflag_, bath_, amp_, bathacrosstrack_, bathalongtrack_,
                        ss_, ssacrosstrack_, ssalongtrack_, comment, &error_);
    if (error_ <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
      const int nb = nbuff();
      p.time_interval = nb > 0 ? p.time_d - ping_[nb - 1].time_d : 0.0;
      status = mb_extract_nav(verbose, imbio_ptr_, store_ptr_, &kind, p.time_i, &p.time_d, &p.navlon, &p.navlat, &speed_nav, &p.heading,
                              &draft, &p.roll, &p.pitch, &p.heave, &error_);

      /* ping multiplicity */
      if (mb_sensorhead(verbose, imbio_ptr_, store_ptr_, &sensorhead, &sensorhead_error) == MB_SUCCESS)
        p.multiplicity = sensorhead;
      else if (nb > 0 && fabs(p.time_d - ping_[nb - 1].time_d) < MB_ESF_MAXTIMEDIFF)
        p.multiplicity = ping_[nb - 1].multiplicity + 1;
      else
        p.multiplicity = 0;
      p.distance = nb == 0 ? 0.0 : ping_[nb - 1].distance + p.speed * p.time_interval / 3.6;

      int nbeams = p.beams_bath;
      if (mb_detects(verbose, imbio_ptr_, store_ptr_, &kind, &nbeams, detect_, &detect_error) != MB_SUCCESS) {
        status = MB_SUCCESS;
        for (int i = 0; i < p.beams_bath; i++) {
          detect_[i] = MB_DETECT_UNKNOWN;
          priority_[i] = 0;
        }
      }
      else {
        for (int i = 0; i < p.beams_bath; i++) {
          priority_[i] = (detect_[i] & 0x0000FF00) >> 8;
          detect_[i] = detect_[i] & 0x000000FF;
        }
      }
      if (mb_pulses(verbose, imbio_ptr_, store_ptr_, &kind, &nbeams, pulses_, &detect_error) != MB_SUCCESS) {
        status = MB_SUCCESS;
        for (int i = 0; i < p.beams_bath; i++)
          pulses_[i] = MB_PULSE_UNKNOWN;
      }
    }
    if (error_ <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA &&
        (error_ == MB_ERROR_NO_ERROR || error_ == MB_ERROR_TIME_GAP || error_ == MB_ERROR_OUT_BOUNDS || error_ == MB_ERROR_OUT_TIME ||
         error_ == MB_ERROR_SPEED_TOO_SMALL)) {
      status = MB_SUCCESS;
      error_ = MB_ERROR_NO_ERROR;
    }
    else if (error_ <= MB_ERROR_NO_ERROR) {
      status = MB_FAILURE;
      error_ = MB_ERROR_OTHER;
    }

    if (status == MB_SUCCESS) {
      const int n = p.beams_bath;
      p.beamflag.assign(beamflag_, beamflag_ + n);
      p.beamflagorg = p.beamflag;
      p.beamflagsession = p.beamflag;
      p.bath.assign(bath_, bath_ + n);
      p.amp.assign(amp_, amp_ + n);
      p.bathacrosstrack.assign(bathacrosstrack_, bathacrosstrack_ + n);
      p.bathalongtrack.assign(bathalongtrack_, bathalongtrack_ + n);
      p.detect.resize(n);
      p.priority.resize(n);
      p.pulses.resize(n);
      for (int i = 0; i < n; i++) {
        p.detect[i] = (unsigned char)detect_[i];
        p.priority[i] = (unsigned char)priority_[i];
        p.pulses[i] = (signed char)pulses_[i];
      }
      p.has_amp = n > 0 && namp == n;
      p.record = nbuff() + ndump_total_;

      /* a dual profile, as in Kongsberg dual swath: the acrosstrack distance jumps back within the ping */
      double xtrack_min = 0.0, xtrack_max = 0.0;
      bool first = true;
      for (int i = 0; i < n; i++) {
        if (mb_beam_ok(p.beamflag[i])) {
          xtrack_min = first ? p.bathacrosstrack[i] : std::min(xtrack_min, p.bathacrosstrack[i]);
          xtrack_max = first ? p.bathacrosstrack[i] : std::max(xtrack_max, p.bathacrosstrack[i]);
          first = false;
        }
      }
      if (first) {
        for (int i = 0; i < n; i++) {
          if (!mb_beam_ok(p.beamflag[i]) && !mb_beam_check_flag_unusable2(p.beamflag[i])) {
            xtrack_min = first ? p.bathacrosstrack[i] : std::min(xtrack_min, p.bathacrosstrack[i]);
            xtrack_max = first ? p.bathacrosstrack[i] : std::max(xtrack_max, p.bathacrosstrack[i]);
            first = false;
          }
        }
      }
      const double xtrack_absmax = std::max(fabs(xtrack_min), fabs(xtrack_max));
      p.dualprofile = false;
      p.dualprofilebeam = 0;
      first = true;
      double xtrack_old = 0.0;
      for (int i = 0; i < n; i++) {
        if (!mb_beam_check_flag_unusable2(p.beamflag[i])) {
          if (first) {
            xtrack_old = p.bathacrosstrack[i];
            first = false;
          }
          else {
            const double xtrack_diff = p.bathacrosstrack[i] - xtrack_old;
            if (xtrack_diff < 0.0 && fabs(xtrack_diff) > 0.5 * xtrack_absmax) {
              p.dualprofile = true;
              p.dualprofilebeam = i;
            }
            xtrack_old = p.bathacrosstrack[i];
          }
        }
      }

      ping_.push_back(p);
      nload++;
      if (nload % 250 == 0)
        progress(std::to_string(nload) + " records loaded so far...");
    }
  } while (error_ <= MB_ERROR_NO_ERROR && nbuff() < buffer_size);
  *nloaded = nload;
  nload_total_ += nload;

  if (nload > 0) {
    status = MB_SUCCESS;
    error_ = MB_ERROR_NO_ERROR;
  }
  else {
    status = MB_FAILURE;
    error_ = MB_ERROR_EOF;
  }

  current_id_ = 0;

  /* The old code went over the whole buffer here, which undid edits made in
     the pings retained from the previous buffer. */
  const int first_new = compatibility ? 0 : nbuff() - nload;

  /* apply the saved edits */
  if (esf_.nedit > 0) {
    progress("Applying saved edits...");
    for (int i = first_new; i < nbuff(); i++) {
      Ping &q = ping_[i];
      status = mb_esf_apply(verbose, &esf_, q.time_d, q.multiplicity, q.beams_bath, q.beamflag.data(), &error_);

      /* beams flagged by the sonar in the data file stay marked as such */
      for (int j = 0; j < q.beams_bath; j++) {
        if (!mb_beam_ok(q.beamflag[j]) && mb_beam_check_flag_sonar(q.beamflagorg[j]))
          q.beamflag[j] = mb_beam_set_flag_sonar(q.beamflag[j]);
      }
      if (i >= nbuff() - nload)
        q.beamflagsession = q.beamflag;
      if (i % 250 == 0)
        progress("Saved edits applied to " + std::to_string(i) + " of " + std::to_string(nbuff() - 1) + " records so far...");
    }
  }

  /* apply the filters */
  if (filters.any()) {
    progress("Applying bathymetry filters...");
    for (int i = first_new; i < nbuff(); i++) {
      filterPing(i);
      if (i % 250 == 0)
        progress("Filters applied to " + std::to_string(i) + " of " + std::to_string(nbuff() - 1) + " records so far...");
    }
  }
  pending_.clear();
  streamFlush();

  fprintf(stderr, "\n%d data records loaded from input file <%s>\n", nload, ifile_);
  fprintf(stderr, "%d data records now in buffer\n", nbuff());
  return status;
}

/*--------------------------------------------------------------------*/
void Session::setCanvas(int width, int height) {
  classic_layout_ = true;
  box_.width = width;
  box_.height = height;
  setPlotMode(settings.show_time);
}

void Session::setPlotRect(int width, int height, int xmin, int xmax, int ymin, int ymax) {
  classic_layout_ = false;
  box_.width = width;
  box_.height = height;
  box_.margin = ymin;
  box_.xmin = xmin;
  box_.xmax = std::max(xmax, xmin + 1);
  box_.ymin = ymin;
  box_.ymax = std::max(ymax, ymin + 1);
}

void Session::setPlotMode(int show_time) {
  settings.show_time = show_time;
  if (!classic_layout_)
    return;
  box_.margin = box_.width / 16;
  box_.xmin = show_time > PLOT_WIDE ? 5 * box_.margin : 2 * box_.margin + 20;
  box_.xmax = box_.width - box_.margin;
  box_.ymin = box_.margin;
  box_.ymax = box_.height - box_.margin / 2;
}

int Session::plot(bool autoscale) {
  nplot_ = current_id_ + settings.plot_size > nbuff() ? nbuff() - current_id_ : settings.plot_size;

  /* the median depth and the width of the data */
  bathlist_.clear();
  double xtrack_min = 0.0, xtrack_max = 0.0;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    Ping &p = ping_[i];
    p.record = i + ndump_total_;
    p.outbounds = OUTBOUNDS_NONE;
    for (int j = 0; j < p.beams_bath; j++) {
      if (mb_beam_ok(p.beamflag[j])) {
        xtrack_min = bathlist_.empty() ? p.bathacrosstrack[j] : std::min(xtrack_min, p.bathacrosstrack[j]);
        xtrack_max = bathlist_.empty() ? p.bathacrosstrack[j] : std::max(xtrack_max, p.bathacrosstrack[j]);
        bathlist_.push_back(p.bath[j]);
      }
    }
  }

  /* if there is not enough in the unflagged bathymetry look into the flagged bathymetry */
  if (bathlist_.empty() || xtrack_max <= 0.0) {
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      for (int j = 0; j < p.beams_bath; j++) {
        if (!mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j])) {
          xtrack_min = bathlist_.empty() ? p.bathacrosstrack[j] : std::min(xtrack_min, p.bathacrosstrack[j]);
          xtrack_max = bathlist_.empty() ? p.bathacrosstrack[j] : std::max(xtrack_max, p.bathacrosstrack[j]);
          bathlist_.push_back(p.bath[j]);
        }
      }
    }
  }
  double xtrack_absmax = std::max(fabs(xtrack_min), fabs(xtrack_max));
  bathmedian_ = 0.0;
  if (!bathlist_.empty()) {
    qsort(bathlist_.data(), bathlist_.size(), sizeof(double), mb_double_compare);
    bathmedian_ = bathlist_[bathlist_.size() / 2];
  }

  if (autoscale && xtrack_absmax < 0.5)
    xtrack_absmax = 1000.0;
  else if (autoscale && xtrack_absmax > 100000.0)
    xtrack_absmax = 100000.0;

  /* reset the plot width and the ticks */
  if (autoscale) {
    pan_across_ = pan_depth_ = 0.0;
    zoomed_ = false;
  }
  if (autoscale && xtrack_absmax > 0.0) {
    settings.plot_width = (int)(2.4 * xtrack_absmax);
    if (!compatibility)
      settings.plot_width = std::min(settings.plot_width, MAX_PLOT_WIDTH);
    int xntrvl = settings.plot_width / 10;
    if (xntrvl > 1000)
      xntrvl = 1000 * (xntrvl / 1000);
    else if (xntrvl > 500)
      xntrvl = 500 * (xntrvl / 500);
    else if (xntrvl > 250)
      xntrvl = 250 * (xntrvl / 250);
    else if (xntrvl > 100)
      xntrvl = 100 * (xntrvl / 100);
    else if (xntrvl > 50)
      xntrvl = 50 * (xntrvl / 50);
    else if (xntrvl > 25)
      xntrvl = 25 * (xntrvl / 25);
    else if (xntrvl > 10)
      xntrvl = 10 * (xntrvl / 10);
    else if (xntrvl > 5)
      xntrvl = 5 * (xntrvl / 5);
    else if (xntrvl > 2)
      xntrvl = 2 * (xntrvl / 2);
    else
      xntrvl = 1;
    settings.x_interval = xntrvl;
    settings.y_interval = xntrvl;
  }

  /* scaling */
  const int xmin = box_.xmin, xmax = box_.xmax, ymin = box_.ymin, ymax = box_.ymax;
  const int xcen = xmin + (xmax - xmin) / 2;
  const int ycen = ymin + (ymax - ymin) / 2;
  const double dy = ((double)(ymax - ymin)) / settings.plot_size;
  const double xscale = 100.0 * settings.plot_width / (xmax - xmin);
  const double yscale = (xscale * 100.0) / settings.exagger;
  const double dxscale = 100.0 / xscale;
  const double dyscale = 100.0 / yscale;

  /* pixel positions are kept only for the pings in view */
  for (int i = laid_first_; i < std::min(laid_last_, nbuff()); i++) {
    if (i < current_id_ || i >= current_id_ + nplot_) {
      ping_[i].bath_x.clear();
      ping_[i].bath_x.shrink_to_fit();
      ping_[i].bath_y.clear();
      ping_[i].bath_y.shrink_to_fit();
    }
  }
  laid_first_ = current_id_;
  laid_last_ = current_id_ + nplot_;

  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    Ping &p = ping_[i];
    const int y = ymax - (int)(dy / 2) - (int)((i - current_id_) * dy);
    p.label_x = xmin - 5;
    p.label_y = y;
    p.bath_x.resize(p.beams_bath);
    p.bath_y.resize(p.beams_bath);
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_check_flag_unusable2(p.beamflag[j])) {
        if (view_mode_ == VIEW_WATERFALL && p.dualprofile) {
          p.bath_x[j] = (int)(xcen + dxscale * (p.bathacrosstrack[j] - pan_across_));
          p.bath_y[j] = (int)(y + dyscale * (p.bath[j] - bathmedian_ - pan_depth_));
          if (j < p.dualprofilebeam)
            p.bath_y[j] -= (int)(dy / 4);
          else
            p.bath_y[j] += (int)(dy / 4);
        }
        else if (view_mode_ == VIEW_WATERFALL) {
          p.bath_x[j] = (int)(xcen + dxscale * (p.bathacrosstrack[j] - pan_across_));
          p.bath_y[j] = (int)(y + dyscale * (p.bath[j] - bathmedian_ - pan_depth_));
        }
        else if (view_mode_ == VIEW_ALONGTRACK) {
          p.bath_x[j] = (int)(xcen + dxscale * (p.bathacrosstrack[j] - pan_across_));
          p.bath_y[j] = (int)(ycen + dyscale * (p.bath[j] - bathmedian_ - pan_depth_));
        }
        else {
          p.bath_x[j] = (int)(xcen + dxscale * (p.bathalongtrack[j] + p.distance - ping_[current_id_ + nplot_ / 2].distance - pan_across_));
          p.bath_y[j] = (int)(ycen + dyscale * (p.bath[j] - bathmedian_ - pan_depth_));
        }
      }
      else {
        p.bath_x[j] = 0;
        p.bath_y[j] = 0;
      }
    }
    layoutPingLabel(i, true);
  }

  return nplot_ > 0 ? MB_SUCCESS : MB_FAILURE;
}

int Session::replot() {
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

void Session::layoutPingLabel(int iping, bool save) {
  Ping &p = ping_[iping];

  /* the last picked ping may have left the view; its label is set again when it returns */
  if ((int)p.bath_x.size() < p.beams_bath)
    return;

  /* are there soundings outside the plot */
  p.outbounds = OUTBOUNDS_NONE;
  for (int j = 0; !zoomed_ && j < p.beams_bath; j++) {
    if (!mb_beam_check_flag_unusable2(p.beamflag[j]) && outsideBox(iping, j)) {
      if (mb_beam_ok(p.beamflag[j]))
        p.outbounds = OUTBOUNDS_UNFLAGGED;
      else if (p.outbounds != OUTBOUNDS_UNFLAGGED)
        p.outbounds = OUTBOUNDS_FLAGGED;
    }
  }

  char string[MB_PATH_MAXLINE];
  if (settings.show_time == PLOT_TIME || save) {
    double depth = 0.0;
    if (p.beams_bath > 0 && mb_beam_ok(p.beamflag[p.beams_bath / 2]))
      depth = p.bath[p.beams_bath / 2];
    else if (p.beams_bath > 0)
      depth = p.altitude + p.sensordepth;
    snprintf(string, sizeof(string), "%5d %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d %10.3f", p.record + 1, p.time_i[1], p.time_i[2],
             p.time_i[0], p.time_i[3], p.time_i[4], p.time_i[5], (int)(0.001 * p.time_i[6]), depth);
    if (save)
      last_ping_ = string;
  }
  if (settings.show_time != PLOT_TIME) {
    snprintf(string, sizeof(string), "%5d %10.3f", compatibility ? p.record : p.record + 1, centerDepth(p));
    if (save)
      last_ping_ = string;
  }
  p.label = string;
  p.label_width = text_width(string);

  if (p.outbounds == OUTBOUNDS_UNFLAGGED) {
    p.zap_x1 = p.label_x - p.label_width - 20;
    p.zap_x2 = p.zap_x1 + 10;
    p.zap_y1 = p.label_y - text_ascent;
    p.zap_y2 = p.zap_y1 + text_ascent + text_descent;
  }
}

/* The center beam may be null, as is usual in dual swath data, where it is the
   edge of the second swath. Its depth is then 0, which means nothing. */
double Session::centerDepth(const Ping &p) const {
  if (p.beams_bath <= 0)
    return 0.0;
  if (compatibility || mb_beam_ok(p.beamflag[p.beams_bath / 2]))
    return p.bath[p.beams_bath / 2];
  return p.altitude + p.sensordepth;
}

bool Session::outsideBox(int i, int j) const {
  const Ping &p = ping_[i];
  return p.bath_x[j] < box_.xmin || p.bath_x[j] > box_.xmax || p.bath_y[j] < box_.ymin || p.bath_y[j] > box_.ymax;
}

int Session::step(int nsteps) {
  resetInfo();
  int status = MB_SUCCESS;
  if (file_open_ && nbuff() > 0) {
    const int old_id = current_id_;
    int new_id = current_id_ + nsteps;
    if (new_id < 0)
      new_id = 0;
    if (new_id >= nbuff())
      new_id = nbuff() - 1;
    current_id_ = new_id;
    status = plot(false);
    if (new_id == old_id)
      status = MB_FAILURE;
  }
  else {
    status = MB_FAILURE;
    current_id_ = 0;
  }
  beam_save_ = false;
  return status;
}

/*--------------------------------------------------------------------*/
void Session::finishAction(const char *label) {
  streamFlush();
  if (!stroke_open_ && !pending_.empty()) {
    undo_total_ += (long)pending_.size();
    undo_.push_back({label, std::move(pending_)});
    pending_.clear();
    redo_.clear();
    /* a limit on the memory: the oldest steps go first */
    while (undo_total_ > UNDO_LIMIT && undo_.size() > 1) {
      undo_total_ -= (long)undo_.front().changes.size();
      undo_.erase(undo_.begin());
    }
  }
}

void Session::beginStroke() { stroke_open_ = true; }

void Session::endStroke(const char *label) {
  stroke_open_ = false;
  finishAction(label);
}

void Session::clearUndo() {
  undo_.clear();
  redo_.clear();
  pending_.clear();
  undo_total_ = 0;
  stroke_open_ = false;
}

const std::string &Session::undoLabel() const {
  static const std::string none;
  return undo_.empty() ? none : undo_.back().label;
}

const std::string &Session::redoLabel() const {
  static const std::string none;
  return redo_.empty() ? none : redo_.back().label;
}

/* Only flags in memory and the stream change: the edit save file is written later from the flags. */
int Session::applyStep(const UndoStep &step, bool forward) {
  resetInfo();
  const int count = (int)step.changes.size();
  for (int k = 0; k < count; k++) {
    const BeamChange &change = step.changes[forward ? k : count - 1 - k];
    Ping &p = ping_[change.ping];
    const char flag = forward ? change.after : change.before;
    if (esffile_open_)
      mb_ess_save(verbose, &esf_, p.time_d, change.beam + p.multiplicity * MB_ESF_MULTIPLICITY_FACTOR, esf_action(flag), &error_);
    p.beamflag[change.beam] = flag;
  }
  streamFlush();
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

int Session::undo() {
  if (!file_open_ || undo_.empty())
    return MB_FAILURE;
  UndoStep step = std::move(undo_.back());
  undo_.pop_back();
  undo_total_ -= (long)step.changes.size();
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
  undo_total_ += (long)step.changes.size();
  undo_.push_back(std::move(step));
  return status;
}

/*--------------------------------------------------------------------*/
void Session::zoomWidth(int x, double factor) {
  if (!file_open_ || nbuff() == 0)
    return;
  const int xcen = box_.xmin + (box_.xmax - box_.xmin) / 2;
  const double old_scale = (box_.xmax - box_.xmin) / (double)settings.plot_width;
  const double across = pan_across_ + (x - xcen) / old_scale;
  settings.plot_width = std::min(std::max((int)std::lround(settings.plot_width * factor), 1), MAX_PLOT_WIDTH);
  const double new_scale = (box_.xmax - box_.xmin) / (double)settings.plot_width;
  pan_across_ = across - (x - xcen) / new_scale;
  zoomed_ = true;
  plot(false);
}

void Session::zoomExaggeration(double factor) {
  if (!file_open_ || nbuff() == 0)
    return;
  settings.exagger = std::max((int)std::lround(settings.exagger * factor), 1);
  plot(false);
}

void Session::panBy(int dx, int dy) {
  if (!file_open_ || nbuff() == 0)
    return;
  const double xscale = (box_.xmax - box_.xmin) / (double)settings.plot_width;
  const double yscale = xscale * settings.exagger / 100.0;
  pan_across_ -= dx / xscale;
  pan_depth_ -= dy / yscale;
  zoomed_ = true;
  plot(false);
}

void Session::resetView() {
  pan_across_ = 0.0;
  pan_depth_ = 0.0;
  zoomed_ = false;
  if (file_open_ && nbuff() > 0)
    plot(true);
}

bool Session::hasAmplitude() const {
  for (const Ping &p : ping_) {
    if (p.has_amp)
      return true;
  }
  return false;
}

/*--------------------------------------------------------------------*/
/* Every change of a flag goes through here. */
void Session::setFlag(int i, int j, int action, char flag) {
  Ping &p = ping_[i];
  if (esffile_open_)
    mb_ess_save(verbose, &esf_, p.time_d, j + p.multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action, &error_);
  if (p.beamflag[j] != flag)
    pending_.push_back({i, j, p.beamflag[j], flag});
  p.beamflag[j] = flag;
  if (verbose >= 1)
    fprintf(stderr, "\nping: %d beam:%d depth:%10.3f flag:0x%2.2x\n", i, j, p.bath[j], (unsigned char)flag);
}

/* a beam already flagged in the data file goes back to its original flag */
void Session::flagBeam(int i, int j) {
  const Ping &p = ping_[i];
  setFlag(i, j, MBP_EDIT_FLAG, mb_beam_ok(p.beamflagorg[j]) ? mb_beam_set_flag_manual(p.beamflag[j]) : p.beamflagorg[j]);
}

void Session::unflagBeam(int i, int j) { setFlag(i, j, MBP_EDIT_UNFLAG, mb_beam_set_flag_none(ping_[i].beamflag[j])); }

void Session::filterBeam(int i, int j) {
  const Ping &p = ping_[i];
  setFlag(i, j, MBP_EDIT_FILTER, mb_beam_ok(p.beamflagorg[j]) ? mb_beam_set_flag_filter2(p.beamflag[j]) : p.beamflagorg[j]);
}

void Session::saveBeam(int i, int j) {
  beam_save_ = true;
  iping_save_ = i;
  jbeam_save_ = j;
}

bool Session::zapHit(int x, int y, int *zap_ping) const {
  bool hit = false;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    if (p.outbounds == OUTBOUNDS_UNFLAGGED && x >= p.zap_x1 && x <= p.zap_x2 && y >= p.zap_y1 && y <= p.zap_y2) {
      hit = true;
      *zap_ping = i;
    }
  }
  return hit;
}

int Session::mouseToggle(int x, int y) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  int zap_ping = 0;
  if (zapHit(x, y, &zap_ping)) {
    status = zapOutbounds(zap_ping);
    finishAction("Toggle");
    return status;
  }

  /* the nearest sounding; soundings that are not drawn cannot be picked */
  int iping = 0, jbeam = 0, range_min = 100000;
  const bool skip_flagged = !compatibility && !settings.show_flaggedsoundings;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_check_flag_unusable2(p.beamflag[j]) && !(skip_flagged && !mb_beam_ok(p.beamflag[j])) && !hidden(i, j) && !reasonHidden(p.beamflag[j])) {
        const int ix = x - p.bath_x[j];
        const int iy = y - p.bath_y[j];
        const int range = (int)sqrt((double)(ix * ix + iy * iy));
        if (range < range_min) {
          range_min = range;
          iping = i;
          jbeam = j;
        }
      }
    }
  }
  if (range_min <= PICK_DISTANCE) {
    if (mb_beam_ok(ping_[iping].beamflag[jbeam]))
      flagBeam(iping, jbeam);
    else if (!mb_beam_check_flag_unusable2(ping_[iping].beamflag[jbeam]))
      unflagBeam(iping, jbeam);
    saveBeam(iping, jbeam);
    if (outsideBox(iping, jbeam))
      layoutPingLabel(iping, false);
    finishAction("Toggle");
  }
  return status;
}

int Session::mousePick(int x, int y) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  int zap_ping = 0;
  if (zapHit(x, y, &zap_ping)) {
    status = zapOutbounds(zap_ping);
    finishAction("Pick");
    return status;
  }

  int iping = 0, jbeam = 0, range_min = 100000;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (mb_beam_ok(p.beamflag[j]) && !hidden(i, j)) {
        const int ix = x - p.bath_x[j];
        const int iy = y - p.bath_y[j];
        const int range = (int)sqrt((double)(ix * ix + iy * iy));
        if (range < range_min) {
          range_min = range;
          iping = i;
          jbeam = j;
        }
      }
    }
  }
  if (range_min <= PICK_DISTANCE) {
    flagBeam(iping, jbeam);
    saveBeam(iping, jbeam);
    if (outsideBox(iping, jbeam))
      layoutPingLabel(iping, false);
    finishAction("Pick");
  }
  return status;
}

int Session::mouseErase(int x, int y) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  bool zap_box = false;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    if (p.outbounds == OUTBOUNDS_UNFLAGGED && x >= p.zap_x1 && x <= p.zap_x2 && y >= p.zap_y1 && y <= p.zap_y2) {
      zap_box = true;
      status = zapOutbounds(i);
    }
  }
  if (!zap_box) {
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      bool replot_label = false;
      for (int j = 0; j < p.beams_bath; j++) {
        if (mb_beam_ok(p.beamflag[j]) && !hidden(i, j)) {
          const int ix = x - p.bath_x[j];
          const int iy = y - p.bath_y[j];
          const int range = (int)sqrt((double)(ix * ix + iy * iy));
          if (range < ERASE_DISTANCE) {
            flagBeam(i, j);
            saveBeam(i, j);
            if (outsideBox(i, j))
              replot_label = true;
          }
        }
      }
      if (replot_label)
        layoutPingLabel(i, false);
    }
  }
  finishAction("Erase");
  return status;
}

int Session::mouseRestore(int x, int y) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  bool zap_box = false;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    if (p.outbounds == OUTBOUNDS_UNFLAGGED && x >= p.zap_x1 && x <= p.zap_x2 && y >= p.zap_y1 && y <= p.zap_y2) {
      zap_box = true;
      status = zapOutbounds(i);
    }
  }
  /* soundings that are not drawn cannot be restored */
  if (!zap_box && (compatibility || settings.show_flaggedsoundings)) {
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      bool replot_label = false;
      for (int j = 0; j < p.beams_bath; j++) {
        if (!mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j]) && !hidden(i, j) && !reasonHidden(p.beamflag[j])) {
          const int ix = x - p.bath_x[j];
          const int iy = y - p.bath_y[j];
          const int range = (int)sqrt((double)(ix * ix + iy * iy));
          if (range < ERASE_DISTANCE) {
            unflagBeam(i, j);
            saveBeam(i, j);
            if (outsideBox(i, j))
              replot_label = true;
          }
        }
      }
      if (replot_label)
        layoutPingLabel(i, false);
    }
  }
  finishAction("Restore");
  return status;
}

bool Session::grabRect(int &xmin, int &xmax, int &ymin, int &ymax) const {
  xmin = std::min(grab_start_x_, grab_end_x_);
  xmax = std::max(grab_start_x_, grab_end_x_);
  ymin = std::min(grab_start_y_, grab_end_y_);
  ymax = std::max(grab_start_y_, grab_end_y_);
  return grab_set_;
}

int Session::mouseGrab(int mode, int x, int y) {
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  resetInfo();
  int status = MB_SUCCESS;
  int xgmin, xgmax, ygmin, ygmax;
  if (mode == GRAB_START) {
    grab_set_ = true;
    grab_start_x_ = grab_end_x_ = x;
    grab_start_y_ = grab_end_y_ = y;
  }
  else if (mode == GRAB_MOVE) {
    grab_set_ = true;
    grab_end_x_ = x;
    grab_end_y_ = y;
    grabRect(xgmin, xgmax, ygmin, ygmax);

    /* soundings near the edges of the rectangle may refresh their ping label */
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      bool replot_label = false;
      for (int j = 0; j < p.beams_bath; j++) {
        if (!mb_beam_check_flag_unusable2(p.beamflag[j]) &&
            (abs(p.bath_x[j] - xgmin) <= 10 || abs(p.bath_x[j] - xgmax) <= 10 || abs(p.bath_y[j] - ygmin) <= 10 ||
             abs(p.bath_y[j] - ygmax) <= 10) &&
            outsideBox(i, j))
          replot_label = true;
      }
      if (replot_label)
        layoutPingLabel(i, false);
    }
  }
  else if (mode == GRAB_END) {
    grab_set_ = false;
    grab_end_x_ = x;
    grab_end_y_ = y;
    grabRect(xgmin, xgmax, ygmin, ygmax);

    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      if (p.outbounds == OUTBOUNDS_UNFLAGGED && xgmin <= p.zap_x1 && xgmax >= p.zap_x2 && ygmin <= p.zap_y1 && ygmax >= p.zap_y2)
        status = zapOutbounds(i);
    }
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      for (int j = 0; j < p.beams_bath; j++) {
        if (mb_beam_ok(p.beamflag[j]) && !hidden(i, j) && p.bath_x[j] >= xgmin && p.bath_x[j] <= xgmax && p.bath_y[j] >= ygmin && p.bath_y[j] <= ygmax) {
          flagBeam(i, j);
          saveBeam(i, j);
        }
      }
    }
    finishAction("Grab");
    status = plot(false);
  }
  return status;
}

int Session::mouseLasso(const std::vector<std::pair<int, int>> &polygon) {
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  resetInfo();
  auto inside = [&polygon](int x, int y) {
    bool in = false;
    const size_t n = polygon.size();
    for (size_t a = 0, b = n - 1; a < n; b = a++) {
      const int xa = polygon[a].first, ya = polygon[a].second, xb = polygon[b].first, yb = polygon[b].second;
      if ((ya > y) != (yb > y) && x < (double)(xb - xa) * (y - ya) / (double)(yb - ya) + xa)
        in = !in;
    }
    return in;
  };
  if (polygon.size() >= 3) {
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      if (p.outbounds == OUTBOUNDS_UNFLAGGED && inside(p.zap_x1, p.zap_y1) && inside(p.zap_x2, p.zap_y2))
        zapOutbounds(i);
    }
    for (int i = current_id_; i < current_id_ + nplot_; i++) {
      const Ping &p = ping_[i];
      for (int j = 0; j < p.beams_bath; j++) {
        if (mb_beam_ok(p.beamflag[j]) && !hidden(i, j) && inside(p.bath_x[j], p.bath_y[j])) {
          flagBeam(i, j);
          saveBeam(i, j);
        }
      }
    }
  }
  finishAction("Lasso");
  return plot(false);
}

int Session::mouseInfo(int x, int y) {
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  resetInfo();
  int iping = 0, jbeam = 0, range_min = 100000;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_check_flag_unusable2(p.beamflag[j]) && !hidden(i, j)) {
        const int ix = x - p.bath_x[j];
        const int iy = y - p.bath_y[j];
        const int range = (int)sqrt((double)(ix * ix + iy * iy));
        if (range < range_min) {
          range_min = range;
          iping = i;
          jbeam = j;
        }
      }
    }
  }
  if (range_min <= PICK_DISTANCE) {
    const Ping &p = ping_[iping];
    info_.set = true;
    info_.ping = iping;
    info_.beam = jbeam;
    std::copy(p.time_i, p.time_i + 7, info_.time_i);
    info_.time_d = p.time_d;
    info_.navlon = p.navlon;
    info_.navlat = p.navlat;
    info_.speed = p.speed;
    info_.heading = p.heading;
    info_.altitude = p.altitude;
    info_.beams_bath = p.beams_bath;
    info_.beamflag = p.beamflag[jbeam];
    info_.bath = p.bath[jbeam];
    info_.amp = p.amp[jbeam];
    info_.bathacrosstrack = p.bathacrosstrack[jbeam];
    info_.bathalongtrack = p.bathalongtrack[jbeam];
    info_.detect = p.detect[jbeam];
    if (info_.detect < MB_DETECT_UNKNOWN || info_.detect > MB_DETECT_PHOTOGRAMMETRY)
      info_.detect = MB_DETECT_UNKNOWN;
    info_.priority = p.priority[jbeam];
    info_.pulse = p.pulses[jbeam];
    if (info_.pulse < MB_PULSE_UNKNOWN || info_.pulse > MB_PULSE_LIDAR)
      info_.pulse = MB_PULSE_UNKNOWN;
  }
  return MB_SUCCESS;
}

bool Session::nearestBeam(int x, int y, bool pick_mode, int *iping, int *jbeam) const {
  if (!file_open_)
    return false;
  int range_min = 100000;
  const bool skip_flagged = !settings.show_flaggedsoundings;
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      const bool ok = mb_beam_ok(p.beamflag[j]);
      if (!hidden(i, j) && !reasonHidden(p.beamflag[j]) && (pick_mode ? ok : (!mb_beam_check_flag_unusable2(p.beamflag[j]) && !(skip_flagged && !ok)))) {
        const int ix = x - p.bath_x[j];
        const int iy = y - p.bath_y[j];
        const int range = (int)sqrt((double)(ix * ix + iy * iy));
        if (range < range_min) {
          range_min = range;
          *iping = i;
          *jbeam = j;
        }
      }
    }
  }
  return range_min <= PICK_DISTANCE;
}

std::vector<std::string> Session::infoText() const {
  static const char *detect_name[] = {"Unknown", "Amplitude", "Phase", "Lidar", "Photogrammetry"};
  static const char *pulse_name[] = {"Unknown", "CW", "Up-Chirp", "Down-Chirp", "Lidar"};
  std::vector<std::string> lines;
  if (!info_.set)
    return lines;
  char text[MB_PATH_MAXLINE];
  const int *t = info_.time_i;
  snprintf(text, sizeof(text), "Ping:%d  Beam:%d  Time: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d", info_.ping, info_.beam, t[1], t[2], t[0], t[3], t[4],
           t[5], (int)(0.001 * t[6]));
  lines.push_back(text);
  snprintf(text, sizeof(text), "Lon:%.5f deg  Lat:%.5f deg  Hdg:%.1f deg  Spd:%.1f km/hr", info_.navlon, info_.navlat, info_.heading, info_.speed);
  lines.push_back(text);
  snprintf(text, sizeof(text), "Depth:%.2f  X:%.2f  L:%.2f  Alt:%.2f  Amp:%.2f  Detect:%s  Pulse:%s", info_.bath, info_.bathacrosstrack,
           info_.bathalongtrack, info_.altitude, info_.amp, detect_name[info_.detect], pulse_name[info_.pulse]);
  lines.push_back(text);
  if (reference_ != nullptr && info_.ping < nbuff()) {
    std::vector<double> values;
    residuals(info_.ping, nullptr, values);
    if (info_.beam < (int)values.size() && !std::isnan(values[info_.beam])) {
      snprintf(text, sizeof(text), "  Reference:%.2f", values[info_.beam]);
      lines.back() += text;
    }
  }
  return lines;
}

int Session::zapOutbounds(int iping) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  const Ping &p = ping_[iping];
  bool found = false;
  for (int j = 0; j < p.beams_bath; j++) {
    if (mb_beam_ok(p.beamflag[j]) && outsideBox(iping, j)) {
      flagBeam(iping, j);
      saveBeam(iping, j);
      found = true;
    }
  }
  if (found)
    layoutPingLabel(iping, false);
  return MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::badPing() {
  resetInfo();
  if (!file_open_ || !beam_save_)
    return MB_FAILURE;
  const Ping &p = ping_[iping_save_];
  for (int j = 0; j < p.beams_bath; j++) {
    if (mb_beam_ok(p.beamflag[j]))
      flagBeam(iping_save_, j);
  }
  if (p.outbounds != OUTBOUNDS_NONE)
    layoutPingLabel(iping_save_, false);
  finishAction("Bad Ping");
  return MB_SUCCESS;
}

int Session::goodPing() {
  resetInfo();
  if (!file_open_ || !beam_save_)
    return MB_FAILURE;
  const Ping &p = ping_[iping_save_];
  for (int j = 0; j < p.beams_bath; j++) {
    if (!mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j]))
      unflagBeam(iping_save_, j);
  }
  if (p.outbounds != OUTBOUNDS_NONE)
    layoutPingLabel(iping_save_, false);
  finishAction("Good Ping");
  return MB_SUCCESS;
}

int Session::leftPing() {
  resetInfo();
  if (!file_open_ || !beam_save_)
    return MB_FAILURE;
  const Ping &p = ping_[iping_save_];
  for (int j = 0; j <= jbeam_save_; j++) {
    if (mb_beam_ok(p.beamflag[j]))
      flagBeam(iping_save_, j);
  }
  if (p.outbounds != OUTBOUNDS_NONE)
    layoutPingLabel(iping_save_, false);
  finishAction("Flag Left");
  return MB_SUCCESS;
}

int Session::rightPing() {
  resetInfo();
  if (!file_open_ || !beam_save_)
    return MB_FAILURE;
  const Ping &p = ping_[iping_save_];
  for (int j = jbeam_save_; j < p.beams_bath; j++) {
    if (mb_beam_ok(p.beamflag[j]))
      flagBeam(iping_save_, j);
  }
  if (p.outbounds != OUTBOUNDS_NONE)
    layoutPingLabel(iping_save_, false);
  finishAction("Flag Right");
  return MB_SUCCESS;
}

int Session::zeroPing() {
  resetInfo();
  if (!file_open_ || !beam_save_)
    return MB_FAILURE;
  Ping &p = ping_[iping_save_];
  for (int j = 0; j < p.beams_bath; j++) {
    if (!mb_beam_check_flag_unusable2(p.beamflag[j]))
      setFlag(iping_save_, j, MBP_EDIT_ZERO, mb_beam_set_flag_null(p.beamflag[j]));
    else if (p.beamflag[j] != (char)mb_beam_set_flag_null(p.beamflag[j])) {
      pending_.push_back({iping_save_, j, p.beamflag[j], (char)mb_beam_set_flag_null(p.beamflag[j])});
      p.beamflag[j] = mb_beam_set_flag_null(p.beamflag[j]);
    }
  }
  if (p.outbounds != OUTBOUNDS_NONE)
    layoutPingLabel(iping_save_, false);
  finishAction("Zero Ping");
  return MB_SUCCESS;
}

int Session::flagView() {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (mb_beam_ok(p.beamflag[j]) && !hidden(i, j)) {
        flagBeam(i, j);
        saveBeam(i, j);
      }
    }
  }
  finishAction("Flag View");
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

int Session::unflagView() {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  for (int i = current_id_; i < current_id_ + nplot_; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j]) && !hidden(i, j)) {
        unflagBeam(i, j);
        saveBeam(i, j);
      }
    }
  }
  finishAction("Unflag View");
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

int Session::unflagForward() {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  for (int i = current_id_; i < nbuff(); i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_ok(p.beamflag[j]) && !mb_beam_check_flag_unusable2(p.beamflag[j])) {
        unflagBeam(i, j);
        beam_save_ = false;
      }
    }
  }
  finishAction("Unflag Forward");
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::filterForward() {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  progress("Applying bathymetry filters...");
  for (int i = current_id_; i < nbuff(); i++) {
    filterPing(i);
    if (i % 250 == 0)
      progress("Filters applied to " + std::to_string(i) + " of " + std::to_string(nbuff() - current_id_ - 1) + " records so far...");
  }
  finishAction("Filter");
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

/* The filters, one ping at a time: they depend on the flags changed before, so a
   preview must follow exactly the same order. */
template <typename FlagAt, typename Unflag, typename Flag>
static void run_filters(const std::vector<Ping> &pings, int iping, const Filters &f, std::vector<double> &bathlist, FlagAt flag_at, Unflag unflag, Flag flag) {
  const Ping &p = pings[iping];
  const int nb = (int)pings.size();

  /* clear the flags of earlier filter runs */
  for (int j = 0; j < p.beams_bath; j++) {
    const char flg = flag_at(iping, j);
    if (mb_beam_check_flag_filter2(flg))
      unflag(j);
  }
  auto ok = [&](int i, int j) {
    const char flg = flag_at(i, j);
    return mb_beam_ok(flg) != 0;
  };

  if (f.medianspike) {
    for (int jbeam = 0; jbeam < p.beams_bath; jbeam++) {
      if (ok(iping, jbeam)) {
        bathlist.clear();
        double bathmedian = 0.0;
        const int ltrack_half = std::min(f.medianspike_ltrack / 2, (MAX_PLOT_PINGS - 1) / 2);
        const int istart = std::max(iping - ltrack_half, 0);
        const int iend = std::min(iping + ltrack_half, nb - 1);
        for (int i = istart; i <= iend; i++) {
          const Ping &q = pings[i];
          const int jstart = std::max(jbeam - f.medianspike_xtrack / 2, 0);
          const int jend = std::min(jbeam + f.medianspike_xtrack / 2, q.beams_bath - 1);
          for (int j = jstart; j <= jend; j++) {
            if (ok(i, j))
              bathlist.push_back(q.bath[j]);
          }
        }
        if (!bathlist.empty()) {
          qsort(bathlist.data(), bathlist.size(), sizeof(double), mb_double_compare);
          bathmedian = bathlist[bathlist.size() / 2];
        }
        if (p.altitude > 0.0 && 100 * fabs(p.bath[jbeam] - bathmedian) / p.altitude > f.medianspike_threshold)
          flag(jbeam);
      }
    }
  }

  if (f.wrongside) {
    const int port_end = (p.beams_bath / 2) - f.wrongside_threshold;
    for (int j = 0; j < port_end; j++) {
      if (ok(iping, j) && p.bathacrosstrack[j] > 0.0)
        flag(j);
    }
    for (int j = (p.beams_bath / 2) + f.wrongside_threshold; j < p.beams_bath; j++) {
      if (ok(iping, j) && p.bathacrosstrack[j] < 0.0)
        flag(j);
    }
  }

  /* for the three cuts, a begin above the end means both edges of the swath */
  if (f.cutbeam) {
    if (f.cutbeam_begin <= f.cutbeam_end) {
      const int start = std::max(f.cutbeam_begin, 0);
      const int end = std::min(f.cutbeam_end, p.beams_bath - 1);
      for (int j = start; j <= end; j++) {
        if (ok(iping, j))
          flag(j);
      }
    }
    else {
      for (int j = 0; j < p.beams_bath; j++) {
        if ((j <= f.cutbeam_end || j >= f.cutbeam_begin) && ok(iping, j))
          flag(j);
      }
    }
  }

  if (f.cutdistance) {
    const bool inside = f.cutdistance_begin <= f.cutdistance_end;
    for (int j = 0; j < p.beams_bath; j++) {
      if (ok(iping, j)) {
        const double xtrack = p.bathacrosstrack[j];
        if (inside ? (xtrack >= f.cutdistance_begin && xtrack <= f.cutdistance_end)
                   : (xtrack >= f.cutdistance_begin || xtrack <= f.cutdistance_end))
          flag(j);
      }
    }
  }

  if (f.cutangle) {
    const bool inside = f.cutangle_begin <= f.cutangle_end;
    for (int j = 0; j < p.beams_bath; j++) {
      if (ok(iping, j) && p.altitude > 0.0) {
        const double angle = RTD * atan(p.bathacrosstrack[j] / p.altitude);
        if (inside ? (angle >= f.cutangle_begin && angle <= f.cutangle_end) : (angle >= f.cutangle_begin || angle <= f.cutangle_end))
          flag(j);
      }
    }
  }
}

int Session::filterPing(int iping) {
  resetInfo();
  if (!file_open_ || iping < 0 || iping >= nbuff())
    return MB_SUCCESS;
  run_filters(
      ping_, iping, filters, bathlist_, [this](int i, int j) { return ping_[i].beamflag[j]; }, [this, iping](int j) { unflagBeam(iping, j); },
      [this, iping](int j) { filterBeam(iping, j); });
  return MB_SUCCESS;
}

Session::FlagCopy Session::copyFlags() const {
  FlagCopy flags(ping_.size());
  for (size_t i = 0; i < ping_.size(); i++)
    flags[i] = ping_[i].beamflag;
  return flags;
}

static void add_preview_item(FlagPreview &preview, int ping, int beam, char before, char after, std::vector<char> &touched) {
  preview.items.push_back({ping, beam, after});
  if (mb_beam_ok(before) && !mb_beam_ok(after))
    preview.nflag++;
  else if (!mb_beam_ok(before) && mb_beam_ok(after))
    preview.nunflag++;
  touched[ping] = 1;
}

FlagPreview Session::previewFilters(const Filters &f, FlagCopy flags, int first, const std::atomic<bool> *cancel) const {
  FlagPreview preview;
  std::vector<double> bathlist;
  std::vector<char> touched(ping_.size(), 0);
  for (int iping = first; iping < nbuff(); iping++) {
    if (cancel != nullptr && cancel->load())
      return FlagPreview();
    const Ping &p = ping_[iping];
    const std::vector<char> before = flags[iping];
    run_filters(
        ping_, iping, f, bathlist, [&flags](int i, int j) { return flags[i][j]; }, [&flags, iping](int j) { flags[iping][j] = mb_beam_set_flag_none(flags[iping][j]); },
        [&flags, &p, iping](int j) {
          flags[iping][j] = mb_beam_ok(p.beamflagorg[j]) ? (char)mb_beam_set_flag_filter2(flags[iping][j]) : p.beamflagorg[j];
        });
    for (int j = 0; j < p.beams_bath; j++) {
      if (flags[iping][j] != before[j])
        add_preview_item(preview, iping, j, before[j], flags[iping][j], touched);
    }
  }
  for (const char t : touched)
    preview.npings += t;
  return preview;
}

bool Session::previewEditFile(const std::string &path, FlagPreview &preview, std::string &message) const {
  preview = FlagPreview();
  mb_path name;
  if (path.size() >= sizeof(name)) {
    message = "The path is too long: " + path;
    return false;
  }
  snprintf(name, sizeof(name), "%s", path.c_str());
  struct mb_esf_struct esf;
  memset(&esf, 0, sizeof(esf));
  int error = MB_ERROR_NO_ERROR;
  if (mb_esf_open(0, PROGRAM_NAME, name, true, MBP_ESF_NOWRITE, &esf, &error) != MB_SUCCESS) {
    message = "Unable to read " + path;
    return false;
  }
  std::vector<char> touched(ping_.size(), 0);
  for (int iping = 0; iping < nbuff(); iping++) {
    const Ping &p = ping_[iping];
    std::vector<char> flags = p.beamflagorg;
    mb_esf_apply(0, &esf, p.time_d, p.multiplicity, p.beams_bath, flags.data(), &error);
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_ok(flags[j]) && mb_beam_check_flag_sonar(p.beamflagorg[j]))
        flags[j] = mb_beam_set_flag_sonar(flags[j]);
      if (flags[j] != p.beamflag[j])
        add_preview_item(preview, iping, j, p.beamflag[j], flags[j], touched);
    }
  }
  for (const char t : touched)
    preview.npings += t;
  for (int i = 0; i < esf.nedit; i++) {
    if (esf.edit[i].use == 0)
      preview.complete = false;
  }
  mb_esf_close(0, &esf, &error);
  return true;
}

int Session::applyPreview(const FlagPreview &preview, const char *label) {
  if (!file_open_)
    return MB_FAILURE;
  resetInfo();
  for (const FlagPreview::Item &item : preview.items) {
    if (item.ping < nbuff() && item.beam < ping_[item.ping].beams_bath)
      setFlag(item.ping, item.beam, esf_action(item.after), item.after);
  }
  finishAction(label);
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
int Session::flagReason(char flag) {
  if (mb_beam_ok(flag) || mb_beam_check_flag_unusable2(flag))
    return 0;
  if (mb_beam_check_flag_filter2(flag) || mb_beam_check_flag_filter(flag))
    return REASON_FILTER;
  if (mb_beam_check_flag_sonar(flag))
    return REASON_SONAR;
  return REASON_MANUAL;
}

long Session::countReason(int reason, int scope) const {
  if (!file_open_)
    return 0;
  const int first = scope == SCOPE_BUFFER ? 0 : current_id_;
  const int last = scope == SCOPE_VIEW ? current_id_ + nplot_ : nbuff();
  long count = 0;
  for (int i = first; i < last; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if ((flagReason(p.beamflag[j]) & reason) != 0 && !(scope == SCOPE_VIEW && hidden(i, j)))
        count++;
    }
  }
  return count;
}

int Session::unflagReason(int reason, int scope) {
  resetInfo();
  if (!file_open_) {
    current_id_ = 0;
    return MB_FAILURE;
  }
  const int first = scope == SCOPE_BUFFER ? 0 : current_id_;
  const int last = scope == SCOPE_VIEW ? current_id_ + nplot_ : nbuff();
  for (int i = first; i < last; i++) {
    const Ping &p = ping_[i];
    for (int j = 0; j < p.beams_bath; j++) {
      if ((flagReason(p.beamflag[j]) & reason) != 0 && !(scope == SCOPE_VIEW && hidden(i, j)))
        unflagBeam(i, j);
    }
  }
  finishAction(reason == REASON_MANUAL ? "Unflag Manual" : reason == REASON_FILTER ? "Unflag Filter" : "Unflag Sonar");
  return nbuff() > 0 ? plot(false) : MB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/* The review queue. Nothing here changes a flag. */
std::vector<Suspect> Session::findSuspects(const FlagCopy &flags, const std::atomic<bool> *cancel) const {
  constexpr int HALF_WINDOW = 4;            // beams to each side for the local median
  constexpr double SPIKE_FRACTION = 0.0025; // of the altitude; the best of those tried against manual edits
  constexpr double REFERENCE_FRACTION = 0.02; // lower values made the order worse against manual edits
  constexpr int NEIGHBOURS = 10;            // pings to each side for the count of good soundings
  constexpr double DEFICIT_MIN = 0.2;
  constexpr double JUMP_FACTOR = 5.0;       // times the 90th percentile of the changes between pings
  constexpr double GAP_FACTOR = 5.0;        // the same for the ping interval, as dual ping sonars alternate
  const int nb = nbuff();
  std::vector<Suspect> all(nb);
  std::vector<int> good(nb, 0);
  std::vector<double> window, reference_values;

  for (int i = 0; i < nb; i++) {
    if (cancel != nullptr && cancel->load())
      return {};
    const Ping &p = ping_[i];
    const std::vector<char> &f = flags[i];
    all[i].ping = i;
    if (reference_ != nullptr) {
      residuals(i, &f, reference_values);
      for (int j = 0; j < p.beams_bath; j++) {
        if (mb_beam_ok(f[j]) && !std::isnan(reference_values[j]) && fabs(reference_values[j]) > REFERENCE_FRACTION * p.altitude)
          all[i].reference_spikes++;
      }
    }
    for (int j = 0; j < p.beams_bath; j++) {
      if (!mb_beam_ok(f[j]))
        continue;
      good[i]++;
      window.clear();
      for (int k = std::max(j - HALF_WINDOW, 0); k <= std::min(j + HALF_WINDOW, p.beams_bath - 1); k++) {
        if (mb_beam_ok(f[k]))
          window.push_back(p.bath[k]);
      }
      if (window.size() >= 3 && p.altitude > 0.0) {
        std::nth_element(window.begin(), window.begin() + window.size() / 2, window.end());
        if (fabs(p.bath[j] - window[window.size() / 2]) > SPIKE_FRACTION * p.altitude)
          all[i].spikes++;
      }
    }
  }

  /* jumps against what is usual in this buffer */
  auto typical = [&](auto value) {
    std::vector<double> changes;
    for (int i = 1; i < nb; i++)
      changes.push_back(value(i));
    if (changes.empty())
      return 0.0;
    const size_t k = changes.size() * 9 / 10;
    std::nth_element(changes.begin(), changes.begin() + k, changes.end());
    return changes[k];
  };
  auto roll_change = [this](int i) { return fabs(ping_[i].roll - ping_[i - 1].roll); };
  auto pitch_change = [this](int i) { return fabs(ping_[i].pitch - ping_[i - 1].pitch); };
  auto interval = [this](int i) { return ping_[i].time_d - ping_[i - 1].time_d; };
  const double usual_roll = typical(roll_change), usual_pitch = typical(pitch_change), usual_interval = typical(interval);

  std::vector<int> counts;
  for (int i = 0; i < nb; i++) {
    if (cancel != nullptr && cancel->load())
      return {};
    Suspect &s = all[i];
    counts.clear();
    for (int k = std::max(i - NEIGHBOURS, 0); k <= std::min(i + NEIGHBOURS, nb - 1); k++) {
      if (k != i)
        counts.push_back(good[k]);
    }
    if (!counts.empty()) {
      std::nth_element(counts.begin(), counts.begin() + counts.size() / 2, counts.end());
      const int usual = counts[counts.size() / 2];
      if (usual > 0 && good[i] < usual) {
        const double deficit = (double)(usual - good[i]) / usual;
        if (deficit >= DEFICIT_MIN)
          s.deficit = deficit;
      }
    }
    if (i > 0) {
      s.attitude_jump = (usual_roll > 0.0 && roll_change(i) > JUMP_FACTOR * usual_roll) || (usual_pitch > 0.0 && pitch_change(i) > JUMP_FACTOR * usual_pitch);
      s.time_gap = usual_interval > 0.0 && interval(i) > GAP_FACTOR * usual_interval;
    }
    s.score = s.spikes + s.reference_spikes + 0.5 * s.deficit * ping_[i].beams_bath + (s.attitude_jump ? 5.0 : 0.0) + (s.time_gap ? 2.0 : 0.0);
  }

  std::vector<Suspect> suspects;
  for (const Suspect &s : all) {
    if (s.score > 0.0)
      suspects.push_back(s);
  }
  std::stable_sort(suspects.begin(), suspects.end(), [](const Suspect &a, const Suspect &b) { return a.score > b.score; });
  return suspects;
}

void Session::filterLimits(int *beams_max, double *distance_max) const {
  *beams_max = 0;
  *distance_max = 0.0;
  if (file_open_) {
    for (const Ping &p : ping_) {
      for (int j = 0; j < p.beams_bath; j++) {
        if (mb_beam_ok(p.beamflag[j])) {
          *beams_max = std::max(*beams_max, p.beams_bath);
          *distance_max = std::max(*distance_max, fabs(p.bathacrosstrack[j]));
        }
      }
    }
  }
  if (*beams_max == 0)
    *beams_max = 200;
  if (*distance_max == 0.0)
    *distance_max = 10000.0;
}

/*--------------------------------------------------------------------*/
bool Session::loadReference(const std::string &path, std::string &message) {
  mb_path name, projection = "";
  if (path.size() >= sizeof(name)) {
    message = "The path is too long: " + path;
    return false;
  }
  snprintf(name, sizeof(name), "%s", path.c_str());
  int mode = MB_PROJECTION_GEOGRAPHIC, nxy = 0, nx = 0, ny = 0, error = MB_ERROR_NO_ERROR;
  float nodata = 0.0f;
  double zmin, zmax, xmin, xmax, ymin, ymax, dx, dy;
  float *grid = nullptr;
  if (mb_read_gmt_grd(verbose, name, &mode, projection, &nodata, &nxy, &nx, &ny, &zmin, &zmax, &xmin, &xmax, &ymin, &ymax, &dx, &dy, &grid, nullptr, nullptr,
                      &error) != MB_SUCCESS) {
    message = "Unable to read the grid " + path;
    return false;
  }
  void *projection_ptr = nullptr;
  if (mode == MB_PROJECTION_PROJECTED && mb_proj_init(verbose, projection, &projection_ptr, &error) != MB_SUCCESS) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&grid, &error);
    message = std::string("Unable to use the projection of the grid: ") + projection;
    return false;
  }
  clearReference();
  reference_ = grid;
  reference_name_ = path;
  ref_nx_ = nx;
  ref_ny_ = ny;
  ref_dx_ = dx;
  ref_dy_ = dy;

  /* gridline or pixel registration */
  const long reg_x = lrint((xmax - xmin) / dx) + 1 - nx;
  const long reg_y = lrint((ymax - ymin) / dy) + 1 - ny;
  ref_x0_ = xmin + 0.5 * (reg_x == 1 ? 1 : 0) * dx;
  ref_y0_ = ymin + 0.5 * (reg_y == 1 ? 1 : 0) * dy;
  ref_nodata_ = nodata;
  ref_projection_ = projection_ptr;
  ref_sign_ = 1.0;

  if (updateReferenceSign() == 0 && nbuff() == 0 && zmax <= 0.0)
    ref_sign_ = -1.0;
  return true;
}

/* A depth grid agrees with the depths, and for a topography grid their difference
   is about twice the depth. */
long Session::updateReferenceSign() {
  if (reference_ == nullptr || nbuff() == 0)
    return 0;
  ref_sign_ = 1.0;
  std::vector<double> differences, values;
  double depth = 0.0;
  for (int i = 0; i < nbuff(); i += std::max(nbuff() / 50, 1)) {
    residuals(i, nullptr, values);
    const Ping &p = ping_[i];
    if (depth == 0.0 && p.altitude > 0.0)
      depth = p.altitude + p.sensordepth;
    for (int j = 0; j < p.beams_bath; j++) {
      if (mb_beam_ok(p.beamflag[j]) && !std::isnan(values[j]))
        differences.push_back(values[j]);
    }
  }
  if (differences.empty())
    return 0;
  std::nth_element(differences.begin(), differences.begin() + differences.size() / 2, differences.end());
  if (fabs(differences[differences.size() / 2]) > 0.5 * std::max(depth, 1.0))
    ref_sign_ = -1.0;
  return (long)differences.size();
}

void Session::clearReference() {
  int error = MB_ERROR_NO_ERROR;
  if (reference_ != nullptr)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&reference_, &error);
  reference_ = nullptr;
  reference_name_.clear();
  if (ref_projection_ != nullptr)
    mb_proj_free(verbose, &ref_projection_, &error);
  ref_projection_ = nullptr;
}

double Session::referenceAt(double lon, double lat) const {
  double x = lon, y = lat;
  if (ref_projection_ != nullptr) {
    /* the projection library is not safe for threads */
    const std::lock_guard<std::mutex> lock(ref_mutex_);
    int error = MB_ERROR_NO_ERROR;
    mb_proj_forward(0, ref_projection_, lon, lat, &x, &y, &error);
  }
  const double fx = (x - ref_x0_) / ref_dx_, fy = (y - ref_y0_) / ref_dy_;
  const int i = (int)std::floor(fx), j = (int)std::floor(fy);
  if (i < 0 || j < 0 || i >= ref_nx_ || j >= ref_ny_)
    return std::nan("");
  const int i1 = std::min(i + 1, ref_nx_ - 1), j1 = std::min(j + 1, ref_ny_ - 1);
  const double tx = fx - i, ty = fy - j;
  /* as GMT does it: the nodes that have a value, if they carry at least half of the weight */
  const int nodes[4][2] = {{i, j}, {i1, j}, {i, j1}, {i1, j1}};
  const double weights[4] = {(1.0 - tx) * (1.0 - ty), tx * (1.0 - ty), (1.0 - tx) * ty, tx * ty};
  double sum = 0.0, total = 0.0;
  for (int k = 0; k < 4; k++) {
    const float value = reference_[nodes[k][0] * ref_ny_ + nodes[k][1]];
    if (value != ref_nodata_) {
      sum += weights[k] * value;
      total += weights[k];
    }
  }
  return total >= 0.5 ? sum / total : std::nan("");
}

/* The position of a sounding as mbclean finds it. The navigation is of the mapping sensor. */
void Session::residuals(int iping, const std::vector<char> *flags, std::vector<double> &values) const {
  const Ping &p = ping_[iping];
  values.assign(p.beams_bath, std::nan(""));
  if (reference_ == nullptr)
    return;
  double mtodeglon, mtodeglat;
  mb_coor_scale(0, p.navlat, &mtodeglon, &mtodeglat);
  const double headingx = sin(p.heading * DTR), headingy = cos(p.heading * DTR);
  for (int j = 0; j < p.beams_bath; j++) {
    const char flag = flags != nullptr ? (*flags)[j] : p.beamflag[j];
    if (mb_beam_check_flag_unusable2(flag))
      continue;
    const double lon = p.navlon + mtodeglon * (headingy * p.bathacrosstrack[j] + headingx * p.bathalongtrack[j]);
    const double lat = p.navlat + mtodeglat * (-headingx * p.bathacrosstrack[j] + headingy * p.bathalongtrack[j]);
    const double value = referenceAt(lon, lat);
    if (!std::isnan(value))
      values[j] = p.bath[j] - ref_sign_ * value;
  }
}

std::vector<Session::HistoryVersion> Session::historyVersions(const std::string &swath_path) {
  std::vector<HistoryVersion> versions;
  const std::string esf = esfName(swath_path);
  mb_path dir, base;
  int error = MB_ERROR_NO_ERROR, nentry = 0;
  struct mb_history_entry *entries = nullptr;
  if (mb_history_paths(0, esf.c_str(), dir, sizeof(dir), base, sizeof(base), &error) != MB_SUCCESS ||
      mb_history_read_manifest(0, esf.c_str(), &nentry, &entries, &error) != MB_SUCCESS)
    return versions;
  for (int i = 0; i < nentry; i++) {
    HistoryVersion v;
    v.version = entries[i].version;
    v.utc = entries[i].utc;
    v.program = entries[i].program;
    v.user = entries[i].user;
    v.note = entries[i].note;
    v.path = std::string(dir) + "/" + entries[i].snapshot;
    v.nedits = entries[i].size >= (long)ESF_HEADER_SIZE ? (entries[i].size - (long)ESF_HEADER_SIZE) / 16 : 0;
    versions.push_back(v);
  }
  if (entries != nullptr)
    mb_freed(0, __FILE__, __LINE__, (void **)&entries, &error);
  return versions;
}

/*--------------------------------------------------------------------*/
const char *Session::tsLabel(int data_id) {
  switch (data_id) {
  case PLOT_WIDE:
    return "WIDE PLOT";
  case PLOT_TIME:
    return "TIME STAMP";
  case PLOT_INTERVAL:
    return "Ping Interval (sec)";
  case PLOT_LON:
    return "Longitude (deg)";
  case PLOT_LAT:
    return "Latitude (deg)";
  case PLOT_HEADING:
    return "Heading (deg)";
  case PLOT_SPEED:
    return "Speed (km/hr)";
  case PLOT_DEPTH:
    return "Center Beam Depth (m)";
  case PLOT_ALTITUDE:
    return "Sonar Altitude (m)";
  case PLOT_SENSORDEPTH:
    return "Sonar Depth (m)";
  case PLOT_ROLL:
    return "Roll (deg)";
  case PLOT_PITCH:
    return "Pitch (deg)";
  case PLOT_HEAVE:
    return "Heave (m)";
  }
  return "";
}

double Session::tsValue(int iping, int data_id) const {
  if (iping < 0 || iping >= nbuff())
    return 0.0;
  const Ping &p = ping_[iping];
  switch (data_id) {
  case PLOT_INTERVAL:
    return p.time_interval;
  case PLOT_LON:
    return p.navlon;
  case PLOT_LAT:
    return p.navlat;
  case PLOT_HEADING:
    return p.heading;
  case PLOT_SPEED:
    return p.speed;
  case PLOT_DEPTH:
    return centerDepth(p);
  case PLOT_ALTITUDE:
    return p.altitude;
  case PLOT_SENSORDEPTH:
    return p.sensordepth;
  case PLOT_ROLL:
    return p.roll;
  case PLOT_PITCH:
    return p.pitch;
  case PLOT_HEAVE:
    return p.heave;
  }
  return 0.0;
}

void Session::tsMinMax(int iping, int nping, int data_id, double *tsmin, double *tsmax) const {
  *tsmin = 0.0;
  *tsmax = 0.0;
  if (iping >= 0 && nbuff() > iping && nping > 0 && iping + nping - 1 < nbuff()) {
    *tsmin = *tsmax = tsValue(iping, data_id);
    for (int i = iping; i < iping + nping; i++) {
      const double value = tsValue(i, data_id);
      *tsmin = std::min(*tsmin, value);
      *tsmax = std::max(*tsmax, value);

      /* the roll plot also shows the acrosstrack slope */
      if (data_id == PLOT_ROLL) {
        const double slope = xtrackSlope(i);
        *tsmin = std::min(*tsmin, slope);
        *tsmax = std::max(*tsmax, slope);
        *tsmin = std::min(*tsmin, value - slope);
        *tsmax = std::max(*tsmax, value - slope);
      }
    }
  }

  auto centered = [&](double smallest) {
    const double halfwidth = std::max(smallest, 0.55 * (*tsmax - *tsmin));
    const double center = 0.5 * (*tsmin + *tsmax);
    *tsmin = center - halfwidth;
    *tsmax = center + halfwidth;
  };
  auto symmetric = [&](double smallest) {
    *tsmax = 1.1 * std::max(fabs(*tsmin), fabs(*tsmax));
    *tsmax = std::max(*tsmax, smallest);
    *tsmin = -(*tsmax);
  };
  switch (data_id) {
  case PLOT_WIDE:
  case PLOT_TIME:
    *tsmin = 0.0;
    *tsmax = 1.0;
    break;
  case PLOT_INTERVAL:
    *tsmin = 0.0;
    *tsmax = std::max(1.1 * (*tsmax), 0.01);
    break;
  case PLOT_LON:
  case PLOT_LAT:
    centered(0.001);
    break;
  case PLOT_HEADING:
    *tsmin = 0.0;
    *tsmax = 360.0;
    break;
  case PLOT_SPEED:
    *tsmin = 0.0;
    *tsmax = std::max(*tsmax, 5.0);
    break;
  case PLOT_DEPTH:
  case PLOT_ALTITUDE:
  case PLOT_SENSORDEPTH:
    centered(1.0);
    break;
  case PLOT_ROLL:
  case PLOT_PITCH:
    symmetric(1.0);
    break;
  case PLOT_HEAVE:
    symmetric(0.25);
    break;
  }
}

double Session::xtrackSlope(int iping) const {
  if (iping < 0 || iping >= nbuff())
    return 0.0;
  const Ping &p = ping_[iping];
  double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
  int ns = 0;
  for (int j = 0; j < p.beams_bath; j++) {
    if (mb_beam_ok(p.beamflag[j])) {
      sx += p.bathacrosstrack[j];
      sy += p.bath[j];
      sxx += p.bathacrosstrack[j] * p.bathacrosstrack[j];
      sxy += p.bathacrosstrack[j] * p.bath[j];
      ns++;
    }
  }
  if (ns > 1) {
    const double delta = ns * sxx - sx * sx;
    if (delta != 0.0)
      return -RTD * atan((ns * sxy - sx * sy) / delta);
  }
  return 0.0;
}

}  // namespace mbe
