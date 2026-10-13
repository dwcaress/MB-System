# Using Claude Code for MB-System Development

*Draft, 13 October 2026. Originally written by Christian Ferreira (revisions 1 to 6);
revised by David Caress with Claude Code to match project practice.*

This guide explains how MB-System developers use Claude Code, Anthropic's AI coding
assistant, on their own machines: how the project instructions are organized, how to
set up a build Claude can work with, how to work with Claude safely, and how
Claude-assisted work is reviewed, verified and disclosed. It covers the Claude desktop
app (Code tab), the `claude` terminal command, and Remote Control from the Claude app
or a phone.

The rules Claude actually follows are in the top-level **`CLAUDE.md`**. This guide
explains them and does not repeat them; if the two ever disagree, `CLAUDE.md` wins.

## Contents
1. Principles
2. How project instructions are organized
3. Example setup: macOS and Homebrew
4. Other platforms
5. Ways to run Claude Code
6. Worktrees
7. Working loop
8. Documentation, ChangeLog and disclosure
9. Commits and pull requests
10. Permissions and guardrails
11. Patterns that have worked
12. Troubleshooting
13. First-run checklist
14. Sources

## 1. Principles

- **A human developer supervises and reviews every change.** Claude proposes, edits,
  builds and tests; the developer decides what is kept, committed and pushed.
- **Claude never commits, pushes, opens pull requests or posts on GitHub without
  explicit approval** for that action.
- **Verification is real.** Changes are built and tested (ctest, real data, other
  platforms where relevant), and reports say plainly what was *not* tested.
- **AI assistance is disclosed** in the ChangeLog and in commit messages (section 8).
- **Upstream-synced code is respected.** TRN (`src/mbtrn*`), GSF (`src/gsf`) and
  vendored third-party code are edited only after asking (see `CLAUDE.md` section A).
- **Code execution and data stay local.** Claude Code runs on your machine. The
  conversation is sent to Anthropic, so do not paste proprietary or client survey
  data into prompts.

## 2. How project instructions are organized

Claude Code reads instruction files at the start of every session:

| File | Committed | Contents |
|---|---|---|
| `CLAUDE.md` (top level) | Yes | Shared project rules: architecture, build and test policy, conventions, upstream-synced code, ChangeLog and commit rules |
| `CLAUDE.local.md` (top level) | **No** (git-ignored) | Your own setup: paths, install prefix, remotes, build options, where your notes go, your preferences |
| `src/<dir>/CLAUDE.md` | Yes | Optional rules that load only when Claude works in that directory |
| `~/.claude/CLAUDE.md` | No | Your rules for all projects |
| `.claude/settings.json` | Yes | Shared permission rules (deny rules for destructive commands) |
| `.claude/settings.local.json` | **No** (git-ignored) | Your own permission allow-list, built up as you approve commands |

Notes:
- Put only what Claude cannot work out from the code. Keep each file under about 200
  lines (`wc -l CLAUDE.md`).
- Your `CLAUDE.local.md` overrides the shared file for your sessions. For example, a
  developer who builds with `-DbuildTRN=OFF` says so there, so Claude knows not to
  claim TRN was tested.
- Example `CLAUDE.local.md` files exist for the maintainer's setup and for the macOS
  sandbox setup in section 3. Ask either author for a copy.
- `/context` in a session shows which files were loaded. After editing `CLAUDE.md`
  mid-session, tell Claude to re-read it.
- Claude Code also keeps its own memory across sessions. It is useful for personal
  context, but project rules that every contributor needs belong in `CLAUDE.md`.
- The repository `.gitignore` excludes `CLAUDE.local.md`,
  `.claude/settings.local.json`, `.claude/worktrees/` and other runtime files. Never
  commit them.

## 3. Example setup: macOS and Homebrew

This is one complete, tested setup: macOS Tahoe 26 or later, Apple Silicon, Homebrew
only, everything under `~/sandbox` so no `sudo` is needed. Other layouts work just as
well (for example the maintainer installs into `/usr/local`); record yours in
`CLAUDE.local.md`.

### Prerequisites

Homebrew 6 requires third-party taps to be trusted before their code runs, hence
`brew trust`.

```bash
brew update
brew tap dwcaress/mbsystem
brew trust dwcaress/mbsystem
brew install cmake pkg-config fftw gdal netcdf gmt proj libx11 libxt openmotif opencv qt@6 vtk
brew install dwcaress/mbsystem/mb-mesa
brew install dwcaress/mbsystem/mb-mesa-glu
cpan Parallel::ForkManager     # needed by mbm_multiprocess, mbm_multicopy, mbm_multidatalist
```

