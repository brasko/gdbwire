%name-prefix "gdbwire_mi_"
%define api.pure
%define api.push_pull "push"
%defines
%code requires {
    typedef void *yyscan_t;
    struct gdbwire_mi_output;
}
%parse-param {yyscan_t yyscanner}
%parse-param {struct gdbwire_mi_output **gdbwire_mi_output}

%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gdbwire_mi_grammar.h"
#include "gdbwire_mi_pt.h"
#include "gdbwire_mi_pt_alloc.h"

char *gdbwire_mi_get_text(yyscan_t yyscanner);
struct gdbwire_mi_position gdbwire_mi_get_extra(yyscan_t yyscanner);

void gdbwire_mi_error(yyscan_t yyscanner,
    struct gdbwire_mi_output **gdbwire_mi_output, const char *s)
{ 
    char *text = gdbwire_mi_get_text(yyscanner);
    struct gdbwire_mi_position pos = gdbwire_mi_get_extra(yyscanner);

    *gdbwire_mi_output = gdbwire_mi_output_alloc();
    (*gdbwire_mi_output)->kind = GDBWIRE_MI_OUTPUT_PARSE_ERROR;
    (*gdbwire_mi_output)->variant.error.token = strdup(text);
    (*gdbwire_mi_output)->variant.error.pos = pos;
}

/**
 * GDB/MI escapes characters in the c-string rule.
 *
 * The c-string starts and ends with a ".
 * Each " in the c-string is escaped with a \. So GDB turns " into \".
 * Each \ in the string is then escaped with a \. So GDB turns \ into \\.
 *
 * Remove the GDB/MI escape characters to provide back to the user the
 * original characters that GDB was intending to transmit. So
 *   \" -> "
 *   \\ -> \
 *   \n -> new line
 *   \r -> carriage return
 *   \t -> tab
 *
 * See gdbwire_mi_grammar.txt (GDB/MI Clarifications) for more information.
 *
 * @param str
 * The escaped GDB/MI c-string data.
 *
 * @return
 * An allocated strng representing str with the escaping undone.
 */
static char *gdbwire_mi_unescape_cstring(char *str)
{
    char *result;
    size_t r, s, length;

    //assert(str);

    result = strdup(str);
    length = strlen(str);

    /* a CSTRING should start and end with a quote */
    //assert(result);
    //assert(length >= 2);

    for (r = 0, s = 1; s < length - 1; ++s) {
        if (str[s] == '\\') {
            switch (str[s+1]) {
                case 'n':
                    result[r++] = '\n';
                    ++s;
                    break;
                case 'r':
                    result[r++] = '\r';
                    ++s;
                    break;
                case 't':
                    result[r++] = '\t';
                    ++s;
                    break;
                case '"':
                    break;
                case '\\':
                    break;
                default:
                    result[r++] = str[s];
                    break;
            }
        } else {
            result[r++] = str[s];
        }
    }

    result[r] = 0;

    return result;
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
  struct gdbwire_mi_output *u_output;
  struct gdbwire_mi_oob_record *u_oob_record;
  struct gdbwire_mi_result_record *u_result_record;
  int u_result_class;
  int u_async_record_kind;
  struct gdbwire_mi_result *u_result;
  char *u_token;
  struct gdbwire_mi_async_record *u_async_record;
  struct gdbwire_mi_stream_record *u_stream_record;
  int u_async_class;
  char *u_variable;
  char *u_cstring;
  struct gdbwire_mi_result *u_tuple;
  struct gdbwire_mi_result *u_list;
  int u_stream_record_kind;
}

%type <u_output> output
%type <u_output> output_variant
%type <u_oob_record> oob_record
%type <u_result_record> result_record
%type <u_result_class> result_class
%type <u_async_record_kind> async_record_class
%type <u_variable> opt_variable variable
%type <u_result> result_list
%type <u_result> result
%type <u_token> opt_token token
%type <u_async_record> async_record
%type <u_stream_record> stream_record
%type <u_async_class> async_class
%type <u_cstring> cstring
%type <u_tuple> tuple
%type <u_list> list
%type <u_stream_record_kind> stream_record_class

