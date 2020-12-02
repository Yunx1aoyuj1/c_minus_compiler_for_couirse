/****************************************************/
/* Module: code.c                                   */
/* Generate quaternion                              */
/****************************************************/

#include "globals.h"
#include "code.h"

const char * toStringInstruction(enum instrucao i) {
    const char * strings[] = {
        "+", "-", "*", "/"," ",
	" ", " ", " ", " ", " ", " ",
	" ", " ","vector_value", "vector_address",
	"jz", "jnz", "j<", "j<=",
	"j>", "j>=", "=",
	"function", "return", "get_param", "set_param", "call", "param_list",
	"j_if_false", "j", "label", "syscall", "halt"
    };
    return strings[i];
}

void emitSpaces(int indent){
    int i;
    for(i = 0; i < indent; ++i) {
        fprintf(code, " ");
    }
}

void emitCode(const char * c) {
    fprintf(code, "%s\n", c);
}

void emitComment(const char * c, int indent) {
    if (TraceCode) {
        emitSpaces(indent);
        fprintf(code, "# %s\n", c);
    }
}

