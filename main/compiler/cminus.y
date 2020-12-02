/****************************************************/
/* Module: cminus.y                                 */
/* Grammar for Cminus                                */
/****************************************************/

%{
    #define YYPARSER /* distinguishes Yacc output from other code files */

    #include "globals.h"
    #include "util.h"
    #include "scan.h"
    #include "parse.h"

    #define YYSTYPE TreeNode *
    char * savedName; /* for use in assignments */
    int savedLineNo;  
    TreeNode * savedTree; /* stores syntax tree for later return */
    int yylex(void);
    int yyerror(char * message);
    TreeNode * createSysCall(ExpType type, SysCallKind syscall);
    void insertNewNode(TreeNode * node);
%}

%token IF ELSE WHILE RETURN
%token ID NUM
%token GT GE LT LE EQUAL NEQUAL PLUS MINUS MUTI DIV 
%token ASSIGN
%token LPAREN RPAREN SEMI LBRACKET RBRACKET COMMA LKEY RKEY
%token ERROR
%token INT VOID
%nonassoc RPAREN
%nonassoc ELSE

%% /* Grammar for C- */

program
    : declarationList
        {
            insertNewNode(createSysCall(INTEGER_TYPE, INPUT));                  // Insert
            insertNewNode(createSysCall(VOID_TYPE, OUTPUT));                    // Output
            TreeNode * temp;
            temp = savedTree;
            while (temp->sibling != NULL) {
                temp = temp->sibling;
            }
            temp->sibling = $1;
        }
    ;

declarationList
    : declarationList declaration
        {
            YYSTYPE t = $1;
            if (t != NULL) {
                while (t->sibling != NULL) {
                    t = t->sibling;
                }
                t->sibling = $2;
                $$ = $1;
            } else {
                $$ = $2;
            }
        }
    | declaration { $$ = $1; }
    ;

declaration
    : varDeclaration { $$ = $1; }
    | funDeclaration { $$ = $1; }
    ;

varDeclaration
    : typeSpecifier id SEMI
        {
            $$ = $1;
            $$->child[0] = $2;
            $$->child[0]->type = $$->type;
            $$->child[0]->kind.var.mem = LOCALK;
        }
    | typeSpecifier id LBRACKET num RBRACKET SEMI
        {
            $$ = $1;
            $$->child[0] = $2;
            $$->child[0]->type = $$->type;
            $$->child[0]->kind.var.mem = LOCALK;
            $$->child[0]->kind.var.varKind = VECTORK;
            $$->child[0]->child[0] = $4;
            $$->child[0]->child[0]->type = INTEGER_TYPE;
        }
    ;

typeSpecifier
    : INT
        {
            $$ = newStmtNode(INTEGERK);
            $$->type = INTEGER_TYPE;
            $$->op = INT;
        }
    | VOID
        {
            $$ = newStmtNode(VOIDK);
            $$->type = VOID_TYPE;
            $$->op = VOID;
        }
    ;

funDeclaration
    : typeSpecifier id LPAREN params RPAREN compoundStmt
        {
            $$ = $1;
            $$->child[0] = $2;
            $$->child[0]->type = $$->type;
            $$->child[0]->kind.var.varKind = FUNCTIONK;
            $$->child[0]->kind.var.mem = FUNCTION_MEM;
            $$->child[0]->child[0] = $4;
            $$->child[0]->child[1] = $6;
        }
    ;

params
    : paramList { $$ = $1; }
    | VOID { $$ = NULL; }
    ;

paramList
    : paramList COMMA param
        {
            YYSTYPE t = $1;
            if (t != NULL) {
            	while (t->sibling != NULL) {
            		t = t->sibling;
            	}
            	t->sibling = $3;
            	$$ = $1;
            } else {
            	$$ = $3;
            }
        }
    | param { $$ = $1; }
    ;

