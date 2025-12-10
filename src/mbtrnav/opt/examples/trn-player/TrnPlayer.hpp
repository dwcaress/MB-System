///
/// @file TrnPlayer.hpp
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
/// @see TrnPlayer:: run, TrnPlayer::configure
///
/// Use -h option for help
/// Output goes to trn-player-TRN[-N] or select using -o option
///
/// Tip: make sim links as follows:
/// maps: map directory or file
/// data: directory containing TRN data (MbTrn.log, TerrainNav.log, or TerrainAid.log)
/// and config (vehicle spec, sensor spec(s))

#ifndef _TRN_PLAYER_HPP_
#define _TRN_PLAYER_HPP_

#include "structDefs.h"
#include "TerrainNav.h"
#include "TrnPlayerCtx.hpp"

#define CSV_OBUF_SZ 2048

class TrnPlayer {
public:
    TrnPlayer();
    ~TrnPlayer();

    // Parse options and configure TRN
    // May also call run directly (w/ arguments)
    int configure(int argc, char **argv);

    // Configure and run TRN
    // Must pass arguments if configure
    // not previously called
    int run(int argc=0, char **argv=NULL);

    // Print configuration summary
    void show();

private:
    // Record readers/parsers
    int getMbTrnRecord(poseT *pt, measT *mt);
    int getTNavRecord(poseT *pt, measT *mt);
    int getTrnAidRecord(poseT *pt, measT *mt);
    int parseCSV(char *src, char ***dest, int *r_nfields);
    int getCSVRecordDVL(poseT *pt, measT *mt);
    int getCSVRecordIDT(poseT *pt, measT *mt);
    int getCSVRecordMB(poseT *pt, measT *mt);
    int getCSVRecord(poseT *pt, measT *mt);
    int getNextRecord( poseT *pt, measT *mt);

    // helper functions
    void init_io();
    void copy_config();
    void show_summary();
    void getSensorGeometry();
    double vnorm(double v[]);
    int decimate(double timestamp);
    static int reset_pt(poseT **pt, unsigned int len);
    static int reset_mt(measT **mt, unsigned int len);

    // Output formatters
    void out_pretty(poseT &pt, poseT &mse, poseT &mle);
    void out_csv_dvl(poseT &pt, measT &mt);
    void out_csv_idt(poseT &pt, measT &mt);
    void out_csv_mb(poseT &pt, measT &mt);
    void out_est_csv(poseT &pt, poseT &mse, poseT &mle);
    void idt_to_mb(measT &mt);
    void dvl_to_mb(measT &mt);
    void write_output(poseT &pt, measT &mt, poseT &mse, poseT &mle);

    // Context: TRN configuration and TrnPlayer state variables
    TrnPlayerCtx *ctx;

    // TRN instance
    TerrainNav *trn;

    // status variables
    int status = 0;
    int err_n = 0;
    int rec_n = 0;
    int val_n = 0;
    int est_n = 0;
    int dec_n = 0;

};
#endif
