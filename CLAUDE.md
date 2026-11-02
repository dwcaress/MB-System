# MB-System: instructions for Claude Code

Shared project rules for every contributor. Personal paths, install prefixes and
preferences go in a git-ignored `CLAUDE.local.md` (see
MB-System_Claude_Code_Guide.md). Directory-level CLAUDE.md files may add rules.

## A. Project brief
- MB-System: open-source C/C++ (plus Perl/shell macros) package for processing
  and display of swath sonar bathymetry, amplitude, sidescan and related data.
  GPL v3. Repository: github.com/dwcaress/MB-System, default branch `master`.
- Core = MBIO library (src/mbio): format-independent I/O through mb_read,
  mb_get_all, mb_extract, mb_insert, mb_put_all, ... Each format has an
  mbr_* (read/write) module and an mbsys_* (data structure) module. Utilities
  (src/utilities: mbinfo, mblist, mbprocess, mbgrid, mbclean, ...) stay
  format-agnostic. Use `mbformat -L` for format ids; never guess them.
- Processing model: mbpreprocess -> editing (mbedit/mbeditviz/mbclean) ->
  mbprocess driven by `<file>.par`. Ancillary files (.par .inf .fbt .fnv .esf
  .nve .svp) are stable interfaces: change their formats only with maintainer
  approval.
- Upstream-synced code (ASK before editing any source file there, even for
  "obviously safe" warning or bug fixes; prefer a patch for the upstream team):
  - TRN: src/mbtrn, src/mbtrnav, src/mbtrnframe, src/mbtrnutils (maintained
    by a separate TRN team; local edits cause merge conflicts on each sync).
    CMake build-structure fixes in these directories are fine.
  - src/gsf (Leidos GSF library), third_party/, src/mbgrd2gltf vendored code
    (tinygltf, draco). Do not fix warnings in vendored code.
- Supported platforms: macOS (Homebrew), Linux (including glibc older than
  2.34), and Windows via MinGW-w64. Motif/X11 GUIs are Unix-only. MSVC is not
  a supported target; MSVC-friendly fixes are welcome only if they do not
  break MinGW-w64. Do not assume your own machine is the only platform.

## B. Build and test
- Two build systems are maintained in parallel: CMake (primary) and
  Autotools. New source files, libraries and programs must be added to BOTH
  the directory's CMakeLists.txt and Makefile.am (and configure.ac when a
  new directory or library is added). Do not regenerate configure,
  Makefile.in, aclocal.m4 etc. unless asked; the maintainer does that.
  Exception: the Qt prototype programs (src/qt-*, src/pointCloudEditor)
  are CMake-only (-DbuildQt=ON); never add them to the Autotools build.
  Header install lists differ today: src/mbio/Makefile.am installs the
  per-format headers (mbf_*.h, mbsys_*.h) via include_HEADERS, while the
  CMake HEADERS list holds only the core mb_*.h headers. Follow each build
  system's existing convention; do not change either without asking.
- CMake forbids in-source builds. Typical:
    mkdir -p build && cd build && cmake .. [options] && make -j8
  macOS/Homebrew GUIs: add -DmacosUseMbMesa=ON (uses the dwcaress/mbsystem
  tap kegs mb-mesa and mb-mesa-glu together; never mix with standard
  mesa-glu, which blanks the mbgrdviz 3D view).
- Common options: buildGUIs, buildOpenCV, buildGSF, buildTRN (all ON),
  buildQt, buildPCL, buildDeprecated (OFF), buildTests (ON),
  CMAKE_BUILD_TYPE (Release by default), otpsDir (defaults to
  /usr/local/src/otps). Use $HOME, not ~, inside -D arguments. Do not change
  the developer's configured option set without asking; check
  build/CMakeCache.txt for what is in use.
- Tests: the suite in test/ (CMake + Python, buildTests=ON) is maintained and
  must pass:  cd build && ctest --output-on-failure   (or make test)
  Never delete or weaken a test to make it pass. If a test is stale because
  of an intended change (e.g. -H usage text), update the assertion and say so.
- Windows portability check (when touching portable code or build files):
  MinGW-w64 cross-check, e.g. `x86_64-w64-mingw32-gcc/g++ -fsyntax-only` over
  compile_commands.json from a scratch build. State honestly what was and
  was not compiled (MSVC is normally not tested).