param
    : typeSpecifier id
        {
            $$ = $1;
            $$->child[0] = $2;
            $$->child[0]->kind.var.mem = PARAMK;
        }
    | typeSpecifier id LBRACKET RBRACKET
        {
            $$ = $1;
            $$->child[0] = $2;
            $$->child[0]->kind.var.mem = PARAMK;
            $$->child[0]->kind.var.varKind = VECTORK;
        }
    ;

compoundStmt
    : LKEY localDeclarations statementList RKEY
        {
            $$ = newStmtNode(COMPK);
            $$->child[0] = $2;
            $$->child[1] = $3;
            $$->op = COMPK;
        }
    ;

localDeclarations
    : localDeclarations varDeclaration
        {
        	YYSTYPE t = $1;
        	if (t != NULL) {
        		while (t->sibling != NULL) {
        			t = t->sibling;
        		}
        		t->sibling = $2;
        		$$ = $1;
        	} else {
        		$$ = $2;
        	}
        }
    | empty { $$ = $1; }
    ;

statementList
    : statementList statement
        {
        	YYSTYPE t = $1;
        	if (t != NULL) {
        		while (t->sibling != NULL) {
        			t = t->sibling;
        		}
        		t->sibling = $2;
        		$$ = $1;
        	} else {
        		$$ = $2;
        	}
        }
    | empty { $$ = $1; }
    ;

statement
    : expressionStmt { $$ = $1; }
    | compoundStmt { $$ = $1; }
    | selectionStmt { $$ = $1; }
    | iterationStmt { $$ = $1; }
    | returnStmt { $$ = $1; }
    ;

expressionStmt
    : expression SEMI { $$ = $1; }
    | SEMI { $$ = NULL; }
    ;

selectionStmt
    : IF LPAREN expression RPAREN statement
        {
            $$ = newStmtNode(IFK);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->op = IFK;
        }
    | IF LPAREN expression RPAREN statement ELSE statement
        {
            $$ = newStmtNode(IFK);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->child[2] = $7;
            $$->op = IFK;
        }
    ;

iterationStmt
    : WHILE LPAREN expression RPAREN statement
        {
        	$$ = newStmtNode(WHILEK);
        	$$->child[0] = $3;
        	$$->child[1] = $5;
            $$->op = WHILEK;
        }
    ;

returnStmt
    : RETURN SEMI
        {
        	$$ = newStmtNode(RETURNK);
            $$->op = RETURNK;
        }
    | RETURN expression SEMI
        {
        	$$ = newStmtNode(RETURNK);
        	$$->child[0] = $2;
            $$->op = RETURNK;
        }
    ;

expression
    : var assigmentOperator expression
        {
            $$ = $2;
            $$->child[0] = $1;
        	$$->child[1] = $3;
        }
    |equalityExpression{$$ = $1;}
    ;

assigmentOperator
    : ASSIGN
        {
        	$$ = newExpNode(ATRIBK);
        	$$->op = ASSIGN;
        }
    ;

var
    : id
        {
        	$$ = $1;
            $$->kind.var.access = ACCESSK;
        }
    | id LBRACKET expression RBRACKET
        {
        	$$ = $1;
        	$$->kind.var.varKind = VECTORK;
            $$->kind.var.access = ACCESSK;
        	$$->child[0] = $3;
        }
   ;

equalityExpression
    : equalityExpression equalityOperator relationalExpression
        {
            $$ = $2;
            $$->child[0] = $1;
            $$->child[1] = $3;
        }
    | relationalExpression { $$ = $1; }
    ;

equalityOperator
    : EQUAL
        {
            $$ = newExpNode(RELK);
            $$->op = EQUAL;
        }
    | NEQUAL
        {
            $$ = newExpNode(RELK);
            $$->op = NEQUAL;
        }
    ;

relationalExpression
    : relationalExpression relationalOperator additiveExpression
        {
        	$$ = $2;
        	$$->child[0] = $1;
        	$$->child[1] = $3;
        }
    |additiveExpression{$$ =$1; }
    ;

