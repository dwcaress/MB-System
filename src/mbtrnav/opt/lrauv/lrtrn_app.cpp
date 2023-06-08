/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    lrtrn_app.cpp
 * @authors r. henthorn
 * @date    03/10/2021
 * @brief   Main program of TRN implementation interfacing with LCM. For LRAUV.
 *
 * Project: Precision Control
 * Summary: A Terrain-Relative Navigation implementation that uses LCM for
            external comms.
 *****************************************************************************/
/*****************************************************************************
 * Copyright Information:
 * Copyright 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *
 * Terms of Use:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version. You can access the GPLv3 license at
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details (http://www.gnu.org/licenses/gpl-3.0.html).
 *
 * MBARI provides the documentation and software code "as is", with no
 * warranty, express or implied, as to the software, title, non-infringement
 * of third party rights, merchantability, or fitness for any particular
 * purpose, the accuracy of the code, or the performance or results which you
 * may obtain from its use. You assume the entire risk associated with use of
 * the code, and you agree to be responsible for the entire cost of repair or
 * servicing of the program with which you are using the code.
 *
 * In no event shall MBARI be liable for any damages,whether general, special,
 * incidental or consequential damages, arising out of your use of the
 * software, including, but not limited to,the loss or corruption of your data
 * or damages of any kind resulting from use of the software, any prohibited
 * use, or your inability to use the software. You agree to defend, indemnify
 * and hold harmless MBARI and its officers, directors, and employees against
 * any claim,loss,liability or expense,including attorneys' fees,resulting from
 * loss of or damage to property or the injury to or death of any person
 * arising out of the use of the software.
 *
 * The MBARI software is provided without obligation on the part of the
 * Monterey Bay Aquarium Research Institute to assist in its use, correction,
 * modification, or enhancement.
 *
 * MBARI assumes no responsibility or liability for any third party and/or
 * commercial software required for the database or applications. Licensee
 * agrees to obtain and maintain valid licenses for any additional third party
 * software required.
 *****************************************************************************/

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include "gitversion.h"
#include "LcmTrn.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

/***********************************************************************
 * Code
 ***********************************************************************/

using namespace lcmTrn;

// Return a default configuration file pathname using the environment variable,
// etc.
char *getDefaultConfig()
{
    // Use the environment variable for the config directory. If the
    // variable is not defined, use the local directory.
    char *cfg =
        LcmTrn::constructFullName(LCMTRN_CONFIG_ENV, LCMTRN_DEFAULT_CONFIG);
    return cfg;
}

void usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  lrtrn_app [cfgfile]\n\n");
    fprintf(stderr, "  cfgfile: optional path to lrtrn configuration file\n");
    fprintf(stderr, "           default is $%s/%s\n", LCMTRN_CONFIG_ENV,
            LCMTRN_DEFAULT_CONFIG);
    fprintf(stderr, "           local directory used if $%s is undefined\n\n",
            LCMTRN_CONFIG_ENV);
}

int main(int argc, char **argv)
{
    //**************************************************************************
    // Initialization phase
    //**************************************************************************

    fprintf(stderr,
        "##################################################################\n");
    fprintf(stderr, "lrtrn_app %s, built on %s %s\n",
        __GIT_VERSION__, __DATE__, __TIME__);
    fprintf(stderr,
        "##################################################################\n");

    // Use default configuration file if none was specified on the command line.
    char *configfile = NULL;
    if (argc == 1) {
        configfile = getDefaultConfig();
    } else if (argc == 2) {
        configfile = strdup(argv[1]);
        // Ensure that homeDir exists
        struct stat sb{};
        if (stat(configfile, &sb) != 0 || !S_ISREG(sb.st_mode)) {
            fprintf(stderr, "Bad config file specified %s\n\n", configfile);
            usage();
            exit(1);
        }

    } else {
        usage();
        exit(1);
    }

    // Quit if no config
    if (NULL == configfile) {
        exit(0);
    }

    // Create LcmTrn object and run it
    lcmTrn::LcmTrn _trn(configfile);

    // Run if the LcmTrn object setup was a success.
    // run() only stops when the LcmTrn object is not good.
    if (!_trn.good()) {
        fprintf(stderr, "Initialization failed!\n");
    } else {
        fprintf(stderr, "%s %s listening for messages...\n",
            argv[0], __GIT_VERSION__);
        _trn.run();
    }

    fprintf(stderr, "%s Done\n", argv[0]);

    // Clean
    DELOBJ(configfile);

    return 0;
}