The tap supplies `mb-mesa` and `mb-mesa-glu`, private kegs with a pinned Mesa version.
`openmotif`, `libx11` and `libxt` are needed for the GUI programs, `opencv` for the
photomosaicing tools, and `qt@6` and `vtk` only with `-DbuildQt=ON`.

Optional, for the documentation targets:

```bash
brew install pandoc weasyprint groff    # htmldocs also needs perl
```

Do not keep MacPorts (`/opt/local`) on your `PATH`: it can pull in the wrong GMT,
GDAL, PROJ or netCDF.

### Directory layout

```text
~/sandbox/MB-System            # git clone
~/sandbox/MB-System/build      # CMake build dir (in-source builds are refused)
~/sandbox/mbsystem             # install prefix
~/sandbox/otps                 # OTPS tidal prediction install
~/sandbox/testdata             # small sample files per format (read-only)
~/sandbox/out                  # scratch outputs and session notes
~/sandbox/MB-System-base       # optional: git worktree of master for baseline comparison
~/sandbox/mbsystem-base        # optional: baseline install prefix
```

Keep the clone outside iCloud Drive, Dropbox and similar sync folders; they can
corrupt `.git`.

### Git remotes

Contributors work from a fork:

```bash
git clone git@github.com:<you>/MB-System.git ~/sandbox/MB-System
cd ~/sandbox/MB-System
git remote add upstream https://github.com/dwcaress/MB-System.git
git fetch upstream
git checkout -b dev/<topic> upstream/master
```

Maintainers with write access work on topic branches of the main repository and merge
by pull request.

### Configure, build, test, install

```bash
cd ~/sandbox/MB-System
mkdir -p build && cd build
cmake .. -DmacosUseMbMesa=ON \
         -DMBMESA_PREFIX=/opt/homebrew/opt/mb-mesa \
         -DMBMESAGLU_PREFIX=/opt/homebrew/opt/mb-mesa-glu \
         -DCMAKE_INSTALL_PREFIX=$HOME/sandbox/mbsystem \
         -DotpsDir=$HOME/sandbox/otps
make -j"$(sysctl -n hw.ncpu)" 2>&1 | tee build.log
ctest --output-on-failure
make install                  # prefix is under $HOME: no sudo
```

Use `$HOME`, not `~`, inside `-D` arguments: the shell does not expand `~` in the
middle of a word. If an earlier `sudo make install` left root-owned files, run
`sudo chown -R $USER ~/sandbox/mbsystem` once yourself.

### What the flags do

| Flag | Purpose |
|---|---|
| `-DCMAKE_INSTALL_PREFIX` | Install location; the default is `/usr/local` |
| `-DotpsDir` | OTPS location; defaults to `/usr/local/src/otps`, so omitting it silently uses the wrong path |
| `-DmacosUseMbMesa=ON` | Use the private `mb-mesa` and `mb-mesa-glu` kegs for OpenGL |
| `-DMBMESA_PREFIX`, `-DMBMESAGLU_PREFIX` | Keg locations. If omitted, CMake asks `brew --prefix`; it stops with an error if a prefix does not exist |
| `-DbuildTRN=OFF` | Optional: skip the TRN directories. If you use it, say so in `CLAUDE.local.md` |

`mb-mesa-glu` is built against `mb-mesa`'s own libGL. Mixing it with the standard
Homebrew GLU loads two libGL images in one process, GLU calls silently do nothing, and
the mbgrdviz 3D view goes blank. Always use the two kegs together.

### Other options

| Option | Default | Use |
|---|---|---|
| `CMAKE_BUILD_TYPE` | Release | `Debug` for lldb sessions (use a separate build dir) |
| `BUILD_SHARED_LIBS` | ON | Shared vs static libraries |
| `buildGUIs` | ON | OFF skips mbview, mbedit, mbgrdviz, mbeditviz, mbnavedit, mbvelocitytool |
| `buildOpenCV` | ON | OFF skips the photomosaicing tools |
| `buildGSF` | ON | OFF removes GSF format support |
| `buildTRN` | ON | OFF skips the TRN tools |
| `buildTests` | ON | Configures the `test/` suite run by `ctest` |
| `buildQt` | OFF | Qt GUIs (VTK 9.3 or later) |
| `buildPCL` | OFF | Point-cloud-library tools |
| `buildDeprecated` | OFF | Deprecated programs |

