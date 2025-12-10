///
/// @file TrnPlayer.cpp
/// @authors k. headley
/// @date 2025-09-16
///
/// Copyright 2025 Monterey Bay Aquarium Research Institute
/// see LICENSE file for terms of use and license information.
///
/// @brief Minimal TRN example application
/// @details Processes (replays) a TRN log file (TerrainAid.log, MbTrn.log)
/// Demonstrates simplest use of TRN and related data structures
///     - create/configure TRN instance
///     - update TRN using poseT, measT
///     - get TRN estimate, covariances
///
/// @see TrnPlayer.hpp
/// @see TrnPlayer::run

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
// for basename
#include <libgen.h>
#include <time.h>

#include "TrnPlayer.hpp"
#include "TerrainNav.h"
#include "DataLogReader.h"
#include "NavUtils.h"
#include "newmat.h"

// Max time difference between nav and TRN record (sec)
const double MAX_RDT_SEC = 0.2;

typedef enum {
    RI_ERR = -1,
    RI_OK = 0,
    RI_EOF = 1,
    RI_DEC = 2
} ReturnCodes;

// Default constructor
TrnPlayer::TrnPlayer()
: ctx(NULL)
, trn(NULL)
, status(0)
, err_n(0)
, rec_n(0)
, val_n(0)
, est_n(0)
, dec_n(0)
{}

// Destructor
TrnPlayer::~TrnPlayer()
{
    if(ctx != NULL)
        delete ctx;
    if(trn != NULL)
        delete trn;
}

// Parse input from command line or config file
// configure TRN instance
// Replaces both ctx and trn

int TrnPlayer::configure(int argc, char **argv)
{
    if(ctx != NULL)
        delete ctx;

    ctx = new TrnPlayerCtx();

    // load command line options
    int test = TrnPlayerCtx::parse(argc, argv, ctx);

    if(ctx->verbose) {
        show();
    }

    if(ctx->is_help_set) {
        // show help and exit
        ctx->show_help(basename(argv[0]));
        return -1;
    }

    if(test != 0)
        return test;

    if(trn != NULL)
        delete trn;

    // configure a TRN instance
    try {
        if(ctx->is_particles_set == 0) {
            trn = new TerrainNav(ctx->mpath, ctx->vpath, ctx->filter_type, ctx->map_type, ctx->odir);
        } else {
            trn = new TerrainNav(ctx->mpath, ctx->vpath, ctx->ppath, ctx->filter_type, ctx->map_type, ctx->odir);
        }
    } catch(...) {
        fprintf(stderr, "%s ERR TerrainNav failed; check input directories and file names; use -v\n", __func__);
        return -1;
    }

    // copy config file to output directory
    copy_config();

    trn->setFilterReinit(ctx->reinit_en);
    trn->setModifiedWeighting(ctx->mod_weight);
    trn->setMapInterpMethod(ctx->map_interp);
    if(ctx->force_lgf)
        trn->useLowGradeFilter();
    else {
        trn->useHighGradeFilter();
    }

    if(ctx->pf_omode != PFO_NONE)
        trn->tNavFilter->setDistribToSave(ctx->pf_omode);

    getSensorGeometry();

    return 0;
}

// Main application logic:
// - configure a TRN instance
// - iterate over log records
// - fill in poseT, measT from log records
// - update TRN state using motionUpdate, measUpdate
// - get TRN estimate using estimatePose
// - output estimate
int TrnPlayer::run(int argc, char **argv)
{
    // configure if needed
    if(trn == NULL) {
        int test = configure(argc, argv);
        // return on error/help request
        if(test != 0)
            return test;
    }

    if(trn == NULL || ctx == NULL) {
        // configuration error
        fprintf(stderr, "%s - ERR trn (%p) or ctx (%p) NULL; call configure or pass options\n", __func__, trn, ctx);
        return -1;
    }

    // open IO files
    init_io();

    // poseT, measT structs for TRN IO
    poseT pt, mse, mle;
    measT mt;
    poseT *pt_set[3] = {&pt, &mse, &mle};
    measT *mt_set[1] = {&mt};

    // init measT (may be changed by record reader)
    mt.dataType   = ctx->sensor_type;
    mt.numMeas    = 4;
    mt.ranges     = (double *) malloc(mt.numMeas * sizeof(double));
    mt.crossTrack = (double *) malloc(mt.numMeas * sizeof(double));
    mt.alongTrack = (double *) malloc(mt.numMeas * sizeof(double));
    mt.altitudes  = (double *) malloc(mt.numMeas * sizeof(double));
    mt.alphas     = (double *) malloc(mt.numMeas * sizeof(double));
    mt.covariance = (double *) malloc(mt.numMeas * sizeof(double));
    mt.beamNums   = (int *) malloc(mt.numMeas * sizeof(int));
    mt.measStatus = (bool *) malloc(mt.numMeas * sizeof(bool));

    // iterate over data records
    // until EOF or error limit exceeded
    do {

        // ----- Read bathymetry and navigation data -----
        // populate poseT, measT
        status = getNextRecord(&pt, &mt);
        rec_n++;

        if(status == 0) {
            // read valid
            val_n++;

            // udpate range stats
            if(mt.ping_number < ctx->ping_range[0])
                ctx->ping_range[0] = mt.ping_number;
            if(mt.ping_number > ctx->ping_range[1])
                ctx->ping_range[1] = mt.ping_number;
            if(mt.time < ctx->time_range[0])
                ctx->time_range[0] = mt.time;
            if(mt.time > ctx->time_range[1])
                ctx->time_range[1] = mt.time;

            // ----- Update TRN -----
            // call motionUpdate, measUpdate
            // ensure motionUpdate valid before measUpdate
            trn->motionUpdate(&pt);
            trn->measUpdate(&mt, ctx->sensor_type);

            // ----- TRN estimate -----
            trn->estimatePose(&mse, TRN_EST_MMSE);
            trn->estimatePose(&mle, TRN_EST_MLE);

            // ----- do something with TRN -----
            if( (ctx->last_meas = trn->lastMeasSuccessful())) {
                // estimate valid
                est_n++;

                if (NULL != ctx->part_out) {
                    // write particle states
                    trn->tNavFilter->saveCurrDistrib(*ctx->part_out);
                }
            }

            // write output
            write_output(pt, mt, mse, mle);

            reset_pt(pt_set, 3);
            reset_mt(mt_set, 1);

        } else {
            // record read invalid
            if(status < 0)
                err_n++;
            if(status == RI_DEC)
                dec_n++;
        }

    } while(err_n <= MAX_ERRS && status != 1);

    if(ctx->verbose) {
        show_summary();
    }

    // release memory resources
    delete trn;
    trn = NULL;

    mt.clean();

    return (err_n >= MAX_ERRS ? -1 : 0);
}

// Print configuration summary to console
void TrnPlayer::show()
{
    TrnPlayerCtx::show(ctx);
}