- Docs targets (need pandoc + weasyprint; htmldocs also groff, perl):
    cmake --build . --target changelog   -> ChangeLog.html/.pdf
    cmake --build . --target buildandinstall -> BuildAndInstall.pdf
    cmake --build . --target htmldocs    -> src/html/*.html/.pdf
- Formatting config is `_clang-format`, but most files predate it: match the
  surrounding code and never reformat code you are not changing.

## C. Code conventions
- C99-compatible C and C++11 (CMAKE_CXX_STANDARD 11). Some targets set
  C++17 for themselves (src/mbgrd2gltf, src/mbmesh, src/photo and the
  GoogleTest tests in test/mbio); follow the target's own setting. Match the
  surrounding file's style, naming, comment density and idiom. Minimal diffs.
- MBIO API pattern (see src/mbio/mb_define.h, mb_status.h): functions take
  `int verbose, ..., int *error`, return `int status` (MB_SUCCESS /
  MB_FAILURE), set *error to MB_ERROR_* codes, and keep the verbose
  entry/exit debug-print blocks like their neighbours.
- Memory: use mb_mallocd / mb_reallocd / mb_freed as neighbours do. Check the
  returned status before using the buffer (on failure mb_reallocd leaves
  the pointer NULL; do not keep using old sizes or counts); free on every
  error path.
- Strings: bound every copy from optarg, files or environment (snprintf,
  "%1023s"-style sscanf widths). No unbounded strcpy/strcat/sprintf into
  fixed buffers. Check getenv() and strstr() results for NULL.
- Data read from files: validate beam/sample/record counts against the
  destination array sizes before copying.
- Format drivers: copy the closest sibling module; never assume struct
  packing; do not change mbsys_* struct layouts without checking every
  reader/writer. A new format touches, in src/mbio: mb_format.h (MBF_ id;
  MB_SYS_ id if it is a new system; MB_FORMATS must equal the number of
  registered formats), mb_format.c (a branch in BOTH mb_format_register()
  and mb_format_info(); mb_get_format() only if the format has a
  distinctive filename prefix/suffix), the new mbr_*.c / mbsys_*.c and
  headers, CMakeLists.txt (SRC) and Makefile.am (libmbio_la_SOURCES and
  include_HEADERS), and the format list in the MBIO man page
  (src/man/man3/mbio.3). Some MBF_ ids (MBF_NONE, MBF_RESONS8K,
  MBF_SB2000RW, MBF_SBATPROC) are defined but intentionally unregistered;
  the pseudo-formats MBF_DATALIST and MBF_IMAGELIST are handled only in
  mb_format_info().
- Endianness: legacy drivers swap inside `#ifdef BYTESWAPPED` using
  mb_swap_short/mb_swap_int (macros, mb_swap.h) and mb_swap_float/double/
  long (functions). CMake defines BYTESWAPPED when CMAKE_C_BYTE_ORDER is
  LITTLE_ENDIAN. Newer drivers use mb_get_binary_*/mb_put_binary_* with an
  explicit bool `swapped` argument. Follow the closest sibling driver and
  never assume host byte order.
- Record kinds, errors, notices (src/mbio/mb_status.h): record kinds are
  MB_DATA_* (MB_DATA_NONE = 0, real kinds start at 1). MB_SUCCESS = 1,
  MB_FAILURE = 0. Fatal MB_ERROR_* codes are positive, non-fatal ones are
  negative (reading and writing can continue). The notice_list[] layout in
  mb_io_struct is: kind k at slot k, non-fatal error e at
  MB_DATA_KINDS - e, problem p at MB_DATA_KINDS - MB_ERROR_MIN + p, size
  MB_NOTICE_MAX; notice_msg[] must have one entry per slot. When adding a
  record kind, update MB_DATA_KINDS, notice_msg[] and anything sized from
  them (e.g. per-kind counter arrays) together.
