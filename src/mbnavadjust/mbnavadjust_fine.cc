/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_fine.c        	8/4/2018
 *
 *    Copyright (c) 2018-2023 by
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
 *    Code to utilize an ICP algorithm in Point Cloud Library (PCL) 
 *    to match point clouds in MBnavadjust, including this source file,
 *    was developed by MBARI summer intern Ethan Slattery, working
 *    with David Caress, during June-August 2018.
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Mbnavadjustmerge merges two existing mbnavadjust projects. The result
 * can be to add one project to another or to create a new, third project
 * combining the two source projects.
 *
 * Author:	D. W. Caress
 * Date:	June 18, 2018
 *
 *
 */
#include "mbnavadjust_icp.h"

/* standard include files */
#include <algorithm>
#include <chrono>
#include <getopt.h>
#include <iostream>
#include <string.h>
#include <thread>

/* C++ namespaces for the lazy typer */
using std::endl;
using std::vector;
using mbsystem::bathPoint;
using mbsystem::MBSystem_ICP;
using mbsystem::icp_param;
using mbsystem::icp_results;
using mbsystem::load_crossing;
using mbsystem::perform_icp;

/* Static strings for command line output */
static const char version_id[]    = "$Id: mbnavadjusttest.c 2339 2018-07-17 14:33:00Z Slattery $";
static const char program_name[]  = "mbnavadjust-icp";

/// Structure to hold the command line parameters and ICP project settings
struct mbnavadjust_align_params {
    mb_path      project_path;  // path of project file input (typedef of char[])
    int          verbose;       // The verbosity of the debug info printed to stderr
    int          iFile1;        // file1 number for specific crossing
    int          iFile2;        // file2 number for specific crossing
    int          iSection1;     // section1 number for specific crossing
    int          iSection2;     // section2 number for specific crossing
    unsigned int minOverlap;    // minimum overlap to attempt
    bool         ignoreTies;    // ignore rough alignment settings from ties if set
    bool         tryAll;        // try all crossings if set
    unsigned int numThreads;    // number of threads to use for processing a complete project
    icp_param    icpSettings;   // the ICP algorithm settings structure
};

/// Prints the usage / help output of the program
void print_usage(bool verbose);

vector<vector<mbna_crossing*> > get_divided_crossings(const mbnavadjust_align_params params, mbna_project &project);

int mbnavadjust_align_arguments(int argc, char** argv, mbnavadjust_align_params &params);

void do_icp_thread(const int verbose, mbna_project project, const vector<mbna_crossing*> &crossings, const icp_param &parameters, const bool ignoreTies = false);