/** 
 * Some symbols in the grammar require destructor directives to free
 * memory when bison goes into error recovery mode. The below symbols
 * are only used as the right most symbol in a rule, making them
 * not possible to be allocated and then later determined to be
 * unnecessary through error recory. The destructor directive for
 * these are omitted as adding them adds dead code.
 *
 * result_record
 * oob_record
 * async_record
 * cstring
 * tuple
 * list
 * stream_record
 * token
 */
%destructor { gdbwire_mi_output_free($$); } output_variant
%destructor { gdbwire_mi_result_free($$); } result_list
%destructor { gdbwire_mi_result_free($$); } result
%destructor { free($$); } opt_variable
%destructor { free($$); } variable
%destructor { free($$); } opt_token

%start output_list
%%

output_list: {
};

output_list: output_list output {
};

output: output_variant NEWLINE {
  *gdbwire_mi_output = $1;
};

output: error NEWLINE {
  yyerrok;
};

output_variant: oob_record {
  $$ = gdbwire_mi_output_alloc();
  $$->kind = GDBWIRE_MI_OUTPUT_OOB;
  $$->variant.oob_record = $1;
}

output_variant: result_record {
  $$ = gdbwire_mi_output_alloc();
  $$->kind = GDBWIRE_MI_OUTPUT_RESULT;
  $$->variant.result_record = $1;
}

output_variant: OPEN_PAREN variable {
      if (strcmp("gdb", $2) != 0) {
          /* Destructor will be called to free $2 on error */
          yyerror(yyscanner, gdbwire_mi_output, "");
          YYERROR;
      }
    } CLOSED_PAREN {
      $$ = gdbwire_mi_output_alloc();
      $$->kind = GDBWIRE_MI_OUTPUT_PROMPT;
      free($2);
    }

result_record: opt_token CARROT result_class result_list {
  $$ = gdbwire_mi_result_record_alloc();
  $$->token = $1;
  $$->result_class = $3;
  $$->result = $4;
};

oob_record: async_record {
  $$ = gdbwire_mi_oob_record_alloc();
  $$->kind = GDBWIRE_MI_ASYNC;
  $$->variant.async_record = $1;
};

oob_record: stream_record {
  $$ = gdbwire_mi_oob_record_alloc();
  $$->kind = GDBWIRE_MI_STREAM;
  $$->variant.stream_record = $1;
};

async_record: opt_token async_record_class async_class result_list {
  $$ = gdbwire_mi_async_record_alloc();
  $$->token = $1;
  $$->kind = $2;
  $$->async_class = $3;
  $$->result = $4;
};

async_record_class: MULT_OP {
  $$ = GDBWIRE_MI_EXEC;
};

async_record_class: ADD_OP {
  $$ = GDBWIRE_MI_STATUS;
};

async_record_class: EQUAL_SIGN {
  $$ = GDBWIRE_MI_NOTIFY;	
};

result_class: STRING_LITERAL {
  char *text = gdbwire_mi_get_text(yyscanner);
  if (strcmp("done", text) == 0) {
    $$ = GDBWIRE_MI_DONE;
  } else if (strcmp("running", text) == 0) {
    $$ = GDBWIRE_MI_RUNNING;
  } else if (strcmp("connected", text) == 0) {
    $$ = GDBWIRE_MI_CONNECTED;
  } else if (strcmp("error", text) == 0) {
    $$ = GDBWIRE_MI_ERROR;
  } else if (strcmp("exit", text) == 0) {
    $$ = GDBWIRE_MI_EXIT;
  } else {
    $$ = GDBWIRE_MI_UNSUPPORTED;
  }
};

