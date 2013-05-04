#ifndef __GDBMI_PT_H__
#define __GDBMI_PT_H__

typedef long gdbmi_token_t;

struct gdbmi_pdata {
    int parsed_one;
    struct gdbmi_output *tree;
};

/* A choice of result's that GDB is capable of producing  */
enum gdbmi_result_class {
    GDBMI_DONE,
    GDBMI_RUNNING,
    GDBMI_CONNECTED,
    GDBMI_ERROR,
    GDBMI_EXIT
};

/* This is the root of a parsed GDB/MI Output command.  */
struct gdbmi_output {
    /* Every output command has a list of optional oob_record's.  This will be 
       the head of the list, otherwise NULL.  */
    struct gdbmi_oob_record *oob_record;

    /* Every output command has an optional result_record list, or NULL.  */
    struct gdbmi_result_record *result_record;

    /* A pointer to the next output  */
    struct gdbmi_output *next;
};

/* A result record represents the result of a command sent to GDB.  */
struct gdbmi_result_record {
    /* This is the unique identifier that was passed to GDB when asking for a 
       command to be done on behalf of the front end  */
    gdbmi_token_t token;

    /* The choice of result that this class represents  */
    enum gdbmi_result_class result_class;

    /* The results of the command  */
    struct gdbmi_result *result;
};

/* There are several kinds of output that GDB can send  */
enum gdbmi_oob_record_choice {
    /* GDBMI_ASYNC
       I believe that the asyncronous records contain data that was not asked
       for by the front end. An asyncronous event occured within the inferior
       or GDB and GDB needs to update the front end.

       For instance, I believe this is useful for when breakpoints are modified, 
       instead of having the front end ask if the breakpoints were modified 
       every time.  */
    GDBMI_ASYNC,
    /* GDBMI_STREAM
       This is the result of normal output from the console, target or GDB.  */
    GDBMI_STREAM
};

/* This is an out of band record.  */
struct gdbmi_oob_record {
    /* This is the choice of oob_record  */
    enum gdbmi_oob_record_choice record;
    union {
        /* If it's an GDBMI_ASYNC record  */
        struct gdbmi_async_record *async_record;
        /* If it's an GDBMI_STREAM record  */
        struct gdbmi_stream_record *stream_record;
    } option;

    /* A pointer to the next oob_record  */
    struct gdbmi_oob_record *next;
};

/* This represents each choice of asyncronous record GDB is capable of 
   sending.  */
enum gdbmi_async_record_choice {
    /* Contains on-going status information about the progress of a slow 
       operation. It can be discarded.  */
    GDBMI_STATUS,
    /* Contains asynchronous state change on the target (stopped, started, 
       disappeared).  */
    GDBMI_EXEC,

    /* Contains supplementary information that the client should handle 
       (e.g., a new breakpoint information).  */
    GDBMI_NOTIFY
};

/* This represents each choice of stream record GDB is capable of sending.  */
enum gdbmi_stream_record_choice {
    /* Output that should be displayed as is in the console. It is the textual 
       response to a CLI command.  */
    GDBMI_CONSOLE,
    /* Output produced by the target program.  */
    GDBMI_TARGET,
    /* Output text coming from GDB's internals, for instance messages 
     * that should be displayed as part of an error log.  */
    GDBMI_LOG
};

enum gdbmi_async_class {
    GDBMI_STOPPED
};

/* An asyncronous record  */
struct gdbmi_async_record {
    /* This is the unique identifier that was passed to GDB when asking for a 
       command to be done on behalf of the front end.  */
    gdbmi_token_t token;

    /* The choice of asyncronous record this represents  */
    enum gdbmi_async_record_choice async_record;

    enum gdbmi_async_class async_class;

    struct gdbmi_result *result;
};

/* The result from GDB. This is a linked list. If the result is a key/value 
   pair, then 'variable' is the key and 'value' is the value.  */
struct gdbmi_result {
    /* Key  */
    char *variable;
    /* Value  */
    struct gdbmi_value *value;
    /* Pointer to the next result  */
    struct gdbmi_result *next;
};

enum gdbmi_value_choice {
    GDBMI_CSTRING,
    GDBMI_TUPLE,
    GDBMI_LIST
};

struct gdbmi_value {
    enum gdbmi_value_choice value_choice;

