/****************************************************/
/* Module: cgen.c                                   */
/* The code generator implementation                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/* number of arguments
 * of current function call to be inserted
 * in the intermediate code instruction list
 */
ParamStack paramHead = NULL;

/* Head of the Quadruple List of Representations */
Quadruple head = NULL;

LocationStack locationHead = NULL;

/* Number for generating temporary variable names*/
static int temporary = 1;

/* Number for label generation */
static int label = 1;

Operand operandoAtual;

InstructionKind instrucaoAtual;

/* Useful to know when to return vector address*/
Quadruple lastQuadrupla;

static int linha = 0;

/* temporary string to help printing text */
static char tempString[20];

/* Prototype for recursive internal code generator */
static void cGen (TreeNode * tree);

static char * createLabelName() {
    char * temp = (char *) malloc(5);
    sprintf(temp, "L%d", label);
    ++label;
    return temp;
}

static char * createTemporaryOperandName() {
    char * temp = (char *) malloc(5);
    sprintf(temp, "t%d", temporary);
    ++temporary;
    return temp;
}

Operand createOperand(void) {
    Operand op = (Operand) malloc(sizeof(struct operand));
    return op;
}

static Operand createTemporaryOperand() {
    Operand temp = createOperand();
    temp->kind = String;
    temp->contents.variable.scope = NULL;
    temp->contents.variable.name = createTemporaryOperandName();
    return temp;
}