int main(int argc, char **argv) {
    int status = MB_SUCCESS;            // MBSYSTEM status variable
    int error  = MB_ERROR_NO_ERROR;     // MBSYSTEM error variable
    mbnavadjust_align_params params;    // Project parameters for the program

    status = mbnavadjust_align_arguments(argc, argv, params);

    /** Command line option variables and stack variables **/
    struct mbna_project   project;  // holds the loaded project in memory

    /* initialize the project structure */
    memset(&project, 0, sizeof(struct mbna_project));

    /* read the input project */
    status = mbnavadjust_read_project(params.verbose, params.project_path, &project, &error);
    if (status == MB_SUCCESS) {
        fprintf(stderr, "\nInput project loaded:\n\t%s\n", params.project_path);
        fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project.num_files,
                project.num_crossings, project.num_ties);
    }
    else {
        fprintf(stderr, "Load failure for input project:\n\t%s\n", params.project_path);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_USAGE;
        exit(error);
    }

    /*** Do the entire project! ***/
    if(params.iFile1 < 0 && params.iFile2 < 0) {
        vector<std::thread>         threads;    // vector of threads

        //create 2D vector, of crossings divided evenly across number of threads
        vector<vector<mbna_crossing*>> thread_crossings = get_divided_crossings(params, project);

        // print out CSV heading to stdout
        std::cout << "crossing, overlap, targetPoints, sourcePoints, milliseconds, fitness_rough, fitness_fine, correspondenceCount, Tx, Ty, Tz, Rx, Ry, Rz, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15\n";

        // reserve memory in the vectors, prevents re-sizing mid algorithm
        threads.reserve(params.numThreads);

        auto t1 = std::chrono::system_clock::now();
        for(auto &cross : thread_crossings) {
            threads.push_back( std::thread(do_icp_thread, params.verbose, std::ref(project), std::cref(cross), std::cref(params.icpSettings), params.ignoreTies) );
        }

        for(auto &t : threads) {
            t.join();
        }

        auto t2 = std::chrono::system_clock::now();

        if(params.verbose) {
            std::cerr << project.num_crossings << " processed in "
                      << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
                      << "seconds.\n";
        }

    } // end of if statement, do entire project
    else {
        struct mbna_crossing* crossing;             // pointer to the crossing currently being processed
        struct mbna_section*  section1;             // pointer to section data
        struct mbna_section*  section2;             // pointer to section data
        struct swath*         swath1     = nullptr; // pointer to swath data
        struct swath*         swath2     = nullptr; // pointer to swath data
        bool                  crossFound = false;   // set to true if the specified crossing is found

        /* We are doing a single crossing, check if the crossing exists */
        for (int icrossing = 0; (icrossing < project.num_crossings) && !crossFound; icrossing++) {
            // Get the crossing structure
            crossing = &(project.crossings[icrossing]);

            // If this is the specified crossing, mark found true and start ICP
            crossFound = crossing->file_id_1 == params.iFile1 &&
                         crossing->section_1 == params.iSection1 &&
                         crossing->file_id_2 == params.iFile2 &&
                         crossing->section_2 == params.iSection2;
        }

        // process the crossing if it was found
        if(crossFound) {
            mbsystem::icp_results result;   // algorithm results

            // load the crossing data into swath1 and swath2
            status = load_crossing(params.verbose, project, crossing, section1, section2, swath1, swath2);

            // set the rough alignment and overlap estimation from the tie, or ignore if we are ignoring ties
            if(!params.ignoreTies) {
                params.icpSettings.overlap = static_cast<unsigned int>(crossing->overlap);
                params.icpSettings.xEst   = crossing->ties[0].offset_x_m;
                params.icpSettings.yEst   = crossing->ties[0].offset_y_m;
                params.icpSettings.zEst   = crossing->ties[0].offset_z_m;
            }

            // pre-add the file/crossing to results so ICP can see it for debug output. Move to parameters if we keep this long term.
            result.tgtFile    = crossing->file_id_1;
            result.srcFile    = crossing->file_id_2;
            result.tgtSection = crossing->section_1;
            result.srcSection = crossing->section_2;

            auto t1 = std::chrono::system_clock::now();

            perform_icp(params.verbose, section1, section2, swath1, swath2, result, params.icpSettings, params.verbose);

            auto t2 = std::chrono::system_clock::now();
            result.milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            mbsystem::Log(std::cout) << result;

            // print the results in a verbose way if desired
            if(params.verbose) {
                mbsystem::Log errOut;
                errOut << "Alignment complete on crossing "
                       << crossing->file_id_1 << ":" << crossing->section_1 << "/"
                       << crossing->file_id_2 << ":" << crossing->section_2 << " in " << result.milliseconds << " milliSeconds\n";
            }

        }
        else {
            std::cerr << "\nERROR: Crossing " << crossing->file_id_1 << ":" << crossing->section_1 << "/"
                                              << crossing->file_id_2 << ":" << crossing->section_2
                      << "not found in the specified project!\n";
        }

    } // end of else statement, do specific crossing only

    return 0;
}

// TODO: Move return value to param and have function return selected frossing count as a number
std::vector<std::vector<mbna_crossing*> > get_divided_crossings(const mbnavadjust_align_params params, mbna_project &project)
{
    std::vector<mbna_crossing*>              crossings;
    std::vector<std::vector<mbna_crossing*>> thread_crossings;

    // create a vector of crossings that match the filter criteria
    for(int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        if((project.crossings[icrossing].num_ties > 0 || params.tryAll) && (project.crossings[icrossing].overlap > static_cast<int>(params.minOverlap))) {
            crossings.push_back( &(project.crossings[icrossing]) );
        }
    }

    if(params.verbose) {
        std::cerr << "Project contains " << project.num_crossings << " crossings, " << crossings.size() << " were selected for processing\n";

        for(auto &f : crossings) {
            std::cerr << f->file_id_1 << ":" << f->section_1 << "/" << f->file_id_2 << ":" << f->section_2 << endl;
        }
        std::cerr << "\n\n";
    }

    // split vector of crossings into equally sized vectors for each thread
    size_t vLength   = crossings.size() / static_cast<unsigned long>(params.numThreads);
    size_t vLeftover = crossings.size() % static_cast<unsigned long>(params.numThreads);
    long head        = 0;
    long tail        = 0;

    for(size_t i = 0; i < std::min(params.numThreads, static_cast<unsigned int>(crossings.size())); i++) {
        tail += (vLeftover > 0 ? (vLength + !!(vLeftover--)) : vLength);
        thread_crossings.push_back(std::vector<mbna_crossing*>(crossings.begin()+head, crossings.begin()+tail));
        head = tail;
    }

    if(params.verbose) {
        std::cerr << "selected crossings were split into " << params.numThreads << " vectors for threaded processing\n";
        std::cerr << "Threaded crossing vector sizes: ";
        for(auto &v : thread_crossings) {
            std::cerr << v.size() << ", ";
        }
        std::cerr << std::endl;
    }

    return thread_crossings;
}

