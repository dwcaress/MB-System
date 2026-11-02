// Tests for the edit save file history functions in src/mbio/mb_history.c
// and the snapshot made by mb_esf_open() before an esf file is modified.
//
// See README file for copying and redistribution conditions.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "mbio/mb_define.h"
#include "mbio/mb_process.h"
#include "mbio/mb_status.h"
#include "mbio/mb_swap.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace {

// Make an empty scratch directory for one test.
fs::path MakeScratchDir(const std::string &name) {
  const fs::path dir = fs::temp_directory_path() / ("mb_history_test_" + name);
  fs::remove_all(dir);
  fs::create_directories(dir);
  return dir;
}

std::string ReadFile(const fs::path &path) {
  std::ifstream in(path, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void WriteFile(const fs::path &path, const std::string &content) {
  std::ofstream out(path, std::ios::binary);
  out << content;
}

// Write an edit save file in the byte order MB-System uses (big endian)
// with a version 3 header and the given number of edits.
void WriteEsf(const fs::path &path, int nedit, int action, int mode = 0, const char *program = "mb_history_test") {
  FILE *fp = fopen(path.string().c_str(), "wb");
  ASSERT_NE(nullptr, fp);
  char header[MB_PATH_MAXLINE];
  memset(header, 0, sizeof(header));
  snprintf(header, sizeof(header), "ESFVERSION03\nESF Mode: %d\nProgram: %s\n", mode, program);
  fwrite(header, sizeof(header), 1, fp);
  const bool byteswapped = mb_swap_check();
  for (int i = 0; i < nedit; i++) {
    double time_d = 1761000000.0 + i;
    int beam = i % 100;
    if (byteswapped) {
      mb_swap_double(&time_d);
      beam = mb_swap_int(beam);
      action = mb_swap_int(action);
    }
    fwrite(&time_d, sizeof(double), 1, fp);
    fwrite(&beam, sizeof(int), 1, fp);
    fwrite(&action, sizeof(int), 1, fp);
    if (byteswapped)
      action = mb_swap_int(action);
  }
  fclose(fp);
}

int CountSnapshots(const fs::path &dir) {
  if (!fs::exists(dir))
    return 0;
  int n = 0;
  for (const auto &entry : fs::directory_iterator(dir)) {
    if (entry.path().filename().string().find(".v") != std::string::npos)
      n++;
  }
  return n;
}

}  // namespace

TEST(MbHistoryTest, Paths) {
  int error = MB_ERROR_NO_ERROR;
  mb_path dir;
  mb_path base;
  EXPECT_EQ(MB_SUCCESS, mb_history_paths(0, "/data/survey/line.mb88.esf", dir, sizeof(dir), base, sizeof(base), &error));
  EXPECT_STREQ("/data/survey/mbhistory", dir);
  EXPECT_STREQ("line.mb88.esf", base);
  EXPECT_EQ(MB_ERROR_NO_ERROR, error);

  EXPECT_EQ(MB_SUCCESS, mb_history_paths(0, "line.mb88.esf", dir, sizeof(dir), base, sizeof(base), &error));
  EXPECT_STREQ("mbhistory", dir);
  EXPECT_STREQ("line.mb88.esf", base);

  EXPECT_EQ(MB_FAILURE, mb_history_paths(0, "/data/survey/", dir, sizeof(dir), base, sizeof(base), &error));
  EXPECT_EQ(MB_ERROR_BAD_PARAMETER, error);
}

TEST(MbHistoryTest, Crc32) {
  const fs::path scratch = MakeScratchDir("crc");
  const fs::path file = scratch / "check.txt";
  WriteFile(file, "123456789");
  int error = MB_ERROR_NO_ERROR;
  unsigned int crc = 0;
  long size = 0;
  EXPECT_EQ(MB_SUCCESS, mb_history_crc32(0, file.string().c_str(), &crc, &size, &error));
  EXPECT_EQ(0xCBF43926U, crc);  // standard CRC-32 check value
  EXPECT_EQ(9, size);

  EXPECT_EQ(MB_FAILURE, mb_history_crc32(0, (scratch / "missing").string().c_str(), &crc, &size, &error));
  EXPECT_EQ(MB_ERROR_OPEN_FAIL, error);
  fs::remove_all(scratch);
}

TEST(MbHistoryTest, SnapshotModes) {
  const fs::path scratch = MakeScratchDir("modes");
  const fs::path esf = scratch / "line.mb88.esf";
  const std::string esfname = esf.string();
  const fs::path history = scratch / MB_HISTORY_DIR;
  int error = MB_ERROR_NO_ERROR;
  int version = -1;

  // A missing file is not an error and makes no snapshot.
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(0, version);
  EXPECT_FALSE(fs::exists(history));

  // First snapshot.
  WriteEsf(esf, 10, MBP_EDIT_FLAG);
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(1, version);
  EXPECT_EQ(1, CountSnapshots(history));

  // Unchanged file: no new snapshot, the newest version is returned.
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(1, version);
  EXPECT_EQ(1, CountSnapshots(history));

  // Forced snapshot, with the version number put into the note.
  EXPECT_EQ(MB_SUCCESS,
            mb_history_snapshot(0, esfname.c_str(), "test", "restore (v%d -> v1)", MB_HISTORY_MODE_FORCE, &version, &error));
  EXPECT_EQ(2, version);
  EXPECT_EQ(2, CountSnapshots(history));

  // Changed file: new snapshot with identical content.
  WriteEsf(esf, 20, MBP_EDIT_UNFLAG);
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(3, version);

  int nentry = 0;
  struct mb_history_entry *entries = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_history_read_manifest(0, esfname.c_str(), &nentry, &entries, &error));
  ASSERT_EQ(3, nentry);
  EXPECT_EQ(1, entries[0].version);
  EXPECT_STREQ("test", entries[0].program);
  EXPECT_STREQ("restore (v2 -> v1)", entries[1].note);
  EXPECT_STREQ("", entries[2].note);
  EXPECT_THAT(std::string(entries[2].snapshot), ::testing::StartsWith("line.mb88.esf.v0003_"));
  EXPECT_EQ(ReadFile(esf), ReadFile(history / entries[2].snapshot));
  EXPECT_EQ(static_cast<long>(fs::file_size(esf)), entries[2].size);

  // Rewriting the manifest keeps the entries given.
  EXPECT_EQ(MB_SUCCESS, mb_history_write_manifest(0, esfname.c_str(), 2, &entries[1], &error));
  mb_freed(0, __FILE__, __LINE__, (void **)&entries, &error);
  ASSERT_EQ(MB_SUCCESS, mb_history_read_manifest(0, esfname.c_str(), &nentry, &entries, &error));
  ASSERT_EQ(2, nentry);
  EXPECT_EQ(2, entries[0].version);
  EXPECT_EQ(3, entries[1].version);
  mb_freed(0, __FILE__, __LINE__, (void **)&entries, &error);

  fs::remove_all(scratch);
}

