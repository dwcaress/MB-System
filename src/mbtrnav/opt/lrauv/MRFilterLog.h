/****************************************************************************/
/* Copyright (c) 2022 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : MRFilterLog.h                                                 */
/* Author   : henthorn                                                      */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 01/10/2022                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _MRFilterLog_H
#define _MRFilterLog_H

#include <string>
#include <stdint.h>
#include <armadillo>

#include "DataLogWriter.h"
#include "FloatData.h"
#include "DoubleData.h"
#include "IntegerData.h"
#include "ShortData.h"
#include "conav.h"

#define MRFilterLogName "MRFilter"
#define ORIGIN_THRESHOLD_FAIL 2
#define ORIGIN_MEASUREMENT    1
#define ORIGIN_MOTION         0

class MRFilterLog : public DataLogWriter
{

public:

    ///////////////////////////////////////////////////////////////////
    // Constructor
    MRFilterLog(int id, const std::string& logname,
                DataLog::FileFormat fileFormat=BinaryFormat);

    ~MRFilterLog();

    int setMRFilterState(const struct CoNav::MRFilterState& state);
    int setMRFilterP(int origin, const arma::mat& p);
    int setMRFilterPbest(const arma::mat& pb);
    int setMRFilterQbar(const arma::mat& qb);
    int setMRFilterDeltaz(const arma::mat& deltaz);
    int setMRFilterRij(const arma::mat& rij);
    int setMRFilterHi(const arma::mat& hi);
    int setMRFilterHj(const arma::mat& hj);
    int setMRFilterPj(const arma::mat& pj);
    int setMRFilterMeasPosition(double measN, double measE);
    int setMRFilterWopt(double wopt);
    int setMRFilterMotion(const struct CoNav::ERNavInput& nd);
    int setMRFilterMeas(const struct CoNav::MRDATInput& cnd);

    virtual void setFields();

protected:

    void init(void);

private:
    int _id;

    DoubleData *_missionTime;

    // Filter state fields
    DoubleData *_northing;
    DoubleData *_easting;
    DoubleData *_deltaN;
    DoubleData *_deltaE;
    DoubleData *_distance;

    // Ego vehicle motion update fields
    DoubleData *_motionTime;
    DoubleData *_motionN;
    DoubleData *_motionE;
    DoubleData *_motionZ;

    // Filter vehicle conav update fields
    DoubleData *_measTime;
    DoubleData *_measTrnN;
    DoubleData *_measTrnE;
    DoubleData *_measTrnZ;
    DoubleData *_measTrnNVar;
    DoubleData *_measTrnEVar;
    DoubleData *_measTrnZVar;
    DoubleData *_measRange;
    DoubleData *_measBear;
    DoubleData *_measRangeVar;
    DoubleData *_measBearVar;

    // Filter P matrix fields
    IntegerData *_origin;     // 0 => motion update, 1 => measure update
    DoubleData *_p11;
    DoubleData *_p12;
    DoubleData *_p21;
    DoubleData *_p22;

    // Filter PBest matrix fields from CoNav
    DoubleData *_pb11;
    DoubleData *_pb12;
    DoubleData *_pb21;
    DoubleData *_pb22;

    // Filter QBar matrix fields
    DoubleData *_qb11;
    DoubleData *_qb12;
    DoubleData *_qb21;
    DoubleData *_qb22;

    // Filter Deltaz matrix fields
    DoubleData *_dz11;
    DoubleData *_dz21;

    // Filter Rij matrix fields
    DoubleData *_rij11;
    DoubleData *_rij12;
    DoubleData *_rij21;
    DoubleData *_rij22;

    // Filter Hi matrix fields
    DoubleData *_hi11;
    DoubleData *_hi12;
    DoubleData *_hi21;
    DoubleData *_hi22;

    // Filter Hj matrix fields
    DoubleData *_hj11;
    DoubleData *_hj12;
    DoubleData *_hj21;
    DoubleData *_hj22;

    // Filter Pj matrix fields
    DoubleData *_pj11;
    DoubleData *_pj12;
    DoubleData *_pj21;
    DoubleData *_pj22;

    // Measure position (interpolated position at the time of a measurement)
    DoubleData *_measN;
    DoubleData *_measE;

    DoubleData *_wopt;
};

#endif
