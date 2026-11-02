/*--------------------------------------------------------------------
 *    The MB-system:	mbhistory.cc	10/25/2026
 *
 *    Copyright (c) 2026 by
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
 *--------------------------------------------------------------------*/
/*
 * mbhistory lists, compares, tags, prunes and restores the versions of
 * edit save (*.esf) files kept by MB-System. Before a program such as
 * mbedit, mbeditviz, mbclean or mbareaclean modifies an edit save file,
 * the MBIO library saves a snapshot of it in a directory named mbhistory
 * next to the file (see src/mbio/mb_history.c). The input may be a single
 * swath file or a datalist, so that a whole survey can be restored to the
 * state it had at a given date.
 *
 * Author:	C. dos Santos Ferreira
 * Date:	October 25, 2026
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

typedef enum {
	ACTION_LIST = 0,
	ACTION_SHOW = 1,
	ACTION_DIFF = 2,
	ACTION_RESTORE = 3,
	ACTION_TAG = 4,
	ACTION_PRUNE = 5,
} action_t;

typedef enum {
	TARGET_VERSION = 0,
	TARGET_TAG = 1,
	TARGET_DATE = 2,
} target_t;

/* version id used for the current (live) file */
constexpr int VERSION_NOW = 0;

constexpr char program_name[] = "mbhistory";
constexpr char help_message[] =
    "mbhistory lists, compares, tags, prunes and restores the versions of\n"
    "MB-System edit save (*.esf) files. Snapshots are kept in a directory\n"
    "named mbhistory next to the swath files. The input may be a swath file\n"
    "or a datalist. All dates and times are UTC.";
constexpr char usage_message[] =
    "mbhistory --input=file|datalist\n"
    "\t[--list (default) --show=version --diff=version[:version]\n"
    "\t--restore=version --restore-tag=name --restore-date=yyyy-mm-dd[Thh:mm[:ss]]\n"
    "\t--tag=name --prune --keep=number --older-than=days\n"
    "\t--dry-run --yes --force --process --verbose --help]\n"
    "\tversion may be a number or \"now\" (the current file)";

/* a tag line in <dir>/mbhistory/tags.txt: name file version utc */
struct tag_struct {
	char name[MB_HISTORY_NAME_LENGTH];
	char file[MB_PATH_MAXLINE];
	int version;
	char utc[32];
};

/* edit counts of an edit save file */
struct edit_count_struct {
	int nedit;
	int nflag;
	int nunflag;
	int nzero;
	int nfilter;
	int nother;
};

/*--------------------------------------------------------------------*/
/* parse "now" or a version number, returns false on bad input */
static bool parse_version(const char *text, int *version) {
	if (strcmp(text, "now") == 0) {
		*version = VERSION_NOW;
		return true;
	}
	char *end = nullptr;
	const long value = strtol(text, &end, 10);
	if (end == text || *end != '\0' || value < 1 || value > 999999)
		return false;
	*version = (int)value;
	return true;
}
/*--------------------------------------------------------------------*/
/* parse a UTC date: yyyy-mm-dd, yyyy-mm-ddThh:mm or yyyy-mm-ddThh:mm:ss
   (a space or '/' may replace the 'T', '/' may replace '-').
   A bare day means the end of that day. */
