///
/// @file trnmin_app.cpp
/// @authors k. headley
/// @date 2025-09-16
///
/// Copyright 2025 Monterey Bay Aquarium Research Institute
/// see LICENSE file for terms of use and license information.
///
/// @brief Application wrapper for TrnMin TRN example
/// @details Processes (replays) a TRN log file (TerrainAid.log, MbTrn.log)
/// Demonstrates simplest use of TRN and related data structures
///     - create/configure TRN instance
///     - update TRN using poseT, measT
///     - get TRN estimate, covariances
///
/// @see TrnMin:: run, TrnMin::configure
///
/// Use -h option for help
/// Output goes to trnmin-TRN[-N] or select using -o option
///
/// Tip: make sim links as follows:
/// maps: map directory or file
/// data: directory containing TRN data (MbTrn.log, TerrainNav.log, or TerrainAid.log)
/// and config (vehicle spec, sensor spec(s))

#include "TrnPlayer.hpp"

// application entry point
int main(int argc, char **argv)
{
    // Get a TrnMin instance
    TrnPlayer *trn = new TrnPlayer();

    // configure and run TRN
    int status = trn->run(argc, argv);

    // release resources
    delete trn;

    return status;
}