void print_usage(bool verbose)
{
    static const char usage[]         = "Usage: %s --input=PATH [OPTIONS]... \n";
    static const char example[]       = "Example: %s --input=./project.nvh -ta --min-overlap=10\n";
    static const char description[]   = "%s loads a mbnavadjust project and performs fine scale alignment on existing ties.\n";
    static const char help_message[]  = "\nMandatory arguments to long options are mandatory for short options too.\n"
                                        "Input Options:\n"
                                        "      --input=PATH             Path to a navadjust project file\n"
                                        "      --crossing=CROSS         Specific crossing to process in the project.\n"
                                        "                               Specified crossing will be the only one processed.\n"
                                        "                               CROSS must be in form file1:section1/file2:section2\n\n"

                                        "ICP Options:\n"
                                        "  -o, --min-overlap=NUM        The minimum overlap between two swaths for a tie to get processed\n"
                                        "                               NUM must be between 0 and 100, must follow short arg without spaces\n"
                                        "  -t, --ignore-ties            Ignore any previous rough alignment associated with ties\n"
                                        "  -a, --try-all                Attempt to process all crossings, even those without ties.\n"
                                        "      --max-iterations=NUM     Maximum iterations for a single ICP run, defaults to 50\n"
                                        "      --max-distance=DIST      Maximum distance between two correspondence points in meters, defaults to 1.5 meters\n"
                                        "      --epsilon-transform=NUM  Minimum change in the transform, any smaller change will signal a finished ICP. defaults to 1e-6\n"
                                        "      --epsilon-fitness=NUM    Minimum change in the fitness value, any smaller change will signal a finished ICP. defaults to 1e-6\n"
                                        "  -n, --one-to-many            By default all correspondence points are matched one-to-one. This setting will\n"
                                        "                               enable one-to-many matching of points.\n"
                                        "      --SOR=SRC,TGT[,N,STDDEV] Perform Statistical Outlier Removal with the specified settings.\n"
                                        "                               SRC and TGT are boolean values, if true SOR will operate on SOURCE or TARGET\n"
                                        "                               N is the number of point neighbors to use for averaging\n"
                                        "                               STDDEV is the standard deviation to use, points falling outside this will be be removed.\n\n"
                                        "  -j, --threads=NUM            Number of threads to use for processing, Only used if specific crossing is not set\n"

                                        "Output Options:\n"
                                        "      --output=PATH            Choose an output navadjust project to save generated alignments to\n"
                                        "      --verbose=[NUM]          Verbosity of the output. unset or set to 0 the output will\n"
                                        "                               only be machine readable crossing data. If a specific crossing is specified\n"
                                        "                               then verbosity > 0 will include the output of intermediate cloud files.\n"
                                        "  -h, --help                   Display this help file\n";

    if(verbose) {
        fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
        fprintf(stderr, "Source File Version %s\n", version_id);
    }

    printf(usage, program_name);
    printf(example, program_name);

    if(verbose) {
        printf(description, program_name);
        printf(help_message);
    }

}

