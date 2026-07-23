/*--------------------------------------------------------------------
 *    The MB-System:  mbmakedatalist.cc
 *
 *    Copyright (c) 2006-2026 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------
 *
 * mbmakedatalist generates an MB-System datalist file referencing
 * all identifiable swath files in the specified directory. If no
 * directory is specified with the -I option, the current directory
 * is used. The resulting datalist is named datalist.mb-1 by default.
 *
 * This is a C++ rewrite of the original Perl script mbmakedatalist.
 * The key performance improvement is that format detection calls
 * mb_get_format() from libmbio directly instead of spawning a child
 * process for every file.
 *
 * Usage:
 *   mbmakedatalist [-B size|--min-size=size]
 *                    [-F format|--format=format]
 *                    [-I directory|--input=directory]
 *                    [-L|--skip-latest]
 *                    [-O datalist|--output=datalist]
 *                    [-P|--ignore-processed]
 *                    [-S suffix|--suffix=suffix]
 *                    [-T|--no-time-sort]
 *                    [-H|--help] [-V|--verbose]
 *
 * This file must be compiled as part of MB-System, linked against libmbio:
 *   c++ -O2 -o mbmakedatalist mbmakedatalist.cc \
 *       -I/usr/local/include/mbsystem -L/usr/local/lib -lmbio -lm
 *
 * The rewrite of the perl script mbm_makedatalist as C++ program 
 * mbmakedatalist was accomplished on May 31, 2026 by David Caress
 * utilizing the Claude Sonnet 4.6 AI for code generation.
 *
 *--------------------------------------------------------------------*/

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

/*--------------------------------------------------------------------
 * Constants
 *--------------------------------------------------------------------*/
constexpr char   program_name[]    = "mbmakedatalist";
constexpr char   default_datalist[] = "datalist.mb-1";
constexpr int    max_path          = 4096;
constexpr int    initial_capacity  = 1024;

/*--------------------------------------------------------------------
 * A record describing one swath file candidate.
 *--------------------------------------------------------------------*/
struct FileRecord {
    char path[max_path];     /* full path as it will appear in the datalist */
    char basename[max_path]; /* filename only, used for pattern matching   */
    int  format;
    /* Fields used for Kongsberg time-order sorting */
    bool sort_ok;            /* true if the filename matched a timestamp pattern */
    int  sort_case;          /* 0 = HHMMSS (6 digits), 1 = HHMM (4 digits)   */
    char yyyymmdd[16];
    char hhmmss[16];
    char seq[16];            /* leading sequence number, e.g. "0124"          */
    char tail[max_path];     /* everything after the timestamp field           */
};

/*--------------------------------------------------------------------
 * Dynamic array of FileRecord
 *--------------------------------------------------------------------*/
struct FileList {
    FileRecord *data;
    int         size;
    int         capacity;
};

static void filelist_init(FileList *fl) {
    fl->data     = static_cast<FileRecord *>(malloc(initial_capacity * sizeof(FileRecord)));
    fl->size     = 0;
    fl->capacity = initial_capacity;
    if (!fl->data) { perror("malloc"); exit(1); }
}

static void filelist_push(FileList *fl, const FileRecord *rec) {
    if (fl->size == fl->capacity) {
        fl->capacity *= 2;
        fl->data = static_cast<FileRecord *>(realloc(fl->data, fl->capacity * sizeof(FileRecord)));
        if (!fl->data) { perror("realloc"); exit(1); }
    }
    fl->data[fl->size++] = *rec;
}

static void filelist_free(FileList *fl) {
    free(fl->data);
    fl->data     = nullptr;
    fl->size     = 0;
    fl->capacity = 0;
}

/*--------------------------------------------------------------------
 * Helpers
 *--------------------------------------------------------------------*/

/* Return 1 if suffix matches one of the Kongsberg multibeam suffixes
 * that trigger time-order sorting. */
static int is_kongsberg_suffix(const char *suffix) {
    if (!suffix || suffix[0] == '\0') return 0;
    return (strcmp(suffix, ".all")   == 0 ||
            strcmp(suffix, ".ALL")   == 0 ||
            strcmp(suffix, ".kmall") == 0 ||
            strcmp(suffix, ".KMALL") == 0 ||
            strcmp(suffix, ".mb56")  == 0 ||
            strcmp(suffix, ".mb57")  == 0 ||
            strcmp(suffix, ".mb58")  == 0 ||
            strcmp(suffix, ".mb59")  == 0 ||
            strcmp(suffix, ".mb261") == 0);
}