// Read poseT, measT from MbTrn.log
// return 0 on success, 1 on EOF, -1 otherwise
 int TrnPlayer::getMbTrnRecord(poseT *pt, measT *mt)
{
    DataField *f = NULL;

     DataLogReader *mbtrn_log = ctx->trn_log;

    try {
        // Read a TRN record. TRN logs every 3 seconds, or 0.33 HZ
        mbtrn_log->read();
        double lat, lon;

        // get poseT parameters
        mbtrn_log->fields.get( 1,&f); pt->time = atof(f->ascii());

        // apply decimation filter, if enabled
        if(decimate(pt->time) != 0)
            return RI_DEC;

        mbtrn_log->fields.get( 2,&f); lat = atof(f->ascii());
        mbtrn_log->fields.get( 3,&f); lon = atof(f->ascii());

        NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon),
                           NavUtils::geoToUtmZone(Math::degToRad(lat),Math::degToRad(lon)),
                           &pt->x, &pt->y);

        mbtrn_log->fields.get( 4,&f); pt->z = atof(f->ascii());
        mbtrn_log->fields.get( 5,&f); pt->psi = atof(f->ascii());
        pt->phi = pt->theta = 0.;
        pt->dvlValid = True;
        pt->gpsValid = pt->z < 2; // Depths below 2 m  have no GPS
        pt->bottomLock = 1;

        // wx, wy, wz: rotation rates for dead reckoned solution not required
        pt->wx = pt->wy = pt->wz = 0.;

        // set vx if unset; required to init TRN motion
        if(pt->vx == 0.)
            pt->vx = 0.01;
        pt->vy = pt->vz = 0.01;

        // Get measT parameters
        mt->time = pt->time;
        mt->x = pt->x;
        mt->y = pt->y;
        mt->z = pt->z;
        mt->phi   = pt->phi;
        mt->theta = pt->theta;
        mt->psi   = pt->psi;

        mbtrn_log->fields.get( 6,&f); mt->ping_number = (unsigned)atoi(f->ascii());
        mbtrn_log->fields.get( 7,&f); mt->numMeas = atoi(f->ascii());

        if(mt->numMeas > 0) {
            mt->ranges      = (double *)realloc(mt->ranges, mt->numMeas*sizeof(double));
            mt->alongTrack  = (double *)realloc(mt->alongTrack, mt->numMeas*sizeof(double));
            mt->crossTrack  = (double *)realloc(mt->crossTrack, mt->numMeas*sizeof(double));
            mt->altitudes   = (double *)realloc(mt->altitudes, mt->numMeas*sizeof(double));
            mt->alphas      = (double *)realloc(mt->alphas, mt->numMeas*sizeof(double));
            mt->covariance      = (double *)realloc(mt->covariance, mt->numMeas*sizeof(double));
            mt->beamNums    = (int *)realloc(mt->beamNums, mt->numMeas*sizeof(int));
            mt->measStatus  = (bool *)realloc(mt->measStatus, mt->numMeas*sizeof(bool));

            for (int i = 0; i < mt->numMeas; i++) {
                mbtrn_log->fields.get(8+((i*4)+0),&f); mt->beamNums[i] = atoi(f->ascii());
                mbtrn_log->fields.get(8+((i*4)+1),&f); mt->alongTrack[i] = atof(f->ascii());
                mbtrn_log->fields.get(8+((i*4)+2),&f); mt->crossTrack[i] = atof(f->ascii());
                mbtrn_log->fields.get(8+((i*4)+3),&f); mt->altitudes[i] = atof(f->ascii());
                double rho[3] = {mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]};
                double rhoNorm = vnorm( rho );
                mt->ranges[i] = rhoNorm;
                if (rhoNorm > 1) {
                    mt->measStatus[i] = True;
                } else {
                    mt->measStatus[i] = False;
                }
            }
        }

        if(ctx->debug > 0) {

           fprintf(stderr, "--- %s ---\n", __func__);
           fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
           fprintf(stderr, "mt->dataType %d\n", mt->dataType);
           fprintf(stderr, "mt->time     %.3lf\n", mt->time);
           fprintf(stderr, "mt->x        %.3lf\n", mt->x);
           fprintf(stderr, "mt->y        %.3lf\n", mt->y);
           fprintf(stderr, "mt->z        %.3lf\n", mt->z);
           fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
           fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
           fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
           fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
           fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
           fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
           fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
           fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
           fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
            for(int i=0; i< mt->numMeas; i++) {
               fprintf(stderr, "[%3d] n,stat,rng : %3d,  %d, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i]);
            }
        }

        return 0;
    }
    catch (...) {
        fprintf(stderr, "\nEnd of log\n");
        return RI_EOF;
    }

    return -1;
}