int mbnavadjust_align_arguments(int argc, char** argv, mbnavadjust_align_params &params)
{
    extern char* optarg;                     // extern string where the parameter is stored after getopt
    int          c      = -1;                // return value of getopt_long() function
    int          status = MB_SUCCESS;        // MBSYSTEM status codes
    int          error  = MB_ERROR_NO_ERROR; // MBSYSTEM error codes

    static char          short_args[] = "athno:j:";
    static struct option long_args[]  ={{"input",             required_argument, nullptr, 'I'},
                                        {"crossing",          required_argument, nullptr, 'c'},
                                        {"min-overlap",       required_argument, nullptr, 'o'},
                                        {"ignore-ties",       no_argument,       nullptr, 't'},
                                        {"try-all",           no_argument,       nullptr, 'a'},
                                        {"max-iterations",    required_argument, nullptr, 'm'},
                                        {"max-distance",      required_argument, nullptr, 'd'},
                                        {"epsilon-transform", required_argument, nullptr, 'r'},
                                        {"epsilon-fitness",   required_argument, nullptr, 'f'},
                                        {"one-to-many",       no_argument,       nullptr, 'n'},
                                        {"SOR",               required_argument, nullptr, 's'},
                                        {"output",            required_argument, nullptr, 'O'},
                                        {"threads",           required_argument, nullptr, 'j'},
                                        {"verbose",           optional_argument, nullptr, 'v'},
                                        {"help",              no_argument,       nullptr, 'h'},
                                        {nullptr,             0,                 nullptr,  0 }};

    if(argc < 2) {
        print_usage(false);
        exit(EXIT_FAILURE);
    }

    /** Set the defualt settings for all program settings **/
    strcpy(params.project_path, "\0\0\0\0\0");
    params.verbose      =  0;
    params.iFile1       = -1;
    params.iFile2       = -1;
    params.iSection1    = -1;
    params.iSection2    = -1;
    params.minOverlap   =  0;
    params.ignoreTies   = false;
    params.tryAll       = false;
    params.numThreads   =  1;

    /** Set default settings for ICP algorithm **/
    params.icpSettings.maxIterations = 50;
    params.icpSettings.overlap       = 0;
    params.icpSettings.maxDistance   = 1.5;
    params.icpSettings.one2many      = false;
    params.icpSettings.tgtSOR        = false;
    params.icpSettings.srcSOR        = false;
    params.icpSettings.SOR_neighbors = 10;
    params.icpSettings.SOR_stdDev    = 1.0;
    params.icpSettings.epsilonT      = 1e-6;
    params.icpSettings.epsilonFit    = 1e-6;
    params.icpSettings.xEst          = 0.0;
    params.icpSettings.yEst          = 0.0;
    params.icpSettings.zEst          = 0.0;

    /* process argument list */
    while ((c = getopt_long(argc, argv, short_args, long_args, nullptr)) != -1) {
        switch(c) {
            case 'I' :  if(optarg != nullptr) {
                            strcpy(params.project_path, optarg);
                        }
                        break;
            case 'c' :  error = sscanf(optarg, "%d:%d/%d:%d", &params.iFile1, &params.iSection1, &params.iFile2, &params.iSection2);
                        if (error != 4) {
                            std::cerr << "Failure to parse --crossing=" << optarg << "\n\tmod command ignored\n\n";
                            params.iFile1    = -1;
                            params.iFile2    = -1;
                            params.iSection1 = -1;
                            params.iSection2 = -1;
                        }
                        break;
            case 'o' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.minOverlap = static_cast<unsigned int>(strtoul(optarg, nullptr, 0));
                            if(params.minOverlap > 100) {
                                fprintf(stderr, "\nERROR: Invalid overlap value of %d, exiting...", params.minOverlap);
                                exit(EXIT_FAILURE);
                            }
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 't' :  params.ignoreTies = true;
                        break;
            case 'a' :  params.tryAll = true;
                        break;
            case 'O' :  std::cerr << "ERROR: output file not implemented, ignoring argument\n";
                        break;
            case 'v' :  if(optarg != nullptr) {
                            params.verbose = static_cast<int>(strtol(optarg, nullptr, 0));
                        }
                        else {
                            params.verbose++;
                        }
                        break;
            case 'm' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.icpSettings.maxIterations = static_cast<unsigned int>(strtoul(optarg, nullptr, 0));
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 'd' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.icpSettings.maxDistance = strtol(optarg, nullptr, 10);
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 'r' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.icpSettings.epsilonT = strtod(optarg, nullptr);
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 'f' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.icpSettings.epsilonFit = strtod(optarg, nullptr);
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 'n' :  params.icpSettings.one2many = true;
                        break;
            case 'j' :  if(optarg != nullptr && optarg[0] != '-') {
                            params.numThreads = std::min(static_cast<unsigned int>(strtoul(optarg, nullptr, 0)), 8u);
                        }
                        else {
                            std::cerr << argv[0] << ": option requires an argument -- 'o'\n";
                            print_usage(false);
                            exit(EXIT_FAILURE);
                        }
                        break;
            case 's' : error = sscanf(optarg, "%u,%u,%u,%lf", &params.icpSettings.srcSOR, &params.icpSettings.tgtSOR, &params.icpSettings.SOR_neighbors, &params.icpSettings.SOR_stdDev);
                       if (error < 2) {
                           std::cerr << "Failure to parse --SOR=" << optarg << "\n\tAborting Program.\n\n";
                       }
                       exit(EXIT_FAILURE);
                       break;
            default  :  print_usage('h' == c);
                        exit(EXIT_FAILURE);

        } // end of switch statement
    } // end of argument parsing while loop

    /* print starting message */
    if (params.verbose) {
        fprintf(stderr, "\nProgram %s\n", program_name);
        fprintf(stderr, "Source File Version %s\n", version_id);
        fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    }

    /* print starting debug statements */
    if (params.verbose) {
        fprintf(stderr, "\nControl Parameters:\n");
        fprintf(stderr, "     verbose:              %d\n",  params.verbose);
        fprintf(stderr, "     threads:              %d\n",  params.numThreads);
        fprintf(stderr, "     project_path:         %s\n",  params.project_path);
        fprintf(stderr, "     ifile1:               %d\n",  params.iFile1);
        fprintf(stderr, "     isection1:            %d\n",  params.iSection1);
        fprintf(stderr, "     ifile2:               %d\n",  params.iFile2);
        fprintf(stderr, "     isection2:            %d\n",  params.iSection2);
        fprintf(stderr, "     minimum Overlap:      %d%\n", params.minOverlap);
        fprintf(stderr, "     Ignore Ties:          %d\n",  params.ignoreTies);
        fprintf(stderr, "     Try All:              %d\n",  params.tryAll);
        fprintf(stderr, "     Max Iterations:       %d\n",  params.icpSettings.maxIterations);
        fprintf(stderr, "     Max Distance:         %f\n",  params.icpSettings.maxDistance);
        fprintf(stderr, "     Transform Epsilon:    %f\n",  params.icpSettings.epsilonT);
        fprintf(stderr, "     Fitness Epsilon:      %f\n",  params.icpSettings.epsilonFit);
        fprintf(stderr, "     One to Many Matching: %d\n",  params.icpSettings.one2many);
        fprintf(stderr, "     SOR on Source:        %d\n",  params.icpSettings.srcSOR);
        fprintf(stderr, "     SOR on Target:        %d\n",  params.icpSettings.tgtSOR);
        fprintf(stderr, "     SOR Neighbors:        %d\n",  params.icpSettings.SOR_neighbors);
        fprintf(stderr, "     SOR StdDev:           %f\n",  params.icpSettings.SOR_stdDev);
    }

    return status;
}