/* Return 1 if this filename is a known Kongsberg junk file. */
static int is_kongsberg_junk(const char *basename) {
    return (strcmp(basename, "9999.all")   == 0 ||
            strcmp(basename, "9999.ALL")   == 0 ||
            strcmp(basename, "9999.kmall") == 0 ||
            strcmp(basename, "9999.KMALL") == 0);
}

/* Case-insensitive suffix check: does 'filename' end with 'suffix'? */
static int has_suffix(const char *filename, const char *suffix) {
    size_t flen = strlen(filename);
    size_t slen = strlen(suffix);
    if (slen > flen) return 0;
    /* Use case-sensitive compare; suffixes are exact strings in MB-System. */
    return strcmp(filename + flen - slen, suffix) == 0;
}

/* Return 1 if 'filename' ends with "p<suffix>", i.e. is a processed file. */
static int is_processed_file(const char *filename, const char *suffix) {
    /* A processed file is one whose name ends with p<suffix>, where the
     * character before the suffix is literally the letter 'p'. */
    size_t flen = strlen(filename);
    size_t slen = strlen(suffix);
    if (flen <= slen) return 0;
    if (!has_suffix(filename, suffix)) return 0;
    return filename[flen - slen - 1] == 'p';
}

/*--------------------------------------------------------------------
 * Format detection: calls mb_get_format() from libmbio directly,
 * returning the format id (>0) or 0 if the file is not recognised
 * as a swath data file.
 *--------------------------------------------------------------------*/
static int get_format(const char *filepath) {
    int format  = 0;
    int verbose = 0;
    int error   = MB_ERROR_NO_ERROR;
    mb_get_format(verbose, const_cast<char *>(filepath), nullptr, &format, &error);
    return format;
}

/*--------------------------------------------------------------------
 * Kongsberg filename time-sort comparator.
 * Sort key is "YYYYMMDD^HHMMSS" — plain lexicographic order works
 * because the fields are zero-padded numeric strings.
 *--------------------------------------------------------------------*/
static int kongsberg_cmp(const void *a, const void *b) {
    const FileRecord *ra = static_cast<const FileRecord *>(a);
    const FileRecord *rb = static_cast<const FileRecord *>(b);
    /* Primary: date */
    int c = strcmp(ra->yyyymmdd, rb->yyyymmdd);
    if (c != 0) return c;
    /* Secondary: time */
    return strcmp(ra->hhmmss, rb->hhmmss);
}

/*--------------------------------------------------------------------
 * Parse a Kongsberg-style filename for its embedded timestamp.
 *
 * Two known patterns:
 *   Case 0 (seconds): NNNN_YYYYMMDD_HHMMSS<tail>
 *                     e.g. 0124_20100908_191912_Healy.all
 *   Case 1 (minutes): N+_YYYYMMDD_HHMM_<tail>
 *                     e.g. 0250_20220620_2201_sentrye.kmall
 *
 * Returns 1 on success and fills rec->yyyymmdd, hhmmss, seq, tail,
 * sort_case.  Returns 0 if the name does not match either pattern.
 *--------------------------------------------------------------------*/
