/********************************************************************
 *
 * Module Name : GSF_COMPRESS
 *
 * Author/Date : Conrad Curry / May 2017
 *
 * Description : This source file contains an implementation of a
 *   compression scheme using a combination of intra-channel decorrelation
 *   and entropy coding of the residual signal.
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the type short is 16 bits, and that the type int is 32 bits.
 *
 * Change Descriptions :
 * who          when      what
 * ---          ----      ----
 * cwc          05-04-17  Initial release.
 *
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 * There is no charge to use the library, and it may be accessed at:
 * https://www.leidos.com/products/ocean-marine.
 * This library may be redistributed and/or modified under the terms of the GNU Lesser General Public License
 * version 2.1, as published by the Free Software Foundation.  A copy of the LGPL 2.1 license is included
 * with the GSF distribution and is available at: http://opensource.org/licenses/LGPL-2.1.
 *
 * Leidos, Inc. configuration manages GSF, and provides GSF releases. Users are strongly encouraged to
 * communicate change requests and change proposals to Leidos, Inc.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ********************************************************************/

/* Standard C Library Includes */
#include <stdlib.h>
#include <string.h>

/* Get network byte swap functions */
#if !defined WIN32 && !defined WIN64
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

/* GSF library interface description */
#include "gsf.h"

/* Compression Format Version Number */
#define GSF_COMPRESSION_VERSION  1

#define LPC_ABS(x) ((x) >= 0 ? (x) : -(x))


/* Global external data defined in this module */
extern int gsfError;       /* Defined in gsf.c */

/* TODO: Remove this from here and in gsf_dec.c and move into the filetable structure.
         The decode routines should be modified to return the (re)allocated array size. */
extern short arraySize[GSF_MAX_OPEN_FILES][GSF_MAX_PING_ARRAY_SUBRECORDS];


/* Function prototypes for this file */
static size_t CompressDoubleArray (gsfuLong *out, int *p, const double *in, const size_t n, const double offset, const double multiplier);
static size_t UncompressDoubleArray (double *out, const gsfuLong *in, const size_t m, const size_t n, const int p, const double offset, const double multiplier);
static void ConvertArrayToInt (gsfsLong *out, const double *in, const size_t n, const double offset, const double multiplier);
static void ConvertArrayToDouble (double *out, const gsfsLong *in, const size_t n, const double offset, const double multiplier);
static size_t CompressIntArray (gsfuLong *out, int *p, const gsfsLong *in, const size_t n);
static size_t UncompressIntArray (gsfsLong *out, const gsfuLong *in, const size_t m, const size_t n, const int p);
static int LpcEncode (gsfsLong *out, const gsfsLong *in, const size_t n);
static void LpcDecode (gsfsLong *out, const gsfsLong *in, const size_t n, const int p);
static int LpcOrder (const gsfsLong *x, const size_t n);
static void SignEncode (gsfuLong *out, const gsfsLong *in, const size_t n);
static void SignDecode (gsfsLong *out, const gsfuLong *in, const size_t n);
static size_t RleEncode (gsfuLong *out, const gsfuLong *in, const size_t n);
static size_t RleDecode (gsfuLong *out, const gsfuLong *in, const size_t n, const size_t m);
static size_t S16Encode (gsfuLong *out, const gsfuLong *in, const size_t n);
static size_t S16Decode (gsfuLong *out, const gsfuLong *in, const size_t m);
static size_t S16Pack (gsfuLong *out, const gsfuLong *in, const size_t n);
static size_t S16Unpack (gsfuLong *out, const gsfuLong in);