/* Procedure genStmt generates code at a statement node */
static void genStmt(TreeNode * tree) {
    Quadruple * q;
    Operand op1, op2, op3;
    TreeNode * p1, * p2, * p3;
    switch (tree->kind.stmt) {
        case INTEGERK:
            p1 = tree->child[0];
            cGen(p1);
            break; /* INTEGERK */

        case VOIDK:
            p1 = tree->child[0];
            cGen(p1);
            break; /* VOIDK */

        case IFK:
            p1 = tree->child[0];
            p2 = tree->child[1];
            p3 = tree->child[2];
            /* Generate code for test expression */
            cGen(p1);
            /* Assigns as the first operand*/
            op1 = operandoAtual;
            /* Assigns instruction type */
            instrucaoAtual = JP;
            /* Create and insert a new intermediate code representation*/
            q = insertQuad(createQuad(instrucaoAtual, op1, NULL, NULL));
            /* Save if's IR to update with label representing end of block */
            pushLocation(q);
            /* Generate code for block */
            cGen(p2);
            /* set second operand */
            op2 = createOperand();
            op2->kind = String;
            op2->contents.variable.name = createLabelName();
            op2->contents.variable.scope = tree->kind.var.scope;
            /* update if intermediate code instruction */
            updateLocation(op2);
            popLocation();

            if(p3 != NULL) {
                q = insertQuad(createQuad(GOTO, NULL, NULL, NULL));
                pushLocation(q);
            }

            /* Label used to mark end of block */
            insertQuad(createQuad(LBL, op2, NULL, NULL));
            cGen(p3);

            if(p3 != NULL) {
                op1 = createOperand();
                op1->kind = String;
                op1->contents.variable.name = createLabelName();
                op1->contents.variable.scope = tree->kind.var.scope;
                /* update if intermediate code instruction */
                updateLocation(op1);
                popLocation();

                /* Label used to mark end of block else*/
                insertQuad(createQuad(LBL, op1, NULL, NULL));
            }
            break; /* IFK */

        case WHILEK:
            p1 = tree->child[0];
            p2 = tree->child[1];

            op1 = createOperand();
            op1->kind = String;
            op1->contents.variable.name = createLabelName();
            op1->contents.variable.scope = tree->kind.var.scope;
            insertQuad(createQuad(LBL, op1, NULL, NULL));
            /* build code for test expression */
            cGen(p1);
            /* set as first operand */
            op2 = operandoAtual;
            /* set intermediate code kind */
            instrucaoAtual = JP;
            /* Create and insert a new intermediate code representation */
            q = insertQuad(createQuad(instrucaoAtual, op2, NULL, NULL));
            /* Save if's IR to update with label representing end of block then */
            pushLocation(q);
            /* build code for while block */
            cGen(p2);
            /* go back to while test expression */
            /* set intermediate code kind */
            instrucaoAtual = GOTO;
            /* Create and insert a new intermediate code representation*/
            insertQuad(createQuad(instrucaoAtual, op1, NULL, NULL));

            op3 = createOperand();
            op3->kind = String;
            op3->contents.variable.name = createLabelName();
            op3->contents.variable.scope = tree->kind.var.scope;
            insertQuad(createQuad(LBL, op3, NULL, NULL));
            updateLocation(op3);
            popLocation();
            break; /* WHILEK */

        case RETURNK:
            p1 = tree->child[0];
            /* Generate code for the expression*/
            cGen(p1);
            if(p1) {
                /* Assigns as the first operand */
                op1 = operandoAtual;
            } else {
                /*return nothing*/
                op1 = NULL;
            }
            /* Assigns instruction type */
            instrucaoAtual = RTN;
            /* Create and insert a new intermediate code representation */
            insertQuad(createQuad(instrucaoAtual, op1, NULL, NULL));
            break; /* RETURNK */

        case COMPK:
            p1 = tree->child[0];
            if(p1) { /* Parameters */
                cGen(p1);
            }
            p2 = tree->child[1];
            if(p2) { /* Expression */
                cGen(p2);
            }
            break; /* COMPK */
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp(TreeNode * tree) {
    TreeNode * p1, * p2;
    Operand op1, op2, op3;
    switch (tree->kind.exp) {
        case ATRIBK:
            p1 = tree->child[0];
            p2 = tree->child[1];
            /* Generate code for the right operand */
            cGen(p2);
            /* Assigns as the second operand */
            op2 = operandoAtual;
            /* Generate code for the left operand*/
            cGen(p1);
            /* Assigns as the first operand */
            op1 = operandoAtual;
            op3 = NULL;

            // ------------------- Special case ------------------- //
            /* In assignment, if the left operand is a vector, you must
	     * store the memory position of this variable
             */
            if(p1->node == VARK && p1->kind.var.varKind == VECTORK) {
                lastQuadrupla->instruction = VEC_ADDR;
                if(lastQuadrupla->op2->kind == IntConst) {
                    op3 = createOperand();
                    op3->kind = IntConst;
                    op3->contents.val = lastQuadrupla->op2->contents.val;
                    op1 = lastQuadrupla->op1;
                }
            }
            instrucaoAtual = ASN;
            insertQuad(createQuad(instrucaoAtual, op1, op2, op3));
            break; /* ATRIBK */

        case RELK:
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p1);
            op1 = operandoAtual;
            cGen(p2);
            op2 = operandoAtual;
            switch (tree->op) {
                case EQUAL:
                    instrucaoAtual = EQ;
                    break;
                case NEQUAL:
                    instrucaoAtual = NE;
                    break;
                case LT:
                    instrucaoAtual =_LT;
                    break;
                case LE:
                    instrucaoAtual = LET;
                    break;
                case GT:
                    instrucaoAtual = _GT;
                    break;
                case GE:
                    instrucaoAtual = GET;
                    break;
            }
            operandoAtual = createTemporaryOperand();
            insertQuad(createQuad(instrucaoAtual, op1, op2, operandoAtual));
            break; /* RELK */

        case ARITHK:
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p1);
            op1 = operandoAtual;
            cGen(p2);
            op2 = operandoAtual;
            switch (tree->op) {
                case PLUS:
                    instrucaoAtual = ADD;
                    break;
                case MINUS:
                    instrucaoAtual = SUB;
                    break;
                case MUTI:
                    instrucaoAtual = MULT;
                    break;
                case DIV:
                    instrucaoAtual = _DIV;
                    break;
            }
            operandoAtual = createTemporaryOperand();
            insertQuad(createQuad(instrucaoAtual, op1, op2, operandoAtual));
            break; /* ARITHK */
    }
} /* genExp */
void VerifyEndPreviousInstruction(void) {
	if(head != NULL) {
	Quadruple temp = head;
	while(temp->next != NULL) {
	temp = temp->next;
	}
	if(temp->instruction != RTN) {
	insertQuad(createQuad(RTN, NULL, NULL, NULL));
	}
	}
}
static void genVar(TreeNode * tree) {
    Quadruple q;
    TreeNode * p1, * p2;
    Operand op1, op2, op3;
    int qtdParams, offset, display = -1;
    switch (tree->kind.var.varKind) {
        case CONSTK:
            /* Assigns the current operand */
            operandoAtual = createOperand();
            operandoAtual->kind = IntConst;
            operandoAtual->contents.val = tree->kind.var.attr.val;
            break; /* CONSTK */

        case IDK:
            /* Assigns the current operand */
            operandoAtual = createOperand();
            operandoAtual->kind = String;
            operandoAtual->contents.variable.name = tree->kind.var.attr.name;
            operandoAtual->contents.variable.scope = tree->kind.var.scope;
            break; /* IDK */

        case VECTORK:
            p1 = tree->child[0];
            /* Updates current operand as vector id and arrow as op1 */
            operandoAtual = createOperand();
            operandoAtual->kind = String;
            operandoAtual->contents.variable.name = tree->kind.var.attr.name;
            operandoAtual->contents.variable.scope = tree->kind.var.scope;
            op1 = operandoAtual;
            /* Generate code for vector position */
            cGen(p1);
            /* Vector index */
            op2 = operandoAtual;
            /* Assigns the current instruction */
            instrucaoAtual = VEC;
            /* Temporary */
            op3 = createTemporaryOperand();
            /* Update the current operand */
            operandoAtual = op3;
            /* Create and insert a new intermediate code representation, but
	     * only if accessing vector
             */
            if(tree->kind.var.access == ACCESSK) {
                // Save last instruction to handle when accessing vector address
                lastQuadrupla = createQuad(instrucaoAtual, op1, op2, op3);
                insertQuad(lastQuadrupla);
            }
            break; /* VectorK */

        case FUNCTIONK:
            VerifyEndPreviousInstruction();
            op1 = createOperand();
            op1->kind = String;
            op1->contents.variable.name = tree->kind.var.attr.name;
            op1->contents.variable.scope = tree->kind.var.scope;
            insertQuad(createQuad(FUNC, op1, NULL, NULL));

            /* list of parameters */
            p1 = tree->child[0];
            while(p1 != NULL) {
                op2 = createOperand();
                op2->kind = String;
                op2->contents.variable.name = p1->child[0]->kind.var.attr.name;
                op2->contents.variable.scope = p1->child[0]->kind.var.scope;
                insertQuad(createQuad(GET_PARAM, op2, NULL, NULL));
                p1 = p1->sibling;
            }

            /* build code for function block */
            p2 = tree->child[1];
            cGen(p2);
            break;

        case CALLK:
            /* Arguments */
            p1 = tree->child[0];
            op1 = createOperand();
            op1->kind = String;
            op1->contents.variable.name = tree->kind.var.attr.name;
            op1->contents.variable.scope = tree->kind.var.scope;
            qtdParams = getParametersQuantity(tree);
            pushParam(&qtdParams);
            op2 = createOperand();
            if(qtdParams > 0) {
                op2->kind = IntConst;
                op2->contents.val = qtdParams;
            } else {
                op2->kind = IntConst;
                op2->contents.val = 0;
            }
            /* build code for function call */
            instrucaoAtual = PARAM_LIST;
            op3 = createOperand();
            op3->kind = IntConst;
            op3->contents.val =getParametersQuantity(tree);
            insertQuad(createQuad(instrucaoAtual, op3, NULL, NULL));
            while(p1 != NULL) {
                cGen(p1);
                /*Assigns instruction type */
                instrucaoAtual = SET_PARAM;
                /* Create and insert a new intermediate code representation */
                insertQuad(createQuad(instrucaoAtual, operandoAtual, NULL, NULL));
                /* Decrements qtdParams */
                --qtdParams;
                /* If it is an OUTPUT call, check the display */
                if(!strcmp(tree->kind.var.attr.name, "output") && p1->sibling == NULL) {
                    display = p1->kind.var.attr.val;
                }
                p1 = p1->sibling;
            }
            popParam();
            instrucaoAtual = CALL;
            if (op1->kind == IntConst) {
                offset = operandoAtual->contents.val;
            }
            operandoAtual = createTemporaryOperand();
            q = createQuad(instrucaoAtual, op1, op2, operandoAtual);
            if(display != -1) {
                q->display = display;
            }
            insertQuad(q);
            break;
    }
}