static int parse_kongsberg_filename(const char *basename, FileRecord *rec) {
    /* Try case 0 first: exactly NNNN_YYYYMMDD_HHMMSS then non-whitespace tail.
     * The original Perl regex: /(\d{4})_(\d{8})_(\d{6})(\S+)/ */
    {
        int seq_i, y_i, t_i;
        char tail[max_path];
        if (sscanf(basename, "%4d_%8d_%6d%4095s", &seq_i, &y_i, &t_i, tail) == 4) {
            /* Verify the fields occupy contiguous digit runs by checking
             * positions manually. */
            const char *p = basename;
            /* seq: 4 digits */
            int ok = 1;
            for (int i = 0; i < 4 && ok; i++) ok = isdigit((unsigned char)p[i]);
            if (ok && p[4] == '_') {
                /* yyyymmdd: 8 digits */
                for (int i = 5; i < 13 && ok; i++) ok = isdigit((unsigned char)p[i]);
                if (ok && p[13] == '_') {
                    /* hhmmss: 6 digits */
                    for (int i = 14; i < 20 && ok; i++) ok = isdigit((unsigned char)p[i]);
                    if (ok && p[20] != '\0') {
                        snprintf(rec->seq,      sizeof(rec->seq),      "%04d", seq_i);
                        snprintf(rec->yyyymmdd, sizeof(rec->yyyymmdd), "%08d", y_i);
                        snprintf(rec->hhmmss,   sizeof(rec->hhmmss),   "%06d", t_i);
                        strncpy(rec->tail, p + 20, sizeof(rec->tail) - 1);
                        rec->tail[sizeof(rec->tail) - 1] = '\0';
                        rec->sort_case = 0;
                        return 1;
                    }
                }
            }
        }
    }

    /* Try case 1: N+_DIGITS+_DIGITS+_TAIL (the more general pattern).
     * The original Perl regex: /(\d+)_(\d+)_(\d+)_(\S+)/ */
    {
        const char *p = basename;
        /* seq */
        const char *q = p;
        while (isdigit((unsigned char)*q)) q++;
        if (q == p || *q != '_') return 0;
        size_t seq_len = (size_t)(q - p);
        if (seq_len >= sizeof(rec->seq)) return 0;
        strncpy(rec->seq, p, seq_len);
        rec->seq[seq_len] = '\0';
        p = q + 1;
        /* yyyymmdd */
        q = p;
        while (isdigit((unsigned char)*q)) q++;
        if (q == p || *q != '_') return 0;
        size_t y_len = (size_t)(q - p);
        if (y_len >= sizeof(rec->yyyymmdd)) return 0;
        strncpy(rec->yyyymmdd, p, y_len);
        rec->yyyymmdd[y_len] = '\0';
        p = q + 1;
        /* hhmmss */
        q = p;
        while (isdigit((unsigned char)*q)) q++;
        if (q == p || *q != '_') return 0;
        size_t t_len = (size_t)(q - p);
        if (t_len >= sizeof(rec->hhmmss)) return 0;
        strncpy(rec->hhmmss, p, t_len);
        rec->hhmmss[t_len] = '\0';
        p = q + 1;
        /* tail: everything remaining, must be non-empty */
        if (*p == '\0') return 0;
        strncpy(rec->tail, p, sizeof(rec->tail) - 1);
        rec->tail[sizeof(rec->tail) - 1] = '\0';
        rec->sort_case = 1;
        return 1;
    }
}

/*--------------------------------------------------------------------
 * Reconstruct the basename from parsed Kongsberg fields.
 * Case 0: seq_YYYYMMDD_HHMMSStail   (tail already starts with e.g. '_Healy.all')
 * Case 1: seq_YYYYMMDD_HHMMSS_tail
 *--------------------------------------------------------------------*/
static void build_kongsberg_basename(const FileRecord *rec, char *out, size_t outsz) {
    if (rec->sort_case == 0) {
        snprintf(out, outsz, "%s_%s_%s%s",
                 rec->seq, rec->yyyymmdd, rec->hhmmss, rec->tail);
    } else {
        snprintf(out, outsz, "%s_%s_%s_%s",
                 rec->seq, rec->yyyymmdd, rec->hhmmss, rec->tail);
    }
}

/*--------------------------------------------------------------------
 * Lexicographic comparator for alphabetical directory listing order.
 *--------------------------------------------------------------------*/
static int name_cmp(const void *a, const void *b) {
    return strcmp(static_cast<const FileRecord *>(a)->basename,
                  static_cast<const FileRecord *>(b)->basename);
}

/*--------------------------------------------------------------------
 * Print usage / help
 *--------------------------------------------------------------------*/
