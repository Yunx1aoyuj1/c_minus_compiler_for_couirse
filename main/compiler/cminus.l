/****************************************************/
/* Module: cminus.l                                 */
/* Lexical specification for C-                      */
/****************************************************/

%{
    #include "globals.h"
    #include "util.h"
    #include "scan.h"
    /* lexeme of identifier or reserved word */
    int comment = 0;
    char tokenString[MAXTOKENLEN + 1];
    static int yylex(void);
%}

digit           [0-9]
number          {digit}+
letter          [a-zA-Z_]
letterOrNumber  [a-zA-Z_0-9]
identifier      {letter}{letterOrNumber}*
newline         \n
whitespace      [ \t\r]+

%%

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"return"        { return RETURN; }
"int"           { return INT; }
"void"          { return VOID; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUTI; }
"/"             { return DIV; }

"="             { return ASSIGN; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"=="            { return EQUAL; }
"!="            { return NEQUAL; }

";"             { return SEMI; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"{"             { return LKEY; }
"}"             { return RKEY; }

{number}        { return NUM; }
{identifier}    { return ID; }
{newline}       { lineno++; }
{whitespace}    { /* skip whitespace */ }

.               { return ERROR; }


"//"            {
                    char c;
                    do {
                        c = input();
                        if (c == EOF) break;
                        if (c == '\n') {lineno++; break;}
                    } while (TRUE);
                }

"/*"            {
                    char c;
                    do {
                        c = input();
                        if (c == EOF) break;
                        if (c == '\n') lineno++;
                        if (c == '*') comment = 1;
                        else if (c == '/' && comment == 1) comment = 2;
                        else comment = 0;
                    } while (comment != 2);
                }

%%

TokenType getToken(void) {
    static int firstTime = TRUE;
    TokenType currentToken;
    if (firstTime) {
        firstTime = FALSE;
        lineno++;
        yyin = source;
        yyout = listing;
    }
    currentToken = yylex();
    strncpy(tokenString, yytext, MAXTOKENLEN);
    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    return currentToken;
}
