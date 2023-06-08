#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>           // For atoi()
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <iomanip>

// libgen needed for basename
#include <libgen.h>
#include <stdint.h>
// Octree only needed to compare mmap to original
#include "Octree.hpp"
#include "OctreeTest.hpp"

// print help/use info to stderr
void show_help(char *bin)
{
    fprintf(stderr,"\n");
    fprintf(stderr,"Description: traverse binary tree and show summary; optionally print tree as text\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Usage: %s [-f <mapfile>] [-h]\n",bin);
    fprintf(stderr,"\n");
    fprintf(stderr,"-f <file> : specify map file\n");
    fprintf(stderr,"-p        : print tree to console [a LOT of text]\n");
    fprintf(stderr,"-c        : compare to Octree.Print tree stats\n");
    fprintf(stderr,"-v        : enable verbose output\n");
    fprintf(stderr,"-h        : print this help message\n");
    fprintf(stderr,"\n");
}

// parse command line options
void parse_opts(int argc, char **argv, otree_config *cfg)
{
    int c=0;
    
    if (argc<=2) {
        show_help(basename(argv[0]));
        exit(0);
    }

    while((c = getopt(argc, argv, "cf:pvh")) != -1)
    {
        switch(c) {
            case 'c':
                cfg->do_otprint = true;
                break;
           case 'f':
                cfg->map_name = optarg;
                break;
            case 'p':
                cfg->print = true;
                break;
            case 'v':
                cfg->verbose = true;
                break;
            case 'h':
                show_help(basename(argv[0]));
                exit(0);
                break;
            default:
                break;
        }
    }
}

// print octree header contents to stdout
void header_show(Octree<bool>::MapHeader *map_header)
{
    std::cout << std::endl;
    int wkey=18;
    int wval=12;
    int wxyz=10;
    std::ios_base::fmtflags orig_settings = std::cout.flags();
    int orig_precision = std::cout.precision();

    std::cout << "Map Header" << std::endl;
    std::cout << std::setw(wkey) << " " << " [x, y, z]" << std::endl;
    std::cout << std::setw(wkey) << "Lower Bounds :";
    std::cout << std::setprecision(5);
    std::cout << " [" << std::setw(wxyz) << map_header->LowerBounds.x << ", ";
    std::cout << std::setw(wxyz) << map_header->LowerBounds.y << ", ";
    std::cout << std::setw(wxyz) << map_header->LowerBounds.z << "]" <<  std::endl;

    std::cout << std::setw(wkey) << "Upper Bounds :";
    std::cout << std::setprecision(5);
    std::cout << " [" << std::setw(wxyz) << map_header->UpperBounds.x << ", ";
    std::cout << std::setw(wxyz) << map_header->UpperBounds.y << ", ";
    std::cout << std::setw(wxyz) << map_header->UpperBounds.z << "]" <<  std::endl;

    std::cout << std::setw(wkey) << "Size :";
    std::cout << std::setprecision(5);
    std::cout << " [" << std::setw(wxyz) << map_header->Size.x << ", ";
    std::cout << std::setw(wxyz) << map_header->Size.y << ", ";
    std::cout << std::setw(wxyz) << map_header->Size.z << "]" <<  std::endl;

    std::cout << std::setw(wkey) << "True Resolution :";
    std::cout << std::setprecision(5);
    std::cout << " [" << std::setw(wxyz) << map_header->TrueResolution.x << ", ";
    std::cout << std::setw(wxyz) << map_header->TrueResolution.y << ", ";
    std::cout << std::setw(wxyz) << map_header->TrueResolution.z << "]" <<  std::endl;

    std::cout << std::setw(wkey) << "Max Depth :" << std::setw(wval) << map_header->MaxDepth << std::endl;
    std::cout << std::setw(wkey) << "OffMapValue :" << std::setw(wval) << map_header->OffMapValue << std::endl;
    std::cout << std::setw(wkey) << "EmptyValue :" << std::setw(wval) << map_header->EmptyValue << std::endl;
    std::cout << std::setw(wkey) << "OctreeType :" << std::setw(wval) << (short int)map_header->OctreeNodeType << std::endl;
    std::cout << std::setw(wkey) << "valueType sz :" << std::setw(wval) << sizeof(bool) << std::endl;

    std::cout.flags(orig_settings);
    std::cout.precision(orig_precision);
}

