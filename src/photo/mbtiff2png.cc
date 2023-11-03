/*--------------------------------------------------------------------
 *    The MB-system:    mbtiff2png.cpp    6/6/2021
 *
 *    Copyright (c) 2021-2023 by
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
 * mbtiff2png converts GeoTiff images to PNG format images setting no data regions to be transparent.
 *
 * Author:    D. W. Caress
 * Date:    June 6, 2021
 *
 */

/* standard include files */
#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>

/* MB-System include files */
extern "C"
{
#include "mb_define.h"
#include "mb_status.h"
}

/* OpenCV include files */
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define MBT2P_TRANSPARENCY_NONE         0
#define MBT2P_TRANSPARENCY_WHITE        1
#define MBT2P_TRANSPARENCY_LIGHT        2
#define MBT2P_TRANSPARENCY_BLACK        3
#define MBT2P_TRANSPARENCY_DARK         4

/*--------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    char program_name[] = "mbtiff2png";
    char help_message[] =  "mbtiff2png c";
    char usage_message[] = "mbtiff2png \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--input=tiffimage\n"
                            "\t--world=tiffimage\n"
                            "\t--output=pngimage\n"
                            "\t--transparency-white\n"
                            "\t--transparency-light=threshold\n"
                            "\t--transparency-black\n"
                            "\t--transparency-dark=threshold\n";
    extern char *optarg;
    int option_index;
    int errflg = 0;
    int c;
    int help = 0;
    int flag = 0;
    FILE *stream = NULL;
    bool success = true;

    /* control parameters */
    int verbose = 0;
    int error = MB_ERROR_NO_ERROR;
    bool input_image_specified = false;
    bool input_world_specified = false;
    bool output_image_specified = false;
    mb_path InputImageFile;
    mb_path InputWorldFile;
    mb_path OutputImageFile;
    mb_path OutputWorldFile;
    int transparency_mode = MBT2P_TRANSPARENCY_NONE;
    int transparency_threshold = 0;

    /* command line option definitions */
    /* mbtiff2png
     *    --verbose
     *    --help
     *    --input=tiffimage
     *    --world=tiffimage
     *    --output=pngimage
     *    --transparency-white
     *    --transparency-light=threshold
     *    --transparency-black
     *    --transparency-dark=threshold
     *
     */
    static struct option options[] =
        {
        {"verbose",                     no_argument,            NULL,         0},
        {"help",                        no_argument,            NULL,         0},
        {"input",                       required_argument,      NULL,         0},
        {"world",                       required_argument,      NULL,         0},
        {"output",                      required_argument,      NULL,         0},
        {"transparency-white",          required_argument,      NULL,         0},
        {"transparency-light",          required_argument,      NULL,         0},
        {"transparency-black",          required_argument,      NULL,         0},
        {"transparency-dark",           required_argument,      NULL,         0},
        {NULL,                          0,                      NULL,         0}
        };

    /* initialize some other things */
    memset(InputImageFile, 0, sizeof(mb_path));
    memset(InputWorldFile, 0, sizeof(mb_path));
    memset(OutputImageFile, 0, sizeof(mb_path));

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
      switch (c)
        {
        /* long options all return c=0 */
        case 0:
            /* verbose */
            if (strcmp("verbose", options[option_index].name) == 0)
                {
                verbose++;
                }

            /* help */
            else if (strcmp("help", options[option_index].name) == 0)
                {
                help = MB_YES;
                }

            /*-------------------------------------------------------
             * Filenames */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", InputImageFile);
                input_image_specified = true;
                }

            /* world */
            else if (strcmp("world", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", InputWorldFile);
                input_world_specified = true;
                }

            /* output */
            else if (strcmp("output", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", OutputImageFile);
                output_image_specified = true;
                }

            /*-------------------------------------------------------
             * Define transparency conversion */

            /* transparency-white */
            else if (strcmp("transparency-white", options[option_index].name) == 0)
                {
                transparency_mode = MBT2P_TRANSPARENCY_WHITE;
                }

            /* transparency-light */
            else if (strcmp("transparency-light", options[option_index].name) == 0)
                {
                sscanf (optarg,"%d", &transparency_threshold);
                transparency_mode = MBT2P_TRANSPARENCY_LIGHT;
                }

            /* transparency-black */
            else if (strcmp("transparency-black", options[option_index].name) == 0)
                {
                transparency_mode = MBT2P_TRANSPARENCY_BLACK;
                }

            /* transparency-dark */
            else if (strcmp("transparency-dark", options[option_index].name) == 0)
                {
                sscanf (optarg,"%d", &transparency_threshold);
                transparency_mode = MBT2P_TRANSPARENCY_DARK;
                }

            break;
        case '?':
            errflg++;
        }

    /* if error flagged then print it and exit */
    if (errflg)
        {
        fprintf(stderr,"usage: %s\n", usage_message);
        fprintf(stderr,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_USAGE;
        exit(error);
        }

    /* set output stream */
    if (verbose <= 1)
        stream = stdout;
    else
        stream = stderr;


    /* if input not specified then quit */
    if (!input_image_specified)
        {
        fprintf(stream,"\nInput Tiff image file not specified:\n");
        fprintf(stream,"\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
        }


    /* check file names */
    if (!input_world_specified) {
        strcpy(InputWorldFile, InputImageFile);
        if (strncmp(".tif", &InputWorldFile[strlen(InputWorldFile)-4], 4) == 0
            || strncmp(".TIF", &InputWorldFile[strlen(InputWorldFile)-4], 4) == 0) {
            InputWorldFile[strlen(InputWorldFile)-4] = 0;
        }
        else if (strncmp(".tiff", &InputWorldFile[strlen(InputWorldFile)-5], 5) == 0
            || strncmp(".TIFF", &InputWorldFile[strlen(InputWorldFile)-5], 5) == 0) {
            InputWorldFile[strlen(InputWorldFile)-5] = 0;
        }
        strcat(InputWorldFile, ".tfw");
    }
    if (!output_image_specified) {
        strcpy(OutputImageFile, InputImageFile);
        if (strncmp(".tif", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0
            || strncmp(".TIF", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0) {
            OutputImageFile[strlen(OutputImageFile)-4] = 0;
            strcat(OutputImageFile, ".png");
        }
        else if (strncmp(".tiff", &OutputImageFile[strlen(OutputImageFile)-5], 5) == 0
            || strncmp(".TIFF", &OutputImageFile[strlen(OutputImageFile)-5], 5) == 0) {
            OutputImageFile[strlen(OutputImageFile)-5] = 0;
            strcat(OutputImageFile, ".png");
        }
    }
    if (strncmp(".png", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0) {
        strcat(OutputImageFile, ".png");
    }
    strcpy(OutputWorldFile, OutputImageFile);
    OutputWorldFile[strlen(OutputImageFile)-4] = 0;
    strcat(OutputWorldFile, ".pgw");

    /* print starting message */
    if (verbose == 1 || help)
        {
        fprintf(stream,"\nProgram %s\n",program_name);
        fprintf(stream,"MB-system Version %s\n",MB_VERSION);
        }

    /* print starting debug statements */
    if (verbose >= 2)
        {
        fprintf(stream,"\ndbg2  Program <%s>\n",program_name);
        fprintf(stream,"dbg2  MB-system Version %s\n",MB_VERSION);
        fprintf(stream,"dbg2  Control Parameters:\n");
        fprintf(stream,"dbg2       verbose:                       %d\n",verbose);
        fprintf(stream,"dbg2       help:                          %d\n",help);
        fprintf(stream,"dbg2       input_image_specified:         %d\n",input_image_specified);
        fprintf(stream,"dbg2       input_world_specified:         %d\n",input_world_specified);
        fprintf(stream,"dbg2       output_image_specified:        %d\n",output_image_specified);
        fprintf(stream,"dbg2       InputImageFile:                %s\n",InputImageFile);
        fprintf(stream,"dbg2       InputWorldFile:                %s\n",InputWorldFile);
        fprintf(stream,"dbg2       OutputImageFile:               %s\n",OutputImageFile);
        fprintf(stream,"dbg2       OutputWorldFile:               %s\n",OutputWorldFile);
        fprintf(stream,"dbg2       transparency_mode:             %d\n",transparency_mode);
        fprintf(stream,"dbg2       transparency_threshold:        %d\n",transparency_threshold);
        }
    else if (verbose == 1)
        {
        fprintf(stream,"\nProgram <%s>\n",program_name);
        fprintf(stream,"Control Parameters:\n");
        fprintf(stream,"  verbose:                       %d\n",verbose);
        fprintf(stream,"  help:                          %d\n",help);
        fprintf(stream,"  input_image_specified:         %d\n",input_image_specified);
        fprintf(stream,"  input_world_specified:         %d\n",input_world_specified);
        fprintf(stream,"  output_image_specified:        %d\n",output_image_specified);
        fprintf(stream,"  InputImageFile:                %s\n",InputImageFile);
        fprintf(stream,"  InputWorldFile:                %s\n",InputWorldFile);
        fprintf(stream,"  OutputImageFile:               %s\n",OutputImageFile);
        fprintf(stream,"  OutputWorldFile:               %s\n",OutputWorldFile);
        fprintf(stream,"  transparency_mode:             %d\n",transparency_mode);
        fprintf(stream,"  transparency_threshold:        %d\n",transparency_threshold);
        }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
        }

    /* read the input GeoTiff */
    Mat InputImage = imread(InputImageFile);
    Mat OutputImage;

    /* convert BGR image to BGRA */
    if (!InputImage.empty()) {
        cvtColor(InputImage, OutputImage, COLOR_BGR2BGRA);
        InputImage.release();
    } else {
        fprintf(stream,"\nFailed to read input image: %s\n", InputImageFile);
        fprintf(stream,"\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
    }

    /* create transparency by setting alpha channel values to zero where
        transparent */
    if (!OutputImage.empty()) {
        for (int j = 0; j < OutputImage.rows; ++j) {
            for (int i = 0; i < OutputImage.cols; ++i) {
                if (transparency_mode == MBT2P_TRANSPARENCY_WHITE) {
                    if (OutputImage.at<Vec4b>(j, i)[0] == 255
                        && OutputImage.at<Vec4b>(j, i)[1] == 255
                        && OutputImage.at<Vec4b>(j, i)[2] == 255) {
                        OutputImage.at<Vec4b>(j, i)[3] = 0;
                    }
                }
                else if (transparency_mode == MBT2P_TRANSPARENCY_LIGHT) {
                    int sum = OutputImage.at<Vec4b>(j, i)[0]
                              + OutputImage.at<Vec4b>(j, i)[1]
                              + OutputImage.at<Vec4b>(j, i)[2];
                    int thresholdsum = 3 * transparency_threshold;
                    if (sum >= thresholdsum) {
                        OutputImage.at<Vec4b>(j, i)[3] = 0;
                    }
                }
                else if (transparency_mode == MBT2P_TRANSPARENCY_BLACK) {
                    if (OutputImage.at<Vec4b>(j, i)[0] == 0
                        && OutputImage.at<Vec4b>(j, i)[1] == 0
                        && OutputImage.at<Vec4b>(j, i)[2] == 0) {
                        OutputImage.at<Vec4b>(j, i)[3] = 0;
                    }
                }
                else if (transparency_mode == MBT2P_TRANSPARENCY_DARK) {
                    int sum = OutputImage.at<Vec4b>(j, i)[0]
                              + OutputImage.at<Vec4b>(j, i)[1]
                              + OutputImage.at<Vec4b>(j, i)[2];
                    int thresholdsum = 3 * transparency_threshold;
                    if (sum <= thresholdsum) {
                        OutputImage.at<Vec4b>(j, i)[3] = 0;
                    }
                }
            }
        }
    } else {
        fprintf(stream,"\nColor conversion to BGRA failed: %s\n", InputImageFile);
        fprintf(stream,"\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
    }

    /* write the output png file with transparency */
    success = imwrite(OutputImageFile, OutputImage);
    OutputImage.release();
    if (!success) {
        fprintf(stream,"\nWriting output image failed: %s\n", OutputImageFile);
        fprintf(stream,"\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
    }

    /* copy the input world file to the output world file */
    mb_command command;
    snprintf(command, sizeof(command), "cp %s %s", InputWorldFile, OutputWorldFile);
    system(command);
    fprintf(stream, "Wrote output BGRA png image %s\n", OutputImageFile);
    fprintf(stream, "Copied world file from %s to %s\n", InputWorldFile, OutputWorldFile);

    exit(0);

}
