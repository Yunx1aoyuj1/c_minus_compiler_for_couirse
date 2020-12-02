/****************************************************/
/* Module: code.h                                   */
/* Generate quaternion                              */
/****************************************************/

#ifndef _CODE_H_
#define _CODE_H_

#include "cgen.h"

const char * toStringInstruction(enum instrucao i);

void emitCode(const char * c);

void emitComment(const char * c, int indent);

void emitObjectCode(const char * c, int indent);

#endif