- Beam flags: always use the mb_beam_* macros from mb_status.h, never raw
  bit tests. Bit 0 (0x01) = flagged, bit 1 (0x02) = selected; they are
  exclusive. A null beam (no detection) is exactly 0x01 (MB_FLAG_NULL ==
  MB_FLAG_FLAG). Higher bits record the reason (manual 0x04, filter 0x08,
  filter2 0x10, multipick 0x20, interpolate 0x40, sonar 0x80); the original
  meanings of 0x10/0x20 (IHO uncertainty) and 0x40 (footprint too large)
  are deprecated. The "usable" macros exclude null beams and flagged
  interpolated beams; "usable2" also excludes flagged multipick beams.
- Time: time_i[7] = year, month, day, hour, minute, second, microsecond
  (mb_get_time() accepts years 1930-3000; no leap-second handling).
  time_d = decimal seconds since 1970-01-01 00:00:00.
- Longitude range: lonflip -1 = -360..0, 0 = -180..180, 1 = 0..360
  (mb_apply_lonflip). bounds[4] = lon min, lon max, lat min, lat max.
- User defaults: mb_defaults() sets the built-in defaults (format 0,
  pings 1, lonflip 0, speedmin 0.0, timegap 1.0), then reads only the
  "lonflip:" and "timegap:" lines of ~/.mbio_defaults (the keys written by
  mbdefaults); other values are not read from that file. Do not hard-code
  personal settings.
- Sign conventions (documented in mb_process.h): athwartship offsets are
  positive to starboard, fore-aft positive forward, vertical and
  sensordepth positive down; roll positive starboard up, pitch positive
  forward up (degrees), heave positive up (meters). The man page does not
  state the acrosstrack sign: do not assume one, check the code.
- Platform model (mb_define.h mb_platform_*, mb_io.h mb_platform_struct):
  sensor offsets (position x/y/z, attitude heading/roll/pitch), time
  latency and per-sensor flip-signs (field names: heading_flipsign_heading,
  attitude_flipsign_roll, attitude_flipsign_pitch - copy names from the
  header) live in the platform model, read from a platform file by
  mb_platform_read() and created by mbmakeplatform. mbpreprocess loads it
  with --platform-file (KMALL data without one: the model is taken from the
  installation datagram). mb_preprocess() calls the format's own
  mb_io_preprocess, or mb_preprocess_generic() (lever-arm repositioning and
  attitude rotation) if none is registered. Lever-arm logic belongs here,
  not in generic read/write APIs.
- Keyword matching: when using strncmp() to match a keyword, pass the
  keyword's exact length, and keep reader and writer keyword names
  identical (e.g. platform file keywords in mb_platform_read/write).
- Navigation rule: lon/lat and sensordepth passed through mb_read/mb_write/
  mb_get*/mb_put*/mb_extract*/mb_insert* are those of the MAPPING SENSOR
  (sonar, lidar, camera), with acrosstrack/alongtrack relative to it - not
  the vessel reference point. Formats storing another point convert inside
  their own extract/insert functions and keep on-disk records vendor-
  compatible. Do not add lever-arm hooks to generic APIs.
- Units: be explicit about degrees vs radians, epoch seconds vs Julian day,
  depth positive down, heading/roll/pitch sign conventions.
- Portability: test `_WIN32` (never `__WIN32`/`WIN32`); `_WIN32` is also
  defined for MinGW-w64, so MSVC-only code tests `_MSC_VER`. No VLAs in C++.
  Guard POSIX-only calls (mkdir mode, gethostname, sockets, unistd.h). Any
  __declspec(dllexport) seen by MinGW disables its automatic export of all
  other DLL symbols. Keep line endings as they are (check with `file`).
- Command-line options: utilities support short and long options
  (getopt_long). When adding/changing an option, update the option string,
  long_options[], the -H usage text, the man page, and any test that checks
  usage output.
- No new third-party dependencies without maintainer approval.
- No new compiler warnings. Fix them; do not silence them.
- Never hard-code personal absolute paths (/Users/<name>, /home/<name>) in
  repository files, commit messages or ChangeLog text.

## D. Working rules
- Investigate before editing: confirm the file is compiled (CMakeLists.txt,
  Makefile.am), git grep symbols and error strings, read the nearest
  sibling implementation and the man page, check `git log -S` / blame,
  reproduce with real data if possible. For non-trivial changes, propose the
  patch and verification plan and wait for approval.