/* Local Variables */
static const gsfuLong s16_bits[16][14] =
{
    {  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
    {  3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0 },
    {  3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0 },
    {  4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0 },
    {  4, 4, 4, 4, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0 },
    {  3, 3, 3, 3, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0 },
    {  4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0 },
    {  5, 5, 5, 5, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
    {  4, 4, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
    {  6, 6, 6, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    {  5, 5, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    {  7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 10, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 14,14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 15,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const size_t s16_cnt[16] = { 14, 12, 10, 9, 8, 8, 7, 6, 6, 5, 5, 4, 3, 2, 2, 1 };


/********************************************************************
 *
 * Function Name : EncodeCompressedUnsignedShortArray
 *
 * Description : This function encodes an unsigned short array from internal
 *   form to an external compressed byte stream form.
 *
 * Inputs :
 *   sptr = A pointer to the unsigned char buffer to write to.
 *   array = A pointer to the array of unsigned shorts from which to read.
 *   num_beams = The integer number of beams (number of unsigned shorts in the array).
 *   subrecordId = The array subrecord id.
 *
 * Returns :
 *   This function returns the number of bytes encoded if successful, or
 *   -1 if an error occurred.
 *
 * Error Conditons :
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_COMPRESSION_FAILED
 *
 ********************************************************************/

int
EncodeCompressedUnsignedShortArray (unsigned char *sptr, const unsigned short *array,
    int num_beams, int subrecordID)
{

    unsigned char *p = sptr;
    gsfsLong *in;
    gsfuLong *out;
    gsfuLong ltemp;
    size_t i, m, n;
    int order;


    if (num_beams <= 0)
    {
        gsfError = GSF_INVALID_NUM_BEAMS;
        return (-1);
    }

    n = (size_t) num_beams;

    /* Allocate a temporary output array. */
    out = (gsfuLong *) calloc (n, sizeof (gsfuLong));
    if (!out)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    /* Convert unsigned short array to unsigned int array. */
    for (i = 0; i < n; i++)
    {
        out[i] = array[i];
    }

    /* Alias the input and output arrays.  Compression will be done 'in-place'. */
    in = (gsfsLong *) out;

    m = CompressIntArray (out, &order, in, n);

    if (m == 0)
    {
        gsfError = GSF_COMPRESSION_FAILED;
        return (-1);
    }

    /* Subrecord identifier has array id in the first byte and size in the remaining three bytes. */
    ltemp = subrecordID << 24;
    ltemp |= 1 + m * sizeof (gsfuLong);
    ltemp = htonl (ltemp);
    memcpy (p, &ltemp, 4);
    p += 4;

    /* Encode the order of the best-fit LP model and the format version number. */
    *p = (unsigned char) (order | (GSF_COMPRESSION_VERSION << 5));
    p++;

    for (i = 0; i < m; i++)
    {
        ltemp = htonl (out[i]);
        memcpy (p, &ltemp, 4);
        p += 4;
    }

    free (out);

    return (p - sptr);
}


/********************************************************************
 *
 * Function Name : DecodeCompressedUnsignedShortArray
 *
 * Description : This function decodes an array of beam data from
 *   external compressed byte stream form to internal engineering units
 *   form.  This function allocates the memory for the array if it does
 *   not exist.  This function also reallocates the memory for the
 *   array when the number of beams change.
 *
 * Inputs :
 *   array = The address of a pointer to an unsigned short where the
 *     array of data will be stored.
 *   sptr = A pointer to the unsigned char buffer containing the byte
 *     stream to read from.
 *   num_beams = An integer containing the number of beams which is
 *     used to dimension the array.
 *   compressed_size = The length of the compressed array in bytes.
 *   subrecordID = The integer id for the subrecord, which is used as
 *     the index into the scale factors structure.
 *   handle = The integer handle for the data file being read, which
 *     is used to store the current number of beams.
 *
 * Returns :
 *   This function returns the number of bytes decoded if successful, or
 *   -1 if an error occurred.
 *
 * Error Conditons :
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_COMPRESSION_UNSUPPORTED
 *   GSF_COMPRESSION_FAILED
 *
 ********************************************************************/

int
DecodeCompressedUnsignedShortArray (unsigned short **array, const unsigned char *sptr, int num_beams,
    int compressed_size, int subrecordID, int handle)
{

    const unsigned char *p = sptr;
    gsfsLong *out;
    gsfuLong *in;
    gsfuLong ltemp;
    size_t i, m, n;
    int version;
    int order;

    if (compressed_size - 1 <= 0)
    {
    	gsfError = GSF_COMPRESSION_FAILED;
    	return (-1);
    }
    else if (num_beams <= 0)
    {
        gsfError = GSF_INVALID_NUM_BEAMS;
        return (-1);
    }

    /* Allocate memory for the array if none has been allocated yet. */
    if (*array == (unsigned short *) NULL)
    {
        *array = (unsigned short *) calloc (num_beams, sizeof (unsigned short));

        if (*array == (unsigned short *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][subrecordID - 1] = num_beams;
    }

    /* Make sure the memory allocated for the array is sufficient, some
       systems have a dynamic number of beams depending on depth. */
    if (num_beams > arraySize[handle - 1][subrecordID - 1])
    {
        unsigned short * ustemp = (unsigned short *) realloc ((void *) *array, num_beams * sizeof (unsigned short));
        if (ustemp == (unsigned short *) NULL)
        {
            free(*array);
            *array = (unsigned short *)NULL;
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        *array = ustemp;
        memset (*array, 0, num_beams * sizeof (unsigned short));

        arraySize[handle - 1][subrecordID - 1] = num_beams;
    }

    /* Extract the order of the best-fit LP model to the compressed data and the format version number. */
    order = (int) (*p & 0x1F);
    version = (int) (*p >> 5);
    p++;

    /* Check that the file format version is not more recent than the library. */
    if (version > GSF_COMPRESSION_VERSION)
    {
        gsfError = GSF_COMPRESSION_UNSUPPORTED;
        return (-1);
    }

    m = (compressed_size - 1) / sizeof (gsfuLong);

    /* Allocate a temporary input array. */
    in = (gsfuLong *) calloc (m, sizeof (gsfuLong));
    if (!in)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    for (i = 0; i < m; i++)
    {
        memcpy (&ltemp, p, 4);
        in[i] = (gsfuLong) ntohl (ltemp);
        p += 4;
    }

    out = (gsfsLong *) calloc (num_beams, sizeof (gsfsLong));
    if (!out)
    {
        free (in);
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    n = UncompressIntArray (out, in, m, num_beams, order);

    if (n != (size_t) num_beams)
    {
        free (out);
        free (in);
        gsfError = GSF_COMPRESSION_FAILED;
        return (-1);
    }

    free (in);

    for (i = 0; i < n; i++)
    {
        (*array)[i] = (unsigned short) out[i];
    }

    free (out);

    return compressed_size;
}


/********************************************************************
 *
 * Function Name : EncodeCompressedArray
 *
 * Description : This function encodes a double array from internal
 *   form to an external compressed byte stream form.
 *
 * Inputs :
 *   sptr = A pointer to the unsigned char buffer to write to.
 *   array = A pointer to the array of doubles from which to read.
 *   num_beams = The integer number of beams (number of doubles in the array).
 *   sf = A pointer to the GSF scale factors used to scale the data.
 *   subrecordId = The array subrecord id.
 *
 * Returns :
 *   This function returns the number of bytes encoded if successful, or
 *   -1 if an error occurred.
 *
 * Error Conditons :
 *   GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_COMPRESSION_FAILED
 *
 ********************************************************************/

int
EncodeCompressedArray (unsigned char *sptr, const double *array, int num_beams,
    const gsfScaleFactors *sf, int subrecordID)
{

    unsigned char *p = sptr;
    gsfuLong *out;
    gsfuLong ltemp;
    size_t i, m, n;
    int order;


    /* Make sure we have a valid multiplier for this array. */
    if (sf->scaleTable[subrecordID - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    if (num_beams <= 0)
    {
        gsfError = GSF_INVALID_NUM_BEAMS;
        return (-1);
    }

    n = (size_t) num_beams;

    /* Allocate a temporary output array. */
    out = (gsfuLong *) calloc (n, sizeof (gsfuLong));
    if (!out)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    m = CompressDoubleArray (out, &order, array, n, sf->scaleTable[subrecordID - 1].offset, sf->scaleTable[subrecordID - 1].multiplier);

    if (m == 0)
    {
        free (out);
        gsfError = GSF_COMPRESSION_FAILED;
        return (-1);
    }

    /* Subrecord identifier has array id in first byte, and size in the remaining three bytes. */
    ltemp = subrecordID << 24;
    ltemp |= 1 + m * sizeof (gsfuLong);
    ltemp = htonl (ltemp);
    memcpy (p, &ltemp, 4);
    p += 4;

    /* Encode the order of the best-fit LP model and format version number. */
    *p = (unsigned char) (order | (GSF_COMPRESSION_VERSION << 5));
    p++;

    for (i = 0; i < m; i++)
    {
        ltemp = htonl (out[i]);
        memcpy (p, &ltemp, 4);
        p += 4;
    }

    free (out);

    return (p - sptr);
}


/********************************************************************
 *
 * Function Name : DecodeCompressedArray
 *
 * Description : This function decodes an array of beam data from
 *   external compressed byte stream form to internal engineering units
 *   form.  This function allocates the memory for the array if it does
 *   not exist.  This function also reallocates the memory for the
 *   array when the number of beams change.
 *
 * Inputs :
 *   array = The address of a pointer to a double where the array of
 *     data will be stored.
 *   sptr = A pointer to the unsigned char buffer containing the byte
 *     stream to read from.
 *   num_beams = An integer containing the number of beams which is
 *     used to dimension the array.
 *   compressed_size = The length of the compressed array in bytes.
 *   sf = A pointer to the scale factors structure containing the data
 *     scaling information.
 *   subrecordID = The integer id for the subrecord, which is used as
 *     the index into the scale factors structure.
 *   handle = The integer handle for the data file being read, which
 *     is used to store the current number of beams.
 *
 * Returns :
 *   This function returns the number of bytes decoded if successful, or
 *   -1 if an error occurred.
 *
 * Error Conditons :
 *   GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_COMPRESSION_UNSUPPORTED
 *   GSF_COMPRESSION_FAILED
 *
 ********************************************************************/

int
DecodeCompressedArray (double **array, const unsigned char *sptr, int num_beams,
    int compressed_size, const gsfScaleFactors *sf, int subrecordID, int handle)
{

    const unsigned char *p = sptr;
    gsfuLong ltemp;
	size_t i, m, n;
	int version;
	int order;


    /* Make sure we have a valid multiplier for this array. */
    if (sf->scaleTable[subrecordID - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    if (num_beams <= 0)
    {
        gsfError = GSF_INVALID_NUM_BEAMS;
        return (-1);
    }

    /* Allocate memory for the array if none has been allocated yet. */
    if (*array == (double *) NULL)
    {
        *array = (double *) calloc (num_beams, sizeof (double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][subrecordID - 1] = num_beams;
    }

    /* Make sure the memory allocated for the array is sufficient, some
       systems have a dynamic number of beams depending on depth. */
    if (num_beams > arraySize[handle - 1][subrecordID - 1])
    {
        double * dtemp = (double *) realloc ((void *) *array, num_beams * sizeof (double));

        if (dtemp == (double *) NULL)
        {
            free(*array);
            *array = (double *)NULL;
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        *array = dtemp;
        memset (*array, 0, num_beams * sizeof (double));

        arraySize[handle - 1][subrecordID - 1] = num_beams;
    }

    /* Extract the order of the best-fit LP model to the compressed data and the format version number. */
    order = (int) (*p & 0x1F);
    version = (int) (*p >> 5);
    p++;

    /* Check that the file format version is not more recent than the library. */
    if (version > GSF_COMPRESSION_VERSION)
    {
        gsfError = GSF_COMPRESSION_UNSUPPORTED;
        return (-1);
    }

    m = (compressed_size - 1) / sizeof (gsfuLong);

    /* Allocate a temporary input array. */
    gsfuLong *in = (gsfuLong *) calloc (m, sizeof (gsfuLong));
    if (!in)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    for (i = 0; i < m; i++)
    {
        memcpy (&ltemp, p, 4);
        in[i] = (gsfuLong) ntohl (ltemp);
        p += 4;
    }

	n = UncompressDoubleArray (*array, in, m, num_beams, order, sf->scaleTable[subrecordID - 1].offset, sf->scaleTable[subrecordID - 1].multiplier);

	if (n != (size_t) num_beams)
	{
	    free (in);
	    gsfError = GSF_COMPRESSION_FAILED;
	    return (-1);
	}

	free (in);

    return compressed_size;
}


/********************************************************************
 *
 * Function Name : CompressDoubleArray
 *
 * Description : This function compresses an array of floating-point
 *   numbers in the array IN of length N.  Each floating-point number
 *   is converted to a fixed-point integer after applying a MULTIPLIER
 *   and OFFSET.  The output array OUT can alias the input array IN.
 *
 * Inputs :
 *   out        = A pointer to an array of length at least N (can alias array IN).
 *   in         = An array of the input values to be compressed.
 *   n          = Length of the input array IN.
 *   offset     = Offset applied to the input values before conversion to integer.
 *   multiplier = Reciprocal of the precision of the desired output.
 *
 * Outputs :
 *   out = An array of the compressed output values.
 *   p   = Polynomial order of the best-fit LP model of the input data.
 *
 * Returns :
 *   Length of the output array OUT, or
 *   0 if a value is too large to be encoded.
 *
 ********************************************************************/

static size_t
CompressDoubleArray (gsfuLong *out, int *p, const double *in, const size_t n,
    const double offset, const double multiplier)
{

    gsfsLong *out_s32 = (gsfsLong *) out;


    /* Convert an array of floating point numbers to integers with given precision. */
    ConvertArrayToInt (out_s32, in, n, offset, multiplier);

    /* Compress the resulting integers. */
    return CompressIntArray (out, p, out_s32, n);
}


/********************************************************************
 *
 * Function Name : UncompressDoubleArray
 *
 * Description : This function uncompresses an array of packed integers
 *   in the array IN of length M.  Each uncompressed integer is then
 *   converted to a floating-point number after applying a MULTIPLIER
 *   and OFFSET.
 *
 * Inputs :
 *   out = Pointer to an array large enough to hold the uncompressed output.
 *   in  = Pointer to an array of the input numbers to be uncompressed.
 *   m   = Length of the compressed array IN.
 *   n   = Length of the uncompressed array IN.
 *   p   = Linear-Predictive Coding model order used in compression.
 *   offset = Offset applied to the input values before conversion to integer.
 *   multiplier = Reciprocal of the precision of the desired output.
 *
 * Outputs :
 *   out = Array of uncompressed numbers.
 *
 * Returns :
 *   Length of the uncompressed output array OUT, or
 *   0 if the format is invalid.
 *
 ********************************************************************/

static size_t
UncompressDoubleArray (double *out, const gsfuLong *in, const size_t m, const size_t n, const int p,
    const double offset, const double multiplier)
{

    /* Alias the signed array with the output array. */
    gsfsLong *out_s32 = (gsfsLong *) out;
    size_t nn;


    /* Uncompress the input array into an array aliased on the output array. */
    nn = UncompressIntArray (out_s32, in, m, n, p);
    if (nn != n) return (0);

    /* Convert the signed integers back to floating point with scale and offset. */
    ConvertArrayToDouble (out, out_s32, n, offset, multiplier);

    return (n);
}


/********************************************************************
 *
 * Function Name : ConvertArrayToInt
 *
 * Description : Convert each element of the input array IN of length N from a
 *   floating-point number to a fixed-point (integer) number by applying a
 *   MULTIPLIER and OFFSET value.  The multiplier should be chosen as the
 *   reciprocal of the precision of the desired output.  The result is stored
 *   in the array OUT, which should be at least of size N elements.  The output
 *   array OUT can alias the input array IN.
 *
 * Inputs :
 *   in = Input array of integers to convert of length N.
 *   n  = Length of the input array IN.
 *   offset = Offset applied to the input values before conversion to integer.
 *   multiplier = Reciprocal of the precision of the desired output.
 *
 * Outputs :
 *   out = Array of converted numbers.
 *
 * Returns : None.
 *
 ********************************************************************/

static void
ConvertArrayToInt (gsfsLong *out, const double *in, const size_t n,
    const double offset, const double multiplier)
{

    size_t i;


    for (i = 0; i < n; i++)
    {

        /* Apply offset and multiplier. */
        double d = (in[i] + offset) * multiplier;

        /* Add 0.5 for rounding to nearest. */
        if (d >= 0.0)
        {
            d = d + 0.501;
        }
        else
        {
            d = d - 0.501;
        }

        /* Check if the result will be in the correct range on truncation. */
        if (d < -2147483648.0)
        {
            out[i] = -2147483647 - 1;
        }
        else if (d > 2147483647.0)
        {
            out[i] = 2147483647;
        }
        else
        {
            out[i] = (gsfsLong) d;
        }
    }
}


/********************************************************************
 *
 * Function Name : ConvertArrayToDouble
 *
 * Description : Convert each element of the input array IN of length N to a
 *   floating point number.  The output array OUT can alias the input array IN.
 *
 * Inputs :
 *   out        = Pointer to an array of length at least N (can alias array IN).
 *   in         = Input array of integers to convert of length N.
 *   n          = Length of the input array IN.
 *   offset     = Offset applied to the input values before conversion to integer.
 *   multiplier = Reciprocal of the precision of the desired output.
 *
 * Outputs :
 *   out = Array of converted numbers.
 *
 * Returns : None.
 *
 ********************************************************************/

static void
ConvertArrayToDouble (double *out, const gsfsLong *in, const size_t n,
    const double offset, const double multiplier)
{

    size_t i;


    if ((double *) in == out)
    {
        /* If the output array aliases the input array, then convert the array
           in reverse order.  The indices are chosen here to avoid underflow
           of the unsigned index i. */
        for (i = n; i > 0; i--)
        {
            out[i-1] = in[i-1] / multiplier - offset;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            out[i] = in[i] / multiplier - offset;
        }
    }
}


/********************************************************************
 *
 * Function Name : CompressIntArray
 *
 * Description : This function compresses an array of signed integers
 *   in the array IN of length N.  The result is stored in the array
 *   OUT, which should be at least of size N elements.  The output
 *   array OUT can alias the input array IN.  The model order of the
 *   linear predictive coding is returned in P.
 *
 * Inputs :
 *   out = Pointer to an array of length at least N (can alias array IN).
 *   in  = Input array of integers to compress of length N.
 *   n   = Length of the input array IN.
 *
 * Outputs :
 *   p   = Polynomial order of the best-fit LP model of the input data.
 *   out = Array of compressed integers.
 *
 * Returns:
 *   Length of the output array OUT, or
 *   0 if a value is too large to be encoded.
 *
 ********************************************************************/

static size_t
CompressIntArray (gsfuLong *out, int *p, const gsfsLong *in, const size_t n)
{

    /* Alias the signed array with the unsigned out array so that
       we can perform compression in-place. */
    gsfsLong *out_s32 = (gsfsLong *) out;
	size_t m, nn;
	size_t order;


    /* Apply linear-predictive coding for intra-channel decorrelation. */
    order = LpcEncode (out_s32, in, n);

    /* Return the order of the LP model used. */
    *p = order;

    /* Encode the first 'order' number of values directly, then encode the remaining array. */
    if (n <= order) return n;
    out = out + order;
    out_s32 = out_s32 + order;
    nn = n - order;

    /* Encode the sign bit and convert to unsigned integers. */
    SignEncode (out, out_s32, nn);

    /* Run length encode the array. */
    m = RleEncode (out, out, nn);

    /* Compress the array using the Simple-16 compression method. */
    m = S16Encode (out, out, m);
    if (m == 0) return (0);

    return (m + order);
}


/********************************************************************
 *
 * Function Name : UncompressIntArray
 *
 * Description : This function uncompresses the integer data in the
 *   array IN.  The result is stored in the array OUT, which should be
 *   at least of length N.  Output can't be input.
 *
 * Inputs :
 *   out = Pointer to an array large enough to hold the uncompressed output.
 *   in  = Pointer to an array of the input numbers to be uncompressed.
 *   m   = Length of the compressed array IN.
 *   n   = Length of the uncompressed array IN.
 *   p   = Linear-Predictive Coding model order used in compression.
 *
 * Outputs :
 *   out = Array of uncompressed numbers.
 *
 * Returns :
 *   Length of the uncompressed output array OUT, or
 *   0 if the encoded format is invalid.
 *
 ********************************************************************/

static size_t
UncompressIntArray (gsfsLong *out, const gsfuLong *in, const size_t m,
    const size_t n, const int p)
{

    /* Alias the unsigned array with the signed out array. */
    gsfuLong *out_u32 = (gsfuLong *) out;
    size_t nn;
    int i;


    /* Copy the first p values directly to the output. */
    for (i = 0; i < p; i++)
    {
        out_u32[i] = in[i];
    }

    out_u32 = out_u32 + p;
    in = in + p;

    /* Uncompress the array from the Simple-16 compression method. */
    nn = S16Decode (out_u32, in, m - p);

    /* Decode the run length encoding. */
    nn = RleDecode (out_u32, out_u32, nn, n - p);
    if (nn != n - p) return (0);

    /* Decode the sign bit and convert to signed integers. */
    SignDecode (out + p, out_u32, n - p);

    /* Decode the Linear-Predictive Coding. */
    LpcDecode (out, out, n, p);

    return (n);
}


/********************************************************************
 *
 * Function Name : LpcEncode
 *
 * Description : Encode the input data IN of length N using a linear-predictive
 *   model.  The models used are polynomials of order p=0,1,2,3.  The result is
 *   stored in the array OUT, which should be at least of length N.  Input can
 *   be output.
 *
 * Inputs :
 *   out = Pointer to output array of length at least N (can alias array IN).
 *   in  = Input array of length N.
 *   n   = Length of input/output arrays.
 *
 * Returns :
 *   The order of the linear-predictive model used to encode the data.
 *
 ********************************************************************/

static int
LpcEncode (gsfsLong *out, const gsfsLong *in, const size_t n)
{

    gsfsLong last0, last1, last2;
    gsfsLong diff0, diff1, diff2;
    const gsfsLong *x = in;
    gsfsLong *e = out;
    size_t i;
    int p;


    /* Determine the model order that best fits the data. */
    p = LpcOrder (in, n);

    /* Calculate the residual signal: e_p[i] = x[i] - x_p[i]. */
    switch (p)
    {
        case 0 : if (e != x)
                 {
                     for (i = 0; i < n; i++)
                     {
                         e[i] = x[i];
                     }
                 }
                 break;

        case 1 : last0 = 0;
                 for (i = 0; i < n; i++)
                 {
                     diff0 = x[i];
                     e[i]  = diff0 - last0;
                     last0 = diff0;
                 }
                 break;

        case 2 : last0 = 0;
                 last1 = 0;
                 for (i = 0; i < n; i++)
                 {
                     diff0 = x[i];
                     diff1 = diff0 - last0;
                     e[i]  = diff1 - last1;
                     last0 = diff0;
                     last1 = diff1;
                 }
                 break;

        case 3 : last0 = 0;
                 last1 = 0;
                 last2 = 0;
                 for (i = 0; i < n; i++)
                 {
                     diff0 = x[i];
                     diff1 = diff0 - last0;
                     diff2 = diff1 - last1;
                     e[i]  = diff2 - last2;
                     last0 = diff0;
                     last1 = diff1;
                     last2 = diff2;
                 }
                 break;

        default : break;
    }

    return (p);
}


/********************************************************************
 *
 * Function Name : LpcDecode
 *
 * Description : This function decodes the input data IN of length N
 *   using a linear-predictive model of order P and places it in the
 *   array OUT of length N.  Output can be input.
 *
 * Inputs :
 *   out = Pointer to an array of length at least N (can alias array IN).
 *   in  = An array of encoded input values of length N.
 *   n   = Length of the input/output arrays.
 *   p   = Polynomial order of the best-fit LP model to the input data.
 *
 * Outputs :
 *   out = Output array of length N.
 *
 * Returns : None.
 *
 ********************************************************************/

static void
LpcDecode (gsfsLong *out, const gsfsLong *in, const size_t n, const int p)
{

    const gsfsLong *e = in;
    gsfsLong *x = out;
    gsfsLong last0, last1, last2;
    gsfsLong sum0, sum1, sum2;
    size_t i;


    /* Calculate the signal: x[i] = e_p[i] + x_p[i]. */
    switch (p)
    {
        case 0: if (x != e)
                {
                    for (i = 0; i < n; i++)
                    {
                        x[i] = e[i];
                    }
                }
                break;

        case 1: last0 = 0;
                for (i = 0; i < n; i++)
                {
                    sum0 = e[i] + last0;
                    x[i] = sum0;
                    last0 = sum0;
                }
                break;

        case 2: last0 = 0;
                last1 = 0;
                for (i = 0; i < n; i++)
                {
                    sum0 = e[i] + last0;
                    sum1 = sum0 + last1;
                    x[i] = sum1;
                    last0 = sum0;
                    last1 = sum1;
                }
                break;

        case 3: last0 = 0;
                last1 = 0;
                last2 = 0;
                for (i = 0; i < n; i++)
                {
                    sum0 = e[i] + last0;
                    sum1 = sum0 + last1;
                    sum2 = sum1 + last2;
                    x[i] = sum2;
                    last0 = sum0;
                    last1 = sum1;
                    last2 = sum2;
                }
                break;

        default : break;
    }
}


/********************************************************************
 *
 * Function Name : LpcOrder
 *
 * Description : This function determines the polynomial order of the
 *   a linear-predictive model that best fits the data X of length N.
 *   A polynomial of order P that passes through the points x[n-1],
 *   x[n-2], ..., x[n-p] can be represented by finite differences
 *   (forward/backward differences):
 *
 *   X0[n] =  0,
 *   X1[n] =  x[n-1],
 *   X2[n] = 2x[n-1] -  x[n-2],
 *   X3[n] = 3x[n-1] - 3x[n-2] + x[n-3].
 *
 *   The residual signal can be computed recursively :
 *
 *   e0[n] = X[n],
 *   e1[n] = e0[n] - e0[n-1],
 *   e2[n] = e1[n] - e1[n-1],
 *   e3[n] = e2[n] - e2[n-1].
 *
 *   Since the sum of the absolute values of each residual signal
 *   is linearly related to the variance, then the minimum value
 *   is chosen as the best fit.
 *
 * Inputs :
 *   x = Array of input integers.
 *   n = Length of the input array X.
 *
 * Returns : Polynomial order of the best-fit LP model (0, 1, 2, or 3).
 *
 ********************************************************************/

static int
LpcOrder (const gsfsLong *x, const size_t n)
{

    gsfsLongLong sum0, sum1, sum2, sum3;
    gsfsLong last0, last1, last2;
    size_t i;


    last0 = x[0];
    last1 = last2 = 0;
    sum0 = sum1 = sum2 = sum3 = 0;

    for (i = 0; i < n; i++)
    {
        gsfsLong diff0 = x[i];
        gsfsLong diff1 = diff0 - last0;
        gsfsLong diff2 = diff1 - last1;
        gsfsLong diff3 = diff2 - last2;

        sum0 += LPC_ABS (diff0);
        sum1 += LPC_ABS (diff1);
        sum2 += LPC_ABS (diff2);
        sum3 += LPC_ABS (diff3);

        last0 = diff0;
        last1 = diff1;
        last2 = diff2;
    }

    if (sum0 == 0) return (0);
    if ((sum1 <= sum2) && (sum1 <= sum3)) return (1);
    if (sum2 <= sum3) return (2);

    return (3);
}


/********************************************************************
 *
 * Function Name : SignEncode
 *
 * Description : This function maps the integer values in the input
 *   array to non-negative numbers: [ -2^31, .., -1, 0, 1, ...,
 *   2^31-1 ] => [ 0, 1, 2, 3, ..., 2^32-1 ].
 *
 * Inputs :
 *   out = Pointer to an array of length at least N (can alias array IN).
 *   in  = Input array of signed integers.
 *   n   = Length of the input array IN.
 *
 * Outputs :
 *   out = Array of encoded unsigned integers.
 *
 * Returns : None.
 *
 ********************************************************************/

static void
SignEncode (gsfuLong *out, const gsfsLong *in, const size_t n)
{

    size_t i;


    for (i = 0; i < n; i++)
    {
        gsfuLong x;

        if (in[i] < 0)
        {
            x = -in[i];
            out[i] = 2 * x - 1;
        }
        else
        {
            x = (gsfuLong) in[i];
            out[i] = 2 * x;
        }
    }
}


/********************************************************************
 *
 * Function Name : SignDecode
 *
 * Description : This function converts the array IN of length N of
 *   non-negative sign-encoded numbers back to an array of signed
 *   integers.  The sign is encoded in the least significant bit.
 *
 * Inputs :
 *   out = Pointer to an array of length at least N (can alias array IN).
 *   in  = Input array of unsigned integers.
 *   n   = Length of the input array IN.
 *
 * Outputs :
 *   out = Output array of integers.
 *
 * Returns : None.
 *
 ********************************************************************/

static void
SignDecode (gsfsLong *out, const gsfuLong *in, const size_t n)
{

    size_t i;


    for (i = 0; i < n; i++)
    {
        int sign = in[i] & 1;

        if (sign)
        {
            out[i] = -((in[i] + 1) / 2);
        }
        else
        {
            out[i] = in[i] / 2;
        }
    }
}


/********************************************************************
 *
 * Function Name : RleEncode
 *
 * Description : This function encodes the array IN of length N using a
 *   Run Length Encoding (RLE).  The RLE algorithm used will guarantee
 *   that the output length is never greater than the input length.  A
 *   sentinel value of zero is used, together with a value and count, to
 *   indicate a repeated value.  These are stored in reverse order
 *   (i.e., value, count, then sentinel) since decoding is done by
 *   traversing the array in reverse.  The use of this sentinel value
 *   means that the encoded value must be in the range [ 0, 2^32-2 ].
 *   Output can be input.
 *
 * Inputs :
 *   in  = The input array to be encoded.
 *   n   = The length of the input array IN.
 *   out = Pointer to an output array of length of at least N (can alias
 *     the input array IN).
 *
 * Outputs :
 *   out = The encoded output array.
 *
 * Returns : The length of the encoded output.
 *
 ********************************************************************/

static size_t
RleEncode (gsfuLong *out, const gsfuLong *in, const size_t n)
{

    size_t i, j, k;


    for (i = 0, k = 0; i < n; i++, k++)
    {
        gsfuLong c = 1;

        /* Count the number of consecutive integers that are equal. */
        for (j = i + 1; j < n; j++)
        {
            if (in[i] != in[j]) break;
            c++;
        }

        /* Increment all input values by one to make zero a (unique) sentinel value. */
        out[k] = in[i] + 1;

        /* We need at least four consecutive values to be effective. */
        if (c >= 4)
        {
            out[++k] = c - 4;  /* Add the count to the output (minus 4).     */
            out[++k] = 0;      /* Add the zero sentinel value to the output. */
            i = i + c - 1;
        }
    }

    return (k);
}


/********************************************************************
 *
 * Function Name : RleDecode
 *
 * Description : This function decodes the array IN of length N using
 *   a Run Length Encoding (RLE) and places it in the array OUT of
 *   length M.  The decoding is done by traversing the array in reverse
 *   so that the input array can alias the output array.  Output can
 *   be input
 *
 * Inputs :
 *   in = The input array to be decoded.
 *   n  = The length of the input array IN.
 *   m  = The length of the decoded output array of length at least N.
 *   out = Pointer to an output array of length of at least N (can alias
 *     the input array IN).
 *
 * Outputs :
 *   out = The decoded output array.
 *
 * Returns :
 *   Length of the decoded output array OUT, or
 *   0 if the data encoding is invalid (corrupt).
 *
 ********************************************************************/

static size_t
RleDecode (gsfuLong *out, const gsfuLong *in, const size_t n, const size_t m)
{

	int i, j, k;


	for (i = n - 1, k = m - 1; i >= 0; i--)
	{

    	/* Search for the sentinel value of zero. */
        if (in[i] == 0)
        {
            int c;

            /* Sentinel can't occur at the beginning of the array. */
            if (i < 2) return (0);

            /* Extract the count from the next value. */
            c = in[i-1] + 4;

            /* Count must be less than the remaining space available in the output array. */
            if (c > k + 1) return (0);

            /* Copy the duplicate values into the output array. */
            for (j = 0; j < c; j++)
            {
                if (in[i-2] == 0) return (0);
                out[k--] = in[i-2] - 1;
            }

            i = i - 2;
        }
        else
        {
        	/* Copy the input directly to the output. */
            if (k < 0) return (0);
            out[k--] = in[i] - 1;
        }
    }

	if (k < 0) return (m);

    return (0);
}


/********************************************************************
 *
 * Function Name : S16Encode
 *
 * Description : This function encodes an array of integers using the
 *   Simple-16 algorithm.  Simple-16 (S16) is a method of compressing
 *   small non-negative integers that combines bit-packing with a 4-byte
 *   alignment for speed.  Each 32-bit compressed word has 4 control
 *   bits and 28 data bits.  There are 16 possible packing schemes
 *   determined from the control bits.  The size of the numbers to
 *   encode are limited to the range [ 0, 2^28-1 ].  Output can be input.
 *
 * Inputs :
 *   out = Pointer to an array large enough to hold the compressed output.
 *   in  = An array of the input numbers to be compressed.
 *   n   = Length of the input array IN.
 *
 * Outputs :
 *   out = Array of the compressed numbers.
 *
 * Returns :
 *   Length of the compressed output array OUT, or
 *   0 if the input length is zero or a number is too large.
 *
 ********************************************************************/

static size_t
S16Encode (gsfuLong *out, const gsfuLong *in, const size_t n)
{

    gsfuLong *start = out;
    size_t left = n;


    while (left > 0)
    {
        size_t m = S16Pack (out, in, left);
        if (m == 0) return (0);
        in += m;
        out++;
        left -= m;
    }

    return (out - start);
}


/********************************************************************
 *
 * Function Name : S16Decode
 *
 * Description : This function decodes an array of integers using
 *   the Simple-16 algorithm.  Output can't be input.
 *
 * Inputs :
 *   out = Pointer to an array large enough to hold the uncompressed output.
 *   in  = An array of the input numbers to be uncompressed.
 *   m   = Length of the compressed input array IN.
 *
 * Outputs :
 *   out = Array of the uncompressed numbers.
 *
 * Returns :
 *   Length of the uncompressed output array OUT.
 *
 ********************************************************************/

static size_t
S16Decode (gsfuLong *out, const gsfuLong *in, const size_t m)
{

    /* TODO: Should modify this function to pass uncompressed size and
       check array bound overflow due to corrupt data.  Also, modify
       to uncompress in-place. */

    gsfuLong *start = out;
    size_t i;


    for (i = 0; i < m; i++)
    {
        out += S16Unpack (out, in[i]);
    }

    return (out - start);
}


/********************************************************************
 *
 * Function Name : S16Pack
 *
 * Description : This function packs an array of non-negative integers
 *   into one Simple-16 word. Each packing scheme is attempted until
 *   successful.  Output can be input.
 *
 * Inputs :
 *   out = Pointer to an array for the packed values.
 *   in  = An array of the input numbers to be compressed.
 *   n   = Number of integers in the input array to be compressed.
 *
 * Outputs :
 *   out = Array of the packed values.
 *
 * Returns :
 *   Number of input values packed, or
 *   0 if a number is too large to be encoded.
 *
 ********************************************************************/

static size_t
S16Pack (gsfuLong *out, const gsfuLong *in, const size_t n)
{

    size_t i, j;


    /* Skip any packing scheme that packs more numbers than we have. */
    for (i = 0; i < 16; i++)
    {
        if (s16_cnt[i] <= n) break;
    }

    for (; i < 16; i++)
    {
        const size_t c = s16_cnt[i];
        gsfuLong z, p;

        z = in[0];
        if (z >= (1U << s16_bits[i][0])) continue;
        p = s16_bits[i][0];

        for (j = 1; j < c; j++)
        {
            if (in[j] >= (1U << s16_bits[i][j])) break;
            z |= in[j] << p;
            p += s16_bits[i][j];
        }
        if (j == c)
        {
            /* Encode the 4-bit control value in the output number. */
            gsfuLong w = i;

            *out = z | (w << 28);
            return (c);
        }
    }

    return (0);
}


/********************************************************************
 *
 * Function Name : S16Unpack
 *
 * Description : This function unpacks an array of non-negative
 *   integers packed into one Simple-16 word.
 *
 * Inputs :
 *   in  = Input number to be unpacked.
 *   out = Pointer to an array large enough to hold the uncompressed output.
 *
 * Outputs :
 *   out = Array of uncompressed numbers.
 *
 * Returns :
 *   Number of values unpacked in the OUT array.
 *
 ********************************************************************/

static size_t
S16Unpack (gsfuLong *out, const gsfuLong in)
{

    const gsfuLong w = in >> 28;  /* Extract the 4-bit control word. */


    switch (w)
    {
        case 0 :
            out[0] = in & 3;
            out[1] =  (in >>  2) & 3;
            out[2] =  (in >>  4) & 3;
            out[3] =  (in >>  6) & 3;
            out[4] =  (in >>  8) & 3;
            out[5] =  (in >> 10) & 3;
            out[6] =  (in >> 12) & 3;
            out[7] =  (in >> 14) & 3;
            out[8] =  (in >> 16) & 3;
            out[9] =  (in >> 18) & 3;
            out[10] = (in >> 20) & 3;
            out[11] = (in >> 22) & 3;
            out[12] = (in >> 24) & 3;
            out[13] = (in >> 26) & 3;
            break;

        case 1 :
            out[0] = in & 7;
            out[1] =  (in >>  3) & 7;
            out[2] =  (in >>  6) & 7;
            out[3] =  (in >>  9) & 7;
            out[4] =  (in >> 12) & 3;
            out[5] =  (in >> 14) & 3;
            out[6] =  (in >> 16) & 3;
            out[7] =  (in >> 18) & 3;
            out[8] =  (in >> 20) & 3;
            out[9] =  (in >> 22) & 3;
            out[10] = (in >> 24) & 3;
            out[11] = (in >> 26) & 3;
            break;

        case 2 :
            out[0] = in & 7;
            out[1] =  (in >>  3) & 7;
            out[2] =  (in >>  6) & 7;
            out[3] =  (in >>  9) & 7;
            out[4] =  (in >> 12) & 7;
            out[5] =  (in >> 15) & 7;
            out[6] =  (in >> 18) & 7;
            out[7] =  (in >> 21) & 7;
            out[8] =  (in >> 24) & 3;
            out[9] =  (in >> 26) & 3;
            break;

        case 3 :
            out[0] = in & 15;
            out[1] =  (in >>  4) & 7;
            out[2] =  (in >>  7) & 7;
            out[3] =  (in >> 10) & 7;
            out[4] =  (in >> 13) & 7;
            out[5] =  (in >> 16) & 7;
            out[6] =  (in >> 19) & 7;
            out[7] =  (in >> 22) & 7;
            out[8] =  (in >> 25) & 7;
            break;

        case 4 :
            out[0] = in & 15;
            out[1] =  (in >>  4) & 15;
            out[2] =  (in >>  8) & 15;
            out[3] =  (in >> 12) & 15;
            out[4] =  (in >> 16) & 7;
            out[5] =  (in >> 19) & 7;
            out[6] =  (in >> 22) & 7;
            out[7] =  (in >> 25) & 7;
            break;

        case 5 :
            out[0] = in & 7;
            out[1] = (in >>  3) & 7;
            out[2] = (in >>  6) & 7;
            out[3] = (in >>  9) & 7;
            out[4] = (in >> 12) & 15;
            out[5] = (in >> 16) & 15;
            out[6] = (in >> 20) & 15;
            out[7] = (in >> 24) & 15;
            break;

        case 6 :
            out[0] = in & 15;
            out[1] = (in >>  4) & 15;
            out[2] = (in >>  8) & 15;
            out[3] = (in >> 12) & 15;
            out[4] = (in >> 16) & 15;
            out[5] = (in >> 20) & 15;
            out[6] = (in >> 24) & 15;
            break;

        case 7 :
            out[0] = in & 31;
            out[1] = (in >>  5) & 31;
            out[2] = (in >> 10) & 31;
            out[3] = (in >> 15) & 31;
            out[4] = (in >> 20) & 15;
            out[5] = (in >> 24) & 15;
            break;

        case 8 :
            out[0] = in & 15;
            out[1] = (in >>  4) & 15;
            out[2] = (in >>  8) & 31;
            out[3] = (in >> 13) & 31;
            out[4] = (in >> 18) & 31;
            out[5] = (in >> 23) & 31;
            break;

        case 9 :
            out[0] = in & 63;
            out[1] = (in >>  6) & 63;
            out[2] = (in >> 12) & 63;
            out[3] = (in >> 18) & 31;
            out[4] = (in >> 23) & 31;
            break;

        case 10 :
            out[0] = in & 31;
            out[1] = (in >>  5) & 31;
            out[2] = (in >> 10) & 63;
            out[3] = (in >> 16) & 63;
            out[4] = (in >> 22) & 63;
            break;

        case 11 :
            out[0] = in & 127;
            out[1] = (in >>  7) & 127;
            out[2] = (in >> 14) & 127;
            out[3] = (in >> 21) & 127;
            break;

        case 12 :
            out[0] = in & 1023;
            out[1] = (in >> 10) & 511;
            out[2] = (in >> 19) & 511;
            break;

        case 13 :
            out[0] = in & 16383;
            out[1] = (in >> 14) & 16383;
            break;

        case 14 :
            out[0] = in & 32767;
            out[1] = (in >> 15) & 8191;
            break;

        case 15 :
            out[0] = in & 268435455;
            break;

        default : break;
    }

    return (s16_cnt[w]);
}