// print octree node summary to stdout
// (supports indent level)
void node_show(Octree<bool>::OTNode *node,int indent=0)
{
    printf("\n");
//    printf("%*sNode[%d,%p]\n",indent,(indent?" ":""),indent,node);
    printf("%*sNode[%p,%d] \n",indent,(indent?" ":""),node,indent);
    printf("%*sValue       [%d]\n",indent,(indent?" ":""),node->value);
    printf("%*shasChildren [%d]\n",indent,(indent?" ":""),(node->hasChildren?1:0));
    //OctreeNode** children;

    printf("\n");
}

// print map file stat summary to stdout
void fstat_show(char *name, struct stat *sp)
{

    std::cout << std::endl;
    int wkey=18;
    int wval=32;

    std::cout << "File Stats" << std::endl;
    std::cout << std::setw(wkey) << "name :" << std::setw(wval) << basename(name) <<  std::endl;
    std::cout << std::setw(wkey) << "size :" << std::setw(wval) << (long long)sp->st_size<<  std::endl;
    std::cout << std::setw(wkey) << "uid :" << std::setw(wval) << (long)sp->st_uid  <<  std::endl;
    std::cout << std::setw(wkey) << "uid :" << std::setw(wval) << (long)sp->st_gid <<  std::endl;
    std::cout << std::setw(wkey) << "mode :" << std::setw(wval) << (unsigned long)sp->st_mode <<  std::endl;
    std::cout << std::endl;

    struct tm *tmp;
    char outstr[TIME_STR_BYTES];
    
    memset(outstr,0,TIME_STR_BYTES);
    tmp = localtime(&sp->st_mtime);
    if (tmp == NULL) {
        perror("localtime");
        return;
    }else{
        strftime(outstr, TIME_STR_BYTES, "%s %FT%H:%M:%S", tmp);
        std::cout << std::setw(wkey) << "st_mtime :" << std::setw(wval) << outstr <<  std::endl;
    }
    memset(outstr,0,TIME_STR_BYTES);
    tmp = localtime(&sp->st_ctime);
    if (tmp == NULL) {
        perror("localtime");
        return;
    }else{
        strftime(outstr, TIME_STR_BYTES, "%s %FT%H:%M:%S", tmp);
        std::cout << std::setw(wkey) << "st_ctime :" << std::setw(wval) << outstr <<  std::endl;
    }
    
    memset(outstr,0,TIME_STR_BYTES);
    tmp = localtime(&sp->st_atime);
    if (tmp == NULL) {
        perror("localtime");
        return;
    }else{
        strftime(outstr, TIME_STR_BYTES, "%s %FT%H:%M:%S", tmp);
        std::cout << std::setw(wkey) << "st_atime :" << std::setw(wval) << outstr <<  std::endl;
    }
    
    std::cout << std::endl;
}

uint64_t stat_start(TreeStats *ts, bool set)
{
    uint64_t retval=0xFFFFFFFFFFFFFFFF;
    if(ts!=NULL){
        if (set) {
            clock_gettime(CLOCK_MONOTONIC,&ts->t_start);
        }
        retval = TIME2NSEC(&ts->t_start);
    }
    return retval;
}

uint64_t stat_stop(TreeStats *ts, bool set)
{
    uint64_t retval=0xFFFFFFFFFFFFFFFF;
    if(ts!=NULL){
        if (set) {
            clock_gettime(CLOCK_MONOTONIC,&ts->t_end);
        }
        retval = TIME2NSEC(&ts->t_end);
    }
    return retval;
}

uint64_t stat_clk_res()
{
     struct timespec t_res={0};
    
    clock_getres(CLOCK_MONOTONIC,&t_res);

    return(TIME2NSEC(&t_res));
}