static bool parse_date(int verbose, const char *text, double *time_d) {
	int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
	char sep1, sep2, sep3;
	int n = sscanf(text, "%d%c%d%c%d%c%d:%d:%d", &time_i[0], &sep1, &time_i[1], &sep2, &time_i[2], &sep3, &time_i[3],
	               &time_i[4], &time_i[5]);
	bool end_of_day = false;
	if (n == 5) {
		end_of_day = true;
	}
	else if (n == 8 || n == 9) {
		if (sep3 != 'T' && sep3 != ' ' && sep3 != '/')
			return false;
	}
	else {
		return false;
	}
	if ((sep1 != '-' && sep1 != '/') || sep2 != sep1)
		return false;
	if (time_i[1] < 1 || time_i[1] > 12 || time_i[2] < 1 || time_i[2] > 31 || time_i[3] < 0 || time_i[3] > 23 ||
	    time_i[4] < 0 || time_i[4] > 59 || time_i[5] < 0 || time_i[5] > 60)
		return false;
	if (mb_get_time(verbose, time_i, time_d) != MB_SUCCESS)
		return false;
	if (end_of_day)
		*time_d += 86400.0;
	return true;
}
/*--------------------------------------------------------------------*/
static void format_utc(double time_d, char *text, size_t size) {
	const time_t t = (time_t)time_d;
	const struct tm *tm_utc = gmtime(&t);
	if (tm_utc != nullptr)
		strftime(text, size, "%Y-%m-%d %H:%M:%S", tm_utc);
	else
		snprintf(text, size, "%s", "unknown");
}
/*--------------------------------------------------------------------*/
/* count the edit events of an edit save file, skipping version headers */
static bool count_edits(const char *path, struct edit_count_struct *count) {
	memset(count, 0, sizeof(struct edit_count_struct));
	FILE *fp = fopen(path, "rb");
	if (fp == nullptr)
		return false;
	const bool byteswapped = mb_swap_check();
	char header[MB_PATH_MAXLINE];
	if (fread(header, MB_PATH_MAXLINE, 1, fp) != 1 || strncmp(header, "ESFVERSION", 10) != 0)
		rewind(fp);
	unsigned char record[16];
	while (fread(record, sizeof(record), 1, fp) == 1) {
		if (strncmp((const char *)record, "ESFVERSI", 8) == 0) {
			if (fread(header, MB_PATH_MAXLINE - sizeof(record), 1, fp) != 1)
				break;
			continue;
		}
		int action;
		memcpy(&action, &record[12], sizeof(int));
		if (byteswapped)
			action = mb_swap_int(action);
		count->nedit++;
		if (action == MBP_EDIT_FLAG)
			count->nflag++;
		else if (action == MBP_EDIT_UNFLAG)
			count->nunflag++;
		else if (action == MBP_EDIT_ZERO)
			count->nzero++;
		else if (action == MBP_EDIT_FILTER)
			count->nfilter++;
		else
			count->nother++;
	}
	fclose(fp);
	return true;
}
/*--------------------------------------------------------------------*/
static bool file_exists(const char *path) {
	struct stat file_status;
	return (stat(path, &file_status) == 0 && (file_status.st_mode & S_IFMT) == S_IFREG);
}
/*--------------------------------------------------------------------*/
/* read the tags of one file from <dir>/tags.txt; *tags allocated with mb_mallocd */
static int read_tags(int verbose, const char *dir, int *ntag, struct tag_struct **tags, int *error) {
	*ntag = 0;
	*tags = nullptr;
	int status = MB_SUCCESS;
	mb_pathplus tagfile;
	snprintf(tagfile, sizeof(tagfile), "%s/tags.txt", dir);
	FILE *fp = fopen(tagfile, "r");
	if (fp == nullptr)
		return status;
	mb_pathplus line;
	int nline = 0;
	while (fgets(line, sizeof(line), fp) != nullptr)
		nline++;
	if (nline > 0)
		status = mb_mallocd(verbose, __FILE__, __LINE__, nline * sizeof(struct tag_struct), (void **)tags, error);
	if (status == MB_SUCCESS && nline > 0) {
		rewind(fp);
		while (*ntag < nline && fgets(line, sizeof(line), fp) != nullptr) {
			if (line[0] == '#')
				continue;
			struct tag_struct *tag = &(*tags)[*ntag];
			memset(tag, 0, sizeof(struct tag_struct));
			if (sscanf(line, "%255[^\t]\t%1023[^\t]\t%d\t%31[^\t\r\n]", tag->name, tag->file, &tag->version, tag->utc) >= 3)
				(*ntag)++;
		}
	}
	fclose(fp);
	return status;
}
/*--------------------------------------------------------------------*/
/* rewrite <dir>/tags.txt with the given tags */
static int write_tags(const char *dir, int ntag, const struct tag_struct *tags, int *error) {
	mb_pathplus tagfile;
	mb_pathplus tagfile_tmp;
	snprintf(tagfile, sizeof(tagfile), "%s/tags.txt", dir);
	snprintf(tagfile_tmp, sizeof(tagfile_tmp), "%s/tags.txt.tmp", dir);
	FILE *fp = fopen(tagfile_tmp, "w");
	if (fp == nullptr) {
		*error = MB_ERROR_OPEN_FAIL;
		return MB_FAILURE;
	}
	fprintf(fp, "# mbhistory tags version 1\n# tag\tfile\tversion\tutc\n");
	for (int i = 0; i < ntag; i++)
		fprintf(fp, "%s\t%s\t%d\t%s\n", tags[i].name, tags[i].file, tags[i].version, tags[i].utc);
	if (fclose(fp) != 0) {
		*error = MB_ERROR_WRITE_FAIL;
		return MB_FAILURE;
	}
#ifdef _WIN32
	remove(tagfile);
#endif
	if (rename(tagfile_tmp, tagfile) != 0) {
		*error = MB_ERROR_WRITE_FAIL;
		return MB_FAILURE;
	}
	return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static const struct mb_history_entry *find_entry(int nentry, const struct mb_history_entry *entries, int version) {
	for (int i = 0; i < nentry; i++) {
		if (entries[i].version == version)
			return &entries[i];
	}
	return nullptr;
}
/*--------------------------------------------------------------------*/
/* load the net (final) edit of every beam of an edit save file:
   edits are sorted by time and beam (a stable sort keeps file order), so
   the last edit of each time/beam run is the one mb_esf_apply leaves in effect */
static int load_net_edits(int verbose, const char *path, struct mb_esf_struct *esf, int *nnet, int *error) {
	memset(esf, 0, sizeof(struct mb_esf_struct));
	*nnet = 0;
	mb_path esffile;
	snprintf(esffile, sizeof(esffile), "%s", path);
	int status = mb_esf_open(verbose, program_name, esffile, true, MBP_ESF_NOWRITE, esf, error);
	if (status != MB_SUCCESS)
		return status;
	for (int i = 0; i < esf->nedit; i++) {
		if (*nnet > 0 && mb_edit_compare(&esf->edit[*nnet - 1], &esf->edit[i]) == 0)
			esf->edit[*nnet - 1] = esf->edit[i];
		else
			esf->edit[(*nnet)++] = esf->edit[i];
	}
	return status;
}
/*--------------------------------------------------------------------*/
static bool is_flag_action(int action) {
	return (action == MBP_EDIT_FLAG || action == MBP_EDIT_ZERO || action == MBP_EDIT_FILTER);
}
/*--------------------------------------------------------------------*/
/* compare the net beam edits of two edit save files */
static int diff_files(int verbose, const char *path_a, const char *label_a, const char *path_b, const char *label_b,
                      bool details, int *error) {
	struct mb_esf_struct esf_a;
	struct mb_esf_struct esf_b;
	int nnet_a = 0;
	int nnet_b = 0;
	int status = MB_SUCCESS;
	if (path_a != nullptr)
		status = load_net_edits(verbose, path_a, &esf_a, &nnet_a, error);
	else
		memset(&esf_a, 0, sizeof(struct mb_esf_struct));
	if (status == MB_SUCCESS && path_b != nullptr)
		status = load_net_edits(verbose, path_b, &esf_b, &nnet_b, error);
	else if (status == MB_SUCCESS)
		memset(&esf_b, 0, sizeof(struct mb_esf_struct));
	if (status != MB_SUCCESS) {
		fprintf(stderr, "Unable to read edit save file for comparison\n");
		return status;
	}

	int nchanged = 0;
	int nnowflag = 0;
	int nnowunflag = 0;
	int nflagtype = 0;
	int nremoved = 0;
	int npings = 0;
	double first_time = 0.0;
	double last_time = 0.0;
	double last_ping = -1.0;
	int ia = 0;
	int ib = 0;
	while (ia < nnet_a || ib < nnet_b) {
		const struct mb_edit_struct *a = (ia < nnet_a ? &esf_a.edit[ia] : nullptr);
		const struct mb_edit_struct *b = (ib < nnet_b ? &esf_b.edit[ib] : nullptr);
		int cmp;
		if (a == nullptr)
			cmp = 1;
		else if (b == nullptr)
			cmp = -1;
		else
			cmp = mb_edit_compare(a, b);

		const struct mb_edit_struct *changed = nullptr;
		const char *what = nullptr;
		if (cmp < 0) {
			/* edit only in a: removed in b */
			changed = a;
			nremoved++;
			what = "edit removed";
			ia++;
		}
		else if (cmp > 0) {
			changed = b;
			if (is_flag_action(b->action)) {
				nnowflag++;
				what = "now flagged";
			}
			else {
				nnowunflag++;
				what = "now unflagged";
			}
			ib++;
		}
		else {
			if (a->action != b->action) {
				changed = b;
				if (is_flag_action(a->action) && is_flag_action(b->action)) {
					nflagtype++;
					what = "flag type changed";
				}
				else if (is_flag_action(b->action)) {
					nnowflag++;
					what = "now flagged";
				}
				else {
					nnowunflag++;
					what = "now unflagged";
				}
			}
			ia++;
			ib++;
		}

		if (changed != nullptr) {
			nchanged++;
			if (nchanged == 1)
				first_time = changed->time_d;
			last_time = changed->time_d;
			if (last_ping < 0.0 || fabs(changed->time_d - last_ping) >= MB_ESF_MAXTIMEDIFF) {
				npings++;
				last_ping = changed->time_d;
			}
			if (details) {
				int time_i[7];
				mb_get_date(verbose, changed->time_d, time_i);
				fprintf(stdout, "  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  beam:%d  %s\n", time_i[0], time_i[1],
				        time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				        changed->beam % MB_ESF_MULTIPLICITY_FACTOR, what);
			}
		}
	}

	fprintf(stdout, "Differences in final beam edits from %s to %s:\n", label_a, label_b);
	if (nchanged == 0) {
		fprintf(stdout, "  none\n");
	}
	else {
		fprintf(stdout, "  %d beams changed: %d now flagged, %d now unflagged, %d flag type changed, %d edits removed\n",
		        nchanged, nnowflag, nnowunflag, nflagtype, nremoved);
		int time_i[7];
		int time_j[7];
		mb_get_date(verbose, first_time, time_i);
		mb_get_date(verbose, last_time, time_j);
		fprintf(stdout, "  %d pings affected, from %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d to %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d\n",
		        npings, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_j[0], time_j[1], time_j[2],
		        time_j[3], time_j[4], time_j[5]);
	}

	if (path_a != nullptr)
		mb_esf_close(verbose, &esf_a, error);
	if (path_b != nullptr)
		mb_esf_close(verbose, &esf_b, error);
	return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	mb_path input = "";
	bool input_specified = false;
	action_t action = ACTION_LIST;
	int naction = 0;
	int version_a = VERSION_NOW;
	int version_b = VERSION_NOW;
	target_t target_mode = TARGET_VERSION;
	int target_version = 0;
	char target_tag[MB_HISTORY_NAME_LENGTH] = "";
	double target_time_d = 0.0;
	char tag_name[MB_HISTORY_NAME_LENGTH] = "";
	int keep = -1;
	double older_than_days = -1.0;
	bool dry_run = false;
	bool yes = false;
	bool force = false;
	bool process = false;

	/* process argument list */
	{
		const struct option options[] = {
			{"verbose", no_argument, nullptr, 0},
			{"help", no_argument, nullptr, 0},
			{"input", required_argument, nullptr, 0},
			{"list", no_argument, nullptr, 0},
			{"show", required_argument, nullptr, 0},
			{"diff", required_argument, nullptr, 0},
			{"restore", required_argument, nullptr, 0},
			{"restore-tag", required_argument, nullptr, 0},
			{"restore-date", required_argument, nullptr, 0},
			{"tag", required_argument, nullptr, 0},
			{"prune", no_argument, nullptr, 0},
			{"keep", required_argument, nullptr, 0},
			{"older-than", required_argument, nullptr, 0},
			{"dry-run", no_argument, nullptr, 0},
			{"yes", no_argument, nullptr, 0},
			{"force", no_argument, nullptr, 0},
			{"process", no_argument, nullptr, 0},
			{nullptr, 0, nullptr, 0}};
		int option_index;
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt_long(argc, argv, "VvHhI:i:", options, &option_index)) != -1)
		{
			switch (c) {
			/* long options all return c=0 */
			case 0:
				if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				else if (strcmp("input", options[option_index].name) == 0) {
					snprintf(input, sizeof(input), "%s", optarg);
					input_specified = true;
				}
				else if (strcmp("list", options[option_index].name) == 0) {
					action = ACTION_LIST;
					naction++;
				}
				else if (strcmp("show", options[option_index].name) == 0) {
					action = ACTION_SHOW;
					naction++;
					if (!parse_version(optarg, &version_a)) {
						fprintf(stderr, "\nBad version <%s> for --show\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("diff", options[option_index].name) == 0) {
					action = ACTION_DIFF;
					naction++;
					mb_path first = "";
					const char *colon = strchr(optarg, ':');
					if (colon == nullptr) {
						snprintf(first, sizeof(first), "%s", optarg);
						version_b = VERSION_NOW;
					}
					else {
						snprintf(first, sizeof(first), "%.*s", (int)(colon - optarg), optarg);
						if (!parse_version(colon + 1, &version_b)) {
							fprintf(stderr, "\nBad version <%s> for --diff\n", colon + 1);
							errflg = true;
						}
					}
					if (!parse_version(first, &version_a)) {
						fprintf(stderr, "\nBad version <%s> for --diff\n", first);
						errflg = true;
					}
				}
				else if (strcmp("restore", options[option_index].name) == 0) {
					action = ACTION_RESTORE;
					naction++;
					target_mode = TARGET_VERSION;
					if (!parse_version(optarg, &target_version) || target_version == VERSION_NOW) {
						fprintf(stderr, "\nBad version <%s> for --restore\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("restore-tag", options[option_index].name) == 0) {
					action = ACTION_RESTORE;
					naction++;
					target_mode = TARGET_TAG;
					snprintf(target_tag, sizeof(target_tag), "%s", optarg);
				}
				else if (strcmp("restore-date", options[option_index].name) == 0) {
					action = ACTION_RESTORE;
					naction++;
					target_mode = TARGET_DATE;
					if (!parse_date(verbose, optarg, &target_time_d)) {
						fprintf(stderr, "\nBad date <%s> for --restore-date (use yyyy-mm-dd[Thh:mm[:ss]], UTC)\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("tag", options[option_index].name) == 0) {
					action = ACTION_TAG;
					naction++;
					snprintf(tag_name, sizeof(tag_name), "%s", optarg);
					if (strlen(tag_name) == 0 || strcspn(tag_name, " \t\r\n") != strlen(tag_name)) {
						fprintf(stderr, "\nBad tag name <%s>: it must be one word\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("prune", options[option_index].name) == 0) {
					action = ACTION_PRUNE;
					naction++;
				}
				else if (strcmp("keep", options[option_index].name) == 0) {
					if (sscanf(optarg, "%d", &keep) != 1 || keep < 0) {
						fprintf(stderr, "\nBad number <%s> for --keep\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("older-than", options[option_index].name) == 0) {
					if (sscanf(optarg, "%lf", &older_than_days) != 1 || older_than_days < 0.0) {
						fprintf(stderr, "\nBad number of days <%s> for --older-than\n", optarg);
						errflg = true;
					}
				}
				else if (strcmp("dry-run", options[option_index].name) == 0) {
					dry_run = true;
				}
				else if (strcmp("yes", options[option_index].name) == 0) {
					yes = true;
				}
				else if (strcmp("force", options[option_index].name) == 0) {
					force = true;
				}
				else if (strcmp("process", options[option_index].name) == 0) {
					process = true;
				}
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'I':
			case 'i':
				snprintf(input, sizeof(input), "%s", optarg);
				input_specified = true;
				break;
			case '?':
				errflg = true;
			}
		}

		if (naction > 1) {
			fprintf(stderr, "\nOnly one of --list, --show, --diff, --restore*, --tag and --prune may be given\n");
			errflg = true;
		}
		if (action == ACTION_PRUNE && keep < 0 && older_than_days < 0.0) {
			fprintf(stderr, "\n--prune needs --keep and/or --older-than\n");
			errflg = true;
		}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}

		if (verbose == 1) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
			fprintf(stderr, "dbg2       input:            %s\n", input);
			fprintf(stderr, "dbg2       action:           %d\n", action);
			fprintf(stderr, "dbg2       version_a:        %d\n", version_a);
			fprintf(stderr, "dbg2       version_b:        %d\n", version_b);
			fprintf(stderr, "dbg2       target_mode:      %d\n", target_mode);
			fprintf(stderr, "dbg2       target_version:   %d\n", target_version);
			fprintf(stderr, "dbg2       target_tag:       %s\n", target_tag);
			fprintf(stderr, "dbg2       target_time_d:    %f\n", target_time_d);
			fprintf(stderr, "dbg2       tag_name:         %s\n", tag_name);
			fprintf(stderr, "dbg2       keep:             %d\n", keep);
			fprintf(stderr, "dbg2       older_than_days:  %f\n", older_than_days);
			fprintf(stderr, "dbg2       dry_run:          %d\n", dry_run);
			fprintf(stderr, "dbg2       yes:              %d\n", yes);
			fprintf(stderr, "dbg2       force:            %d\n", force);
			fprintf(stderr, "dbg2       process:          %d\n", process);
		}
	}

	if (!input_specified) {
		fprintf(stderr, "\nNo input swath file or datalist specified\n");
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_BAD_USAGE);
	}

	/* restores and prunes change files only with --yes; otherwise they are dry runs */
	const bool modify = (action == ACTION_RESTORE || action == ACTION_PRUNE);
	if (modify && !yes)
		dry_run = true;

	int error = MB_ERROR_NO_ERROR;
	int nfail = 0;
	int nchanged_files = 0;
	int nfiles = 0;

	/* figure out whether the input is a datalist */
	int format = 0;
	mb_path fileroot = "";
	mb_get_format(verbose, input, fileroot, &format, &error);
	error = MB_ERROR_NO_ERROR;
	const bool use_datalist = (format < 0);
	void *datalist = nullptr;
	if (use_datalist) {
		if (mb_datalist_open(verbose, &datalist, input, MB_DATALIST_LOOK_NO, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", input);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
	}

	/* loop over the swath files */
	bool done = false;
	while (!done) {
		mb_path swathfile = "";
		if (use_datalist) {
			mb_path dpath = "";
			double weight;
			int dformat;
			if (mb_datalist_read(verbose, datalist, swathfile, dpath, &dformat, &weight, &error) != MB_SUCCESS) {
				done = true;
				error = MB_ERROR_NO_ERROR;
				break;
			}
		}
		else {
			snprintf(swathfile, sizeof(swathfile), "%s", input);
			done = true;
		}
		nfiles++;

		/* get the edit save file and its history */
		mb_path esffile = "";
		int found = 0;
		mb_esf_check(verbose, swathfile, esffile, &found, &error);
		error = MB_ERROR_NO_ERROR;
		const bool esf_exists = file_exists(esffile);
		mb_path dir = "";
		mb_path base = "";
		if (mb_history_paths(verbose, esffile, dir, sizeof(dir), base, sizeof(base), &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to resolve history directory for %s\n", esffile);
			nfail++;
			continue;
		}
		int nentry = 0;
		struct mb_history_entry *entries = nullptr;
		if (mb_history_read_manifest(verbose, esffile, &nentry, &entries, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to read history manifest of %s\n", esffile);
			nfail++;
			continue;
		}
		int ntag = 0;
		struct tag_struct *tags = nullptr;
		read_tags(verbose, dir, &ntag, &tags, &error);
		error = MB_ERROR_NO_ERROR;

		/* current checksum */
		unsigned int crc_now = 0;
		long size_now = 0;
		if (esf_exists)
			mb_history_crc32(verbose, esffile, &crc_now, &size_now, &error);
		error = MB_ERROR_NO_ERROR;

		fprintf(stdout, "\nFile: %s  (edit save file %s%s)\n", swathfile, esffile, (esf_exists ? "" : ", not present"));

		/* ------------------------------------------------------------ list */
		if (action == ACTION_LIST) {
			if (nentry == 0) {
				fprintf(stdout, "  no history versions\n");
			}
			else {
				fprintf(stdout, "  ver  date (UTC)           before program    user          edits  tag / note\n");
			}
			for (int i = 0; i < nentry; i++) {
				const struct mb_history_entry *entry = &entries[i];
				mb_pathplus snapshot_path;
				snprintf(snapshot_path, sizeof(snapshot_path), "%s/%s", dir, entry->snapshot);
				struct edit_count_struct count;
				const bool present = count_edits(snapshot_path, &count);
				char date[32];
				format_utc(entry->time_d, date, sizeof(date));
				mb_path label = "";
				for (int j = 0; j < ntag; j++) {
					if (tags[j].version == entry->version && strcmp(tags[j].file, base) == 0) {
						const size_t len = strlen(label);
						snprintf(&label[len], sizeof(label) - len, "%s[%s]", (len > 0 ? " " : ""), tags[j].name);
					}
				}
				/* the note "tag <name>" repeats a tag already shown */
				if (strlen(entry->note) > 0 && !(strncmp(entry->note, "tag ", 4) == 0 && strlen(label) > 0)) {
					const size_t len = strlen(label);
					snprintf(&label[len], sizeof(label) - len, "%s%s", (len > 0 ? " " : ""), entry->note);
				}
				if (present)
					fprintf(stdout, "  %3d  %s  %-16.16s  %-12.12s %6d  %s\n", entry->version, date, entry->program,
					        entry->user, count.nedit, label);
				else
					fprintf(stdout, "  %3d  %s  %-16.16s  %-12.12s %6s  %s\n", entry->version, date, entry->program,
					        entry->user, "missing", label);
			}
			if (esf_exists) {
				struct stat file_status;
				stat(esffile, &file_status);
				char date[32];
				format_utc((double)file_status.st_mtime, date, sizeof(date));
				struct edit_count_struct count;
				count_edits(esffile, &count);

				/* note a restore if the current file is the target of the latest restore */
				mb_path label = "";
				for (int i = nentry - 1; i >= 0; i--) {
					int vfrom = 0;
					int vto = 0;
					if (sscanf(entries[i].note, "restore (v%d -> v%d)", &vfrom, &vto) == 2) {
						const struct mb_history_entry *to = find_entry(nentry, entries, vto);
						if (to != nullptr && to->crc32 == crc_now && to->size == size_now)
							snprintf(label, sizeof(label), "restored from v%d", vto);
						break;
					}
				}
				fprintf(stdout, "  now  %s  %-16.16s  %-12.12s %6d  %s\n", date, "(current)", "", count.nedit, label);
			}
		}

		/* ------------------------------------------------------------ show */
		else if (action == ACTION_SHOW) {
			mb_pathplus path;
			const struct mb_history_entry *entry = nullptr;
			if (version_a == VERSION_NOW) {
				snprintf(path, sizeof(path), "%s", esffile);
			}
			else if ((entry = find_entry(nentry, entries, version_a)) != nullptr) {
				snprintf(path, sizeof(path), "%s/%s", dir, entry->snapshot);
			}
			else {
				fprintf(stderr, "  version %d not found\n", version_a);
				nfail++;
			}
			if (version_a == VERSION_NOW || entry != nullptr) {
				struct edit_count_struct count;
				if (!count_edits(path, &count)) {
					fprintf(stderr, "  unable to read %s\n", path);
					nfail++;
				}
				else {
					if (entry != nullptr) {
						char date[32];
						format_utc(entry->time_d, date, sizeof(date));
						fprintf(stdout, "  Version:        %d\n", entry->version);
						fprintf(stdout, "  Snapshot:       %s\n", path);
						fprintf(stdout, "  Saved (UTC):    %s\n", date);
						fprintf(stdout, "  Before program: %s\n", entry->program);
						fprintf(stdout, "  User:           %s\n", entry->user);
						fprintf(stdout, "  Host:           %s\n", entry->host);
						fprintf(stdout, "  Size:           %ld bytes\n", entry->size);
						fprintf(stdout, "  CRC-32:         %8.8x\n", entry->crc32);
						if (strlen(entry->note) > 0)
							fprintf(stdout, "  Note:           %s\n", entry->note);
						for (int j = 0; j < ntag; j++)
							if (tags[j].version == entry->version && strcmp(tags[j].file, base) == 0)
								fprintf(stdout, "  Tag:            %s\n", tags[j].name);
					}
					else {
						fprintf(stdout, "  Version:        now (current file)\n");
						fprintf(stdout, "  File:           %s\n", path);
					}
					fprintf(stdout, "  Edit events:    %d (flag %d, unflag %d, zero %d, filter %d, other %d)\n", count.nedit,
					        count.nflag, count.nunflag, count.nzero, count.nfilter, count.nother);

					/* print the header stored at the start of the edit save file */
					FILE *fp = fopen(path, "rb");
					char header[MB_PATH_MAXLINE + 1];
					if (fp != nullptr && fread(header, MB_PATH_MAXLINE, 1, fp) == 1 && strncmp(header, "ESFVERSION", 10) == 0) {
						header[MB_PATH_MAXLINE] = '\0';
						fprintf(stdout, "  File header:\n");
						char *line = header;
						while (line != nullptr && *line != '\0') {
							char *next = strchr(line, '\n');
							if (next != nullptr)
								*next = '\0';
							fprintf(stdout, "    %s\n", line);
							line = (next != nullptr ? next + 1 : nullptr);
						}
					}
					if (fp != nullptr)
						fclose(fp);
				}
			}
		}

		/* ------------------------------------------------------------ diff */
		else if (action == ACTION_DIFF) {
			mb_pathplus path_a = "";
			mb_pathplus path_b = "";
			char label_a[32];
			char label_b[32];
			bool ok = true;
			const int versions[2] = {version_a, version_b};
			char *paths[2] = {path_a, path_b};
			char *labels[2] = {label_a, label_b};
			for (int k = 0; k < 2; k++) {
				if (versions[k] == VERSION_NOW) {
					snprintf(labels[k], 32, "now");
					if (esf_exists)
						snprintf(paths[k], sizeof(mb_pathplus), "%s", esffile);
				}
				else {
					snprintf(labels[k], 32, "v%d", versions[k]);
					const struct mb_history_entry *entry = find_entry(nentry, entries, versions[k]);
					if (entry == nullptr) {
						fprintf(stderr, "  version %d not found\n", versions[k]);
						ok = false;
					}
					else {
						snprintf(paths[k], sizeof(mb_pathplus), "%s/%s", dir, entry->snapshot);
					}
				}
			}
			if (!ok) {
				nfail++;
			}
			else if (diff_files(verbose, (strlen(path_a) > 0 ? path_a : nullptr), label_a,
			                    (strlen(path_b) > 0 ? path_b : nullptr), label_b, verbose > 0, &error) != MB_SUCCESS) {
				nfail++;
			}
		}

		/* ------------------------------------------------------------ restore */
		else if (action == ACTION_RESTORE) {
			const struct mb_history_entry *target = nullptr;
			bool skip = false;
			if (target_mode == TARGET_VERSION) {
				target = find_entry(nentry, entries, target_version);
				if (target == nullptr) {
					fprintf(stderr, "  version %d not found\n", target_version);
					nfail++;
					skip = true;
				}
			}
			else if (target_mode == TARGET_TAG) {
				for (int j = 0; j < ntag && target == nullptr; j++)
					if (strcmp(tags[j].name, target_tag) == 0 && strcmp(tags[j].file, base) == 0)
						target = find_entry(nentry, entries, tags[j].version);
				if (target == nullptr) {
					fprintf(stdout, "  tag %s not set for this file: unchanged\n", target_tag);
					skip = true;
				}
			}
			else {
				/* a snapshot taken at time T holds the state at T, so the state at
				   the requested time is the first snapshot taken at or after it */
				for (int i = 0; i < nentry; i++) {
					if (entries[i].time_d >= target_time_d && (target == nullptr || entries[i].version < target->version))
						target = &entries[i];
				}
				if (target == nullptr) {
					fprintf(stdout, "  no change since that date: unchanged\n");
					skip = true;
				}
				else {
					/* the history of an edit save file created before mbhistory
					   recorded creations starts with its first rewrite, so the
					   state before the oldest version is not known */
					bool oldest = true;
					for (int i = 0; i < nentry && oldest; i++)
						if (entries[i].version < target->version)
							oldest = false;
					if (oldest && strcmp(target->note, MB_HISTORY_NOTE_CREATED) != 0) {
						char date[32];
						format_utc(target->time_d, date, sizeof(date));
						fprintf(stderr,
						        "  warning: edits made before %s were not recorded; version %d is the oldest known state\n",
						        date, target->version);
					}
				}
			}

			mb_pathplus target_path = "";
			if (!skip) {
				snprintf(target_path, sizeof(target_path), "%s/%s", dir, target->snapshot);
				if (!file_exists(target_path)) {
					fprintf(stderr, "  snapshot file of version %d is missing: %s\n", target->version, target_path);
					nfail++;
					skip = true;
				}
				else if (esf_exists && crc_now == target->crc32 && size_now == target->size) {
					fprintf(stdout, "  current edits already match version %d: unchanged\n", target->version);
					skip = true;
				}
			}

			if (!skip) {
				diff_files(verbose, (esf_exists ? esffile : nullptr), "now", target_path, "restored", verbose > 0, &error);
				error = MB_ERROR_NO_ERROR;
			}

			if (!skip && dry_run) {
				fprintf(stdout, "  would restore version %d (dry run)\n", target->version);
			}
			else if (!skip) {
				/* lock the swath file as bathymetry editors do */
				if (mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error) != MB_SUCCESS) {
					bool locked = false;
					int lock_purpose = 0;
					mb_path lock_program = "";
					mb_path lock_user = "";
					mb_path lock_cpu = "";
					mb_path lock_date = "";
					mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date,
					               &error);
					fprintf(stderr, "  unable to restore: file is locked by %s (user %s, cpu %s, %s)\n", lock_program,
					        lock_user, lock_cpu, lock_date);
					nfail++;
				}
				else {
					int status = MB_SUCCESS;

					/* keep the current state so that the restore can be undone */
					if (esf_exists) {
						char note[MB_HISTORY_NAME_LENGTH];
						snprintf(note, sizeof(note), "restore (v%%d -> v%d)", target->version);
						int version_saved = 0;
						status = mb_history_snapshot(verbose, esffile, program_name, note, MB_HISTORY_MODE_FORCE,
						                             &version_saved, &error);
						if (status != MB_SUCCESS)
							fprintf(stderr, "  unable to save the current edits before restoring: unchanged\n");
						else
							fprintf(stdout, "  current edits saved as version %d\n", version_saved);
					}

					/* copy the snapshot to a temporary file, check it, then replace the edit save file */
					mb_pathplus tmpfile;
					snprintf(tmpfile, sizeof(tmpfile), "%s.mbhistory.tmp", esffile);
					if (status == MB_SUCCESS)
						status = mb_history_copy(verbose, target_path, tmpfile, &error);
					if (status == MB_SUCCESS) {
						unsigned int crc_check = 0;
						long size_check = 0;
						status = mb_history_crc32(verbose, tmpfile, &crc_check, &size_check, &error);
						if (status == MB_SUCCESS && (crc_check != target->crc32 || size_check != target->size)) {
							fprintf(stderr, "  snapshot of version %d does not match its checksum\n", target->version);
							status = MB_FAILURE;
						}
					}
					if (status == MB_SUCCESS) {
#ifdef _WIN32
						remove(esffile);
#endif
						if (rename(tmpfile, esffile) != 0)
							status = MB_FAILURE;
					}
					if (status != MB_SUCCESS) {
						remove(tmpfile);
						fprintf(stderr, "  restore of version %d failed\n", target->version);
						nfail++;
					}
					else {
						fprintf(stdout, "  restored version %d\n", target->version);
						nchanged_files++;

						/* make sure mbprocess uses the edit save file */
						if (!esf_exists)
							mb_pr_update_edit(verbose, swathfile, MBP_EDIT_ON, esffile, &error);
					}

					mb_pr_unlockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);
					error = MB_ERROR_NO_ERROR;

					if (status == MB_SUCCESS && process) {
						/* force processing: mbprocess compares file times in whole
						   seconds and could otherwise treat the output as up to date */
						mb_pathplusplus command;
						snprintf(command, sizeof(command), "mbprocess -I \"%s\" --force", swathfile);
						fprintf(stdout, "  running: %s\n", command);
						fflush(stdout);
						fflush(stderr);
						if (system(command) != 0) {
							fprintf(stderr, "  mbprocess failed for %s\n", swathfile);
							nfail++;
						}
					}
				}
			}
		}

		/* ------------------------------------------------------------ tag */
		else if (action == ACTION_TAG) {
			bool exists = false;
			for (int j = 0; j < ntag; j++)
				if (strcmp(tags[j].name, tag_name) == 0 && strcmp(tags[j].file, base) == 0)
					exists = true;
			if (!esf_exists) {
				fprintf(stdout, "  no edit save file: nothing to tag\n");
			}
			else if (exists && !force) {
				fprintf(stderr, "  tag %s already set for this file (use --force to move it)\n", tag_name);
				nfail++;
			}
			else {
				char note[MB_HISTORY_NAME_LENGTH];
				snprintf(note, sizeof(note), "tag %s", tag_name);
				int version_tag = 0;
				if (mb_history_snapshot(verbose, esffile, program_name, note, MB_HISTORY_MODE_DEFAULT, &version_tag, &error) !=
				    MB_SUCCESS) {
					fprintf(stderr, "  unable to save a snapshot for tag %s\n", tag_name);
					nfail++;
				}
				else {
					/* rewrite the tags, replacing this file's old tag of the same name */
					struct tag_struct *newtags = nullptr;
					int nnewtag = 0;
					int status =
					    mb_mallocd(verbose, __FILE__, __LINE__, (ntag + 1) * sizeof(struct tag_struct), (void **)&newtags, &error);
					if (status == MB_SUCCESS) {
						for (int j = 0; j < ntag; j++)
							if (!(strcmp(tags[j].name, tag_name) == 0 && strcmp(tags[j].file, base) == 0))
								newtags[nnewtag++] = tags[j];
						struct tag_struct *tag = &newtags[nnewtag++];
						memset(tag, 0, sizeof(struct tag_struct));
						snprintf(tag->name, sizeof(tag->name), "%s", tag_name);
						snprintf(tag->file, sizeof(tag->file), "%s", base);
						tag->version = version_tag;
						const time_t now = time(nullptr);
						const struct tm *tm_utc = gmtime(&now);
						if (tm_utc != nullptr)
							strftime(tag->utc, sizeof(tag->utc), "%Y-%m-%dT%H:%M:%SZ", tm_utc);
						status = write_tags(dir, nnewtag, newtags, &error);
						mb_freed(verbose, __FILE__, __LINE__, (void **)&newtags, &error);
					}
					if (status == MB_SUCCESS) {
						fprintf(stdout, "  tagged version %d as %s\n", version_tag, tag_name);
						nchanged_files++;
					}
					else {
						fprintf(stderr, "  unable to write the tags file in %s\n", dir);
						nfail++;
					}
				}
			}
		}

		/* ------------------------------------------------------------ prune */
		else if (action == ACTION_PRUNE) {
			const double now = (double)time(nullptr);
			int nremove = 0;
			int nkept = 0;
			bool *remove_entry = nullptr;
			if (nentry > 0 &&
			    mb_mallocd(verbose, __FILE__, __LINE__, nentry * sizeof(bool), (void **)&remove_entry, &error) != MB_SUCCESS) {
				nfail++;
				nentry = 0;
			}
			for (int i = 0; i < nentry; i++) {
				/* rank of this version from the newest (0 = newest) */
				int rank = 0;
				for (int k = 0; k < nentry; k++)
					if (entries[k].version > entries[i].version)
						rank++;
				bool candidate = true;
				if (keep >= 0 && rank < keep)
					candidate = false;
				if (older_than_days >= 0.0 && entries[i].time_d > now - older_than_days * 86400.0)
					candidate = false;
				bool tagged = false;
				for (int j = 0; j < ntag; j++)
					if (tags[j].version == entries[i].version && strcmp(tags[j].file, base) == 0)
						tagged = true;
				if (candidate && tagged && !force) {
					candidate = false;
					nkept++;
				}
				remove_entry[i] = candidate;
				if (candidate) {
					nremove++;
					fprintf(stdout, "  %s version %d (%s)\n", (dry_run ? "would remove" : "removing"), entries[i].version,
					        entries[i].snapshot);
				}
			}
			if (nkept > 0)
				fprintf(stdout, "  %d tagged versions kept (use --force to remove them)\n", nkept);
			if (nremove == 0)
				fprintf(stdout, "  nothing to prune\n");
			if (nremove > 0 && !dry_run) {
				int nnew = 0;
				for (int i = 0; i < nentry; i++) {
					if (remove_entry[i]) {
						mb_pathplus snapshot_path;
						snprintf(snapshot_path, sizeof(snapshot_path), "%s/%s", dir, entries[i].snapshot);
						remove(snapshot_path);
					}
					else {
						entries[nnew++] = entries[i];
					}
				}
				if (mb_history_write_manifest(verbose, esffile, nnew, entries, &error) != MB_SUCCESS) {
					fprintf(stderr, "  unable to rewrite the manifest of %s\n", esffile);
					nfail++;
				}
				else {
					nchanged_files++;
				}
				/* drop tags that point to removed versions */
				if (force && ntag > 0) {
					int nnewtag = 0;
					for (int j = 0; j < ntag; j++) {
						bool keep_tag = true;
						if (strcmp(tags[j].file, base) == 0 && find_entry(nnew, entries, tags[j].version) == nullptr)
							keep_tag = false;
						if (keep_tag)
							tags[nnewtag++] = tags[j];
					}
					if (nnewtag != ntag && write_tags(dir, nnewtag, tags, &error) != MB_SUCCESS) {
						fprintf(stderr, "  unable to rewrite the tags file in %s\n", dir);
						nfail++;
					}
				}
			}
			if (remove_entry != nullptr)
				mb_freed(verbose, __FILE__, __LINE__, (void **)&remove_entry, &error);
		}

		if (entries != nullptr)
			mb_freed(verbose, __FILE__, __LINE__, (void **)&entries, &error);
		if (tags != nullptr)
			mb_freed(verbose, __FILE__, __LINE__, (void **)&tags, &error);
		error = MB_ERROR_NO_ERROR;
	}

	if (use_datalist) {
		mb_datalist_close(verbose, &datalist, &error);
		if (nfiles == 0) {
			fprintf(stderr, "\nNo swath files found in datalist %s (empty files are skipped)\n", input);
			nfail++;
		}
	}

	if (modify && dry_run)
		fprintf(stdout, "\nDry run: nothing was changed.%s\n", (yes ? "" : " Add --yes to apply."));
	else if (action == ACTION_RESTORE && nchanged_files > 0 && !process)
		fprintf(stdout, "\nRun mbprocess on the restored files to update the processed data.\n");

	if (verbose >= 4)
		mb_memory_list(verbose, &error);

	exit(nfail > 0 ? MB_ERROR_BAD_PARAMETER : MB_ERROR_NO_ERROR);
}
/*--------------------------------------------------------------------*/
