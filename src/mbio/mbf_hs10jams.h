/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hs10jams.h	12/4/00
 *	$Id$
 *
 *    Copyright (c) 2000-2014 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbf_hs10jams.h defines the data structures used by MBIO functions
 * to store Furuno HS-10 multibeam sonar data read from the
 * MBF_HS10JAMS format.
 *
 * Author:	D. W. Caress
 * Date:	December 4, 2000
 *
 * $Log: mbf_hs10jams.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/10 20:24:25  caress
 * Initial revision.
 *
 *
 */
/*
 * Notes on the MBF_HS10JAMS data format:
 *   1. The Furuno HS-10 multibeam sonar generated 45 beams
 *      of bathymetry and amplitude.
 *   2. To my knowledge, only one Furuno HS-10 multibeam sonar
 *      has been operated. It was installed on S/V Yokosuka,
 *      a JAMSTEC research vessel. The Furuno HS-10 has since been
 *      replaced by a SeaBeam 2112 multibeam sonar.
 *   3. A specification for the raw HS-10 data format was
 *      provided by JAMSTEC, and is included below. The raw format
 *      consists of 800 byte binary records in which only the
 *      lower 4 bits of each byte are used.
 *   4. The actual data files provided to WHOI seem to be simple
 *      716 byte ASCII records with time, lat, lon, heading,
 *      center beam depth, 45 depths, 45 acrosstrack distances,
 *      45 beam amplitudes. Format 171 supports the actual data
 *      received.
 *   5. The data received use 5 characters each for depth,
 *      acrosstrack, and amplitude values. Null beams have
 *      depth values of 29999 and acrosstrack values of 99999.
 *      MB-System supports beam flagging by setting flagged
 *      beams negative.
 *   6. The internal data structure supports the data included
 *      in the format 171 files, and does not yet include values
 *      listed in the raw format spec but not seen in the data
 *      provided.
 *   7. Comment records are supported as an MB-System extension
 *      where the first two bytes of the 716 byte ASCII records
 *      are "##"
 *   8. The raw data format specification is as follows:
 *
 *      ----------------------------------------------------------
 *      HS-10 MNBES Data Format - JAMSTEC
 *
 *      800 bytes/record, 10 records/block
 *
 *      Note: 4 bits from LSB is effective in each byte.
 *           zB. 30 30 35 39 ---> 0 0 5 9 (HEX) = 89 (DEC)
 *               30 30 32 3D ---> 0 0 2 D (HEX) = 45 (DEC)
 *      The HS-10 processor calculates the water depth by use of
 *      average sound velocity and by correcting the difference
 *      between the true angle of the sound path (obtained by the
 *      true sound velocity profile) and the nominal angle of each
 *      beam (every 2 degrees). The horizontal distance of the n-th
 *      beam is
 *              Distance(n) = Depth(n) * tan[T(n)],
 *      where T(n) is the nominal angle of the n-th beam:
 *              ( T(n) = 2 * (n-23) degrees, n=1,45 ).
 *
 *      No.  Bytes  Data
 *       1.    4    Year
 *       2.    4    Month
 *       3.    4    Day
 *       4.    4    Hour
 *       5.    4    Minute
 *       6.    4    Second
 *       7.    8    Latitude in 1/10000 minute
 *       8.    8    Longitude in 1/10000 minute
 *       9.    8    X in 1/10 metre
 *      10.    8    Y in 1/10 metre
 *      11.    4    Ship's speed in 1/10 knot
 *      12.    4    Ship's heading in 1/10 degree
 *      13. 4x45    45 Water depths in metre
 *      14. 4x45    45 Intensity of reflection in dB
 *      15.    4    Selection of navigation
 *                    0:HYB, 1:ANS, 2:MANU(L/L) 3:MANU(X/Y)
 *      16.    4    Surface sound velocity in 1/10 m/sec
 *      17.    8    Initial latitude in 1/10000 minute
 *      18.    8    Initial longitude in 1/10000 minute
 *      19.    8    Initial X in 1/10 metre
 *      20.    8    Initial Y in 1/10 metre
 *      21.    4    Manual bearing in 1/10 degree
 *      22.    4    Manual ship's speed in 1/10 knot
 *      23.    4    Ship's draft in 1/10 metre
 *      24.    4    Offset X in 1/10 metre
 *      25.    4    Offset Y in 1/10 metre
 *      26.    4    Selection of sound velocity
 *                    0:no correction, 1:manual input, 2:calculation correction
 *      27.    4    Average sound velocity in 1/10 m/sec
 *      28.    4    Input selection of water temperature
 *                    0:AUTO, 1:MANUAL
 *      29.    4    Water temperature in 1/10 degree
 *      30.    4    Tide level in 1/10 metre
 *      31. 4x10    10 Depth of layer in metre
 *      32. 4x10    10 Temperature of layer in 1/10 degree
 *      33. 4x10    10 Salinity in 1/10 per mille
 *      34. 4x10    10 Sound velocity in 1/10 m/sec
 *      35.    4    Transmitted pulse width
 *                    0:1m, 1:2m, 2:4m, 3:8m
 *      36.    4    Level of transmission [1-16]
 *                    1:Off, 16:Max, -2dB in each step
 *      37.    4    Selection of period of tranmission
 *                    0:Auto, 1:Manual
 *      38:    4    Period of tranmission in second
 *      39:    4    Pre-amp ATT
 *                    0:OFF, 1:ON
 *      40:    4    Receiving gain [1-16]
 *                    1:Off, 16:Max, -2dB in each step
 *      41.    4    TVG [1-4]
 *      42.    4    AVG [1-4]
 *      43.    4    Threshold [1-16]
 *      44.    4    Gate width (R/L) [1-4]
 *      45.    4    Gate width (F/B) [1-4]
 *      46.    4    Selection of beam pattern [1-3]
 *      47.    4    Interferance removal
 *                    0:OFF, 1:ON
 *      48.    4    KP shift [1-32]
 *      49.    4    Sonar mode [0]
 *      50.         not used
 *      ----------------------------------------------------------
 *
 */
 *
 */

#define	MBF_HS10JAMS_MAXLINE	716