// Read poseT, measT from TerrainNav.log
// return 0 on success, 1 on EOF, -1 otherwise

 int TrnPlayer::getTNavRecord(poseT *pt, measT *mt)
{
     bool debug = false;
     // Get the data from the other log files
     DataField *f = NULL;

     DataLogReader *tnav_log = ctx->trn_log;

     try {
         // Read a TRN log record.
         tnav_log->read();

         tnav_log->fields.get( 1,&f); pt->time = atof(f->ascii());

         // apply decimation filter, if enabled
         if(decimate(pt->time) != 0)
             return RI_DEC;

         // Get [x,y,z], [phi,theta,psi], [wx,wy,wz], [vx,vy,vz],
         // and flags from the TRN record
         // navN, navN, depth
         tnav_log->fields.get( 2,&f); pt->x = atof(f->ascii());
         tnav_log->fields.get( 3,&f); pt->y = atof(f->ascii());
         tnav_log->fields.get( 4,&f); pt->z = atof(f->ascii());

         // heading (psi), pitch (theta), roll (phi)
         tnav_log->fields.get( 5,&f); pt->phi   = atof(f->ascii());
         tnav_log->fields.get( 6,&f); pt->theta = atof(f->ascii());
         tnav_log->fields.get( 7,&f); pt->psi   = atof(f->ascii());

         // wx, wy, wz: rotation rates for dead reckoned solution not required
         pt->wx = pt->wy = pt->wz = 0.;

         // fake vx
         if(pt->vx <= 0.) {
             pt->vx = 0.1;
         }
         pt->vy = pt->vz = 0.01;

         // dvlValid, gpsValid, bottomlock flags
         pt->dvlValid = 1;

         if (pt->z > 0.3) {
             pt->gpsValid = false;
         } else {
             pt->gpsValid = true;
         }

         pt->bottomLock = !pt->gpsValid;

         tnav_log->fields.get(8,&f); mt->time   = atof(f->ascii());
         tnav_log->fields.get(9,&f); mt->dataType = atoi(f->ascii());
         tnav_log->fields.get(10,&f); mt->ping_number = (unsigned)atoi(f->ascii());
         tnav_log->fields.get(11,&f); mt->numMeas  = atoi(f->ascii());

         mt->x     = pt->x;
         mt->y     = pt->y;
         mt->z     = pt->z;
         mt->phi   = pt->phi;
         mt->theta = pt->theta;
         mt->psi   = pt->psi;

         int stat_ofs = 13 + (mt->numMeas + 1) * 4;

         for (int i = 0; i < mt->numMeas ; i++) {
             tnav_log->fields.get( 13+i, &f); mt->ranges[i] = atof(f->ascii());
             tnav_log->fields.get(stat_ofs + i,&f); mt->measStatus[i] = atoi(f->ascii());

             // TODO: Remove this HACK
             // For LRAUV, beam status always 0 in TerrainNav.log
             if(ctx->force_status)
                 mt->measStatus[i] = 1;

             if(debug) {
                 fprintf(stderr,"ofs, rng,stat  %d, %.3lf, %d\n", stat_ofs, mt->ranges[i] , mt->measStatus[i]);
             }
         }


         if(ctx->debug > 0) {

            fprintf(stderr, "--- %s ---\n", __func__);
            fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
            fprintf(stderr, "mt->dataType %d\n", mt->dataType);
            fprintf(stderr, "mt->time     %.3lf\n", mt->time);
            fprintf(stderr, "mt->x        %.3lf\n", mt->x);
            fprintf(stderr, "mt->y        %.3lf\n", mt->y);
            fprintf(stderr, "mt->z        %.3lf\n", mt->z);
            fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
            fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
            fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
            fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
            fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
            fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
            fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
            fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
            fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
             for(int i=0; i< mt->numMeas; i++) {
                fprintf(stderr, "[%3d] n,stat,rng : %3d,  %d, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i]);
             }
         }

         return 0;
     }
     catch (...) {
         fprintf(stderr, "\nEnd of log!\n");
         return RI_EOF;
     }

     return -1;
 }

// Read poseT, measT from TerrainAid.log
// return 0 on success, 1 on EOF, -1 otherwise

 int TrnPlayer::getTrnAidRecord(poseT *pt, measT *mt)
{
    // Get the poseT and measT data from the standard Dorado log file set
    DataField *f = NULL;
    DataLogReader *trn_log = ctx->trn_log;
    DataLogReader *nav_log = ctx->nav_log;

    try {
        // Read a TRN record. TRN logs every 3 seconds, or 0.33 HZ
        trn_log->read();
        pt->time = trn_log->timeTag()->value();

        // apply decimation filter, if enabled
        if(decimate(pt->time) != 0)
            return RI_DEC;

        // Get poseT parameters:
        // [x,y,z], [phi,theta,psi], [wx,wy,wz], [vx,vy,vz],
        // and flags from the TRN record
        // navN, navN, depth
        trn_log->fields.get( 3,&f); pt->x = atof(f->ascii());
        trn_log->fields.get( 4,&f); pt->y = atof(f->ascii());
        trn_log->fields.get( 5,&f); pt->z = atof(f->ascii());

        // heading (psi), pitch (theta), roll (phi)
        trn_log->fields.get( 6,&f); pt->phi   = atof(f->ascii());
        trn_log->fields.get( 7,&f); pt->theta = atof(f->ascii());
        trn_log->fields.get( 8,&f); pt->psi   = atof(f->ascii());

        // wx, wy, wz, (omega1, 2, 3)
        trn_log->fields.get( 9,&f); pt->wx = atof(f->ascii());
        trn_log->fields.get(10,&f); pt->wy = atof(f->ascii());
        trn_log->fields.get(11,&f); pt->wz = atof(f->ascii());
        // wx, wy, wz: rotation rates for dead reckoned solution not required
        pt->wx = pt->wy = pt->wz = 0.;

        // dvlVelocity 1, 2, 3 (bottomTrackVelocity or waterMassVelocity)
        trn_log->fields.get(17,&f); pt->vx = atof(f->ascii());
        trn_log->fields.get(18,&f); pt->vy = atof(f->ascii());
        trn_log->fields.get(19,&f); pt->vz = atof(f->ascii());

        // dvlValid, gpsValid, bottomlock flags
        trn_log->fields.get(20,&f); pt->dvlValid   = atoi(f->ascii());
        trn_log->fields.get(21,&f); pt->gpsValid   = atoi(f->ascii());
        trn_log->fields.get(22,&f); pt->bottomLock = atoi(f->ascii());

        // Get measT (bathymetry) parameters
        mt->time = pt->time;

        {
            trn_log->fields.get(13,&f); mt->ranges[0] = atof(f->ascii());
            trn_log->fields.get(14,&f); mt->ranges[1] = atof(f->ascii());
            trn_log->fields.get(15,&f); mt->ranges[2] = atof(f->ascii());
            trn_log->fields.get(16,&f); mt->ranges[3] = atof(f->ascii());
            mt->measStatus[0] = mt->measStatus[1] = mt->measStatus[2]
            = mt->measStatus[3] = true;

            mt->x     = pt->x;
            mt->y     = pt->y;
            mt->z     = pt->z;
            // get mt phi, theta, psi from nav record
            // instead of poseT (TerrainAid.log only)
            // mt->phi   = pt->phi;
            // mt->theta = pt->theta;
            // mt->psi   = pt->psi;

            // Collect the remaining measT elements nav record
            double nav_time = 0.;
            do {
                nav_log->read();
                nav_time = nav_log->timeTag()->value();
            }
            while( (fabs(nav_time - pt->time) > MAX_RDT_SEC) &&
                  (nav_time < pt->time) );

            //fprintf(stderr, "Found matching nav record at time %f\n", nav_time);
            nav_log->fields.get(7,&f); mt->phi   = atof(f->ascii());
            nav_log->fields.get(8,&f); mt->theta = atof(f->ascii());
            nav_log->fields.get(9,&f); mt->psi   = atof(f->ascii());
        }

        if(ctx->debug > 0) {

           fprintf(stderr, "--- %s ---\n", __func__);
           fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
           fprintf(stderr, "mt->dataType %d\n", mt->dataType);
           fprintf(stderr, "mt->time     %.3lf\n", mt->time);
           fprintf(stderr, "mt->x        %.3lf\n", mt->x);
           fprintf(stderr, "mt->y        %.3lf\n", mt->y);
           fprintf(stderr, "mt->z        %.3lf\n", mt->z);
           fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
           fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
           fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
           fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
           fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
           fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
           fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
           fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
           fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
            for(int i=0; i< mt->numMeas; i++) {
               fprintf(stderr, "[%3d] n,stat,rng : %3d,  %d, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i]);
            }
        }

        return 0;
    }
    catch (...) {
        fprintf(stderr, "\nEnd of log\n");
        return RI_EOF;
    }

    return -1;
}

// uses strtok to parse src, stores token pointers in dest
// - modifies src
// - dest must be mallocable, freed by caller
int TrnPlayer::parseCSV(char *src, char ***dest, int *r_nfields)
{
    char **fields = NULL;
    int sz_inc = 32;
    int i = 0;

    // allocate field buffer to initial size
    fields = (char **)malloc(sz_inc * sizeof(char *));

    if(fields == NULL) {
        fprintf(stderr, "%s - ERR could not allocate field buffer [%d]\n", __func__, sz_inc);
        return -1;
    }

    // parse line, store field pointers in fields array
    char *fp = strtok(src, ",");
    if(fp != NULL) {
        do {
            if((i > 0) && (i % sz_inc) == 0) {
                // resize field pointer storage if necessary
                int new_sz = i + sz_inc;
                char **tmp = (char **)realloc(fields, new_sz * sizeof(char *));
                if(tmp == NULL) {
                    fprintf(stderr, "%s - ERR could not realloc field buffer[%d]\n", __func__, i);
                    free(fields);
                    return -1;
                }
                fields = tmp;
            }

            // add field pointer to array
            fields[i++] = fp;

            // get next field token
            fp = strtok(NULL, ",");
        } while(fp != NULL);
    } else {
        fprintf(stderr, "%s - ERR no fields found [%d]\n", __func__, i);
        free(fields);
        r_nfields = 0;
        return -1;
    }

    *r_nfields = i;
    *dest = fields;

    return 0;
}

// return 321 Euler rotation matrix
static Matrix euler321(double phi, double theta, double psi)
{
    double cphi=cos(phi);
    double sphi=sin(phi);
    double cth=cos(theta);
    double sth=sin(theta);
    double cpsi=cos(psi);
    double spsi=sin(psi);

    Matrix Tphi(3,3);
    Matrix Ttheta(3,3);
    Matrix Tpsi(3,3);

    Tphi(1,1) = 1;
    Tphi(1,2) = 0;
    Tphi(1,3) = 0;
    Tphi(2,1) = 0;
    Tphi(2,2) = cphi;
    Tphi(2,3) = sphi;
    Tphi(3,1) = 0;
    Tphi(3,2) = -sphi;
    Tphi(3,3) = cphi;

    Ttheta(1,1) = cth;
    Ttheta(1,2) = 0;
    Ttheta(1,3) = -sth;
    Ttheta(2,1) = 0;
    Ttheta(2,2) = 1;
    Ttheta(2,3) = 0;
    Ttheta(3,1) = sth;
    Ttheta(3,2) = 0;
    Ttheta(3,3) = cth;

    Tpsi(1,1) = cpsi;
    Tpsi(1,2) = spsi;
    Tpsi(1,3) = 0;
    Tpsi(2,1) = -spsi;
    Tpsi(2,2) = cpsi;
    Tpsi(2,3) = 0;
    Tpsi(3,1) = 0;
    Tpsi(3,2) = 0;
    Tpsi(3,3) = 1;

    Matrix mat = Tphi * Ttheta * Tpsi;
    return mat;
}

int TrnPlayer::getCSVRecordDVL(poseT *pt, measT *mt)
{
    int FIELDS_PER_BEAM = 3;
    int MIN_FIELDS = 16 + 4 * FIELDS_PER_BEAM;
    char lbuf[LBUF_SZ] = {0};
    char **fields = NULL;
    int n_fields = 0;

    // field indices
    // [  1] = time
    // [  2] = auvN
    // [  3] = auvE
    // [  4] = depth
    // [  5] = yaw
    // [  6] = pitch
    // [  7] = roll
    // [  8] = 0         flag(0)
    // [  9] = 0         flag(0)
    // [ 10] = 0         flag(0)
    // [ 11] = vx
    // [ 12] = vy
    // [ 13] = vz
    // [ 14] = dvlvalid
    // [ 15] = bottomlock
    // [ 16] = numbeams
    // [ 17] = 0          beam_number[0]
    // [ 18] = measStatus status
    // [ 19] = range      range
    // [ 20] = 1          beam_number[1]
    // [ 21] = measStatus
    // [ 22] = range
    // [ 23] = 2          beam_number[2]
    // [ 24] = measStatus
    // [ 25] = range
    // [ 26] = 3          beam_number[3]
    // [ 27] = measStatus
    // [ 28] = range
    // fixed number of beams (4), 3 fields/beam

    enum {
        TIME = 0,
        UTMN,
        UTME,
        DEPTH,
        HDG,   // psi
        PITCH, // theta (0 for LHV)
        ROLL,  // phi (0 for LHV)
        F0,    // (0)
        F1,    // (0)
        F2,    // (0)
        VX,
        VY,
        VZ,
        FDVL,  // (1)
        FLOCK, // (1)
        NMEAS,
        BEAMS,
        // beam_n
        // status
        // range
        // ...
    };

    if(fgets(lbuf, LBUF_SZ, ctx->csv_log) != NULL) {

        if(parseCSV(lbuf, &fields, &n_fields) != 0) {
            fprintf(stderr, "%s - ERR parsing line %s\n", __func__, lbuf);
            return -1;
        }

        if(n_fields < MIN_FIELDS) {
            fprintf(stderr, "%s - ERR too few fields [%d/%d]\n", __func__, n_fields, MIN_FIELDS);
            free(fields);
            return -1;
        }

        // load field values into poseT, measT
        sscanf(fields[TIME], "%lf", &pt->time);

        // apply decimation filter, if enabled
        if(decimate(pt->time) != 0)
            return RI_DEC;

        sscanf(fields[UTMN], "%lf", &pt->x);
        sscanf(fields[UTME], "%lf", &pt->y);
        sscanf(fields[DEPTH], "%lf", &pt->z);
        double hpr[3] = {0.,0.,0.};
        sscanf(fields[HDG], "%lf", &hpr[0]);
        sscanf(fields[PITCH], "%lf", &hpr[1]);
        sscanf(fields[ROLL], "%lf", &hpr[2]);

        // heading, pitch, roll (rad)
        pt->psi = hpr[0];
        pt->theta = hpr[1];
        pt->phi = hpr[2];

        sscanf(fields[VX], "%lf", &pt->vx);
        sscanf(fields[VY], "%lf", &pt->vy);
        sscanf(fields[VZ], "%lf", &pt->vz);
        int d[2] = {0};
        sscanf(fields[FDVL], "%d", &d[0]);
        sscanf(fields[FLOCK], "%d", &d[1]);
        pt->bottomLock = d[0];
        pt->dvlValid = d[1];
        sscanf(fields[NMEAS], "%d", &mt->numMeas);

        // wx, wy, wz: rotation rates for dead reckoned solution not required
         pt->wx = pt->wy = pt->wz = 0.;

        static unsigned int ping = 0;
        mt->ping_number = ping++;
        mt->time  = pt->time;
        mt->x     = pt->x;
        mt->y     = pt->y;
        mt->z     = pt->z;
        mt->phi   = pt->phi;
        mt->theta = pt->theta;
        mt->psi   = pt->psi;

        if(mt->numMeas != (n_fields - BEAMS)/FIELDS_PER_BEAM){
            fprintf(stderr, "%s - ERR numMeas > fields %d/%d %d\n", __func__, mt->numMeas, n_fields, ((n_fields-BEAMS)/FIELDS_PER_BEAM));
            free(fields);
            return -1;
        }

        if(mt->numMeas > 0) {
            mt->ranges = (double *)realloc(mt->ranges, mt->numMeas * sizeof(double));
            mt->alongTrack  = (double *)realloc(mt->alongTrack, mt->numMeas * sizeof(double));
            mt->crossTrack  = (double *)realloc(mt->crossTrack, mt->numMeas * sizeof(double));
            mt->altitudes   = (double *)realloc(mt->altitudes, mt->numMeas * sizeof(double));
            mt->alphas      = (double *)realloc(mt->alphas, mt->numMeas * sizeof(double));
            mt->covariance  = (double *)realloc(mt->covariance, mt->numMeas * sizeof(double));
            mt->beamNums = (int *)realloc(mt->beamNums, mt->numMeas * sizeof(int));
            mt->measStatus  = (bool *)realloc(mt->measStatus, mt->numMeas * sizeof(bool));

            for(int j = 0; j < mt->numMeas; j++) {
                int x = BEAMS + (j * FIELDS_PER_BEAM);
                int z = 0;
                sscanf(fields[x], "%d", &mt->beamNums[j]);
                sscanf(fields[x+1], "%d", &z);
                sscanf(fields[x+2], "%lf", &mt->ranges[j]);

                mt->measStatus[j] = (z != 0);
                mt->alongTrack[j] = 0.;
                mt->crossTrack[j] = 0.;
                mt->altitudes[j] = 0.;
            }

            if(ctx->debug > 0){
               fprintf(stderr, "--- %s ---\n", __func__);
               fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
               fprintf(stderr, "mt->dataType %d\n", mt->dataType);
               fprintf(stderr, "mt->time     %.3lf\n", mt->time);
               fprintf(stderr, "mt->x        %.3lf\n", mt->x);
               fprintf(stderr, "mt->y        %.3lf\n", mt->y);
               fprintf(stderr, "mt->z        %.3lf\n", mt->z);
               fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
               fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
               fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
               fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
               fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
               fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
               fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
               fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
               fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
                for(int i=0; i< mt->numMeas; i++) {
                   fprintf(stderr, "[%3d] n,stat,rng, [a,x,d] : %3d,  %d, %8.3lf, %8.3lf, %8.3lf, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i], mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]);
                }
            }
        }

        // free field array
        if(fields != NULL)
            free(fields);

        return 0;
    } else {
        fprintf(stderr, "\nEnd of log\n");
        return RI_EOF;
    }

    return -1;}

int TrnPlayer::getCSVRecordIDT(poseT *pt, measT *mt)
{
    int FIELDS_PER_BEAM = 3;
    int MIN_FIELDS = 16 + 120 * FIELDS_PER_BEAM;

    char lbuf[LBUF_SZ] = {0};
    char **fields = NULL;
    int n_fields = 0;

    // field indices
    // [  1] = time(i);
    // [  2] = auvN(i);
    // [  3] = auvE(i);
    // [  4] = depth(i);
    // [  5] = yaw(i);
    // [  6] = pitch(i);
    // [  7] = roll(i);
    // [  8] = 0;       flag(0)
    // [  9] = 0;       flag(0)
    // [ 10] = 0;       flag(0)
    // [ 11] = vx(i);
    // [ 12] = vy(i);
    // [ 13] = vz(i);
    // [ 14] = dvlvalid;
    // [ 15] = bottomlock;
    // [ 16] = numbeams; // 120 beams
    // [ 17] = beam_number
    // [ 18] = beamStatus[i]
    // [ 19] = range[i];
    // ...
    // (fixed number of beams (120), 3 fields per beam)

    enum {
        TIME = 0,
        UTMN,
        UTME,
        DEPTH,
        HDG,   // psi
        PITCH, // theta (0 for LHV)
        ROLL,  // phi (0 for LHV)
        F0,    // (0)
        F1,    // (0)
        F2,    // (0)
        VX,
        VY,
        VZ,
        FDVL,   // (1)
        FLOCK,  // (1)
        NMEAS,
        BEAMS,
        // beam_n
        // measStatus
        // range
        //...
    };

    if(fgets(lbuf, LBUF_SZ, ctx->csv_log) != NULL) {

        if(parseCSV(lbuf, &fields, &n_fields) != 0) {
            fprintf(stderr, "%s - ERR parsing line %s\n", __func__, lbuf);
            return -1;
        }

        if(n_fields < MIN_FIELDS) {
            fprintf(stderr, "%s - ERR too few fields [%d/%d]\n", __func__, n_fields, MIN_FIELDS);
            free(fields);
            return -1;
        }

        // load field values into poseT, measT
        sscanf(fields[TIME], "%lf", &pt->time);

        // apply decimation filter, if enabled
        if(decimate(pt->time) != 0)
            return RI_DEC;

        sscanf(fields[UTMN], "%lf", &pt->x);
        sscanf(fields[UTME], "%lf", &pt->y);
        sscanf(fields[DEPTH], "%lf", &pt->z);
        double hpr[3] = {0.,0.,0.};
        sscanf(fields[HDG], "%lf", &hpr[0]);
        sscanf(fields[PITCH], "%lf", &hpr[1]);
        sscanf(fields[ROLL], "%lf", &hpr[2]);

        // heading, pitch, roll (rad)
        pt->psi = hpr[0];
        pt->theta = hpr[1];
        pt->phi = hpr[2];

        sscanf(fields[VX], "%lf", &pt->vx);
        sscanf(fields[VY], "%lf", &pt->vy);
        sscanf(fields[VZ], "%lf", &pt->vz);
        int d[2] = {0};
        sscanf(fields[FDVL], "%d", &d[0]);
        sscanf(fields[FLOCK], "%d", &d[1]);
        pt->bottomLock = d[0];
        pt->dvlValid = d[1];
        sscanf(fields[NMEAS], "%d", &mt->numMeas);

        // wx, wy, wz: rotation rates for dead reckoned solution not required
        pt->wx = pt->wy = pt->wz = 0.;

        static unsigned int ping = 0;
        mt->ping_number = ping++;
        mt->time  = pt->time;
        mt->x     = pt->x;
        mt->y     = pt->y;
        mt->z     = pt->z;
        mt->phi   = pt->phi;
        mt->theta = pt->theta;
        mt->psi   = pt->psi;

        if(mt->numMeas != (n_fields - BEAMS)/FIELDS_PER_BEAM){
            fprintf(stderr, "%s - ERR numMeas > fields %d/%d %d\n", __func__, mt->numMeas, n_fields, ((n_fields-BEAMS)/FIELDS_PER_BEAM));
            free(fields);
            return -1;
        }

        if(mt->numMeas > 0) {
            mt->ranges = (double *)realloc(mt->ranges, mt->numMeas * sizeof(double));
            mt->alongTrack  = (double *)realloc(mt->alongTrack, mt->numMeas * sizeof(double));
            mt->crossTrack  = (double *)realloc(mt->crossTrack, mt->numMeas * sizeof(double));
            mt->altitudes   = (double *)realloc(mt->altitudes, mt->numMeas * sizeof(double));
            mt->alphas      = (double *)realloc(mt->alphas, mt->numMeas * sizeof(double));
            mt->covariance  = (double *)realloc(mt->covariance, mt->numMeas * sizeof(double));
            mt->beamNums = (int *)realloc(mt->beamNums, mt->numMeas * sizeof(int));
            mt->measStatus  = (bool *)realloc(mt->measStatus, mt->numMeas * sizeof(bool));

            for(int j = 0; j < mt->numMeas; j++) {
                int x = BEAMS + (j * FIELDS_PER_BEAM);
                int z = 0;
                sscanf(fields[x], "%d", &mt->beamNums[j]);
                sscanf(fields[x+1], "%d", &z);
                sscanf(fields[x+2], "%lf", &mt->ranges[j]);
                mt->measStatus[j] = (z != 0);
            }

            if(ctx->debug > 0){
               fprintf(stderr, "--- %s ---\n", __func__);
               fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
               fprintf(stderr, "mt->dataType %d\n", mt->dataType);
               fprintf(stderr, "mt->time     %.3lf\n", mt->time);
               fprintf(stderr, "mt->x        %.3lf\n", mt->x);
               fprintf(stderr, "mt->y        %.3lf\n", mt->y);
               fprintf(stderr, "mt->z        %.3lf\n", mt->z);
               fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
               fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
               fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
               fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
               fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
               fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
               fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
               fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
               fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
                for(int i=0; i< mt->numMeas; i++) {
                   fprintf(stderr, "[%3d] n,stat,rng, [a,x,d] : %3d,  %d, %8.3lf, %8.3lf, %8.3lf, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i], mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]);
                }
            }
        }

        // free field array
        if(fields != NULL)
            free(fields);

        return 0;
    } else {
        fprintf(stderr, "\nEnd of log\n");
        return RI_EOF;
    }

    return -1;
}

int TrnPlayer::getCSVRecordMB(poseT *pt, measT *mt)
{
    int FIELDS_PER_BEAM = 6;
    int MIN_FIELDS = 16;

    char lbuf[LBUF_SZ] = {0};
    char **fields = NULL;
    int n_fields = 0;

    // field indices
    // [  1] = time(i);
    // [  2] = auvN(i);
    // [  3] = auvE(i);
    // [  4] = depth(i);
    // [  5] = yaw(i);   heading/psi
    // [  6] = pitch(i); pitch/theta
    // [  7] = roll(i);  roll/phi
    // [  8] = 0;        flag(0)
    // [  9] = 0;        flag(0)
    // [ 10] = 0;        flag(0)
    // [ 11] = vx(i);
    // [ 12] = vy(i);
    // [ 13] = vz(i);
    // [ 14] = dvlvalid;
    // [ 15] = bottomlock;
    // [ 16] = numbeams;
    // [ 17] = beam_number
    // [ 18] = beamStatus[i]
    // [ 19] = range[i];
    // [ 20] = alongTrack[i];
    // [ 21] = crossTrack[i];
    // [ 22] = altitudes[i]
    // (variable number of beams, 6 fields per beam)

    enum {
        TIME = 0,
        UTMN,
        UTME,
        DEPTH,
        HDG,   // psi
        PITCH, // theta (0 for LHV)
        ROLL,  // phi (0 for LHV)
        F0,    // (0)
        F1,    // (0)
        F2,    // (0)
        VX,
        VY,
        VZ,
        FDVL,  // (1)
        FLOCK, // (1)
        NMEAS,
        BEAMS,
        // beam_n
        // measStatus
        // range
        // along
        // across
        // down
        //...
    };

    if(fgets(lbuf, LBUF_SZ, ctx->csv_log) != NULL) {

        if(parseCSV(lbuf, &fields, &n_fields) != 0) {
            fprintf(stderr, "%s - ERR parsing line %s\n", __func__, lbuf);
            return -1;
        }

        if(n_fields < MIN_FIELDS) {
            fprintf(stderr, "%s - ERR too few fields [%d/%d]\n", __func__, n_fields, MIN_FIELDS);
            free(fields);
            return -1;
        }

        // load field values into poseT, measT
        sscanf(fields[TIME], "%lf", &pt->time);

        // apply decimation filter, if enabled
        if(decimate(pt->time) != 0)
            return RI_DEC;

        sscanf(fields[UTMN], "%lf", &pt->x);
        sscanf(fields[UTME], "%lf", &pt->y);
        sscanf(fields[DEPTH], "%lf", &pt->z);
        double hpr[3] = {0.,0.,0.};
        sscanf(fields[HDG], "%lf", &hpr[0]);
        sscanf(fields[PITCH], "%lf", &hpr[1]);
        sscanf(fields[ROLL], "%lf", &hpr[2]);

        // heading (psi), pitch (theta), roll (phi)
        pt->psi = hpr[0];
        pt->theta = hpr[1];
        pt->phi = hpr[2];

        sscanf(fields[VX], "%lf", &pt->vx);
        sscanf(fields[VY], "%lf", &pt->vy);
        sscanf(fields[VZ], "%lf", &pt->vz);
        int d[2] = {0};
        sscanf(fields[FDVL], "%d", &d[0]);
        sscanf(fields[FLOCK], "%d", &d[1]);
        pt->bottomLock = d[0];
        pt->dvlValid = d[1];
        sscanf(fields[NMEAS], "%d", &mt->numMeas);

        // wx, wy, wz: rotation rates for dead reckoned solution not required
        pt->wx = pt->wy = pt->wz = 0.;

        static unsigned int ping = 0;
        mt->ping_number = ping++;
        mt->time  = pt->time;
        mt->x     = pt->x;
        mt->y     = pt->y;
        mt->z     = pt->z;
        mt->phi   = pt->phi;
        mt->theta = pt->theta;
        mt->psi   = pt->psi;

        if(mt->numMeas != (n_fields - BEAMS)/FIELDS_PER_BEAM){
            fprintf(stderr, "%s - ERR numMeas > fields %d/%d %d\n", __func__, mt->numMeas, n_fields, ((n_fields-BEAMS)/FIELDS_PER_BEAM));
            free(fields);
            return -1;
        }

        if(mt->numMeas > 0) {
            mt->ranges = (double *)realloc(mt->ranges, mt->numMeas * sizeof(double));
            mt->alongTrack  = (double *)realloc(mt->alongTrack, mt->numMeas * sizeof(double));
            mt->crossTrack  = (double *)realloc(mt->crossTrack, mt->numMeas * sizeof(double));
            mt->altitudes   = (double *)realloc(mt->altitudes, mt->numMeas * sizeof(double));
            mt->alphas      = (double *)realloc(mt->alphas, mt->numMeas * sizeof(double));
            mt->covariance  = (double *)realloc(mt->covariance, mt->numMeas * sizeof(double));
            mt->beamNums = (int *)realloc(mt->beamNums, mt->numMeas * sizeof(int));
            mt->measStatus  = (bool *)realloc(mt->measStatus, mt->numMeas * sizeof(bool));

            for(int j = 0; j < mt->numMeas; j++) {
                int x = BEAMS + (j * FIELDS_PER_BEAM);
                int z = 0;
                sscanf(fields[x], "%d", &mt->beamNums[j]);
                sscanf(fields[x+1], "%d", &z);
                sscanf(fields[x+2], "%lf", &mt->ranges[j]);
                sscanf(fields[x+3], "%lf", &mt->alongTrack[j]);
                sscanf(fields[x+4], "%lf", &mt->crossTrack[j]);
                sscanf(fields[x+5], "%lf", &mt->altitudes[j]);
                mt->measStatus[j] = (z != 0);
            }

            if(ctx->debug > 0){
               fprintf(stderr, "--- %s ---\n", __func__);
               fprintf(stderr, "mt->ping     %u\n", mt->ping_number);
               fprintf(stderr, "mt->dataType %d\n", mt->dataType);
               fprintf(stderr, "mt->time     %.3lf\n", mt->time);
               fprintf(stderr, "mt->x        %.3lf\n", mt->x);
               fprintf(stderr, "mt->y        %.3lf\n", mt->y);
               fprintf(stderr, "mt->z        %.3lf\n", mt->z);
               fprintf(stderr, "mt->phi      %.3lf\n", mt->phi);
               fprintf(stderr, "mt->theta    %.3lf\n", mt->theta);
               fprintf(stderr, "mt->psi      %.3lf\n", mt->psi);
               fprintf(stderr, "pt->vx       %.3lf\n", pt->vx);
               fprintf(stderr, "pt->vy       %.3lf\n", pt->vy);
               fprintf(stderr, "pt->vz       %.3lf\n", pt->vz);
               fprintf(stderr, "pt->dvlValid   %d\n", pt->dvlValid);
               fprintf(stderr, "pt->bottomLock %d\n", pt->bottomLock);
               fprintf(stderr, "mt->numMeas %d\n", mt->numMeas);
                for(int i=0; i< mt->numMeas; i++) {
                   fprintf(stderr, "[%3d] n,stat,rng, [a,x,d] : %3d,  %d, %8.3lf, %8.3lf, %8.3lf, %8.3lf\n",i, mt->beamNums[i], (mt->measStatus[i] ? 1 : 0), mt->ranges[i], mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]);
                }
            }
        }

        // free field array
        if(fields != NULL)
            free(fields);

        return 0;
    } else {
        fprintf(stderr, "\nEnd of log\n");
        return RI_EOF;
    }

    return -1;
}

int TrnPlayer::getCSVRecord(poseT *pt, measT *mt)
{

    if(ctx->input_format == IOFMT_CSV_DVL)
        return getCSVRecordDVL(pt, mt);

    if(ctx->input_format == IOFMT_CSV_IDT)
        return getCSVRecordIDT(pt, mt);

    if(ctx->input_format == IOFMT_CSV_MB)
        return getCSVRecordMB(pt, mt);

    fprintf(stderr, "Invalid CSV input format %d/%s\n", ctx->input_format, TrnPlayerCtx::log_name(ctx->input_format));

    return -1;
}

// Read next data log line
// call appropriate reader based on log type

 int TrnPlayer::getNextRecord(poseT *pt, measT *mt)
{
    if(ctx->input_format == IOFMT_MBTRN) {
        // Format : Mbtrn.log
        return getMbTrnRecord(pt, mt);
    } else
    if(ctx->input_format == IOFMT_TRNNAV) {
        // Format : TerrainNav.log
        return getTNavRecord(pt, mt);
    } else
    if(ctx->input_format == IOFMT_TRNAID) {
            // Format : TerrainAid.log
            return getTrnAidRecord(pt, mt);
    } else
    if(ctx->input_format == IOFMT_CSV_DVL || ctx->input_format == IOFMT_CSV_IDT || ctx->input_format == IOFMT_CSV_MB) {
        // Format : TerrainAid.log
        return getCSVRecord(pt, mt);
    }


    // invalid log type
    return -1;
}

// zero mt, pt members

int TrnPlayer::reset_pt(poseT **pt, unsigned int len)
{
    for (int i = 0; i < len; i++) {
        pt[i]->x = pt[i]->y = pt[i]->z = 0.;
        pt[i]->phi = pt[i]->theta = pt[i]->phi = 0.;
        pt[i]->vx = pt[i]->vy = pt[i]->vz = 0.;
        pt[i]->wx = pt[i]->wy = pt[i]->wz = 0.;
        memset(pt[i]->covariance, 0, N_COVAR);
    }
    return 0;
}

int TrnPlayer::reset_mt(measT **mt, unsigned int len)
{
    for (int i = 0; i < len; i++) {
        int sz = mt[i]->numMeas;
        mt[i]->x = mt[i]->y = mt[i]->z = 0.;
        mt[i]->phi = mt[i]->theta = mt[i]->phi = 0.;
        memset(mt[i]->ranges, 0, sz * sizeof(double));
        memset(mt[i]->crossTrack, 0, sz * sizeof(double));
        memset(mt[i]->alongTrack, 0, sz * sizeof(double));
        memset(mt[i]->altitudes, 0, sz * sizeof(double));
        memset(mt[i]->alphas, 0, sz * sizeof(double));
        memset(mt[i]->beamNums, 0, sz * sizeof(int));
        memset(mt[i]->measStatus, 0, sz * sizeof(bool));
        memset(mt[i]->covariance, 0, sz * sizeof(double));
    }
    return 0;
}

// Get sensor geometry (DR,DT) from vehicle spec configuration

void TrnPlayer::getSensorGeometry()
{
    if(trn != NULL && ctx!= NULL) {
        // this only gets the configured input (stype)
        // geometry; it is sufficient for translating
        // CSV output (DVL -> MB, e.g.)
        sensorT *S = trn->tNavFilter->vehicle->sensors;
        transformT *T = trn->tNavFilter->vehicle->T_sv;
        int m = 0;
        for(m = 0; m < trn->tNavFilter->vehicle->numSensors; m++) {

            if(S[m].type == ctx->sensor_type) {
                // phi, thata, psi (pitch, roll, yaw) (rad)
                ctx->geo_dr[0] = T[m].rotation[0];
                ctx->geo_dr[1] = T[m].rotation[1];
                ctx->geo_dr[2] = T[m].rotation[2];
                ctx->geo_dt[0] = T[m].translation[0];
                ctx->geo_dt[1] = T[m].translation[1];
                ctx->geo_dt[2] = T[m].translation[2];
                break;
            }
        }
    }
}

// opwn IO files

void TrnPlayer::init_io()
{
    // open log readers for bathymetry and navigation
    if(ctx->input_format == IOFMT_CSV_DVL || ctx->input_format == IOFMT_CSV_IDT || ctx->input_format == IOFMT_CSV_MB) {
        ctx->csv_log = fopen(ctx->dpath, "r");
    } else {
        ctx->trn_log = new DataLogReader(ctx->dpath);
    }

    if(ctx->input_format == IOFMT_TRNAID) {
        ctx->nav_log = new DataLogReader(ctx->npath);
    }

    // open CSV measurement out file if flag set
    if((ctx->oflags & OUT_MEAS_FILE) != 0 && strlen(ctx->mopath) > 0) {
        ctx->meas_out = fopen(ctx->mopath, "w+");
    }
    // open CSV estimate out file if flag set
    if((ctx->oflags & OUT_EST_FILE) != 0 && strlen(ctx->eopath) > 0) {
        ctx->est_out = fopen(ctx->eopath, "w+");
    }

    // if enabled, open particle filter log
    if(ctx->pf_omode != PFO_NONE) {
        ctx->part_out = new ofstream;
        if(ctx->part_out != NULL) {
            ctx->part_out->open(ctx->pfopath);
        } else {
            fprintf(stderr, "%s:%d: ERR - could not open PF log (%s)\n", __func__, __LINE__, ctx->pfopath);
        }
    }
}

void TrnPlayer::copy_config()
{
    if(strlen(ctx->cpath) > 0) {
        char syscmd[2048]={0};
        snprintf(syscmd, 2048, "cp %s latestTRN/.", ctx->cpath);
        system(syscmd);
    }
}

// show session summary

void TrnPlayer::show_summary()
{
    // print summary
    show();
    int wkey = 10;
    fprintf(stderr, " --- Log Summary ---\n");
    fprintf(stderr, " %*s : %d\n", wkey, "rec_n", rec_n);
    fprintf(stderr, " %*s : %d\n", wkey, "val_n", val_n);
    fprintf(stderr, " %*s : %d\n", wkey, "dec_n", dec_n);
    fprintf(stderr, " %*s : %d\n", wkey, "err_n", err_n);
    fprintf(stderr, " %*s : %d\n", wkey, "est_n", est_n);
    time_t tt[2] = {(time_t)ctx->time_range[0], (time_t)ctx->time_range[1]};
    struct tm t_tm[2];
    gmtime_r(&tt[0], &t_tm[0]);
    gmtime_r(&tt[1], &t_tm[1]);

    fprintf(stderr, " %*s : %.3lf  %4d-%02d-%02dT%02d:%02d:%02d\n",
            wkey, "start", ctx->time_range[0],
            t_tm[0].tm_year+1900, t_tm[0].tm_mon+1, t_tm[0].tm_mday,
            t_tm[0].tm_hour, t_tm[0].tm_min, t_tm[0].tm_sec);
    fprintf(stderr, " %*s : %.3lf  %4d-%02d-%02dT%02d:%02d:%02d\n",
            wkey, "end", ctx->time_range[1],
            t_tm[1].tm_year+1900, t_tm[1].tm_mon+1, t_tm[1].tm_mday,
            t_tm[1].tm_hour, t_tm[1].tm_min, t_tm[1].tm_sec);

    fprintf(stderr, " %*s : %.3lf s\n", wkey, "elapsed", (ctx->time_range[1] - ctx->time_range[0]));

    fprintf(stderr, " %*s : %u %u (%u)\n", wkey, "pings", ctx->ping_range[0], ctx->ping_range[1], (ctx->ping_range[1]- ctx->ping_range[0]));
    fprintf(stderr, "\n");

}


// Return magnitude of a vector v[x,y,z]

 double TrnPlayer::vnorm( double v[] )
{
    double vnsq = 0.;
    int i = 0;
    for(i = 0; i < 3; i++) 
        vnsq += pow(v[i],2.);

    return( sqrt( vnsq ) );
}

// compare timestamp to previous, decimation period
// return 0: accept, RI_DEC: reject
int TrnPlayer::decimate(double timestamp)
{

    // Calculate the sample period in seconds to use
    double period = (double)ctx->dec_period_ms / 1000.;

    // accept if decimation disabled
    if(ctx->dec_period_ms <= 0)
        return 0;

    // reject timestamp inside the previous sample period
    if (timestamp < (ctx->dec_prev_time + period)) {
        // fprintf(stderr, "%s:%d - rejecting %.3lf\n", __func__, __LINE__, timestamp);
      return RI_DEC;
    }

    // update last timestamp
    ctx->dec_prev_time = timestamp;

    return 0;
}


// Output TRN estimate to console in pretty (human readable) format

void TrnPlayer::out_pretty(poseT &pt, poseT &mse, poseT &mle)
{
    // output to console
    // covariances
    double cov[3] = {sqrt(mse.covariance[0]), sqrt(mse.covariance[2]), sqrt(mse.covariance[5])};

    // offset (estimate - navigation)
    double ofs[3] = {(mse.x - pt.x), (mse.y - pt.y), (mse.z - pt.z)};

    if( (ctx->oflags & OUT_MMSE) != 0 ) {
        // output MMSE
        printf("%*s %*s : %0.3lf, %0.3lf, %0.3lf, %0.3lf, %d\n",
               4, "MMSE", 9, "[t,x,y,z,s]",
               mse.time, mse.x, mse.y, mse.z, (ctx->last_meas?1:0));
    }
    if((ctx->oflags & OUT_MLE) != 0){
        // output MLE
        printf("%*s %*s : %0.3lf, %0.3lf, %0.3lf, %0.3lf\n",
               4, "MLE", 9, "[t,x,y,z]",
                mle.time, mle.x, mle.y, mle.z);
    }
    // output offset, covariances
    printf("%*s %*s : %+8.3lf, %+8.3lf, %+8.3lf\n",
           4, "OFS", 9, "[x,y,z]",
           ofs[0], ofs[1], ofs[2]);
    printf("%*s %*s : %8.3f, %8.3f, %8.3f, %.3f\n",
           4, "COV", 9, "[x,y,z,m]",
           cov[0], cov[1], cov[2], vnorm(cov));
    printf("\n");
}

void TrnPlayer::dvl_to_mb(measT &mt)
{
    if (mt.numMeas < 0 || mt.numMeas > 4) {
        fprintf(stderr, "%s ERR invalid beam count (%d/4)\n", __func__, mt.numMeas);
        return;
    }

    // beam[i] angles (deg)
    // beam yaw angles
    //      X
    //      ^
    //      |
    //    1 | 3
    //      |------> Y
    //    4   2
    //
    double yr[4] = {
        Math::degToRad(-45.),
        Math::degToRad(135),
        Math::degToRad(45),
        Math::degToRad(-135.)
    };

    // beam pitch angles (wrt Z axis)
    //      |------>X
    //      |  .
    //      | pr  o
    //      v
    //      Z
    //
    double pr[4] = {
        Math::degToRad(30.),
        Math::degToRad(30.),
        Math::degToRad(30.),
        Math::degToRad(30.)
    };

    // mounting translation?
    // mounting rotation matrix

    if(ctx->debug > 1) {
        fprintf(stderr, "%s:%d - DR[%.2lf, %.2lf, %.2lf] DT[%.2lf, %.2lf, %.2lf]\n",
                __func__, __LINE__,
                Math::radToDeg(ctx->geo_dr[0]),Math::radToDeg(ctx->geo_dr[1]),Math::radToDeg(ctx->geo_dr[2]),
                ctx->geo_dt[0], ctx->geo_dt[1], ctx->geo_dt[2]);
    }

    Matrix SF_R_VF = euler321(ctx->geo_dr[0], ctx->geo_dr[1], ctx->geo_dr[2]);

    // vehicle pitch, roll matrix
    Matrix VF_R_LHV = euler321(mt.phi, mt.theta, 0.);

    for(int i = 0; i < mt.numMeas; i++) {
        double R = mt.ranges[i];

        // Sensor frame beam components
        Matrix SF_BEAMS(3,1);
        // along track (x)
        SF_BEAMS(1,1) = R * (sin(pr[i]) * cos(yr[i]));
        // cross track (y)
        SF_BEAMS(2,1) = R * (sin(pr[i]) * sin(yr[i]));
        // down (z)
        SF_BEAMS(3,1) = R * cos(pr[i]);

        Matrix beami = VF_R_LHV.t() * SF_R_VF.t() * SF_BEAMS;

        mt.alongTrack[i] = beami(1,1);
        mt.crossTrack[i] = beami(2,1);
        mt.altitudes[i] = beami(3,1);
        mt.beamNums[i] = i;
        mt.measStatus[i] = 1;

        if(ctx->debug > 0) {
           fprintf(stderr, "[n s R a x d] %d, %d, %.6lf, %.6lf,%.6lf,%.6lf\n",
                   mt.beamNums[i],
                   (mt.measStatus[i] ? 1 : 1),
                   mt.ranges[i],
                   mt.alongTrack[i], mt.crossTrack[i], mt.altitudes[i]);
        }
    }
}

void TrnPlayer::idt_to_mb(measT &mt)
{
    if (mt.numMeas < 0 || mt.numMeas < 120) {
        fprintf(stderr, "%s ERR invalid beam count (%d/120)\n", __func__, mt.numMeas);
        return;
    }

    // mounting translation?

    // mounting rotation matrix
    double dr_phi = Math::degToRad(ctx->geo_dr[0]);   // 45
    double dr_theta = Math::degToRad(ctx->geo_dr[1]); // 0
    double dr_psi = Math::degToRad(ctx->geo_dr[2]);   // 90

    Matrix SF_R_VF = euler321(dr_phi, dr_theta, dr_psi);

    // vehicle pitch, roll, heading matrix
    Matrix VF_R_LHV = euler321(mt.phi, mt.theta, 0);

    for(int i=0; i < mt.numMeas; i++) {
        double R = mt.ranges[i];
        // beam[i] angle (deg)
        double wd = -60 + i;
        double wr = Math::degToRad(wd);

        Matrix SF_BEAMS(3,1);
        SF_BEAMS(1,1) = (R * sin(wr));
        SF_BEAMS(2,1) =  0.;
        SF_BEAMS(3,1) = (R * cos(wr));

        Matrix beami = VF_R_LHV.t() * SF_R_VF.t() * SF_BEAMS;

        mt.beamNums[i] = i;
        mt.alongTrack[i] = beami(1,1);
        mt.crossTrack[i] = beami(2,1);
        mt.altitudes[i] = beami(3,1);

    }
}

// output measurement in MB CVS format
void TrnPlayer::out_csv_mb(poseT &pt, measT &mt)
{
    // field indices
    // [  1] = time           decimal epoch seconds
    // [  2] = auvN           northing (m)
    // [  3] = auvE           easting (m)
    // [  4] = depth          depth (m)
    // [  5] = yaw            heading/psi (rad)
    // [  6] = pitch          pitch/theta (rad)
    // [  7] = roll           roll/phi (rad)
    // [  8] = 0              flag(0)
    // [  9] = 0              flag(0)
    // [ 10] = 0              flag(0)
    // [ 11] = vx             velocity x (m/s)
    // [ 12] = vy             velocity y (m/s)
    // [ 13] = vz             velocity z (m/s)
    // [ 14] = dvlvalid       dvl valid (1: valid)
    // [ 15] = bottomlock     has bottomlock (1: valid)
    // [ 16] = numbeams       number of beams
    // [ 17] = beam_number[i] beam number
    // [ 18] = beamStatus[i]  beam valid (1:valid)
    // [ 19] = range[i]       beam range (m)
    // [ 20] = alongTrack[i]  along track (m)
    // [ 21] = crossTrack[i]  across track (m)
    // [ 22] = altitudes[i]   down (m)
    // ...
    // (variable number of beams, 6 fields per beam)

    char obuf[CSV_OBUF_SZ] = {0}, *op = obuf;
    int rem = CSV_OBUF_SZ;

    if(mt.dataType == TRN_SENSOR_DELTAT) {
        // convert from IDT to MB (need beam components)
        idt_to_mb(mt);
    } else if(mt.dataType == TRN_SENSOR_DVL) {
        // convert from DVL to MB (need beam components)
        dvl_to_mb(mt);
    } else if(mt.dataType != TRN_SENSOR_MB) {
        fprintf(stderr, "%s - WARN can't translate dataType %d to MB CSV\n", __func__, mt.dataType);
    }

    int br = snprintf(op, rem, "%0.3lf,", mt.time);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
           mt.x, mt.y, mt.z);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
           pt.psi, pt.theta, pt.phi);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    // flags[3] = 0
    br = snprintf(op, rem, "0,0,0,");
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // vx, vy, vz (0)
    br = snprintf(op, rem, "%.3lf,%.3lf,%.3lf,", pt.vx, pt.vy, pt.vz);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // sounding valid, bottom lock [1]
    br = snprintf(op, rem, "1,1,");
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%d", mt.numMeas);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    for(int i=0; i < mt.numMeas; i++) {
        br = snprintf(op, rem, ",%d,%d,%lf,%lf,%lf,%lf",
               mt.beamNums[i],
               (mt.measStatus[i] ? 1 : 1),
               mt.ranges[i],
               mt.alongTrack[i], mt.crossTrack[i], mt.altitudes[i]);
        if(br > 0) {
            rem -= br;
            op += br;
        }
    }
    br = snprintf(op, rem, "\n");

    if(ctx->oflags & OUT_MEAS_CSV)
        printf("%s", obuf);

    if(ctx->meas_out != NULL) {
        fwrite(obuf, strlen(obuf), 1, ctx->meas_out);
        fflush(ctx->meas_out);
    }

}

