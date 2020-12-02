/****************************************************/
/* Module: main.c                                   */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

#include "util.h"
#include "interface.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

/* allocate and set tracing flags */
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;
int Error = FALSE;

int main(int argc, char * argv[]) {
    TreeNode * syntaxTree;
    CodeInfo codeInfo = interface(argc, argv);
    listing = stdout; /* send listing to screen */
    fprintf(listing, "\nCompiler C minus: %s\n",codeInfo.pgm);
#if NO_PARSE
    while (getToken() != ENDFILE);
#else
    syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nBuilding a syntax tree:\n\n");
        printTree(syntaxTree);
    }
#if !NO_ANALYZE
    if (! Error) {
        if (TraceAnalyze) fprintf(listing, "\nBuilding the symbol table...\n");
        buildSymtab(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nChecking type...\n");
        typeCheck(syntaxTree);
        if (TraceAnalyze) fprintf(listing, "\nType check completed!\n");
    }
#if !NO_CODE
    if (! Error) {
        char * codefile;
        int fnlen = strcspn(codeInfo.pgm, ".");
        codefile = (char *) calloc(fnlen + 4, sizeof(char));
        strncpy(codefile, codeInfo.pgm, fnlen);
        strcat(codefile, ".txt");
        code = fopen(codefile, "w");
        if (code == NULL) {
            printf("Cannot open the file %s\n", codefile);
            exit(1);
        }
        if (TraceCode) fprintf(listing, "\nGenerate intermediate code...\n");
        codeGen(syntaxTree, codefile, codeInfo);
        free(syntaxTree);
        fclose(code);
        if (TraceCode) fprintf(listing, "\nIntermediate code generation is complete!\n");
    }
#endif
#endif
#endif
    fclose(source);
    return 0;
}
