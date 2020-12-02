/****************************************************/
/* Module: analyze.c                                */
/* Implementation of the semantic analyzer          */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"
#include <string.h>

char * funcName;

/* Position counter for data memory*/
int location = 0;

/* Flag to check if main function has already been declared*/
int mainDeclaration = FALSE;

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void declError(TreeNode * t, const char * message) {
    fprintf(listing, "Declaration error in line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

static void varError(TreeNode * t, const char * message) {
    fprintf(listing, "Variable error in line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode(TreeNode * t) {
    BucketList l;
    if (t->node == VARK) {
        switch (t->kind.var.varKind) {
            case IDK:
                if(t->kind.var.access == ACCESSK) {
                    /* Checks for any entries with their variable name*/
                    l = st_bucket(t->kind.var.attr.name);
                    /* If you return any results then there is*/
                    if (l != NULL) {
                        if(l->treeNode->kind.var.varKind == FUNCTIONK) {
                            varError(t, "Cannot reference a function as variable");
                            break;
                        } else {
                            st_add_lineno(t);
                            break;
                        }
                    }
                    /* If it has come this far the variable is being accessed without being previously declared */
                    varError(t, "Undeclared variable in scope!");
                } else {
                    /* Checks for any entries with their variable name */
                    BucketList l = st_bucket(t->kind.var.attr.name);
                    if(l != NULL) {
                        if (l->treeNode != NULL) {
                            /* If the result is of type FUNCTIONK, it means it has already been declared as a function */
                            if (l->treeNode->kind.var.varKind == FUNCTIONK) {
                                declError(t, "Variable name is already used to declare a function");
                                break;
                            }
                           /*If the result is of type IDK, check if it has already been declared as a variable
			   * Checks if the scope of the variable is equal to the current scope, if it is equal
			   * goes to else and declares error, otherwise does nothing in if for
			   * st_insert () procedure to be executed*/
                            if(strcmp(l->treeNode->kind.var.scope->funcName, sc_top()->funcName)) {
                            } else {
                                declError(t, "Variable already declared in this scope!");
                                break;
                            }
                        }
                    }
                    /*If no declaration errors occur and no variable name exists,
		     *add variable to symbol table
                     */
                    st_insert(t->kind.var.attr.name, t->lineno, location++, t, 1);
                }
                break;
            case VECTORK:
                if(t->kind.var.access == ACCESSK) {
                    /* Checks for any entries with their variable name*/
                    l = st_bucket(t->kind.var.attr.name);
                    if (l != NULL) {
                        if(l->treeNode->kind.var.varKind == FUNCTIONK) {
                            varError(t, "Unable to reference a function as variable!");
                            break;
                        } else {
                            st_add_lineno(t);
                            break;
                        }
                    }
                    /* If it has come this far the variable is being accessed without being previously declared */
                    varError(t, "Undeclared variable in scope!");
                } else {
                    BucketList l = st_bucket(t->kind.var.attr.name);
                    if(l != NULL) {
                        if (l->treeNode != NULL) {
                            /* If the result is of type FunctionK, it has already been declared as function */
                            if (l->treeNode->kind.var.varKind == FUNCTIONK) {
                                declError(t, "Variable name is already used to declare a function");
                                break;
                            }
                            if(strcmp(l->treeNode->kind.var.scope->funcName, sc_top()->funcName)) {
                                /* Do nothing */
                            } else {
                                declError(t, "Variable already declared in this scope");
                                break;
                            }
                        }
                    }
                    /* If no error,add to symtab*/
                    int Vetorsize = t->child[0] != NULL ? t->child[0]->kind.var.attr.val : 1;
                    st_insert(t->kind.var.attr.name, t->lineno, location++, t, Vetorsize);
                    // Skip n memory locations where not size Vector
                    location += Vetorsize - 1;
                }
                break;
            case CONSTK:
                break;
            case FUNCTIONK:
                funcName = t->kind.var.attr.name;
                if(strcmp(funcName, "main") == 0) mainDeclaration = TRUE; // main being declared
                if (st_lookup_func(funcName)) {
                    declError(t, "Main function already declared");
                    break;
                }
                // Reset the location
                location = 0;
                sc_push(sc_create(funcName));
                st_insert_func(funcName, t->lineno, t);
                break;
            case CALLK:
                funcName = t->kind.var.attr.name;
                if (sys_lookup(funcName) != NULL) {
                    // Do nothing
                } else if (!st_lookup_func(funcName)) {
                    declError(t, "Undeclared Function");
                } else {
                    BucketList l = st_bucket_func(funcName);
                    if (l->treeNode != NULL) {
                        t->type = l->treeNode->type;
                    }
                }
                break;
      	}
  	} else if (t->node == SYSK) {
        sys_insert(sys_create(toStringSysCall(t->kind.sys), t));
    }
}

static void afterInsertNode(TreeNode * t) {
	if (t->kind.var.varKind == FUNCTIONK) {
		if(t->kind.var.attr.name != NULL) {
			sc_pop();
        }
	}
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree) {
	globalScope = sc_create((char *) "ESCOPO_GLOBAL");
	sc_push(globalScope);
	traverse(syntaxTree, insertNode, afterInsertNode);
	sc_pop();
    sys_free();
	if(mainDeclaration == FALSE) {
		fprintf(listing, "Declaration Error: Undeclared main function\n");
    	return;
	}
 	if (TraceAnalyze) {
		fprintf(listing,"\nSymbol Table:\n");
    	printSymTab(listing);
  	}
}

static void typeError(TreeNode * t, const char * message) {
    fprintf(listing, "Type error in line %d: %s\n", t->lineno, message);
    Error = TRUE;
}

static void beforeCheckNode(TreeNode * t) {
	if (t->node == EXPK) {
		if (t->kind.var.varKind == FUNCTIONK) {
			funcName = t->kind.var.attr.name;
		}
  	}
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t) {
    if (t->node == STMTK) {
        switch (t->kind.stmt) {
            case INTEGERK: t->child[0]->type = INTEGER_TYPE; break;
            case VOIDK:
                if (t->child[0]->kind.var.varKind == IDK) {
                    typeError(t, "Variable cannot be void!");
                } else {
                    t->child[0]->type = VOID_TYPE;
                }
                break;
            case IFK: break;
            case WHILEK: break;
            case RETURNK: break;
            case COMPK: break;
        }
    } else if (t->node == EXPK) {
        switch (t->kind.exp) {
            case ATRIBK:
                if ((t->child[0]->type != INTEGER_TYPE) || (t->child[1]->type != INTEGER_TYPE)) {
                    typeError(t, "Invalid assignment, int value expected and void received");
                } else  {
                    t->type = INTEGER_TYPE;
                }
                break;
            case RELK: t->type = VOID_TYPE; break;
            case ARITHK: t->type = INTEGER_TYPE; break;
        }
    } else if (t->node == VARK) {
        switch (t->kind.var.varKind) {
            case IDK: t->type = INTEGER_TYPE; break;
            case VECTORK: t->type = INTEGER_TYPE; break;
            case CONSTK: t->type = INTEGER_TYPE; break;
            case FUNCTIONK: break;
            case CALLK: break;
        }
    }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree) {
	traverse(syntaxTree, beforeCheckNode, checkNode);
}
