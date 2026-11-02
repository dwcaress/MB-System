/*--------------------------------------------------------------------
 *    The MB-system:	mb_history.c	10/25/2026
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
 * mb_history.c contains functions that keep a version history of files
 * that MB-System programs modify, starting with edit save (*.esf) files.
 *
 * Before a file is modified, a byte-for-byte copy (a snapshot) is written
 * to a directory named "mbhistory" next to the file:
 *
 *     <dir>/<file>
 *     <dir>/mbhistory/<file>.v0001_20261020T140211
 *     <dir>/mbhistory/<file>.v0002_20261020T164053
 *     <dir>/mbhistory/<file>.manifest
 *
 * The version number is followed by the UTC date and time of the snapshot.
 * A snapshot holds the state of the file at that time, just before the
 * program named in the manifest modified it. A new edit save file is also
 * saved when it is created (no edits, note "created"), so that the history
 * shows when editing began. No snapshot is made if the file is unchanged
 * since the newest snapshot; for edit save files, a rewrite that changes
 * only the program, user, cpu and date lines of the header counts as
 * unchanged. The manifest is a tab
 * separated text file with one line per snapshot:
 *
 *     version utc time_d program user host size crc32 snapshot note
 *
 * Each file has its own manifest, so programs working on different files
 * at the same time never write to the same manifest. The program mbhistory
 * lists, compares, tags, prunes and restores snapshots.
 *
 * Author:	C. dos Santos Ferreira
 * Date:	October 25, 2026
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#endif

#include "mb_define.h"
#include "mb_process.h"
#include "mb_status.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define MB_HISTORY_BUFFER_SIZE 65536
#define MB_HISTORY_MAX_RETRY 100
#define MB_HISTORY_ONCE_MAX 4096
#define MB_HISTORY_NFIELD 10

/* hashes of the files already snapshotted by this process (MB_HISTORY_MODE_ONCE) */
static uint64_t mb_history_once_hash[MB_HISTORY_ONCE_MAX];
static int mb_history_once_num = 0;

/* CRC-32 (IEEE 802.3, same as zlib crc32) lookup table */
static unsigned int mb_history_crc_table[256];
static bool mb_history_crc_table_set = false;

