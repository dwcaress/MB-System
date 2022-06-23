/****************************************************************************/
/* Copyright (c) 2022 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : CoNavLog.h                                                    */
/* Author   : henthorn                                                      */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 01/27/2022                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _CoNavLog_H
#define _CoNavLog_H

#include <string>
#include <stdint.h>
#include <armadillo>

#include "DataLogWriter.h"
#include "FloatData.h"
#include "DoubleData.h"
#include "IntegerData.h"
#include "ShortData.h"
#include "MRFilterLog.h"

#define CoNavLogName "CoNav"

class CoNavLog : public DataLogWriter
{

public:

    struct CoNavState
    {
      double timestamp;
      double bestNorthing;
      double bestEasting;
    };

    ///////////////////////////////////////////////////////////////////
    // Constructor
    CoNavLog(const std::string& logname,
             DataLog::FileFormat fileFormat=BinaryFormat);

    ~CoNavLog();

    int setCoNavState(const struct CoNavState& state);
    int setCoNavPbest(const arma::mat& pb);
    int setCoNavMotion(const struct MRFilterLog::VehicleNavData& nd);
    int setCoNavMeas(const struct MRFilterLog::CoopVehicleNavData& cnd);

    virtual void setFields();

protected:

    void init(void);

private:

    IntegerData *_event;

    // CoNav state fields
    DoubleData *_timestamp;
    DoubleData *_northing;
    DoubleData *_easting;

    // Ego vehicle motion update fields
    DoubleData *_motionTime;
    DoubleData *_motionN;
    DoubleData *_motionE;
    DoubleData *_motionZ;

    // CoNav measurement update fields
    IntegerData *_id;
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

    // Filter PBest matrix fields from CoNav
    DoubleData *_pb11;
    DoubleData *_pb12;
    DoubleData *_pb21;
    DoubleData *_pb22;

};

#endif