    union {
        char *cstring;
        struct gdbmi_tuple *tuple;
        struct gdbmi_list *list;
    } option;

    struct gdbmi_value *next;
};

struct gdbmi_tuple {
    struct gdbmi_result *result;
    struct gdbmi_tuple *next;
};

enum gdbmi_list_choice {
    GDBMI_VALUE,
    GDBMI_RESULT
};

struct gdbmi_list {
    enum gdbmi_list_choice list_choice;

    union {
        struct gdbmi_value *value;
        struct gdbmi_result *result;
    } option;

    struct gdbmi_list *next;
};

struct gdbmi_stream_record {
    enum gdbmi_stream_record_choice stream_record;
    char *cstring;
};

/* Print result class  */
int print_gdbmi_result_class(enum gdbmi_result_class param);

/* Creating and  Destroying */
struct gdbmi_pdata *create_gdbmi_pdata(void);
int destroy_gdbmi_pdata(struct gdbmi_pdata *param);

/* Creating, Destroying and printing output  */
struct gdbmi_output *create_gdbmi_output(void);
int destroy_gdbmi_output(struct gdbmi_output *param);
struct gdbmi_output *append_gdbmi_output(struct gdbmi_output *list,
        struct gdbmi_output *item);
int print_gdbmi_output(struct gdbmi_output *param);

/* Creating, Destroying and printing record  */
struct gdbmi_result_record *create_gdbmi_result_record(void);
int destroy_gdbmi_result_record(struct gdbmi_result_record *param);
int print_gdbmi_result_record(struct gdbmi_result_record *param);

/* Creating, Destroying and printing result  */
struct gdbmi_result *create_gdbmi_result(void);
int destroy_gdbmi_result(struct gdbmi_result *param);
struct gdbmi_result *append_gdbmi_result(struct gdbmi_result *list,
        struct gdbmi_result *item);
int print_gdbmi_result(struct gdbmi_result *param);

int print_gdbmi_oob_record_choice(enum gdbmi_oob_record_choice param);

/* Creating, Destroying and printing oob_record  */
struct gdbmi_oob_record *create_gdbmi_oob_record(void);
int destroy_gdbmi_oob_record(struct gdbmi_oob_record *param);
struct gdbmi_oob_record *append_gdbmi_oob_record(struct gdbmi_oob_record *list,
        struct gdbmi_oob_record *item);
int print_gdbmi_oob_record(struct gdbmi_oob_record *param);

int print_gdbmi_async_record_choice(enum gdbmi_async_record_choice param);

int print_gdbmi_stream_record_choice(enum gdbmi_stream_record_choice param);

/* Creating, Destroying and printing async_record  */
struct gdbmi_async_record *create_gdbmi_async_record(void);
int destroy_gdbmi_async_record(struct gdbmi_async_record *param);
int print_gdbmi_async_record(struct gdbmi_async_record *param);

int print_gdbmi_async_class(enum gdbmi_async_class param);

int print_gdbmi_value_choice(enum gdbmi_value_choice param);

/* Creating, Destroying and printing value  */
struct gdbmi_value *create_gdbmi_value(void);
int destroy_gdbmi_value(struct gdbmi_value *param);
struct gdbmi_value *append_gdbmi_value(struct gdbmi_value *list,
        struct gdbmi_value *item);
int print_gdbmi_value(struct gdbmi_value *param);

/* Creating, Destroying and printing tuple  */
struct gdbmi_tuple *create_gdbmi_tuple(void);
int destroy_gdbmi_tuple(struct gdbmi_tuple *param);
int print_gdbmi_tuple(struct gdbmi_tuple *param);

int print_gdbmi_list_choice(enum gdbmi_list_choice param);

/* Creating, Destroying and printing list  */
struct gdbmi_list *create_gdbmi_list(void);
int destroy_gdbmi_list(struct gdbmi_list *param);
struct gdbmi_list *append_gdbmi_list(struct gdbmi_list *list,
        struct gdbmi_list *item);
int print_gdbmi_list(struct gdbmi_list *param);

/* Creating, Destroying and printing stream_record  */
struct gdbmi_stream_record *create_gdbmi_stream_record(void);
int destroy_gdbmi_stream_record(struct gdbmi_stream_record *param);
int print_gdbmi_stream_record(struct gdbmi_stream_record *param);

#endif