// print octree stat summary to stdout
void stat_show_summary(TreeStats *ts, bool do_hist)
{
    if (ts!=NULL) {
        std::ios_base::fmtflags orig_settings = std::cout.flags();
        int orig_precision = std::cout.precision();
        std::cout << std::endl;
        int wkey=18;
        int wval=12;
        std::cout << std::setfill(' ');
        std::cout << "Stat Summary" << std::endl;
        std::cout << std::setw(wkey) << "depth :" << std::setw(wval) << ts->depth <<  std::endl;
        std::cout << std::setw(wkey) << "branches :" << std::setw(wval) << ts->branches << std::endl;
        std::cout << std::setw(wkey) << "leaves :" << std::setw(wval) << ts->leaves << std::endl;
        std::cout << std::setw(wkey) << "nodes :" << std::setw(wval) << ts->nodes << std::endl;
        
        std::cout << std::setw(wkey) << "disk size :" << std::setw(wval) << Octree<bool>::DiskSize((OTreeStats *)ts) << std::endl;
        std::cout << std::setw(wkey) << "RAM size :" << std::setw(wval) << Octree<bool>::MemSize((OTreeStats *)ts) << std::endl;


        std::cout << std::setw(wkey) << "t_res :" << std::setw(wval) << stat_clk_res() << std::endl;
        std::cout << std::setw(wkey) << "t_trav :" << std::setw(wval) << (stat_stop(ts,false)-stat_start(ts,false)) << std::endl;

        if (do_hist && ts->depth>0) {
            std::cout << std::setw(wkey) << "nodes v depth :" << std::endl;
            for(long unsigned i=0;i<=ts->depth;i++){
                std::cout << std::setw(wkey-6) << "[" << std::setw(2) << i << "] : " << std::setw(wval) <<    ts->histogram[i] << "\n";
            }
        }
        std::cout << std::endl;
        std::cout.flags(orig_settings);
        std::cout.precision(orig_precision);
    }

}


// stat_traverse_map recursively traverses an octree map (depth first)
// Tree statistics are collected and returned in a TreeStats structure.
// If the show argument is TRUE, the tree nodes are printed to the console
 int stat_traverse_map(Octree<bool>::OTNode *node=NULL, int depth=0, bool show=false, TreeStats *ts=NULL, byte *map=NULL)
{

     int child_count=0;
     int node_total_descendants=0;
     int child_descendants[8]={0};
    
    if (node) {
        // point to next node
        Octree<bool>::OTNode *next=node+1;
        
        if (show) {
            node_show(node, depth);
        }
        
        if (ts !=NULL) {
            ts->histogram[depth]++;
            if(depth>0){
                ts->nodes++;
            }
            if ((long unsigned)depth>ts->depth) {
                ts->depth=depth;
            }
        }
        //    printf("%*snode[%08x] has children[%c]\n",depth," ",((byte *)node-map),(node->hasChildren?'Y':'N'));
        
        if (node->hasChildren) {
            if (ts !=NULL) {
                if(depth>0){
                    ts->branches++;
                }
            }
            
            for (int i=0; i<8; i++) {
                // traverse
                child_descendants[i] += stat_traverse_map((Octree<bool>::OTNode *)next,depth+1,show,ts,map);
                child_count++;
                node_total_descendants+=child_descendants[i];
                next += child_descendants[i]+1;
                //            printf("%*snode[%08x]  child[%d] returned [%4d] os[%d]\n",depth," ",((byte *)node-map),i,child_descendants[i],child_descendants[i]+1);
            }
        }else{
            if (ts !=NULL) {
                ts->leaves++;
            }
        }
        
        node_total_descendants+=child_count;
        //    printf("%*snode[%08x] returning[%d]\n",depth," ",((byte *)node-map),node_total_descendants);
    }

    return node_total_descendants;
}


// get memory address of octree leaf on specified path
Octree<bool>::OTNode* MGetPointerToLeafOnPath(Octree<bool>::OTNode *root, int MaxDepth, const Path& path)
{
    Octree<bool>::OTNode *nodePointer = root;
    unsigned int bitmask = 1 << (MaxDepth - 1);
    int childNumber=0;
    /* Each loop will test to see if there are children.  If there are no children,
     the current node is the one we want.  If there are, children, the path is
     followed to the next layer.
     */
    for(int depth = 0; depth < MaxDepth; depth ++) {

        if(nodePointer->hasChildren==false) {
            return nodePointer;
        }
        
        childNumber =
        ((0 != (path.x & bitmask)?1:0) << 2)
        | ((0 != (path.y & bitmask)?1:0) << 1)
        |  (0 != (path.z & bitmask)?1:0);
        bitmask >>= 1;
        nodePointer += (childNumber+1);
    }
    return nodePointer;
}


//  get offset from path
long unsigned path2offset(){return 0;}
long unsigned offset2path(){return 0;}
long unsigned node_at_offset(){return 0;}
long unsigned node_at_path(){return 0;}