CMake always writes `build/compile_commands.json`, which is also used for the
MinGW-w64 cross-check (section 4).

### Environment after install

```bash
export PATH=$HOME/sandbox/mbsystem/bin:$PATH
gmt gmtset GMT_CUSTOM_LIBS $HOME/sandbox/mbsystem/lib/mbsystem.dylib
```

- CMake sets the RPATH; do not set `DYLD_LIBRARY_PATH` (macOS strips it anyway).
- GMT needs the MB-System plug-in for `gmt mbcontour`, `mbswath`, `mbgrdtiff` and
  `mbgrd2obj`.
- GMT reads `gmt.conf` from the current directory first. A crashed script can leave
  an orphan `gmt.conf` that silently changes later runs.
- Another MB-System on your `PATH` (a Homebrew `mbsystem`, an old `/usr/local`
  install) can shadow your build. Check `which mbinfo`.

### Optional baseline build

For before/after comparisons, build an unmodified `master` in a separate worktree with
the same options and its own prefix:

```bash
git worktree add ../MB-System-base upstream/master
cd ../MB-System-base && mkdir -p build && cd build
cmake .. <same options> -DCMAKE_INSTALL_PREFIX=$HOME/sandbox/mbsystem-base
make -j"$(sysctl -n hw.ncpu)" && make install
```

Then run a tool from both prefixes on the same scratch copy of the data and diff the
text output. Treat the baseline worktree as read-only: do not edit it or start a
Claude session in it.

## 4. Other platforms

- **Linux**: install the equivalent distribution packages (see `BuildAndInstall.md`)
  and configure without the macOS Mesa options. The build must keep working on older
  glibc (before 2.34) as well as current distributions.
- **Windows (MinGW-w64)**: supported for the non-GUI programs. Changes to portable
  code or build files should be checked with a MinGW-w64 cross-compile. A quick
  check from macOS or Linux: from a scratch build, take each command in
  `compile_commands.json`, run it with `x86_64-w64-mingw32-gcc`/`g++ -fsyntax-only`,
  pass host include directories (e.g. `/opt/homebrew/include`) as `-idirafter`, and
  add `src/mbaux`. Claude can script this.
- **MSVC**: not a supported target and usually not testable. Accept MSVC fixes only
  when they do not break MinGW-w64, and say in the commit that MSVC was not tested.
- **Autotools**: still maintained alongside CMake. New files go into `Makefile.am`
  as well as `CMakeLists.txt`; regenerating `configure` is left to the maintainer.

## 5. Ways to run Claude Code

| Mode | How | Good for |
|---|---|---|
| Desktop app, Code tab | Open the repository folder in the Claude desktop app | Day-to-day work; diff, terminal and browser panes; app-managed worktrees for parallel sessions |
| Terminal | `cd ~/sandbox/MB-System && claude` | Working alongside your own shell |
| Remote Control | `claude remote-control`, or `/remote-control` (`/rc`) inside a session | Steering a session on your Mac from the Claude app or a phone, e.g. during long builds |

Requirements: a Pro, Max, Team or Enterprise plan, signed in with a claude.ai account
(`/login`). The first run in a directory asks you to trust it.

### Remote Control notes

Remote Control connects claude.ai/code or the Claude mobile app to a session running
on your Mac. Execution and file access stay local; the transcript passes through
Anthropic's servers. Your Mac must stay awake and the `claude` process running.

- Suggested start: `claude remote-control --name "MBS: <topic>" --spawn same-dir`.
- `--spawn` accepts `same-dir` (default), `worktree` and `session`; `--capacity N`
  limits concurrent sessions. Other flags change between versions: check
  `claude remote-control --help`.
- Unavailable if `ANTHROPIC_BASE_URL` points elsewhere, with Bedrock, Vertex or
  Foundry, or with `CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC` set.
- Run `caffeinate -dims` in another terminal for long jobs; a full build stalls while
  the Mac sleeps.
- From the phone, `/compact`, `/clear`, `/context`, `/model` and `/rename` work;
  `/resume` and `/plugin` are local only. Permission prompts wait until you answer.
  You can attach a screenshot, e.g. of a bad swath plot.

## 6. Worktrees

Several mechanisms create git worktrees. Each is a separate checkout that needs its
own build directory.