// output measurement in IDT CVS format
void TrnPlayer::out_csv_idt(poseT &pt, measT &mt)
{
    // field indices
    // [  1] = time           decimal epoch seconds
    // [  2] = auvN           northing (m)
    // [  3] = auvE           easting (m)
    // [  4] = depth          depth (m)
    // [  5] = yaw            heading/psi (rad)
    // [  6] = pitch          pitch/theta (rad)
    // [  7] = roll           roll/phi (rad)
    // [  8] = 0;             flag(0)
    // [  9] = 0;             flag(0)
    // [ 10] = 0;             flag(0)
    // [ 11] = vx             velocity x (m/s)
    // [ 12] = vy             velocity y (m/s)
    // [ 13] = vz             velocity z (m/s)
    // [ 14] = dvlvalid       dvl valid (1: valid)
    // [ 15] = bottomlock     has bottomlock (1: valid)
    // [ 16] = numbeams       number of beams (120)
    // [ 17] = beam_number[i] beam number
    // [ 18] = beamStatus[i]  beam valid (1:valid)
    // [ 19] = range[i]       beam range (m)
    // ...
    // (fixed number of beams (120), 3 fields per beam)

    char obuf[CSV_OBUF_SZ] = {0}, *op = obuf;
    int rem = CSV_OBUF_SZ;

    if(mt.dataType == TRN_SENSOR_MB) {
        // convert from MB to IDT (120 beams)
        //mb_to_idt(mt);
        fprintf(stderr, "%s:%d - ERR invalid conversion (MB > IDT)\n", __func__, __LINE__);
    } else if(mt.dataType == TRN_SENSOR_DVL) {
        // convert from DVL to IDT (120 beams)
        //dvl_to_idt(mt);
        fprintf(stderr, "%s:%d - ERR invalid conversion (DVL > IDT)\n", __func__, __LINE__);
    } else if(mt.dataType != TRN_SENSOR_DELTAT) {
        fprintf(stderr, "%s - WARN can't translate dataType %d to IDT CSV\n", __func__, mt.dataType);
    }

    int br = snprintf(op, rem, "%0.3lf,", mt.time);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
           mt.x, mt.y, mt.z);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
           pt.psi, pt.theta, pt.phi);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // flags[3] = 0
    br = snprintf(op, rem, "0,0,0,");
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // vx, vy, vz (0)
    br = snprintf(op, rem, "%.3lf,%.3lf,%.3lf,", pt.vx, pt.vy, pt.vz);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // sounding valid, bottom lock [1]
    br = snprintf(op, rem, "1,1,");
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%d", mt.numMeas);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    for(int i=0; i < 120; i++) {
        // 120 beams required; zero missing beam status, range
        int status = (i <  mt.numMeas ? (mt.measStatus[i] ? 1 : 1) : 0);
        double range = (i < mt.numMeas ? mt.ranges[i] : 0.);

        br = snprintf(op, rem, ",%d,%d,%lf",
               i,
               status,
               range);
        if(br > 0) {
            rem -= br;
            op += br;
        }
    }
    snprintf(op, rem, "\n");

    if(ctx->oflags & OUT_MEAS_CSV)
        printf("%s", obuf);

    if(ctx->meas_out != NULL) {
        fwrite(obuf, strlen(obuf), 1, ctx->meas_out);
        fflush(ctx->meas_out);
    }
}

