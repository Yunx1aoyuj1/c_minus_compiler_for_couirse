/****************************************************/
/* Module: util.c	                                */
/* Print results                        	        */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char* tokenString) {
    switch (token) {
        case IF: fprintf(listing,"if\n"); break;
        case ELSE: fprintf(listing, "else\n"); break;
        case WHILE: fprintf(listing,"while\n"); break;
        case RETURN: fprintf(listing,"return\n"); break;
        case INT: fprintf(listing,"int\n"); break;
        case VOID: fprintf(listing,"void\n"); break;
        case PLUS: fprintf(listing,"+\n"); break;
        case MINUS: fprintf(listing,"-\n"); break;
        case MUTI: fprintf(listing,"*\n"); break;
        case DIV: fprintf(listing,"/\n"); break;
        case LT: fprintf(listing, "<\n"); break;
	case ASSIGN: fprintf(listing, "=\n"); break;
        case LE: fprintf(listing, "<=\n"); break;
        case GT: fprintf(listing, ">\n"); break;
        case GE: fprintf(listing, ">=\n"); break;
        case EQUAL: fprintf(listing, "==\n"); break;
        case NEQUAL: fprintf(listing, "!=\n"); break;
        case SEMI: fprintf(listing, ";\n"); break;
        case COMMA: fprintf(listing, ",\n"); break;
        case LPAREN: fprintf(listing, "(\n"); break;
        case RPAREN: fprintf(listing, ")\n"); break;
        case LBRACKET: fprintf(listing, "[\n"); break;
        case RBRACKET: fprintf(listing, "]\n"); break;
        case LKEY: fprintf(listing, "{\n"); break;
        case RKEY: fprintf(listing, "}\n"); break;
        case ENDFILE: fprintf(listing, "EOF\n"); break;
        case NUM: fprintf(listing, "NUM, val= %s\n", tokenString); break;
        case ID: fprintf(listing, "ID, name= %s\n", tokenString); break;
        case ERROR: fprintf(listing, "ERROR: %s\n", tokenString); break;
        default: 
        fprintf(listing, "Unknow Token : %d\n", token);
    }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    } else {
        for (i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node = STMTK;
        t->lineno = lineno;
        t->kind.stmt = kind;
    }
    return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    } else {
        for (i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node = EXPK;
        t->lineno = lineno;
        t->kind.exp = kind;
    }
    return t;
}

TreeNode * newVarNode(VarKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    } else {
        for (i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node = VARK;
        t->lineno = lineno;
        t->kind.var.varKind = kind;
        t->kind.var.access = DECLK;
        t->kind.var.scope = NULL;
    }
    return t;
}

TreeNode * newSysNode(SysCallKind kind) {
    TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    } else {
        for (i = 0; i < MAXCHILDREN; i++) {
            t->child[i] = NULL;
        }
        t->sibling = NULL;
        t->node = SYSK;
        t->lineno = lineno;
        t->kind.sys = kind;
    }
    return t;
}

char * toStringSysCall(SysCallKind syscall) {
    char * strings[] = {
        "input", "output"
    };
    return strings[syscall];
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s) {
    int n;
    char * t;

    if (s == NULL) {
        return NULL;
    }

    n = strlen(s)+1;
    t = (char *) malloc(n);

    if (t == NULL) {
        fprintf(listing,"Out of memory error at line %d\n",lineno);
    } else {
        strcpy(t,s);
    }
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
int indentno = 0;
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
void printSpaces(void) {
    int i;
    for (i = 0; i < indentno; i++) {
        fprintf(listing," ");
    }
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree ) {
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        if (tree->node == STMTK) {
            switch (tree->kind.stmt) {
                case INTEGERK: fprintf(listing, "int\n"); break;
                case VOIDK: fprintf(listing, "void\n"); break;
                case IFK: fprintf(listing, "if\n"); break;
                case WHILEK: fprintf(listing, "while\n"); break;
                case RETURNK: fprintf(listing, "return\n");	break;
		case COMPK: fprintf(listing, "Composite Statement:\n");	break;
            }
        } else if (tree->node == EXPK) {
            switch (tree->kind.exp) {
                case RELK: fprintf(listing, "Judge Op: "); printToken(tree->op, "\0"); break;
                case ARITHK: fprintf(listing, "Arith Op: "); printToken(tree->op, "\0"); break;
            }
        } else if (tree->node == VARK) {
            switch (tree->kind.var.varKind) {
                case IDK: fprintf(listing, "Id: %s\n", tree->kind.var.attr.name); break;
                case VECTORK: fprintf(listing, "Vector: %s\n", tree->kind.var.attr.name); break;
                case CONSTK: fprintf(listing, "Const: %d\n", tree->kind.var.attr.val); break;
                case FUNCTIONK: fprintf(listing, "Function: %s\n", tree->kind.var.attr.name); break;
                case CALLK: fprintf(listing, "Function call: %s\n", tree->kind.var.attr.name); break;
            }
        } else if (tree->node == SYSK) {
            fprintf(listing, "System call: %s\n", toStringSysCall(tree->kind.sys));
        }
        for (i = 0; i < MAXCHILDREN; i++) {
            printTree(tree->child[i]);
        }
        tree = tree->sibling;
    }
    UNINDENT;
}