| | Desktop app / Remote Control worktrees | Baseline worktree (optional) |
|---|---|---|
| Created by | Claude Code, per session (`.claude/worktrees/`, branch `claude/<name>`) | You, once, with `git worktree add` |
| Purpose | Let parallel sessions edit without colliding | Unmodified `master` build for comparisons |
| Claude may edit it | Yes | Never |

Rules:
- Two sessions installing into the same prefix overwrite each other's files. Install
  from one session at a time, or give each its own prefix. Do not install from a
  worktree into the prefix you use for real data processing without deciding to.
- The desktop app can merge the base branch into a worktree for you; ask Claude to
  sync when a worktree falls behind.
- When several sessions or agents have edited code, do one full build of the combined
  result. Each agent's report that its own edit is fine is not proof that the whole
  build works.
- Clean up finished worktrees; uncommitted work in a removed worktree is lost.

## 7. Working loop

1. **Investigate.** Use plan mode (Shift+Tab) for read-only work. Claude confirms
   the file is compiled, finds call sites, reads the nearest sibling driver or
   program and its man page, checks history (`git log -S`, `git blame`), reproduces
   with real data where possible, and proposes a minimal patch plus a verification
   plan.
2. **Approve, then edit.** For audits or large reviews, ask for findings first, in
   batches (e.g. five files at a time), and approve fixes per batch.
3. **Build and test.** Full build with no new warnings, then `ctest
   --output-on-failure`. If a test fails because of an intended change (e.g. new
   usage text), the test is updated and the reason stated; tests are never deleted
   or weakened to pass.
4. **Check real data.** Run the affected programs on a scratch copy of real files.
   For format work, compare against an independent source (another format from the
   same survey, vendor-computed soundings). Optionally diff against a baseline
   build.
5. **Check other platforms** when portable code or build files changed (section 4).
6. **Document** (section 8), then **commit** when you decide to (section 9).

Keep session notes, investigation logs and standalone patches outside the repository
tree (the location goes in `CLAUDE.local.md`).

## 8. Documentation, ChangeLog and disclosure

- Every user-visible change updates the program's man page (`src/man/man1`), its
  `-H` usage text and its long-option table together.
- `ChangeLog.md` and the version number (`MB_VERSION` and `MB_VERSION_DATE` in
  `src/mbio/mb_define.h`) are normally maintained by the maintainer. Contributors put
  ChangeLog-style text in their pull request description instead, unless asked.
- The version and date rules, entry style and regeneration steps are in `CLAUDE.md`
  section E. In short: the next beta after the latest one published on GitHub,
  today's date, the same in all three places, prose paragraphs per component, and
  `ChangeLog.html`/`.pdf` regenerated (`changelog` target) and committed with
  `ChangeLog.md`.
- `src/html` is regenerated with the `htmldocs` target, which rewrites many large
  files. Do it only on purpose.
- **Disclosure.** Claude-assisted work is disclosed in two places:
  - the ChangeLog: one closing paragraph per release entry, for example *"This work
    was done with the assistance of the AI coding assistant Claude Opus 5.5
    (Anthropic, model claude-opus-5-5), operating as Claude Code under developer
    supervision and review, ..."*, naming the model actually used;
  - the commit message: the same paragraph, plus the `Co-Authored-By:` trailer that
    Claude Code adds.

## 9. Commits and pull requests

- Commit only on a topic branch, never directly on `master`. Stage explicit paths
  only.
- Commit messages follow the project style: a short plain subject line, then prose
  paragraphs per component (matching the ChangeLog entry), then how the change was
  verified and what was not tested, then the disclosure paragraph.
- Claude shows the message and `git diff --cached --stat` and waits for approval.
  Pushing, opening a pull request and commenting on GitHub each need separate
  approval.
- Contributors working from a fork rebase on `upstream/master` before opening a pull
  request. If the rebase changes anything, rebuild and re-verify before pushing.
- When reviewing someone else's pull request, Claude can test it on platforms the
  author could not (e.g. MinGW-w64 for an MSVC-oriented change). Adopting part of a
  pull request is fine; say so in the commit message and credit the author.

## 10. Permissions and guardrails

Shared deny rules can go in `.claude/settings.json` (committed). Allow rules are
personal and accumulate in `.claude/settings.local.json` as you approve commands.
Starting point for the shared file; check the current permissions documentation for
exact syntax:

```json
{
  "permissions": {
    "deny": [
      "Bash(git push --force:*)", "Bash(git push -f:*)",
      "Bash(git reset --hard:*)", "Bash(git clean:*)",
      "Bash(sudo:*)", "Bash(rm -rf:*)", "Bash(make uninstall:*)"
    ]
  }
}
```

