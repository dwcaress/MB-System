///
/// @file mkvconf.h
/// @authors k. headley
/// @date 03 apr 2020

/// Key/Value config file reader
/// user implements value parser function

/// @sa doxygen-examples.c for more examples of Doxygen markup

/////////////////////////
// Terms of use
/////////////////////////

// Copyright Information

// Copyright 2002-YYYY MBARI
// Monterey Bay Aquarium Research Institute, all rights reserved.
//
// Terms of Use
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version. You can access the GPLv3 license at
// http://www.gnu.org/licenses/gpl-3.0.html
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details
// (http://www.gnu.org/licenses/gpl-3.0.html)
//
// MBARI provides the documentation and software code "as is", with no warranty,
// express or implied, as to the software, title, non-infringement of third party
// rights, merchantability, or fitness for any particular purpose, the accuracy of
// the code, or the performance or results which you may obtain from its use. You
// assume the entire risk associated with use of the code, and you agree to be
// responsible for the entire cost of repair or servicing of the program with
// which you are using the code.
//
// In no event shall MBARI be liable for any damages, whether general, special,
// incidental or consequential damages, arising out of your use of the software,
// including, but not limited to, the loss or corruption of your data or damages
// of any kind resulting from use of the software, any prohibited use, or your
// inability to use the software. You agree to defend, indemnify and hold harmless
// MBARI and its officers, directors, and employees against any claim, loss,
// liability or expense, including attorneys' fees, resulting from loss of or
// damage to property or the injury to or death of any person arising out of the
// use of the software.
//
// The MBARI software is provided without obligation on the part of the
// Monterey Bay Aquarium Research Institute to assist in its use, correction,
// modification, or enhancement.
//
// MBARI assumes no responsibility or liability for any third party and/or
// commercial software required for the database or applications. Licensee agrees
// to obtain and maintain valid licenses for any additional third party software
// required.

#ifndef MKVCONF_H
#define MKVCONF_H

/// Includes
#include "mframe.h"

/// Macros
#define MF_MEM_CHKFREE(s) if(NULL!=s)free(s)
#define MF_MEM_CHKINVALIDATE(s) do{\
if(NULL!=s)free(s);\
s=NULL;\
}while(0)


#define MKVC_DEL_DFL "="
#define MKVC_LINE_BUF_LEN 512

/// Type Definitions
typedef struct mkvc_s mkvc_reader_t;

// user-defined value parse function
typedef int (* mkvc_parse_fn)(char *key, char *val, void *cfg);

/// Exports

#ifdef __cplusplus
extern "C" {
#endif

    /// @fn mkvc_reader_t *mkvc_new(const char *file, const char *del, mkvc_parse_fn parser)
    /// @brief get new loader instance
    /// @return new loader instance on success, NULL otherwise
    mkvc_reader_t *mkvc_new(const char *file, const char *del, mkvc_parse_fn parser);

    /// @fn void mkvc_destroy(mkvc_reader_t **pself)
    /// @brief release loader instance resources
    /// @return none
    void mkvc_destroy(mkvc_reader_t **pself);
    
    /// @fn int mkvc_load_config(char *config_path)
    /// @brief load configuration file (key/value pairs)
    /// @param[in] self  mvkc instance
    /// @param[in] cfg   user-defined configuration structure (for parser)
    /// @param[in] r_par return number of parsed parameters
    /// @param[in] r_inv return number of invalid parameters
    /// @param[in] r_err return number of parse errors 
    /// @return 0 on success, -1 otherwise
    int mkvc_load_config(mkvc_reader_t *self, void *cfg, int *r_par, int *r_inv, int *r_err);

    /// @fn int mkvc_parse_kv(char *line, const char *del, char **pkey, char **pval)
    /// @brief parse and trim key/value pair. Both key and value must exist.
    /// Equivalent to mkvc_parse_kx(...true). Input may contain comments and leading/trailing whitespace.
    /// @param[in] line input string containing key/value
    /// @param[in] del  key/value delimiter
    /// @param[in] pkey pointer to key char array (dynamically created if key is NULL)
    /// @param[in] pval pointer to key char array (dynamically created if key is NULL)
    /// @return 0 on success, -1 otherwise, key and/or value set
    int mkvc_parse_kv(char *line, const char *del, char **pkey, char **pval);

    /// @fn int mkvc_parse_kx(char *line, const char *del, char **pkey, char **pval, bool val_required)
    /// @brief parse and trim key/value pair. Key must exist, value is optional.
    /// Input may contain comments and leading/trailing whitespace.
    /// @param[in] line input string containing key/value
    /// @param[in] del  key/value delimiter
    /// @param[out] pkey pointer to key char array (dynamically created if key is NULL)
    /// @param[out] pval pointer to key char array (dynamically created if key is NULL)
    /// @param[in] val_required true if value is required, false if optional
    /// @return 0 on success, -1 otherwise, key and/or value set
    int mkvc_parse_kx(char *line, const char *del, char **pkey, char **pval, bool val_required);
    
    /// @fn int mkvc_parse_bool(const char *src, bool *dest)
    /// @brief parse boolean string values. Accepts "true"/"false"/"y"/n"/"0"/"1". Case insensitive
    /// @param[in] src boolean string value
    /// @param[out] dest boolean value
    int mkvc_parse_bool(const char *src, bool *dest);

#ifdef WITH_MKVCONF_TEST
    extern bool g_mkvconf_test_quit;

    /// @fn int32_t mkvc_test()
    /// @brief test mkvconf API
    /// @return 0 on success, -1 otherwise
    int mkvconf_test();
#endif

#ifdef __cplusplus
}
#endif

#endif // include gaurd
