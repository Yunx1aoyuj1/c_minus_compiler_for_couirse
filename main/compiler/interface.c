/****************************************************/
/* Mudele: cli.c                                    */
/* Command line interface                           */
/****************************************************/

#include "globals.h"
#include "interface.h"
CodeInfo interface(int argc, char * argv[]) {
    char pgm[120]; 
    strcpy(pgm, argv[1]) ;
    if (strchr (pgm, '.') == NULL) {
        strcat(pgm, ".c");
    }
    source = fopen(pgm, "r");
    if (source == NULL) {
        fprintf(stderr, "File %s is not found!\n", pgm);
        exit(1);
    }
    CodeInfo codeInfo;
    strcpy(codeInfo.pgm, pgm);
    return codeInfo;
}