TEST(MbHistoryTest, SnapshotOncePerProcess) {
  const fs::path scratch = MakeScratchDir("once");
  const fs::path esf = scratch / "line.mb88.esf";
  const std::string esfname = esf.string();
  int error = MB_ERROR_NO_ERROR;
  int version = -1;

  WriteEsf(esf, 5, MBP_EDIT_FLAG);
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_ONCE, &version, &error));
  EXPECT_EQ(1, version);

  // The file changes, but this process already saved it once.
  WriteEsf(esf, 6, MBP_EDIT_FLAG);
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname.c_str(), "test", nullptr, MB_HISTORY_MODE_ONCE, &version, &error));
  EXPECT_EQ(0, version);
  EXPECT_EQ(1, CountSnapshots(scratch / MB_HISTORY_DIR));
  fs::remove_all(scratch);
}

// mb_esf_open() saves the existing esf file before rewriting it, once per
// process, and not at all when MB_HISTORY=off.
TEST(MbHistoryTest, EsfOpenSnapshot) {
  const fs::path scratch = MakeScratchDir("esfopen");
  const fs::path esf = scratch / "line.mb88.esf";
  const fs::path history = scratch / MB_HISTORY_DIR;
  WriteEsf(esf, 10, MBP_EDIT_FLAG);
  const std::string original = ReadFile(esf);

  mb_path esffile;
  snprintf(esffile, sizeof(esffile), "%s", esf.string().c_str());
  struct mb_esf_struct esf_struct;
  memset(&esf_struct, 0, sizeof(esf_struct));
  int error = MB_ERROR_NO_ERROR;

  // Reading only: no snapshot.
  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, true, MBP_ESF_NOWRITE, &esf_struct, &error));
  EXPECT_EQ(10, esf_struct.nedit);
  mb_esf_close(0, &esf_struct, &error);
  EXPECT_EQ(0, CountSnapshots(history));

  // Rewriting: the original file is saved first.
  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, true, MBP_ESF_WRITE, &esf_struct, &error));
  mb_esf_close(0, &esf_struct, &error);
  ASSERT_EQ(1, CountSnapshots(history));
  int nentry = 0;
  struct mb_history_entry *entries = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_history_read_manifest(0, esffile, &nentry, &entries, &error));
  ASSERT_EQ(1, nentry);
  EXPECT_EQ(original, ReadFile(history / entries[0].snapshot));
  mb_freed(0, __FILE__, __LINE__, (void **)&entries, &error);

  // Reopening the same file in this process for appending: no new snapshot.
  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, false, MBP_ESF_APPEND, &esf_struct, &error));
  mb_esf_close(0, &esf_struct, &error);
  EXPECT_EQ(1, CountSnapshots(history));

  // A different file with MB_HISTORY=off: no snapshot.
  const fs::path esf2 = scratch / "line2.mb88.esf";
  WriteEsf(esf2, 3, MBP_EDIT_FLAG);
  snprintf(esffile, sizeof(esffile), "%s", esf2.string().c_str());
  ASSERT_EQ(0, setenv(MB_HISTORY_ENV, "off", 1));
  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, true, MBP_ESF_WRITE, &esf_struct, &error));
  mb_esf_close(0, &esf_struct, &error);
  unsetenv(MB_HISTORY_ENV);
  EXPECT_EQ(1, CountSnapshots(history));

  fs::remove_all(scratch);
}

