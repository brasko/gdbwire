%name-prefix="gdbmi_"
%define api.pure
%define api.push_pull "push"
%defines
%code requires { struct gdbmi_parser; }
%parse-param { struct gdbmi_parser *gdbmi_parser }

%{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "src/lib/gdbmi/gdbmi_parser.h"
#include "src/lib/gdbmi/gdbmi_pt.h"
#include "src/lib/gdbmi/gdbmi_pt_alloc.h"

extern char *gdbmi_text;
extern int gdbmi_lex (void);
extern int gdbmi_lineno;

/* The below functions are defined in gdbmi_parser.c. It is not desirable
   to move them into the header file because general users of the parser
   should not be accessing the output command directory. */
extern struct gdbmi_parser_callbacks
    gdbmi_parser_get_callbacks(struct gdbmi_parser *parser);

void gdbmi_error (struct gdbmi_parser *gdbmi_parser, const char *s)
{ 
  fprintf (stderr, "%s:%d Error %s", __FILE__, __LINE__, s);
  if (strcmp (gdbmi_text, "\n") == 0)
    fprintf (stderr, "%s:%d at end of line %d\n", __FILE__, __LINE__, 
	     gdbmi_lineno);
  else 
    {
      fprintf (stderr, "%s:%d at token(%s), line (%d)\n", __FILE__, __LINE__, 
	       gdbmi_text, gdbmi_lineno );
      gdbmi_lex();
      fprintf (stderr, "%s:%d before (%s)\n", __FILE__, __LINE__, gdbmi_text);
    }
}
%}

%token OPEN_BRACE	/* { */
%token CLOSED_BRACE 	/* } */
%token OPEN_PAREN	/* ( */
%token CLOSED_PAREN 	/* ) */
%token ADD_OP		/* + */
%token MULT_OP		/* * */
%token EQUAL_SIGN	/* = */
%token TILDA		/* ~ */
%token AT_SYMBOL	/* @ */
%token AMPERSAND	/* & */
%token OPEN_BRACKET 	/* [ */
%token CLOSED_BRACKET 	/* ] */
%token NEWLINE		/* \n \r\n \r */
%token INTEGER_LITERAL 	/* A number 1234 */
%token STRING_LITERAL 	/* A string literal */
%token CSTRING 		/* "a string like \" this " */
%token COMMA		/* , */
%token CARROT		/* ^ */

%union {
  struct gdbmi_output *u_output;
  struct gdbmi_oob_record *u_oob_record;
  struct gdbmi_result_record *u_result_record;
  int u_result_class;
  int u_async_record_kind;
  struct gdbmi_result *u_result;
  long u_token;
  struct gdbmi_async_record *u_async_record;
  struct gdbmi_async_output *u_async_output;
  struct gdbmi_stream_record *u_stream_record;
  int u_async_class;
  char *u_variable;
  struct gdbmi_result *u_tuple;
  struct gdbmi_result *u_list;
  int u_stream_record_kind;
}

%type <u_output> output
%type <u_oob_record> oob_record
%type <u_oob_record> oob_record_list
%type <u_result_record> opt_result_record
%type <u_result_record> result_record
%type <u_result_class> result_class
%type <u_async_record_kind> async_record_class
%type <u_variable> opt_variable
%type <u_result> result_list
%type <u_result> result
%type <u_token> opt_token
%type <u_token> token
%type <u_async_record> async_record
%type <u_async_output> async_output
%type <u_stream_record> stream_record
%type <u_async_class> async_class
%type <u_variable> variable
%type <u_tuple> tuple
%type <u_list> list
%type <u_stream_record_kind> stream_record_class

%start output_list
%%

output_list: {
};

output_list: output_list output {
};

output: oob_record_list opt_result_record OPEN_PAREN variable CLOSED_PAREN NEWLINE { 
  struct gdbmi_parser_callbacks callbacks =
      gdbmi_parser_get_callbacks(gdbmi_parser);

  $$ = gdbmi_output_alloc();
  $$->oob_record = $1;
  $$->result_record = $2;

  if (strcmp ("gdb", $4) != 0)
    gdbmi_error (gdbmi_parser, "Syntax error, expected 'gdb'");

  free ($4);

  callbacks.gdbmi_output_callback(callbacks.context, $$);
} ;

oob_record_list: {
  $$ = NULL;
};