// output measurement in DVL CVS format
void TrnPlayer::out_csv_dvl(poseT &pt, measT &mt)
{
    // field indices
    // [  1] = time           decimal epoch seconds
    // [  2] = auvN           northing (m)
    // [  3] = auvE           easting (m)
    // [  4] = depth          depth (m)
    // [  5] = yaw            heading/psi (rad)
    // [  6] = pitch          pitch/theta (rad)
    // [  7] = roll           roll/phi (rad)
    // [  8] = 0              flag(0)
    // [  9] = 0              flag(0)
    // [ 10] = 0              flag(0)
    // [ 11] = vx             velocity x (m/s)
    // [ 12] = vy             velocity y (m/s)
    // [ 13] = vz             velocity z (m/s)
    // [ 14] = dvlvalid       dvl valid (1: valid)
    // [ 15] = bottomlock     has bottomlock (1: valid)
    // [ 16] = numbeams       number of beams (4)
    // [ 17] = beam_number[i] beam number
    // [ 18] = beamStatus[i]  beam valid (1:valid)
    // [ 19] = range[i]       beam range (m)
    // ...
    // (fixed number of beams (4), 3 fields per beam)

    char obuf[CSV_OBUF_SZ] = {0}, *op = obuf;
    int rem = CSV_OBUF_SZ;

    if(mt.dataType == TRN_SENSOR_MB) {
        // convert from MB to DVL (4 beams)
        //mb_to_dvl(mt);
        fprintf(stderr, "%s:%d - ERR invalid conversion (MB > DVL)\n", __func__, __LINE__);
    } else if(mt.dataType == TRN_SENSOR_DELTAT) {
        // convert from IDT to DVL (4 beams)
        //idt_to_dvl(mt);
        fprintf(stderr, "%s:%d - ERR invalid conversion (IDT > DVL)\n", __func__, __LINE__);
    } else if(mt.dataType != TRN_SENSOR_DVL) {
        fprintf(stderr, "%s - WARN can't translate dataType %d to DVL CSV\n", __func__, mt.dataType);
    }

    int br = snprintf(op, rem, "%0.3lf,", mt.time);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
           mt.x, mt.y, mt.z);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  pt.psi, pt.theta, pt.phi);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // flags[3] = 0
    br = snprintf(op, rem, "0,0,0,");
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // vx, vy, vz (0)
    br = snprintf(op, rem, "%.3lf,%.3lf,%.3lf,", pt.vx, pt.vy, pt.vz);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    // sounding valid, bottom lock [1]
    br = snprintf(op, rem, "%d,%d,", pt.dvlValid, pt.bottomLock);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%d", mt.numMeas);

    for(int i=0; i < 4; i++) {
        // 120 beams required; zero missing beam status, range
        int status = (i <  mt.numMeas ? (mt.measStatus[i] ? 1 : 1) : 0);
        double range = (i < mt.numMeas ? mt.ranges[i] : 0.);

        br = snprintf(op, rem, ",%d,%d,%lf",
               i,
               status,
               range);
        if(br > 0) {
            rem -= br;
            op += br;
        }

    }
    br = snprintf(op, rem, "\n");

    if(ctx->oflags & OUT_MEAS_CSV)
        printf("%s", obuf);

    if(ctx->meas_out != NULL) {
        fwrite(obuf, strlen(obuf), 1, ctx->meas_out);
        fflush(ctx->meas_out);
    }
}