// mb_esf_open() saves a new esf file, which holds no edits, as the first
// version with the note "created", so the history records when editing
// began; not with MB_HISTORY=off.
TEST(MbHistoryTest, EsfOpenNewFile) {
  const fs::path scratch = MakeScratchDir("esfnew");
  const fs::path esf = scratch / "line.mb88.esf";
  const fs::path history = scratch / MB_HISTORY_DIR;

  mb_path esffile;
  snprintf(esffile, sizeof(esffile), "%s", esf.string().c_str());
  struct mb_esf_struct esf_struct;
  memset(&esf_struct, 0, sizeof(esf_struct));
  int error = MB_ERROR_NO_ERROR;

  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, false, MBP_ESF_WRITE, &esf_struct, &error));
  mb_esf_close(0, &esf_struct, &error);
  ASSERT_EQ(1, CountSnapshots(history));
  int nentry = 0;
  struct mb_history_entry *entries = nullptr;
  ASSERT_EQ(MB_SUCCESS, mb_history_read_manifest(0, esffile, &nentry, &entries, &error));
  ASSERT_EQ(1, nentry);
  EXPECT_EQ(1, entries[0].version);
  EXPECT_STREQ(MB_HISTORY_NOTE_CREATED, entries[0].note);
  EXPECT_STREQ("test", entries[0].program);
  const std::string created = ReadFile(history / entries[0].snapshot);
  EXPECT_EQ(MB_PATH_MAXLINE, static_cast<int>(created.size()));
  EXPECT_EQ(0, created.compare(0, 12, "ESFVERSION03"));
  mb_freed(0, __FILE__, __LINE__, (void **)&entries, &error);

  // A different new file with MB_HISTORY=off: no snapshot.
  const fs::path esf2 = scratch / "line2.mb88.esf";
  snprintf(esffile, sizeof(esffile), "%s", esf2.string().c_str());
  ASSERT_EQ(0, setenv(MB_HISTORY_ENV, "off", 1));
  ASSERT_EQ(MB_SUCCESS, mb_esf_open(0, "test", esffile, false, MBP_ESF_WRITE, &esf_struct, &error));
  mb_esf_close(0, &esf_struct, &error);
  unsetenv(MB_HISTORY_ENV);
  EXPECT_EQ(1, CountSnapshots(history));

  fs::remove_all(scratch);
}

// An esf file rewritten with the same edits differs only in the program,
// user, cpu and date lines of its header: no new snapshot. A changed ESF
// mode or changed edits make a new snapshot.
TEST(MbHistoryTest, SnapshotIgnoresEsfHeader) {
  const fs::path scratch = MakeScratchDir("esfheader");
  const fs::path esf = scratch / "line.mb88.esf";
  const fs::path history = scratch / MB_HISTORY_DIR;
  mb_path esfname;
  snprintf(esfname, sizeof(esfname), "%s", esf.string().c_str());
  int version = -1;
  int error = MB_ERROR_NO_ERROR;

  WriteEsf(esf, 5, MBP_EDIT_FLAG);
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname, "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(1, version);

  // Same edits, other program name in the header.
  WriteEsf(esf, 5, MBP_EDIT_FLAG, 0, "mbclean");
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname, "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(1, version);
  EXPECT_EQ(1, CountSnapshots(history));

  // Other ESF mode.
  WriteEsf(esf, 5, MBP_EDIT_FLAG, 1, "mbclean");
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname, "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(2, version);

  // Other edits.
  WriteEsf(esf, 6, MBP_EDIT_FLAG, 1, "mbclean");
  EXPECT_EQ(MB_SUCCESS, mb_history_snapshot(0, esfname, "test", nullptr, MB_HISTORY_MODE_DEFAULT, &version, &error));
  EXPECT_EQ(3, version);
  EXPECT_EQ(3, CountSnapshots(history));

  fs::remove_all(scratch);
}