/* Procedure cGen Generates Code Recursively
 * by the syntactic tree
 */
static void cGen(TreeNode * tree) {
    if (tree != NULL) {
        switch (tree->node) {
            case STMTK:
                genStmt(tree);
                break;
            case EXPK:
                genExp(tree);
                break;
            case VARK:
                genVar(tree);
                break;
            default:
                break;
        }
        /*If the number of parameters is greater than 0, cGen () will be called automatically*/
        if(paramHead == NULL) {
            cGen(tree->sibling);
        } else {
            if(paramHead->count == 0) {
                cGen(tree->sibling);
            }
        }
    }
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode * syntaxTree, char * codefile, CodeInfo codeInfo) {
    cGen(syntaxTree);
        // If it is a Common Program code, add SYSCALL to the end of the code.
        insertQuad(createQuad(SYSCALL, NULL, NULL, NULL));
    

    emitCode("********** Intermediate Code **********\n");
    printIntermediateCode();
}

void printIntermediateCode() {
    Quadruple q = head;
    char quad[100];

    while(q != NULL) {
        sprintf(quad, "%d: (", q->linha);
        strcat(quad, toStringInstruction(q->instruction));

        if(q->op1 == NULL) {
            strcat(quad, ", _");
        } else if(q->op1->kind == String) {
            strcat(quad, ", ");
            strcat(quad, q->op1->contents.variable.name);
        } else {
            sprintf(tempString, ", %d", q->op1->contents.val);
            strcat(quad,tempString);
        }

        if(q->op2 == NULL) {
            strcat(quad, ", _");
        } else if(q->op2->kind == String) {
            strcat(quad, ", ");
            strcat(quad, q->op2->contents.variable.name);
        } else {
            sprintf(tempString, ", %d", q->op2->contents.val);
            strcat(quad,tempString);
        }

        if(q->op3 == NULL) {
            strcat(quad, ", _)");
        } else if(q->op3->kind == String) {
            strcat(quad, ", ");
            strcat(quad, q->op3->contents.variable.name);
            strcat(quad, ")");
        } else {
            sprintf(tempString, ", %d", q->op3->contents.val);
            strcat(quad,tempString);
            strcat(quad, ")");
        }
        emitCode(quad);
        q = q->next;
    }
}

