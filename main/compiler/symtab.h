/****************************************************/
/* Module: symtab.h                                 */
/* Interface for C minus symbol table               */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

#define SIZE 211

typedef struct LineListRec {
    int lineno;
    struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec {
    char * name;
    LineList lines;
    TreeNode * treeNode;
    int memloc; /* Memory Address of Variable */
    int tamanho; /* Variable Size */
    struct BucketListRec * next;
} * BucketList;

typedef struct ScopeRec {
    char * funcName;
    struct ScopeRec * parent;
    int tamanhoBlocoMemoria; /* Size of allocated memory block */
    BucketList hashTable[SIZE]; /* the hash table */
} * Scope;

Scope globalScope;
typedef struct SysCallRec {
    char * name;
    TreeNode * treeNode;
    struct SysCallRec * next;
} * SysCall;
SysCall syscallHead;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
SysCall sys_create(char * name, TreeNode * treeNode);
void sys_insert(SysCall syscall);
SysCall sys_lookup(char * name);
void sys_free(void);
void st_insert(char * name, int lineno, int loc, TreeNode * treeNode, int tamanho);
BucketList st_create(char * name, int lineno, int loc, TreeNode * treeNode, int tamanho);
void st_insert_func(char * name, int lineno, TreeNode * treeNode);
int verifyGlobalScope(TreeNode * treeNode);
/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup (char * name);
BucketList st_bucket(char * name);
BucketList st_bucket_func (char * name);
void st_add_lineno(TreeNode * treeNode);
void incScope();
int st_lookup_top (char * name);
Scope sc_create(char * funcName);
Scope sc_top(void);
void sc_pop(void);
void sc_push(Scope scope);
int st_lookup_func (char * name);
BucketList getVarFromSymtab(char * name, Scope escopo);
int getMemoryLocation(char * name, Scope escopo);
void printSymTab(FILE * listing);
/*
 * getQuantidadeParameters returns the number of function parameters
 * is necessarily the tree node that represents the function
 */
int getParametersQuantity(TreeNode * functionNode);
/*
 * getQuantidadeVariaveis eturns the number of variables declared in function scope
 * functionNode is necessarily the tree node that represents the function
 */
int getVariablesQuantity(TreeNode * functionNode);
/*
 * getTamanhoBlocoMemoriaEscopo returns the size of the memory block that should be allocated in the stack
 * scopeName must be scope name
 */
int getScopeMemoryBlockSize(char * scopeName);
int getGlobalScopeMemoryBlockSize(void);

#endif