async_class: STRING_LITERAL {
  char *text = gdbwire_mi_get_text(yyscanner);
  if (strcmp("download", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_DOWNLOAD;
  } else if (strcmp("stopped", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_STOPPED;
  } else if (strcmp("running", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_RUNNING;
  } else if (strcmp("thread-group-added", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_GROUP_ADDED;
  } else if (strcmp("thread-group-removed", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_GROUP_REMOVED;
  } else if (strcmp("thread-group-started", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_GROUP_STARTED;
  } else if (strcmp("thread-group-exited", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_GROUP_EXITED;
  } else if (strcmp("thread-created", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_CREATED;
  } else if (strcmp("thread-exited", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_EXITED;
  } else if (strcmp("thread-selected", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_THREAD_SELECTED;
  } else if (strcmp("library-loaded", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_LIBRARY_LOADED;
  } else if (strcmp("library-unloaded", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_LIBRARY_UNLOADED;
  } else if (strcmp("traceframe-changed", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_TRACEFRAME_CHANGED;
  } else if (strcmp("tsv-created", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_TSV_CREATED;
  } else if (strcmp("tsv-modified", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_TSV_MODIFIED;
  } else if (strcmp("tsv-deleted", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_TSV_DELETED;
  } else if (strcmp("breakpoint-created", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED;
  } else if (strcmp("breakpoint-modified", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_BREAKPOINT_MODIFIED;
  } else if (strcmp("breakpoint-deleted", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_BREAKPOINT_DELETED;
  } else if (strcmp("record-started", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_RECORD_STARTED;
  } else if (strcmp("record-stopped", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_RECORD_STOPPED;
  } else if (strcmp("cmd-param-changed", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_CMD_PARAM_CHANGED;
  } else if (strcmp("memory-changed", text) == 0) {
      $$ = GDBWIRE_MI_ASYNC_MEMORY_CHANGED;
  } else {
      $$ = GDBWIRE_MI_ASYNC_UNSUPPORTED;
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
  $$ = append_gdbwire_mi_result ($1, $3);
};

result: opt_variable cstring {
  $$ = gdbwire_mi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBWIRE_MI_CSTRING;
  $$->variant.cstring = $2;
};

result: opt_variable tuple {
  $$ = gdbwire_mi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBWIRE_MI_TUPLE;
  $$->variant.result = $2;
};

result: opt_variable list {
  $$ = gdbwire_mi_result_alloc();
  $$->variable = $1;
  $$->kind = GDBWIRE_MI_LIST;
  $$->variant.result = $2;
};

variable: STRING_LITERAL {
  char *text = gdbwire_mi_get_text(yyscanner);
  $$ = strdup(text);
};

cstring: CSTRING {
  char *text = gdbwire_mi_get_text(yyscanner);
  $$ = gdbwire_mi_unescape_cstring(text);
};

tuple: OPEN_BRACE CLOSED_BRACE {
  $$ = NULL;
};

tuple: OPEN_BRACE result result_list CLOSED_BRACE {
    if ($3) {
        $$ = append_gdbwire_mi_result($2, $3);
    } else {
        $$ = $2;
    }
};

list: OPEN_BRACKET CLOSED_BRACKET {
  $$ = NULL;
};

list: OPEN_BRACKET result result_list CLOSED_BRACKET {
    if ($3) {
        $$ = append_gdbwire_mi_result($2, $3);
    } else {
        $$ = $2;
    }
};

stream_record: stream_record_class cstring {
  $$ = gdbwire_mi_stream_record_alloc();
  $$->kind = $1;
  $$->cstring = $2;
};

stream_record_class: TILDA {
  $$ = GDBWIRE_MI_CONSOLE;
};

stream_record_class: AT_SYMBOL {
  $$ = GDBWIRE_MI_TARGET;
};

stream_record_class: AMPERSAND {
  $$ = GDBWIRE_MI_LOG;
};

opt_token: {
  $$ = NULL;	
};

opt_token: token {
  $$ = $1;
};

token: INTEGER_LITERAL {
  char *text = gdbwire_mi_get_text(yyscanner);
  $$ = strdup(text);
};
