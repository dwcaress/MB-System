/*--------------------------------------------------------------------
 *    The MB-system:  mbedit_qt_session_test.cc  8/11/2026
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
/*
 * Several editing sessions in one process: each must write the same edit save
 * file as when it runs alone, also when the sessions run at the same time, and
 * none may change what the others or the process see. Arguments: the MBIO format
 * and two or more swath files. They are edited, so they must be scratch copies.
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbe_core.h"

namespace {

int checks = 0, failures = 0;
int format = 0;

#define CHECK(condition)                                  \
  do {                                                    \
    checks++;                                             \
    if (!(condition)) {                                   \
      failures++;                                         \
      printf("FAIL line %d: %s\n", __LINE__, #condition); \
    }                                                     \
  } while (0)

const int ROUNDS = 12;

std::string read_file(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

/* the edits, without the dated header */
std::string edits_of(const std::string &path) {
  const std::string data = read_file(path + ".esf");
  return data.size() > MB_PATH_MAXLINE ? data.substr(MB_PATH_MAXLINE) : std::string();
}

bool open_session(mbe::Session &session, const std::string &path) {
  session.on_log = [](const std::string &) {};
  session.on_error = [](const std::string &a, const std::string &b, const std::string &c) { printf("error: %s %s %s\n", a.c_str(), b.c_str(), c.c_str()); };
  return session.open(path, format, false, mbe::OUTPUT_EDIT) == MB_SUCCESS;
}

void edit_round(mbe::Session &session, int round) {
  session.step(session.settings.plot_size / 2);
  if (round % 3 == 0) {
    session.flagView();
  }
  else if (round % 3 == 1) {
    session.step(1);
    session.unflagView();
    session.flagView();
  }
  else {
    session.filters.medianspike = true;
    session.filterForward();
    session.filters.medianspike = false;
  }
}

void run_alone(const std::string &path) {
  mbe::Session session;
  if (open_session(session, path)) {
    for (int round = 0; round < ROUNDS; round++)
      edit_round(session, round);
    session.closeSave();
  }
}

}  // namespace

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: mbedit_qt_session_test format file file ...\n");
    return 2;
  }
  format = atoi(argv[1]);
  const std::vector<std::string> files(argv + 2, argv + argc);

  /* what a program with several sessions at the same time has to do */
  int error = MB_ERROR_NO_ERROR;
  mb_mem_list_disable(0, &error);

  const char *history = getenv(MB_HISTORY_ENV);
  const std::string history_before = history != nullptr ? history : "(unset)";

  std::vector<std::string> alone;
  for (const std::string &file : files) {
    run_alone(file);
    alone.push_back(edits_of(file));
    CHECK(!alone.back().empty());
  }

  /* all open at the same time, one thread, edits in turn */
  {
    std::vector<std::unique_ptr<mbe::Session>> sessions;
    for (const std::string &file : files) {
      sessions.emplace_back(new mbe::Session);
      CHECK(open_session(*sessions.back(), file));
    }
    for (int round = 0; round < ROUNDS; round++)
      for (auto &session : sessions)
        edit_round(*session, round);
    for (auto &session : sessions)
      session->closeSave();
    for (size_t i = 0; i < files.size(); i++)
      CHECK(edits_of(files[i]) == alone[i]);
  }

  /* each in its own thread */
  for (int loop = 0; loop < 10; loop++) {
    std::vector<std::thread> threads;
    for (const std::string &file : files)
      threads.emplace_back([file] { run_alone(file); });
    for (std::thread &thread : threads)
      thread.join();
    for (size_t i = 0; i < files.size(); i++)
      CHECK(edits_of(files[i]) == alone[i]);
    history = getenv(MB_HISTORY_ENV);
    CHECK((history != nullptr ? history : "(unset)") == history_before);
  }

  /* the program name goes into the lock file and the edit save file */
  {
    mbe::Session session;
    session.program_name = "mbedit_qt_session_test";
    CHECK(open_session(session, files[0]));
    CHECK(read_file(files[0] + ".lck").find("Locking Program: mbedit_qt_session_test") != std::string::npos);
    edit_round(session, 0);
    session.closeSave();
    CHECK(read_file(files[0] + ".esf").find("Program: mbedit_qt_session_test") != std::string::npos);
    CHECK(read_file(files[0] + ".esf.stream").find("Program: mbedit_qt_session_test") != std::string::npos);
  }

  /* the messages reach the callback */
  {
    mbe::Session session;
    std::string messages;
    open_session(session, files[0]);
    session.on_log = [&messages](const std::string &text) { messages += text; };
    session.closeDiscard();
    CHECK(messages.find("closed") != std::string::npos);
  }

  printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
