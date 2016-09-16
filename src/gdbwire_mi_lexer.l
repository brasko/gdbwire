%option prefix="gdbwire_mi_"
%option outfile="lex.yy.c"
%option extra-type="struct gdbwire_mi_position"
%option reentrant
%option noyywrap
%option nounput
%option noinput

DIGIT       [0-9]
L       [a-zA-Z_]
T       [0-9a-zA-Z_-]
IDENTIFIER {L}+{T}*

%{

#include <stdio.h>
#include "gdbwire_mi_grammar.h"
#include "gdbwire_mi_pt.h"

/**
 * This macro sets the beginning and ending column position of each
 * token in the pure lexer. No global state is used.
 *
 * The parser can then use this position to determine the location
 * of any token it desires.
 *
 * Currently only the column is stored as the parser only uses
 * the lexer on a line at a time. Currently, the caller of the
 * lexer sets the column position back to 1 each time a new
 * line is set to be parsed in the lexer.
 */
#define YY_USER_ACTION \
    { \
    struct gdbwire_mi_position pos = { yycolumn, yycolumn+yyleng-1 }; \
    yyextra = pos; \
    yycolumn += yyleng; \
    }
%}

%%

"^"                      { return CARROT; }
","                      { return COMMA; }
"+"                      { return ADD_OP; }
"*"                      { return MULT_OP; }
"="                      { return EQUAL_SIGN; }
"~"                      { return TILDA; }
"@"                      { return AT_SYMBOL; }
"&"                      { return AMPERSAND; }
"["                      { return OPEN_BRACKET; }
"]"                      { return CLOSED_BRACKET; }
"{"                      { return OPEN_BRACE; }
"}"                      { return CLOSED_BRACE; }
"("                      { return OPEN_PAREN; }
")"                      { return CLOSED_PAREN; }

\n                      { return NEWLINE; }
\r\n                    { return NEWLINE; }
\r                      { return NEWLINE; }

{DIGIT}+                { return INTEGER_LITERAL; }

[ \t\v\f]               {}
{IDENTIFIER}            { return STRING_LITERAL;    }
.                       { return STRING_LITERAL;    }

\"(\\.|[^\\"])*\"       { return CSTRING; }

%%