int test_octree_funcs(Octree<bool>::OTNode *ot_root, Octree<bool>::MapHeader *map_header)
{
    int retval=-1;
    
    if (ot_root!=NULL && map_header!=NULL) {
        // Test PointerToLeafOnPath
        Path path;
        path.x=0x653;
        path.y=0x342;
        path.z=0x114;
        
        Octree<bool>::OTNode *leaf=MGetPointerToLeafOnPath(ot_root, map_header->MaxDepth, path);
        long r_ofs=((byte *)leaf-(byte *)ot_root);
        long f_ofs=((byte *)leaf-(byte *)map_header);
        
        if (r_ofs>0 && f_ofs>0) {
            std::ios_base::fmtflags orig_settings = std::cout.flags();
            int orig_precision = std::cout.precision();

            std::cout << std::endl;
            int wkey=18;
            int wval=18;
            std::cout << "Pointer to Leaf" <<  std::endl;
            std::cout << std::hex << std::showbase;
            std::cout << std::setw(wkey) << "path :" ;
            std::cout << std::setw(wval-12) << path.x << "," << path.y << "," << path.z << std::endl;
            std::cout << std::noshowbase;
            std::cout << std::setw(wkey) << "leaf :" << std::setw(wval) << leaf << std::endl;
            std::cout << std::setw(wkey) << "root_ofs :" << std::setw(wval-8) << "0x" << std::setfill('0') << std::setw(8) << r_ofs << std::endl;
            std::cout << std::setfill(' ') ;
            std::cout << std::setw(wkey) << "file_ofs :" << std::setw(wval-8) << "0x" << std::setfill('0') << std::setw(8) << f_ofs << std::endl;
            std::cout.flags(orig_settings);
            std::cout.precision(orig_precision);

            retval=0;
        }else{
            fprintf(stderr,"ERR - root offset or file offset < 0\n");
        }
    }else{
        fprintf(stderr,"ERR - NULL octree or header\n");
    }
    return retval;
}