// Output TRN estimate to console in CSV format

void TrnPlayer::out_est_csv(poseT &pt, poseT &mse, poseT &mle)
{
    // Format fields:
    // [  1] mse time      (epoch sec, double)
    // [  2] mse N         (m)
    // [  3] mse E         (m)
    // [  4] mse D         (m)
    // [  5] mse vx        (m/s)
    // [  6] mse vy        (m/s)
    // [  7] mse vz        (m/s)
    // [  8] mse roll      (phi) (rad)
    // [  9] mse pitch     (theta) (rad)
    // [ 10] mse rheading  (psi) (rad)
    // [ 11] nav (pt) time (epoch sec, double)
    // [ 12] nav N         (m)
    // [ 13] nav E         (m)
    // [ 14] nav D         (m)
    // [ 15] nav roll      (phi) (rad)
    // [ 16] nav pitch     (theta) (rad)
    // [ 17] nav rheading  (psi) (rad)
    // [ 18] offset N      (mse - pt) (m)
    // [ 19] offset E      (mse - pt) (m)
    // [ 20] offset D      (mse - pt) (m)
    // [ 21] covariance N  (m)
    // [ 22] covariance E  (m)
    // [ 23] covariance D  (m)
    // [ 24] covariance magnitude (m)

    char obuf[CSV_OBUF_SZ] = {0}, *op = obuf;
    int rem = CSV_OBUF_SZ;

    // covariances
    double cov[3] = {sqrt(mse.covariance[0]), sqrt(mse.covariance[2]), sqrt(mse.covariance[5])};

    // offset (estimate - navigation)
    double ofs[3] = {(mse.x - pt.x), (mse.y - pt.y), (mse.z - pt.z)};

    // output to console
    int br = snprintf(op, rem, "%0.3lf,", mse.time);
    if(br > 0) {
        rem -= br;
        op += br;
    }

    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  mse.x, mse.y, mse.z);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  mse.vx, mse.vy, mse.vz);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  mse.phi, mse.theta, mse.psi);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,", pt.time);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  pt.x, pt.y, pt.z);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,",
                  pt.phi, pt.theta, pt.psi);
    if(br > 0) {
        rem -= br;
        op += br;
    }
    br = snprintf(op, rem, "%0.3lf,%0.3lf,%0.3lf,%0.3f,%0.3f,%0.3f,%0.3f\n",
                  ofs[0], ofs[1], ofs[2],
                  cov[0], cov[1], cov[2], vnorm(cov));

    if(ctx->oflags & OUT_EST_CSV)
        printf("%s", obuf);

    if(ctx->est_out != NULL) {
        fwrite(obuf, strlen(obuf), 1, ctx->est_out);
        fflush(ctx->est_out);
    }
}

// Output entry point, format selector

void TrnPlayer::write_output(poseT &pt, measT &mt, poseT &mse, poseT &mle)
{
    // output pretty (human readble) format
    if(ctx->oflags & OUT_PRETTY) {
        out_pretty(pt, mse, mle);
    }

    // output estimate CSV
    if(ctx->oflags & (OUT_EST_CSV | OUT_EST_FILE)) {
        out_est_csv(pt, mse, mle);
    }

    // output measurement CSV
    if(ctx->oflags & (OUT_MEAS_CSV | OUT_MEAS_FILE)) {

        if(ctx->meas_out_format == IOFMT_CSV_DVL) {
            out_csv_dvl(pt, mt);
        } else if(ctx->meas_out_format == IOFMT_CSV_IDT) {
            out_csv_idt(pt, mt);
        } else if(ctx->meas_out_format == IOFMT_CSV_MB) {
            out_csv_mb(pt, mt);
        }
    }
}