- Deny rules take precedence over allow rules. Matching is by command prefix, so
  `CLAUDE.md` also states these rules in words to cover variants.
- Avoid "bypass permissions" modes on a machine that holds survey data.
- Review each permission prompt; approving "always allow" adds a rule to your local
  settings file.

## 11. Patterns that have worked

- **Audits in batches.** Memory-safety reviews of `src/utilities` and the GUI editors
  went file by file, smallest first, five files per batch, findings reported before
  fixes. Recurring bug families (unchecked reallocations, unbounded string copies,
  unvalidated counts from files) were then searched for everywhere.
- **Evaluating an external pull request.** The Windows build pull request (#1603) was
  cross-compiled with MinGW-w64, its platform-independent parts adopted, and the
  MSVC-only parts that broke MinGW-w64 left out, all explained in the commit.
- **Validating format changes against independent data.** kmall changes were checked
  against GSF from the same survey and against Kongsberg's own soundings after
  raytracing.
- **Recording design decisions.** When a session settles a design question (for
  example: navigation passed through MBIO is that of the mapping sensor, not the
  vessel reference point), add it to `CLAUDE.md` so later sessions and other
  contributors follow it.
- **Stale tests vs regressions.** When tests fail after an intended change, compare
  the actual program output with the test's expectation before assuming a
  regression.

## 12. Troubleshooting

| Symptom | Likely cause and fix |
|---|---|
| Test failures after changing options or usage text | Test assertions expect the old text; update them and explain |
| CMake fatal error about `MBMESA_PREFIX` or `MBMESAGLU_PREFIX` | Kegs not installed; install from the `dwcaress/mbsystem` tap |
| `brew install dwcaress/mbsystem/...` refuses to run | Tap not trusted; `brew trust dwcaress/mbsystem` |
| "In-source builds are not allowed" | cmake ran in the repo root; remove `CMakeCache.txt` and `CMakeFiles` there |
| OTPS tools cannot find OTPS | `-DotpsDir` omitted; reconfigure with it |
| A directory literally named `~` appears | `~` used inside a `-D` argument; use `$HOME` |
| Wrong libraries picked up | `/opt/local` on `PATH` or a stale cache; rebuild from an empty build dir |
| `mbinfo` reports a different version | Another install on `PATH`; call your prefix's `bin/mbinfo` |
| `gmt mbswath` not found | `GMT_CUSTOM_LIBS` not set to your `mbsystem.dylib` |
| mbgrdviz 3D view blank | Mixed Mesa; use `mb-mesa` and `mb-mesa-glu` together |
| Root-owned files in your prefix | Earlier `sudo make install`; `sudo chown -R $USER <prefix>` |
| Claude ignores a rule | Check `/context`; ask Claude to re-read `CLAUDE.md` |
| Nothing happens during a long build | A permission prompt is waiting, or the Mac is asleep |
| `claude remote-control` exits with an error | Not signed in with a claude.ai account; `/login` |
| Session not listed in the app | Different account, or a base URL override is set |

## 13. First-run checklist

1. Install the prerequisites for your platform (section 3 or 4).
2. Clone the repository (or your fork) and configure, build and install. Run
   `ctest`.
3. Write your `CLAUDE.local.md`: paths, prefix, remotes, build options (especially
   anything that differs from the defaults, such as `buildTRN=OFF`), where session
   notes go.
4. Open the repository in the Claude desktop app or run `claude`, and check with
   `/context` that `CLAUDE.md` and `CLAUDE.local.md` were loaded.
5. Put a few small sample files per format somewhere read-only.
6. Try a trivial change (e.g. a man-page typo) through the whole loop: investigate,
   edit, build, test, document, commit on a topic branch.
7. Only then move to real work.

## 14. Sources

- Claude Code documentation: https://code.claude.com/docs
  - Memory and CLAUDE.md: https://code.claude.com/docs/en/memory
  - Permissions: https://code.claude.com/docs/en/permissions
  - Remote Control: https://code.claude.com/docs/en/remote-control
  - Best practices: https://code.claude.com/docs/en/best-practices
- Homebrew tap trust: https://docs.brew.sh/Tap-Trust
- MB-System installation: https://www.mbari.org/technology/mb-system/installation/
  and `BuildAndInstall.md`
- MB-System repository: https://github.com/dwcaress/MB-System