/*--------------------------------------------------------------------*/
static uint64_t mb_history_hash_path(const char *path) {
	/* FNV-1a 64 bit hash */
	uint64_t hash = 14695981039346656037ULL;
	for (const unsigned char *c = (const unsigned char *)path; *c != '\0'; c++) {
		hash ^= (uint64_t)(*c);
		hash *= 1099511628211ULL;
	}
	return (hash);
}
/*--------------------------------------------------------------------*/
static bool mb_history_once_check(const char *path) {
	const uint64_t hash = mb_history_hash_path(path);
	for (int i = 0; i < mb_history_once_num; i++) {
		if (mb_history_once_hash[i] == hash)
			return (true);
	}
	return (false);
}
/*--------------------------------------------------------------------*/
static void mb_history_once_set(const char *path) {
	if (!mb_history_once_check(path) && mb_history_once_num < MB_HISTORY_ONCE_MAX) {
		mb_history_once_hash[mb_history_once_num] = mb_history_hash_path(path);
		mb_history_once_num++;
	}
}
/*--------------------------------------------------------------------*/
/* replace tabs and line ends so that a value fits in one manifest field */
static void mb_history_clean_field(char *value) {
	for (char *c = value; *c != '\0'; c++) {
		if (*c == '\t' || *c == '\n' || *c == '\r')
			*c = ' ';
	}
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_paths returns the history directory for a file
 * 	(the directory MB_HISTORY_DIR next to the file) and the file name
 * 	without its directory. */
int mb_history_paths(int verbose, const char *file, char *dir, size_t dirsize, char *base, size_t basesize, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* find the last directory separator */
	const char *slash = strrchr(file, '/');
#ifdef _WIN32
	const char *backslash = strrchr(file, '\\');
	if (backslash != NULL && (slash == NULL || backslash > slash))
		slash = backslash;
#endif

	int nchar;
	if (slash == NULL) {
		nchar = snprintf(dir, dirsize, "%s", MB_HISTORY_DIR);
		if (nchar >= 0 && (size_t)nchar < dirsize)
			nchar = snprintf(base, basesize, "%s", file);
	}
	else {
		nchar = snprintf(dir, dirsize, "%.*s/%s", (int)(slash - file), file, MB_HISTORY_DIR);
		if (nchar >= 0 && (size_t)nchar < dirsize)
			nchar = snprintf(base, basesize, "%s", slash + 1);
	}
	if (nchar < 0 || (size_t)nchar >= dirsize || (size_t)nchar >= basesize || strlen(base) == 0) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       dir:        %s\n", dir);
		fprintf(stderr, "dbg2       base:       %s\n", base);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_crc32 returns the CRC-32 checksum (as computed by
 * 	zlib crc32) and the size in bytes of a file. */
int mb_history_crc32(int verbose, const char *file, unsigned int *crc32, long *size, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*crc32 = 0;
	*size = 0;

	if (!mb_history_crc_table_set) {
		for (unsigned int i = 0; i < 256; i++) {
			unsigned int c = i;
			for (int k = 0; k < 8; k++)
				c = (c & 1) ? (0xEDB88320U ^ (c >> 1)) : (c >> 1);
			mb_history_crc_table[i] = c;
		}
		mb_history_crc_table_set = true;
	}

	FILE *fp = fopen(file, "rb");
	if (fp == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
	}
	else {
		unsigned char buffer[MB_HISTORY_BUFFER_SIZE];
		unsigned int crc = 0xFFFFFFFFU;
		size_t nread;
		while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
			for (size_t i = 0; i < nread; i++)
				crc = mb_history_crc_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
			*size += (long)nread;
		}
		if (ferror(fp)) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		fclose(fp);
		*crc32 = crc ^ 0xFFFFFFFFU;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       crc32:      %8.8x\n", *crc32);
		fprintf(stderr, "dbg2       size:       %ld\n", *size);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* copy an open input stream to an open output stream */
static int mb_history_copy_stream(FILE *ifp, FILE *ofp, int *error) {
	int status = MB_SUCCESS;
	unsigned char buffer[MB_HISTORY_BUFFER_SIZE];
	size_t nread;
	while (status == MB_SUCCESS && (nread = fread(buffer, 1, sizeof(buffer), ifp)) > 0) {
		if (fwrite(buffer, 1, nread, ofp) != nread) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}
	if (status == MB_SUCCESS && ferror(ifp)) {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}
	return (status);
}
/*--------------------------------------------------------------------*/
/* true if two edit save files hold the same edits and differ at most in
   the program, user, cpu and date lines of their ESFVERSION03 header
   (MB_PATH_MAXLINE bytes), which every rewrite of an esf file updates */
static bool mb_history_same_edits(const char *file_a, const char *file_b) {
	FILE *fpa = fopen(file_a, "rb");
	FILE *fpb = fopen(file_b, "rb");
	bool same = (fpa != NULL && fpb != NULL);

	/* the format and mode lines (the first two header lines) must match */
	char header_a[MB_PATH_MAXLINE];
	char header_b[MB_PATH_MAXLINE];
	if (same)
		same = (fread(header_a, MB_PATH_MAXLINE, 1, fpa) == 1 && fread(header_b, MB_PATH_MAXLINE, 1, fpb) == 1);
	if (same)
		same = (strncmp(header_a, "ESFVERSION03", 12) == 0 && strncmp(header_b, "ESFVERSION03", 12) == 0);
	if (same) {
		const char *end_a = memchr(header_a, '\n', MB_PATH_MAXLINE);
		if (end_a != NULL)
			end_a = memchr(end_a + 1, '\n', MB_PATH_MAXLINE - (size_t)(end_a + 1 - header_a));
		same = (end_a != NULL && memcmp(header_a, header_b, (size_t)(end_a - header_a)) == 0);
	}

	/* the edits after the header must be identical */
	while (same) {
		unsigned char buffer_a[MB_PATH_MAXLINE];
		unsigned char buffer_b[MB_PATH_MAXLINE];
		const size_t nread_a = fread(buffer_a, 1, sizeof(buffer_a), fpa);
		const size_t nread_b = fread(buffer_b, 1, sizeof(buffer_b), fpb);
		if (nread_a != nread_b || memcmp(buffer_a, buffer_b, nread_a) != 0 || ferror(fpa) || ferror(fpb))
			same = false;
		else if (nread_a == 0)
			break;
	}

	if (fpa != NULL)
		fclose(fpa);
	if (fpb != NULL)
		fclose(fpb);
	return (same);
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_copy copies a file byte for byte. */
int mb_history_copy(int verbose, const char *ifile, const char *ofile, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       ifile:      %s\n", ifile);
		fprintf(stderr, "dbg2       ofile:      %s\n", ofile);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	FILE *ifp = fopen(ifile, "rb");
	if (ifp == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
	}
	else {
		FILE *ofp = fopen(ofile, "wb");
		if (ofp == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
		else {
			status = mb_history_copy_stream(ifp, ofp, error);
			if (fclose(ofp) != 0 && status == MB_SUCCESS) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
		}
		fclose(ifp);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* parse one manifest line, returns true if the line holds an entry */
static bool mb_history_parse_line(char *line, struct mb_history_entry *entry) {
	/* strip the line end */
	const size_t len = strcspn(line, "\r\n");
	line[len] = '\0';
	if (line[0] == '#' || line[0] == '\0')
		return (false);

	/* split into tab separated fields, keeping empty fields */
	char *field[MB_HISTORY_NFIELD];
	int nfield = 0;
	char *start = line;
	while (nfield < MB_HISTORY_NFIELD) {
		field[nfield++] = start;
		char *tab = strchr(start, '\t');
		if (tab == NULL)
			break;
		*tab = '\0';
		start = tab + 1;
	}
	if (nfield < MB_HISTORY_NFIELD - 1)
		return (false);

	memset(entry, 0, sizeof(struct mb_history_entry));
	if (sscanf(field[0], "%d", &entry->version) != 1 || entry->version < 1)
		return (false);
	snprintf(entry->utc, sizeof(entry->utc), "%s", field[1]);
	entry->time_d = atof(field[2]);
	snprintf(entry->program, sizeof(entry->program), "%s", field[3]);
	snprintf(entry->user, sizeof(entry->user), "%s", field[4]);
	snprintf(entry->host, sizeof(entry->host), "%s", field[5]);
	entry->size = atol(field[6]);
	entry->crc32 = (unsigned int)strtoul(field[7], NULL, 16);
	snprintf(entry->snapshot, sizeof(entry->snapshot), "%s", field[8]);
	if (nfield >= MB_HISTORY_NFIELD)
		snprintf(entry->note, sizeof(entry->note), "%s", field[9]);
	return (true);
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_read_manifest reads all snapshot entries of a file.
 * 	If entries are found, *entries is allocated with mb_mallocd and must be
 * 	freed by the caller with mb_freed. A missing manifest is not an error:
 * 	*nentry is then zero. */
int mb_history_read_manifest(int verbose, const char *file, int *nentry, struct mb_history_entry **entries, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
	}

	*nentry = 0;
	*entries = NULL;
	*error = MB_ERROR_NO_ERROR;

	mb_path dir;
	mb_path base;
	int status = mb_history_paths(verbose, file, dir, sizeof(dir), base, sizeof(base), error);
	mb_pathplus manifest;
	if (status == MB_SUCCESS) {
		const int nchar = snprintf(manifest, sizeof(manifest), "%s/%s%s", dir, base, MB_HISTORY_MANIFEST_SUFFIX);
		if (nchar < 0 || (size_t)nchar >= sizeof(manifest)) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
	}

	FILE *fp = NULL;
	if (status == MB_SUCCESS && (fp = fopen(manifest, "r")) != NULL) {
		/* count the lines to size the array */
		mb_pathplus line;
		int nline = 0;
		while (fgets(line, sizeof(line), fp) != NULL)
			nline++;
		if (nline > 0)
			status = mb_mallocd(verbose, __FILE__, __LINE__, nline * sizeof(struct mb_history_entry), (void **)entries, error);
		if (status == MB_SUCCESS && nline > 0) {
			rewind(fp);
			while (*nentry < nline && fgets(line, sizeof(line), fp) != NULL) {
				if (mb_history_parse_line(line, &(*entries)[*nentry]))
					(*nentry)++;
			}
			if (*nentry == 0)
				mb_freed(verbose, __FILE__, __LINE__, (void **)entries, error);
		}
		else if (status != MB_SUCCESS) {
			*entries = NULL;
		}
		fclose(fp);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nentry:     %d\n", *nentry);
		for (int i = 0; i < *nentry; i++)
			fprintf(stderr, "dbg2       entry[%d]:   v%d %s %s %ld %8.8x %s\n", i, (*entries)[i].version, (*entries)[i].utc,
			        (*entries)[i].program, (*entries)[i].size, (*entries)[i].crc32, (*entries)[i].snapshot);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
static void mb_history_write_header(FILE *fp) {
	fprintf(fp, "# mbhistory manifest version 1\n");
	fprintf(fp, "# version\tutc\ttime_d\tprogram\tuser\thost\tsize\tcrc32\tsnapshot\tnote\n");
}
/*--------------------------------------------------------------------*/
static void mb_history_write_entry(FILE *fp, const struct mb_history_entry *entry) {
	fprintf(fp, "%d\t%s\t%.0f\t%s\t%s\t%s\t%ld\t%8.8x\t%s\t%s\n", entry->version, entry->utc, entry->time_d, entry->program,
	        entry->user, entry->host, entry->size, entry->crc32, entry->snapshot, entry->note);
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_write_manifest replaces the manifest of a file
 * 	with the given entries (used when pruning). The new manifest is
 * 	written to a temporary file that then replaces the old one. */
int mb_history_write_manifest(int verbose, const char *file, int nentry, struct mb_history_entry *entries, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       nentry:     %d\n", nentry);
	}

	*error = MB_ERROR_NO_ERROR;

	mb_path dir;
	mb_path base;
	int status = mb_history_paths(verbose, file, dir, sizeof(dir), base, sizeof(base), error);
	mb_pathplus manifest;
	mb_pathplus manifest_tmp;
	if (status == MB_SUCCESS) {
		const int nchar = snprintf(manifest, sizeof(manifest), "%s/%s%s", dir, base, MB_HISTORY_MANIFEST_SUFFIX);
		const int nchar2 = snprintf(manifest_tmp, sizeof(manifest_tmp), "%s/%s%s.tmp", dir, base, MB_HISTORY_MANIFEST_SUFFIX);
		if (nchar < 0 || (size_t)nchar >= sizeof(manifest) || nchar2 < 0 || (size_t)nchar2 >= sizeof(manifest_tmp)) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
	}

	if (status == MB_SUCCESS) {
		FILE *fp = fopen(manifest_tmp, "w");
		if (fp == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
		else {
			mb_history_write_header(fp);
			for (int i = 0; i < nentry; i++)
				mb_history_write_entry(fp, &entries[i]);
			if (fclose(fp) != 0) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
		}
	}
	if (status == MB_SUCCESS) {
#ifdef _WIN32
		/* rename() does not replace an existing file on Windows */
		remove(manifest);
#endif
		if (rename(manifest_tmp, manifest) != 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_history_snapshot saves a copy of a file in its history
 * 	directory and appends an entry to its manifest.
 *
 * 	mode:  MB_HISTORY_MODE_DEFAULT - no new snapshot if the file is
 * 	                                 identical to the newest snapshot (for
 * 	                                 esf files: holds the same edits)
 * 	       MB_HISTORY_MODE_ONCE    - as default, and at most one snapshot
 * 	                                 per file per process (for programs
 * 	                                 that reopen the same file)
 * 	       MB_HISTORY_MODE_FORCE   - always make a new snapshot
 *
 * 	note:  free text stored in the manifest (may be NULL); the first "%d"
 * 	       in it is replaced by the new version number.
 *
 * 	On return *version is the version holding the current file content,
 * 	or 0 if no snapshot was needed (the file does not exist or is empty,
 * 	or MB_HISTORY_MODE_ONCE and the file was already handled); this is
 * 	not an error. */
int mb_history_snapshot(int verbose, const char *file, const char *program_name, const char *note, int mode, int *version,
                        int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       file:         %s\n", file);
		fprintf(stderr, "dbg2       program_name: %s\n", program_name);
		fprintf(stderr, "dbg2       note:         %s\n", (note != NULL ? note : ""));
		fprintf(stderr, "dbg2       mode:         %d\n", mode);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*version = 0;

	/* nothing to do if the file does not exist or is empty */
	struct stat file_status;
	const bool file_ok =
	    (stat(file, &file_status) == 0 && (file_status.st_mode & S_IFMT) == S_IFREG && file_status.st_size > 0);
	bool done = !file_ok;

	/* at most one snapshot per file per process if requested */
	if (!done && mode == MB_HISTORY_MODE_ONCE && mb_history_once_check(file))
		done = true;

	/* get the paths */
	mb_path dir;
	mb_path base;
	if (!done)
		status = mb_history_paths(verbose, file, dir, sizeof(dir), base, sizeof(base), error);

	/* get the checksum of the current file */
	unsigned int crc = 0;
	long size = 0;
	if (!done && status == MB_SUCCESS)
		status = mb_history_crc32(verbose, file, &crc, &size, error);

	/* read the existing manifest and find the newest version */
	int nentry = 0;
	struct mb_history_entry *entries = NULL;
	int newest = -1;
	if (!done && status == MB_SUCCESS)
		status = mb_history_read_manifest(verbose, file, &nentry, &entries, error);
	if (!done && status == MB_SUCCESS) {
		for (int i = 0; i < nentry; i++) {
			if (newest < 0 || entries[i].version > entries[newest].version)
				newest = i;
		}

		/* no new snapshot if the file is unchanged since the newest one, or if
		    only the header of an esf file was rewritten */
		bool unchanged = false;
		if (mode != MB_HISTORY_MODE_FORCE && newest >= 0 && entries[newest].size == size) {
			unchanged = (entries[newest].crc32 == crc);
			if (!unchanged) {
				mb_pathplus newest_path = "";
				const int nchar = snprintf(newest_path, sizeof(newest_path), "%s/%s", dir, entries[newest].snapshot);
				unchanged = (nchar > 0 && (size_t)nchar < sizeof(newest_path) && mb_history_same_edits(file, newest_path));
			}
		}
		if (unchanged) {
			*version = entries[newest].version;
			done = true;
			if (mode == MB_HISTORY_MODE_ONCE)
				mb_history_once_set(file);
		}
	}

	/* make the history directory if needed */
	if (!done && status == MB_SUCCESS) {
		struct stat dir_status;
		if (stat(dir, &dir_status) != 0) {
#ifdef _WIN32
			const int mkdir_status = mkdir(dir);
#else
			const int mkdir_status = mkdir(dir, 0775);
#endif
			if (mkdir_status != 0 && errno != EEXIST) {
				status = MB_FAILURE;
				*error = MB_ERROR_OPEN_FAIL;
			}
		}
		else if ((dir_status.st_mode & S_IFMT) != S_IFDIR) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
	}

	/* create the snapshot file, never overwriting an existing one */
	struct mb_history_entry entry;
	memset(&entry, 0, sizeof(struct mb_history_entry));
	mb_pathplus snapshot_path = "";
	FILE *ofp = NULL;
	if (!done && status == MB_SUCCESS) {
		const time_t now = time(NULL);
		const struct tm *tm_utc = gmtime(&now);
		char stamp[32] = "";
		if (tm_utc != NULL) {
			strftime(stamp, sizeof(stamp), "%Y%m%dT%H%M%S", tm_utc);
			strftime(entry.utc, sizeof(entry.utc), "%Y-%m-%dT%H:%M:%SZ", tm_utc);
		}
		entry.time_d = (double)now;
		entry.version = (newest >= 0 ? entries[newest].version + 1 : 1);
		for (int iretry = 0; ofp == NULL && status == MB_SUCCESS && iretry < MB_HISTORY_MAX_RETRY; iretry++) {
			const int nname = snprintf(entry.snapshot, sizeof(entry.snapshot), "%s.v%4.4d_%s", base, entry.version, stamp);
			const int nchar = snprintf(snapshot_path, sizeof(snapshot_path), "%s/%s", dir, entry.snapshot);
			if (nname < 0 || (size_t)nname >= sizeof(entry.snapshot) || nchar < 0 || (size_t)nchar >= sizeof(snapshot_path)) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_PARAMETER;
			}
			else {
				const int fd = open(snapshot_path, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, 0664);
				if (fd >= 0) {
					ofp = fdopen(fd, "wb");
					if (ofp == NULL) {
						close(fd);
						status = MB_FAILURE;
						*error = MB_ERROR_OPEN_FAIL;
					}
				}
				else if (errno == EEXIST) {
					entry.version++;
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_OPEN_FAIL;
				}
			}
		}
		if (ofp == NULL && status == MB_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
	}

	/* copy the file into the snapshot */
	if (!done && status == MB_SUCCESS) {
		FILE *ifp = fopen(file, "rb");
		if (ifp == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
		else {
			status = mb_history_copy_stream(ifp, ofp, error);
			fclose(ifp);
		}
		if (fclose(ofp) != 0 && status == MB_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		ofp = NULL;

		/* check that the snapshot matches the file */
		unsigned int crc_check = 0;
		long size_check = 0;
		if (status == MB_SUCCESS)
			status = mb_history_crc32(verbose, snapshot_path, &crc_check, &size_check, error);
		if (status == MB_SUCCESS && (crc_check != crc || size_check != size)) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status != MB_SUCCESS)
			remove(snapshot_path);
	}

	/* append the entry to the manifest */
	if (!done && status == MB_SUCCESS) {
		char user[256] = "";
		char host[256] = "";
		char date[32] = "";
		int error_user = MB_ERROR_NO_ERROR;
		mb_user_host_date(verbose, user, host, date, &error_user);
		snprintf(entry.program, sizeof(entry.program), "%s", (program_name != NULL ? program_name : ""));
		snprintf(entry.user, sizeof(entry.user), "%s", user);
		snprintf(entry.host, sizeof(entry.host), "%s", host);
		entry.size = size;
		entry.crc32 = crc;
		if (note != NULL) {
			const char *pct = strstr(note, "%d");
			if (pct != NULL)
				snprintf(entry.note, sizeof(entry.note), "%.*s%d%s", (int)(pct - note), note, entry.version, pct + 2);
			else
				snprintf(entry.note, sizeof(entry.note), "%s", note);
		}
		mb_history_clean_field(entry.program);
		mb_history_clean_field(entry.user);
		mb_history_clean_field(entry.host);
		mb_history_clean_field(entry.note);

		mb_pathplus manifest;
		const int nchar = snprintf(manifest, sizeof(manifest), "%s/%s%s", dir, base, MB_HISTORY_MANIFEST_SUFFIX);
		struct stat manifest_status;
		const bool manifest_new = (stat(manifest, &manifest_status) != 0);
		FILE *fp = NULL;
		if (nchar < 0 || (size_t)nchar >= sizeof(manifest) || (fp = fopen(manifest, "a")) == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
		}
		else {
			if (manifest_new)
				mb_history_write_header(fp);
			mb_history_write_entry(fp, &entry);
			if (fclose(fp) != 0) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
		}
		if (status == MB_SUCCESS) {
			*version = entry.version;
			if (mode == MB_HISTORY_MODE_ONCE)
				mb_history_once_set(file);
		}
	}

	if (entries != NULL) {
		int error_free = MB_ERROR_NO_ERROR;
		mb_freed(verbose, __FILE__, __LINE__, (void **)&entries, &error_free);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       version:    %d\n", *version);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