void do_icp_thread(const int verbose, mbna_project project, const vector<mbna_crossing *> &crossings,
                   const icp_param &parameters, const bool ignoreTies) {
    mbsystem::Log   errOut;                       // thread atomic debug output
    mbna_section*   tgtSection     = nullptr;     // pointer to section data
    mbna_section*   srcSection     = nullptr;     // pointer to section data
    swath*          tgtSwath       = nullptr;     // pointer to swath data
    swath*          srcSwath       = nullptr;     // pointer to swath data
    int             status         = MB_SUCCESS;  // mb-status status return
    int             error          = MB_SUCCESS;  // mb-sysyem error return value

    for (auto &cross : crossings) {
        status = load_crossing(verbose, project, cross, tgtSection, srcSection, tgtSwath, srcSwath);

        // load parameters and change the tie specifc ones
        icp_param params = parameters;
        params.overlap = static_cast<unsigned int>(cross->overlap);
        params.xEst    = cross->ties[0].offset_x_m;
        params.yEst    = cross->ties[0].offset_y_m;
        params.zEst    = cross->ties[0].offset_z_m;

        // if ignore rough alignment option is on, clear the rough alignment
        if(ignoreTies) {
            params.xEst = 0.0;
            params.yEst = 0.0;
            params.zEst = 0.0;
        }

        icp_results result; // variable for holding the results

        // pre-add the file/crossing to results so ICP can see it for debug output. Move to parameters if we keep this long term.
        result.tgtFile    = cross->file_id_1;
        result.srcFile    = cross->file_id_2;
        result.tgtSection = cross->section_1;
        result.srcSection = cross->section_2;

        auto t1 = std::chrono::system_clock::now();
        status = perform_icp(verbose, tgtSection, srcSection, tgtSwath, srcSwath, result, params);
        auto t2 = std::chrono::system_clock::now();

        result.milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        mbsystem::Log(std::cout) << result;

        // print the results in a verbose way if desired
        if(verbose) {
            errOut << "Alignment complete on crossing " << cross->file_id_1 << ":" << cross->section_1 << "/" << cross->file_id_2 << ":" << cross->section_2 << "\n";
        }

        // deallocate swath memory
        status = mb_contour_deall(verbose, tgtSwath, &error);
        status = mb_contour_deall(verbose, srcSwath, &error);

    } // end of for loop
}