// This application traverses an octree file
// on disk using mmap, instead of expanding
// the tree into memory. It accumulates statistics
// about the tree and prints a summary
int main(int argc, char **argv)
{
    // configuration for this app
    otree_config cfg={NULL,false,false};

    // parse command line options
    parse_opts(argc,argv,&cfg);
    
    // validate options
    if (cfg.map_name == NULL) {
        errno=EINVAL;
        handle_error("filename not defined");
    }
    
    // octree map file descriptor
    int map_fd = open(cfg.map_name,O_RDONLY|O_NONBLOCK);
    // octree map file info
    struct stat map_stat;
    
    if (map_fd == -1) {
        fprintf(stderr,"Error opening %s\n",cfg.map_name);
        handle_error("open");
    }
    
    // To obtain file size in map_stat.st_size
    if (fstat(map_fd, &map_stat) == -1){
        fprintf(stderr,"Could not stat %s\n",cfg.map_name);
        handle_error("fstat");
    }
    
    // initialize map variables
    // map_size   : size of map file (bytes, includes headers, etc.)
    // map_offset : file offset of start of map headers/data (bytes)
    // pa_offset  : page-aligned offset
    size_t map_size=map_stat.st_size;
    off_t map_offset=0, pa_offset=0;
    
    // offset for mmap() must be page aligned
    pa_offset = map_offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    // create mapping of octree memory in virtual address space
    byte *map_ptr=(byte *)mmap(NULL, map_size+map_offset-pa_offset, PROT_READ, MAP_PRIVATE, map_fd, pa_offset);
    
    if (map_ptr == MAP_FAILED) {
        handle_error("mmap");
    }

    // OK to close the file
    // file count won't decrement until
    // after munmap
    close(map_fd);

    // init pointer to map header
    Octree<bool>::MapHeader *map_header=(Octree<bool>::MapHeader *)map_ptr;
    // init pointer to octree root
    Octree<bool>::OTNode* ot_root = (Octree<bool>::OTNode *)(map_ptr + sizeof(Octree<bool>::MapHeader));//-1

    // init tree statistics struct
    TreeStats ts={0};

    if (cfg.verbose) {
        std::cout << std::endl;
        int wkey=42;
        int wval=12;

        std::cout << std::setw(wkey) << "ot_root :" << std::setw(wval) << ot_root <<  std::endl;
        std::cout << std::setw(wkey) << "map_header :" << std::setw(wval) << map_header <<  std::endl;
        std::cout << std::setw(wkey) << "os :" << std::setw(wval) << ((byte *)ot_root-(byte *)map_header) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(Octree<bool>::OTNode) :" << std::setw(wval) << sizeof(Octree<bool>::OTNode) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(OctreeNode) :" << std::setw(wval) << Octree<bool>::NodeSize()<<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(bool) :" << std::setw(wval) << sizeof(bool) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(Vector) :" << std::setw(wval) << sizeof(Vector) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(int) :" << std::setw(wval) << sizeof(int) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(OctreeType::EnumOctreeType) :" << std::setw(wval) << sizeof(OctreeType::EnumOctreeType) <<  std::endl;
        std::cout << std::setw(wkey) << "sizeof(Octree<bool>::MapHeader) :" << std::setw(wval) << sizeof(Octree<bool>::MapHeader) <<  std::endl;

        size_t sum = 4*sizeof(Vector)+sizeof(int)+2*sizeof(bool)+sizeof(OctreeType::EnumOctreeType);
        std::cout << std::setw(wkey) << "sum of header members :" << std::setw(wval) << sum <<  std::endl;
        std::cout << std::setw(wkey) << "map_ptr :" << std::setw(wval) << (void *)map_ptr <<  std::endl;
        std::cout << std::setw(wkey) << "map_header :" << std::setw(wval) << map_header <<  std::endl;
        std::cout << std::setw(wkey) << "header.OffMapValue :" << std::setw(wval) << &map_header->OffMapValue <<  std::endl;
        std::cout << std::setw(wkey) << "header.EmptyValue :" << std::setw(wval) << &map_header->EmptyValue <<  std::endl;
        std::cout << std::setw(wkey) << "header.MaxDepth :" << std::setw(wval) << &map_header->MaxDepth <<  std::endl;
        std::cout << std::setw(wkey) << "header.OctreeNodeType :" << std::setw(wval) << &map_header->OctreeNodeType <<  std::endl;
        std::cout << std::setw(wkey) << "header.LowerBounds :" << std::setw(wval) << &map_header->LowerBounds <<  std::endl;
        std::cout << std::setw(wkey) << "header.UpperBounds :" << std::setw(wval) << &map_header->UpperBounds <<  std::endl;
        std::cout << std::setw(wkey) << "header.Size :" << std::setw(wval) << &map_header->Size <<  std::endl;
        std::cout << std::setw(wkey) << "header.TrueResolution :" << std::setw(wval) << &map_header->TrueResolution <<  std::endl;
        std::cout << std::setw(wkey) << "map_ptr+sizeof(Octree<bool>::MapHeader) :" << std::setw(wval)  << (void *)(map_ptr+sizeof(Octree<bool>::MapHeader)) <<  std::endl;

    }

    // start timing
    stat_start(&ts, true);
    
	// traverse the tree to gather statistics
    // and optionally print the tree to stdout
    int nodes=stat_traverse_map(ot_root, 0, cfg.print, &ts);

    // stop timing
    stat_stop(&ts, true);

    // dump file stats and map header
    fstat_show(cfg.map_name, &map_stat);
    header_show(map_header);
    
    // dump tree stats
    stat_show_summary(&ts,cfg.verbose);
    
    if (cfg.verbose) {
        printf("traverse_tree returned [%d]\n",nodes);
    }

//	Should just make MOctree variant - subclass?
//    Octree<bool> tree = Octree<bool>((Octree<bool>::MapHeader *)map_header, (Octree<bool>::OTNode *)ot_root);
//    bool foo = tree.Query( Vector(1000.0,1000.0,1000.0));
//    printf("Query [%d]\n",(int)foo);
    
	// test octree manipulation functions
    test_octree_funcs(ot_root, map_header);
  
    munmap(map_ptr,map_size);
    
    // optionally, compare to Octree.Print
    if (cfg.do_otprint) {
            printf("\nUsing Octree.Print\n");
            OTreeStats ots={0};
            Octree<bool> octree;
            if(octree.LoadFromFile(cfg.map_name)){
                octree.Print(&ots);
            }
    }
    printf("\n");
}