static void print_help() {
    printf("\n%s:\n", program_name);
    printf("Macro to generate an MB-System datalist file referencing all\n");
    printf("identifiable swath files in the specified directory. If no directory\n");
    printf("is specified with the -I option, then the current directory is used.\n");
    printf("The resulting datalist will be named datalist.mb-1 by default.\n\n");
    printf("Mbmakedatalist is a macro to generate an MB-System datalist file\n");
    printf("referencing all identifiable swath files in the specified target directory.\n");
    printf("Datalists are fundamental structures in MB-System workflows because they\n");
    printf("allow programs to operate on sets of swath data files.\n");
    printf("Datalist files are text lists of swath data files and their format ids with each\n");
    printf("file entry taking up a single line. These lists may contain references to other\n");
    printf("datalists, making them recursive. Datalists may also contain comments and parsing\n");
    printf("directives that, for example, determine whether parsing returns references to\n");
    printf("raw or processed data files. See the MB-System manual page for details\n");
    printf("on the format and structure of datalists.\n\n");
    printf("Usage:\n");
    printf("  %s [options]\n\n", program_name);
    printf("Options (short and long forms are equivalent):\n");
    printf("  -B size,  --min-size=size        Minimum file size in KB; smaller files are ignored\n");
    printf("  -F format,--format=format        Format id assigned to all files (default: inferred)\n");
    printf("  -I dir,   --input=dir            Directory to scan (default: current directory)\n");
    printf("  -L,       --skip-latest          Omit the last file in the listing\n");
    printf("  -O file,  --output=file          Output datalist filename (default: datalist.mb-1)\n");
    printf("  -P,       --ignore-processed     Exclude processed files (e.g. *p.mb88)\n");
    printf("  -S suffix,--suffix=suffix        Consider only files with this suffix\n");
    printf("  -T,       --no-time-sort         Disable time-order sorting of Kongsberg files\n");
    printf("  -H,       --help                 Print this help message and exit\n");
    printf("  -V,       --verbose              Print verbose status messages\n");
    printf("\n");
}