relationalOperator
    : LE
		{
			$$ = newExpNode(RELK);
			$$->op = LE;
		}
    | LT
		{
			$$ = newExpNode(RELK);
			$$->op = LT;
		}
    | GT
        {
        	$$ = newExpNode(RELK);
        	$$->op = GT;
        }
    | GE
        {
        	$$ = newExpNode(RELK);
        	$$->op = GE;
        }
    ;

additiveExpression
    : additiveExpression additiveOperator term
        {
        	$$ = $2;
        	$$->child[0] = $1;
        	$$->child[1] = $3;
        }
	| term { $$ = $1; }
	;

additiveOperator
    : PLUS
        {
        	$$ = newExpNode(ARITHK);
        	$$->op = PLUS;
        }
    | MINUS
        {
        	$$ = newExpNode(ARITHK);
        	$$->op = MINUS;
        }
    ;

term
    : term termop factor
        {
        	$$ = $2;
        	$$->child[0] = $1;
        	$$->child[1] = $3;
        }
    | factor { $$ = $1; }
    ;

termop
    : MUTI
        {
        	$$ = newExpNode(ARITHK);
        	$$->op = MUTI;
        }
    | DIV
        {
        	$$ = newExpNode(ARITHK);
        	$$->op = DIV;
        }
    ;

factor
    : LPAREN expression RPAREN { $$ = $2; }
    | var { $$ = $1; }
    | activation { $$ = $1; }
    | num { $$ = $1; }
    ;

activation
    : var LPAREN args RPAREN
        {
        	$$ = $1;
        	$$->kind.var.varKind = CALLK;
        	$$->child[0] = $3;
            $$->op = CALLK;
        }
    ;

args
    : arg_list { $$ = $1; }
    | empty { $$ = $1; }
    ;

arg_list
    : arg_list COMMA expression
        {
        	YYSTYPE t = $1;
        	if (t != NULL) {
        		while (t->sibling != NULL) {
        			t = t->sibling;
        		}
        		t->sibling = $3;
        		$$ = $1;
        	} else {
        		$$ = $3;
        	}
        }
    | expression { $$ = $1; }
    ;

id
    : ID
        {
        	$$ = newVarNode(IDK);
        	$$->kind.var.attr.name = copyString(tokenString);
            $$->type = INTEGER_TYPE;
        }
    ;

num
    : NUM
		{
            $$ = newVarNode(CONSTK);
            $$->kind.var.attr.val = atoi(tokenString);
            $$->type = INTEGER_TYPE;
		}
	;

empty
    : { $$ = NULL; }
    ;

%%

int yyerror(char * message) {
    fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
    fprintf(listing,"Current token: ");
    printToken(yychar,tokenString);
    Error = TRUE;
    return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
int yylex(void) {
    return getToken();
}

TreeNode * parse(void) {
    yyparse();
    return savedTree;
}

TreeNode * getIntNode(TreeNode * childNode) {
    TreeNode * intNode = newStmtNode(INTEGERK);
    intNode->op = INT;
    intNode->type = INTEGER_TYPE;
    intNode->child[0] = childNode;
    return intNode;
}

TreeNode * getVoidNode(TreeNode * childNode) {
    TreeNode * voidNode = newStmtNode(VOIDK);
    voidNode->op = VOID;
    voidNode->type = VOID_TYPE;
    voidNode->child[0] = childNode;
    return voidNode;
}

TreeNode * createSysCall(ExpType type, SysCallKind syscall) {
    TreeNode * function = newSysNode(syscall);
    function->lineno = 0;
    function->op = ID;
    function->type = type;
    return type == INTEGER_TYPE ? getIntNode(function) : getVoidNode(function);
}

void insertNewNode(TreeNode * node) {
    TreeNode * temp;
    if (savedTree == NULL) {
        savedTree = node;
        savedTree->sibling = NULL;
    } else {
        temp = savedTree;
        while (temp->sibling != NULL) {
            temp = temp->sibling;
        }
        temp->sibling = node;
        temp->sibling->sibling = NULL;
    }
}
