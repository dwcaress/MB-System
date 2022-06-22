
#ifndef OCTREE_TEST_HPP
#define OCTREE_TEST_HPP

// MAP_HEADER_BYTES
// number of bytes in an octree
// map header.
// NOTE: sizeof(Octree<bool>::MapHeader)
// returns different values on
// Cygwin and Linux, since
// Cygwin word-aligns struct members.
// Vector is 24 bytes on both systems.
// within the MapHeader struct on Cygwin,
// it is padded to 32 bytes. So
// sizeof(Octree<bool>::MapHeader) is
// 4 bytes larger on Cygwin (there are
// 4 Vectors, each padded by 1 byte)
//
// Changing the header format to include
// size info would be a better solution

#define NSEC_PER_SEC 1000000000
#define TIME_STR_BYTES 128
#define TIME2NSEC(t) ((uint64_t)((struct timespec *)t)->tv_sec*NSEC_PER_SEC + (uint64_t)((struct timespec *)t)->tv_nsec)
#define HISTO_DEPTH 32

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct otree_config_s{
    // map file path
    char *map_name;
    // print tree structure to stdout
    bool print;
    // run PrintOctree method for comparison
    bool do_otprint;
    // enable verbose output
    bool verbose;
};

typedef otree_config_s otree_config;

struct TreeStats_s{
    unsigned long depth;
    unsigned long nodes;
    unsigned long leaves;
    unsigned long branches;
    unsigned long histogram[HISTO_DEPTH];
    struct timespec t_start;
    struct timespec t_end;
};
typedef TreeStats_s TreeStats;
typedef unsigned char byte;


#endif