- Large reviews/audits: report findings first (in batches), fix only what
  the developer approves.
- Do not trust remembered function names, struct fields or options: grep
  the source or run `<program> -H`.
- The working tree often holds deliberate uncommitted work in progress. Do
  not revert, "clean up" or reformat changes you did not make.
- Verify with real builds. After multi-agent or multi-worktree edits, do one
  full build at the end; agent self-reports are not proof.
- Report faithfully: state what was built and tested (platforms, ctest
  result, data files) and what was NOT tested. Never claim otherwise.
- Work on scratch copies of data; stale .inf/.fbt/.fnv/.par/.esf files next
  to data can hide bugs. Never modify or delete original survey data.
- GMT reads gmt.conf from the current directory first; remove stray ones.
- Session notes, investigation logs and standalone patches do not go in the
  repository tree (use a directory outside it; see CLAUDE.local.md).
- Never run without explicit approval: git push --force, git reset --hard,
  git clean, rm -rf, sudo, make uninstall. Ask before deleting anything.

## E. Documentation and release notes
- Man pages: src/man/man1/*.1 (programs), src/man/man3 (library). Keep
  SYNOPSIS/OPTIONS/EXAMPLES consistent with the code and -H text.
- Generated files, never hand-edit: ChangeLog.html/.pdf,
  BuildAndInstall.pdf, src/html/*, configure, Makefile.in and other
  Autotools output.
- ChangeLog.md and the version number are normally updated by the
  maintainer. Other contributors: unless asked, put ChangeLog-style text in
  the pull request description instead of editing ChangeLog.md.
- When updating ChangeLog.md:
  - Version id = latest beta tag published on GitHub + 1:
      git ls-remote --tags https://github.com/dwcaress/MB-System.git | grep -oE '[0-9]+\.[0-9]+\.[0-9]+beta[0-9]+$' | sort -V | tail -1
    (The full URL works whatever the remotes are called, e.g. in a fork
    where origin has no tags. Requires three version components: old tags
    such as 5.79beta45 would otherwise sort above 5.8.3betaNN.) Do not
    trust the local ChangeLog heading; it may name an unreleased beta.
  - Date = today, moved forward on every edit even if the id is unchanged.
  - Put the same id/date in: the version list near the top, the release's
    "#### <id> (<Month D, YYYY>)" heading, and MB_VERSION / MB_VERSION_DATE
    ("D Month YYYY") in src/mbio/mb_define.h; put the same id (no date) in
    the AC_INIT version in configure.ac.
  - Entries are prose paragraphs per component, following recent entries:
    "Program mbcopy (src/utilities/mbcopy.cc): <what changed and why>."
  - AI disclosure: a release entry containing Claude-assisted work has ONE
    statement, after its Claude-assisted paragraphs, exactly in this form
    (using the model actually used): "The above work was done with the
    assistance of the AI coding assistant <model name> (Anthropic, model
    <model-id>), operating as Claude Code under developer supervision and
    review." Notes for work done without Claude go below it, before the
    `---` separator; add new Claude-assisted paragraphs above it.
  - Then regenerate ChangeLog.html/.pdf (changelog target) and commit them
    together with ChangeLog.md.
- src/html (htmldocs target) rewrites many large files: ask before
  regenerating or staging it.

## F. Commits and pull requests (only when the developer asks)
- Never commit directly on master; use a topic branch. Stage explicit paths
  only (never `git add -A` / `.`); exclude build products, data, ancillary
  files, editor files and anything under .claude/ except shared config.
- Message: short plain subject line (e.g. "Build system fix for glibc older
  than 2.34"). Body: per-component prose paragraphs like the ChangeLog
  entry; then how it was verified and what was not tested; then the AI
  disclosure statement for Claude-assisted work; then the Co-Authored-By
  trailer for the Claude model.
- Show the message and `git diff --cached --stat` and wait for approval
  before committing. Pushing, opening or commenting on pull requests and
  issues each need explicit approval.
- When evaluating an external pull request, check it on every platform it
  could affect (e.g. a Windows change under MinGW-w64 as well as MSVC
  intent); adopting part of a PR is fine if the commit says so.
- Never: --force, --no-verify, amend published commits, rewrite history on
  shared branches.