/*--------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------*/
int main(int argc, char **argv) {

    /* --- Option variables (mirrors the Perl variables) --- */
    long        size_threshold   = 0;      /* -B: minimum file size in KB       */
    int         format_specified = 0;      /* -F: use this format for all files */
    bool        help             = false;  /* -H */
    char        directory[max_path] = ""; /* -I */
    bool        skiplatest       = false;  /* -L */
    char        datalist[max_path];        /* -O */
    bool        ignoreprocessed  = false;  /* -P */
    char        suffix[256]      = "";     /* -S */
    bool        disablesorting   = false;  /* -T */
    int         verbose          = 0;      /* -V */

    strcpy(datalist, default_datalist);

    /* --- Parse command line (getopt_long; short and long forms are equivalent) --- */
    static struct option long_options[] = {
        { "min-size",         required_argument, nullptr, 'B' },
        { "format",           required_argument, nullptr, 'F' },
        { "help",             no_argument,       nullptr, 'H' },
        { "input",            required_argument, nullptr, 'I' },
        { "skip-latest",      no_argument,       nullptr, 'L' },
        { "output",           required_argument, nullptr, 'O' },
        { "ignore-processed", no_argument,       nullptr, 'P' },
        { "suffix",           required_argument, nullptr, 'S' },
        { "no-time-sort",     no_argument,       nullptr, 'T' },
        { "verbose",          no_argument,       nullptr, 'V' },
        { nullptr,             0,                 nullptr, 0 }
    };
    /* The short-option string accepts both upper- and lower-case letters
     * exactly as the original Perl script did. */
    constexpr char short_opts[] = "B:b:F:f:HhI:i:LlO:o:PpS:s:TtVv";
    int opt;
    int longidx = 0;
    while ((opt = getopt_long(argc, argv, short_opts, long_options, &longidx)) != -1) {
        switch (opt) {
        case 'B': case 'b':
            size_threshold = atol(optarg);
            break;
        case 'F': case 'f':
            format_specified = atoi(optarg);
            break;
        case 'H': case 'h':
            help = true;
            break;
        case 'I': case 'i':
            strncpy(directory, optarg, max_path - 1);
            directory[max_path - 1] = '\0';
            break;
        case 'L': case 'l':
            skiplatest = true;
            break;
        case 'O': case 'o':
            strncpy(datalist, optarg, max_path - 1);
            datalist[max_path - 1] = '\0';
            break;
        case 'P': case 'p':
            ignoreprocessed = true;
            break;
        case 'S': case 's':
            strncpy(suffix, optarg, sizeof(suffix) - 1);
            suffix[sizeof(suffix) - 1] = '\0';
            break;
        case 'T': case 't':
            disablesorting = true;
            break;
        case 'V': case 'v':
            verbose = 1;
            break;
        default:
            fprintf(stderr, "Try '%s --help' for usage information.\n", program_name);
            return 1;
        }
    }

    if (help) {
        print_help();
        return 0;
    }

    /* The Perl script doubles the threshold because `ls -s` reports sizes
     * in 512-byte blocks and the user supplies KB.  1 KB = 2 blocks of 512
     * bytes.  We compare against stat.st_size (bytes), so convert KB→bytes. */
    long size_threshold_bytes = size_threshold * 1024L;

    bool do_kongsberg_sort = is_kongsberg_suffix(suffix) && !disablesorting;

    /* --- Verbose startup messages (mirrors Perl output exactly) --- */
    if (verbose) {
        printf("\nRunning %s...\n\n", program_name);
        if (directory[0])
            printf(" - Checking for swath files in directory %s\n", directory);
        else
            printf(" - Checking for swath files in the current directory\n");

        if (suffix[0]) {
            printf(" - Checking for files with suffix %s\n", suffix);
            if (ignoreprocessed)
                printf(" - Ignoring files with specified suffix preceded by letter p, e.g. p%s\n", suffix);
        } else {
            printf(" - Checking all files for those that meet swath data naming conventions\n");
        }

        if (disablesorting) {
            if (is_kongsberg_suffix(suffix))
                printf(" - Sorting Kongsberg multibeam files into time order is disabled.\n");
            else
                printf(" - Request to disable time sorting of Kongsberg multibeam files ignored"
                       " because a Kongsberg multibeam file\n"
                       "   suffix (.all or .ALL) has not been specified with the -S option\n");
        } else {
            if (is_kongsberg_suffix(suffix))
                printf(" - Attempting to sort Kongsberg multibeam files into time order based on filenames.\n");
        }

        if (format_specified)
            printf(" - Assigning format id %d to all files\n", format_specified);
        else
            printf(" - Using format ids consistent with filenames\n");
    }

    /* --- Scan directory --- */
    const char *scandir_path = (directory[0]) ? directory : ".";
    DIR *dp = opendir(scandir_path);
    if (!dp) {
        fprintf(stderr, "\n%s:\nCannot open directory %s: %s\nExiting...\n",
                program_name, scandir_path, strerror(errno));
        return 1;
    }

    FileList candidates;
    filelist_init(&candidates);

    struct dirent *de;
    while ((de = readdir(dp)) != nullptr) {
        const char *name = de->d_name;

        /* Skip "." and ".." */
        if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
            continue;

        /* Apply suffix filter if given */
        if (suffix[0] && !has_suffix(name, suffix))
            continue;

        /* Apply -P: skip processed files */
        if (ignoreprocessed && suffix[0] && is_processed_file(name, suffix))
            continue;

        /* Build the full path */
        FileRecord rec;
        memset(&rec, 0, sizeof(rec));
        if (directory[0])
            snprintf(rec.path, sizeof(rec.path), "%s/%s", directory, name);
        else
            snprintf(rec.path, sizeof(rec.path), "%s", name);
        strncpy(rec.basename, name, sizeof(rec.basename) - 1);

        /* Apply size threshold */
        if (size_threshold_bytes > 0) {
            struct stat st;
            if (stat(rec.path, &st) != 0) continue;
            if (!S_ISREG(st.st_mode)) continue;
            if (st.st_size < size_threshold_bytes) continue;
        } else {
            /* Still skip directories/special files even with no threshold */
            struct stat st;
            if (stat(rec.path, &st) != 0) continue;
            if (!S_ISREG(st.st_mode)) continue;
        }

        filelist_push(&candidates, &rec);
    }
    closedir(dp);

    /* Sort candidates alphabetically (mirrors the shell `ls` ordering used
     * by the original Perl script, which relied on filesystem/ls ordering).
     * A simple lexicographic sort of basenames matches `ls` on most systems. */
    qsort(candidates.data, candidates.size, sizeof(FileRecord), name_cmp);

    /* --- Determine format for each candidate and build output list --- */
    FileList outlist;
    filelist_init(&outlist);

    for (int i = 0; i < candidates.size; i++) {
        FileRecord *rec = &candidates.data[i];

        /* Determine format */
        int fmt;
        if (format_specified) {
            fmt = format_specified;
        } else {
            fmt = get_format(rec->path);
        }
        rec->format = fmt;

        /* Skip format-0 (unrecognised) files */
        if (fmt == 0) continue;

        /* Skip Kongsberg junk files */
        if (is_kongsberg_suffix(suffix) && is_kongsberg_junk(rec->basename)) {
            printf("File ignored:   file:%s format:%d\n", rec->path, fmt);
            continue;
        }

        /* Skip the output datalist itself (only relevant when directory is
         * "." or empty, matching the Perl condition
         * `$file ne $datalist || $directory ne "."`) */
        if (directory[0] == '\0' || strcmp(directory, ".") == 0) {
            if (strcmp(rec->basename, datalist) == 0) continue;
        }

        if (verbose)
            printf("Adding to list: file:%s format:%d\n", rec->path, fmt);

        filelist_push(&outlist, rec);
    }
    filelist_free(&candidates);

    /* --- Kongsberg time-order sort --- */
    if (do_kongsberg_sort && outlist.size > 0) {
        /* Parse timestamps; collect only the records that matched.
         * The Perl code rebuilds the entire outlist from the sorted subset,
         * so records that did not match a timestamp pattern are dropped (they
         * were never added to @sortfilelist). We replicate that behaviour. */
        bool any_parsed = false;
        for (int i = 0; i < outlist.size; i++) {
            FileRecord *rec = &outlist.data[i];
            rec->sort_ok = parse_kongsberg_filename(rec->basename, rec);
            if (rec->sort_ok) any_parsed = true;
        }

        if (any_parsed) {
            /* Keep only sort_ok records in a temporary list, sort them,
             * then rebuild outlist. */
            FileList sortable;
            filelist_init(&sortable);
            for (int i = 0; i < outlist.size; i++) {
                if (outlist.data[i].sort_ok)
                    filelist_push(&sortable, &outlist.data[i]);
            }
            qsort(sortable.data, sortable.size, sizeof(FileRecord), kongsberg_cmp);

            /* Rebuild paths from sorted timestamps and replace outlist */
            filelist_free(&outlist);
            filelist_init(&outlist);
            for (int i = 0; i < sortable.size; i++) {
                FileRecord *rec = &sortable.data[i];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
                char newbase[max_path];
                build_kongsberg_basename(rec, newbase, sizeof(newbase));
                if (directory[0]) {
                    strncpy(rec->path, directory, sizeof(rec->path) - 1);
                    rec->path[sizeof(rec->path) - 1] = '\0';
                    strncat(rec->path, "/", sizeof(rec->path) - strlen(rec->path) - 1);
                    strncat(rec->path, newbase, sizeof(rec->path) - strlen(rec->path) - 1);
                } else {
                    strncpy(rec->path, newbase, sizeof(rec->path) - 1);
                    rec->path[sizeof(rec->path) - 1] = '\0';
                }
                strncpy(rec->basename, newbase, sizeof(rec->basename) - 1);
                rec->basename[sizeof(rec->basename) - 1] = '\0';
                filelist_push(&outlist, rec);
#pragma GCC diagnostic pop
            }
            filelist_free(&sortable);
        }
    }

    /* --- Apply -L (skip last file) --- */
    int count = outlist.size;
    if (skiplatest && count > 0)
        count--;

    /* --- Write datalist --- */
    if (count > 0) {
        FILE *fp = fopen(datalist, "w");
        if (!fp) {
            fprintf(stderr, "\n%s:\nUnable to open output datalist file %s\nExiting...\n",
                    program_name, datalist);
            filelist_free(&outlist);
            return 1;
        }

        if (verbose)
            printf("\nOutputting %d file listings to datalist file %s\n", count, datalist);

        for (int i = 0; i < count; i++) {
            fprintf(fp, "%s %d\n", outlist.data[i].path, outlist.data[i].format);
            if (verbose)
                printf("%s %d\n", outlist.data[i].path, outlist.data[i].format);
        }

        if (verbose)
            printf("\nAll done!\n\n");

        fclose(fp);
    } else {
        if (verbose)
            printf("No swath files identified therefore no datalist created...\n");
    }

    filelist_free(&outlist);
    return 0;
}
