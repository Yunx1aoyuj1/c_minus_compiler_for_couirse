/****************************************************/
/* Module: util.h                                   */
/* Utility functions for the Cminus compiler        */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

void printToken(TokenType, const char *);

TreeNode * newStmtNode(StmtKind);

TreeNode * newExpNode(ExpKind);

TreeNode * newVarNode(VarKind);

TreeNode * newSysNode(SysCallKind);

char * toStringSysCall(SysCallKind syscall);

char * copyString(char *);

void printTree(TreeNode *);

#endif