oob_record_list: oob_record_list oob_record NEWLINE {
  $$ = append_gdbmi_oob_record ($1, $2);
};

opt_result_record: {
  $$ = NULL;
};

opt_result_record: result_record NEWLINE {
  $$ = $1;
};

result_record: opt_token CARROT result_class result_list {
  $$ = gdbmi_result_record_alloc();
  $$->token = $1;
  $$->result_class = $3;
  $$->result = $4;
};

oob_record: async_record {
  $$ = gdbmi_oob_record_alloc();
  $$->kind = GDBMI_ASYNC;
  $$->variant.async_record = $1;
};

oob_record: stream_record {
  $$ = gdbmi_oob_record_alloc();
  $$->kind = GDBMI_STREAM;
  $$->variant.stream_record = $1;
};

async_record: opt_token async_record_class async_output {
  $$ = gdbmi_async_record_alloc();
  $$->token = $1;
  $$->kind = $2;
  $$->async_output = $3;
};

async_output: async_class result_list {
  $$ = gdbmi_async_output_alloc();
  $$->async_class = $1;
  $$->result = $2;
};

async_record_class: MULT_OP {
  $$ = GDBMI_EXEC;
};

async_record_class: ADD_OP {
  $$ = GDBMI_STATUS;
};

async_record_class: EQUAL_SIGN {
  $$ = GDBMI_NOTIFY;	
};

result_class: STRING_LITERAL {
  if (strcmp ("done", gdbmi_text) == 0)
    $$ = GDBMI_DONE;
  else if (strcmp ("running", gdbmi_text) == 0)
    $$ = GDBMI_RUNNING;
  else if (strcmp ("connected", gdbmi_text) == 0)
    $$ = GDBMI_CONNECTED;
  else if (strcmp ("error", gdbmi_text) == 0)
    $$ = GDBMI_ERROR;
  else if (strcmp ("exit", gdbmi_text) == 0)
    $$ = GDBMI_EXIT;
  else
    gdbmi_error (gdbmi_parser, "Syntax error, expected 'done|running|connected|error|exit");
};

async_class: STRING_LITERAL {
  if (strcmp("stopped", gdbmi_text) == 0) {
      $$ = GDBMI_ASYNC_STOPPED;
  } else if (strcmp("running", gdbmi_text) == 0) {
      $$ = GDBMI_ASYNC_RUNNING;
  } else if (strcmp("download", gdbmi_text) == 0) {
      $$ = GDBMI_ASYNC_DOWNLOAD;
  } else {
      $$ = GDBMI_ASYNC_UNSUPPORTED;
  }
};

opt_variable: {
    $$ = 0;
}

opt_variable: variable EQUAL_SIGN {
    $$ = $1;
}

result_list: {
  $$ = NULL;
};

result_list: result_list COMMA result {
  $$ = append_gdbmi_result ($1, $3);
};

result: opt_variable CSTRING {
  $$ = gdbmi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBMI_CSTRING;
  $$->variant.cstring = strdup(gdbmi_text); 
};

result: opt_variable tuple {
  $$ = gdbmi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBMI_TUPLE;
  $$->variant.result = $2;
};

result: opt_variable list {
  $$ = gdbmi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBMI_LIST;
  $$->variant.result = $2;
};

variable: STRING_LITERAL {
  $$ = strdup (gdbmi_text);
};

tuple: OPEN_BRACE CLOSED_BRACE {
  $$ = NULL;
};

tuple: OPEN_BRACE result result_list CLOSED_BRACE {
  $$ = append_gdbmi_result($2, $3);
};

list: OPEN_BRACKET CLOSED_BRACKET {
  $$ = NULL;
};

list: OPEN_BRACKET result result_list CLOSED_BRACKET {
  $$ = append_gdbmi_result($2, $3);
};

stream_record: stream_record_class CSTRING {
  $$ = gdbmi_stream_record_alloc();
  $$->kind = $1;
  $$->cstring = strdup ( gdbmi_text );
};

stream_record_class: TILDA {
  $$ = GDBMI_CONSOLE;
};

stream_record_class: AT_SYMBOL {
  $$ = GDBMI_TARGET;
};

stream_record_class: AMPERSAND {
  $$ = GDBMI_LOG;
};

opt_token: {
  $$ = -1;	
};

opt_token: token {
  $$ = $1;
};

token: INTEGER_LITERAL {
  $$ = atol (gdbmi_text);
};
