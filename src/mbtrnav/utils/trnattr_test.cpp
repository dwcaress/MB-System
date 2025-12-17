#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <libgen.h>

#include "TrnAttr.h"

int main(int argc, char **argv)
{
    const char *cfg = "terrainAid.cfg";

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"-h") == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, " %s : test TrnAttr\n", basename(argv[0]));
            fprintf(stderr, "    use : test-trnattr [options] <config_file>\n");
            fprintf(stderr, "    options:\n");
            fprintf(stderr, "     -h : print help message\n");
            fprintf(stderr, "\n");
            return 0;
        } else {
            cfg = argv[1];
        }
    }

TrnAttr ta(cfg);
ta.parseConfig();
fprintf(stderr,"\n%s:\n %s \n", cfg, ta.tostring().c_str());
return 0;
}