void pushLocation(Quadruple * quad) {
    // createLocation
    LocationStack ls = (LocationStack) malloc(sizeof(struct Location));
    ls->quad = quad;
    ls->next = NULL;

    if(locationHead == NULL) {
        locationHead = ls;
        locationHead->next = NULL;
    } else {
        ls->next = locationHead;
        locationHead = ls;
    }
}

void popLocation() {
    if(locationHead != NULL) {
        LocationStack ls = locationHead;
        locationHead = locationHead->next;
        free(ls);
        ls = NULL;
    }
}

void updateLocation(Operand op) {
    Quadruple q = *locationHead->quad;
    if(q->instruction != JP) {
        q->op1 = op;
    } else {
        q->op2 = op;
    }
    *locationHead->quad = q;
}

void pushParam(int * count) {
    ParamStack ps = (ParamStack) malloc(sizeof(struct Param));
    ps->count = count;
    ps->next = NULL;
    if(paramHead == NULL) {
        paramHead = ps;
    } else {
        ps->next = paramHead;
        paramHead = ps;
    }
}

void popParam(void) {
    if(paramHead != NULL) {
        ParamStack ps = paramHead;
        paramHead = paramHead->next;
        free(ps);
        ps = NULL;
    }
}

Quadruple createQuad(InstructionKind instruction, Operand op1, Operand op2, Operand op3) {
    Quadruple q = (Quadruple) malloc(sizeof(struct Quad));
    q->instruction = instruction;
    q->op1 = op1;
    q->op2 = op2;
    q->op3 = op3;
    q->linha = ++linha;
    q->next = NULL;
    return q;
}

Quadruple * insertQuad(Quadruple q) {
    Quadruple * ptr = (Quadruple *) malloc(sizeof(struct Quad));
    if(head == NULL) {
        head = q;
        head->next = NULL;
        ptr = &head;
    } else {
        Quadruple temp = head;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = q;
        temp->next->next = NULL;
        ptr = &temp->next;
    }
    return ptr;
}
