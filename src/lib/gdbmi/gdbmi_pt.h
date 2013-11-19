#ifndef __GDBMI_PT_H__
#define __GDBMI_PT_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * A GDB/MI token.
 *
 * When the client requests information from GDB, it does so in the
 * form of a GDB/MI input command. The client may provide a unique
 * id along with the GDB/MI input command, this is the token.
 *
 * When GDB finally gets around to responding with a GDB/MI output
 * command, it passes back the token that was supplied to it so that
 * the client can associate the GDB/MI output command with the
 * corresponding GDB/MI input command.
 *
 * From the GDB documentation:
 *   Note that for all async output, while the token is allowed by the
 *   grammar and may be output by future versions of gdb for select async
 *   output messages, it is generally omitted. Frontends should treat all
 *   async output as reporting general changes in the state of the target
 *   and there should be no need to associate async output to any prior
 *   command. 
 * In other words, asynchronous output commands will not have the token set.
 */
typedef long gdbmi_token_t;

/**
 * The result records result class.
 *
 * TODO: Describe these more in general.
 */
enum gdbmi_result_class {
    GDBMI_DONE,
    GDBMI_RUNNING,
    GDBMI_CONNECTED,
    GDBMI_ERROR,
    GDBMI_EXIT
};

/**
 * The GDB/MI output command.
 *
 * A GDB/MI output command is the main mechanism in which GDB
 * corresponds with a front end.
 */
struct gdbmi_output {
    /**
     * An optional list of out-of-band records.
     *
     * Will be NULL if there is no list for this output command.
     */
    struct gdbmi_oob_record *oob_record;

    /**
     * An optional result record.
     *
     * Will be NULL if there is no result record for this output command.
     */
    struct gdbmi_result_record *result_record;

    /** The next GDB/MI output command or NULL if none */
    struct gdbmi_output *next;
};

/**
 * The GDB/MI result record in an output command.
 *
 * The result record represents the result data in the GDB/MI output
 * command sent by GDB. This typically contains the content the client
 * was requesting when it sent a GDB/MI input command to GDB.
 */
struct gdbmi_result_record {
    /**
     * The result record token.
     *
     * Please see the documentation for gdbmi_token_t to learn more about this.
     *
     * This value will be zero if the token was ommited in the GDB/MI
     * output command. Otherwise the token will be set to the value
     * the GDB/MI output command has provided. Note: This means that the
     * client should not send GDB/MI input commands with a token of
     * value 0.
     */
    gdbmi_token_t token;

    /** The result records result class. */
    enum gdbmi_result_class result_class;

    /**
     * An optional list of results for this result record.
     *
     * Will be NULL if there is no results for this result record.
     *
     * This is typically where the result data is that the client
     * is looking for.
     */
    struct gdbmi_result *result;
};

/** The out of band record kinds. */
enum gdbmi_oob_record_kind {
    /**
     * An asyncronous out of band record.
     *
     * An asyncronous record occurs when GDB would like to update the
     * client with information that it has not asked for.
     *
     * For instance, if the inferior has stopped, or a new thread has
     * started.
     */
    GDBMI_ASYNC,

    /**
     * A stream out of band record.
     *
     * This is the result of normal output from the console, target or GDB.
     */
    GDBMI_STREAM
};

/* This is an out of band record.  */
struct gdbmi_oob_record {
    /** The kind of out of band record. */
    enum gdbmi_oob_record_kind kind;

    union {
        /** When kind == GDBMI_ASYNC. */
        struct gdbmi_async_record *async_record;
        /** When kind == GDBMI_STREAM. */
        struct gdbmi_stream_record *stream_record;
    } variant;

    /** The next gdbmi out of band record or NULL if none. */
    struct gdbmi_oob_record *next;
};

/** The asynchronous out of band record kinds */
enum gdbmi_async_record_kind {
    /**
     * The asynchronous status record kind.
     *
     * Contains on-going status information about the progress of a slow
     * operation. It can be discarded.
     */
    GDBMI_STATUS,

    /**
     * The asynchronous exec record kind.
     *
     * Contains asynchronous state change regarding the target:
     *  (stopped, started, disappeared).
     */
    GDBMI_EXEC,

    /**
     * The asyncronous notify record kind.
     *
     * Contains supplementary information that the client should handle 
     * (e.g., a new breakpoint information).
     */
    GDBMI_NOTIFY
};

/** The stream out of band record kinds */
enum gdbmi_stream_record_kind {
    /**
     * The console output.
     *
     * Output that should be displayed as is in the console.
     * It is the textual response to a CLI command.
     */
    GDBMI_CONSOLE,

    /**
     * The target output.
     *
     * Output produced by the target program.
     */
    GDBMI_TARGET,

    /**
     * The GDB log output.
     *
     * Output text coming from GDB's internals. For instance messages 
     * that should be displayed as part of an error log.
     */
    GDBMI_LOG
};

/**
 * The GDB/MI asyncronous class.
 *
 * TODO: Describe these more in general.
 */
enum gdbmi_async_class {

    GDBMI_STOPPED
};

/**
 * The GDB/MI asyncronous record in an output command.
 *
 * An asyncronous record occurs when GDB would like to update the
 * client with information that it has not asked for.
 */
struct gdbmi_async_record {
    /**
     * The result record token.
     *
     * Please see the documentation for gdbmi_token_t to learn more about this.
     *
     * Please note that the GDB/MI manual says that asyncronous records
     * do not currently populate this token on output but reserve the right
     * to do so. For that reason, token here should always have the value
     * zero.
     */
    gdbmi_token_t token;

    /** The kind of asynchronous record. */
    enum gdbmi_async_record_kind kind;

    /**
     * The output associated with this asynchronous record.
     *
     * This will never be NULL.
     * TODO: Make this not a pointer?
     */
    struct gdbmi_async_output *async_output;

};

/**
 * The GDB/MI asyncronous output.
 *
 * This contains the output for the asynchronous record. 
 */
struct gdbmi_async_output {
    /** The asynchronous output's class */
    enum gdbmi_async_class async_class;

    /**
     * An optional list of results for this async output.
     *
     * Will be NULL if there is no results.
     */
    struct gdbmi_result *result;
};

/**
 * A GDB/MI result list.
 *
 * This is one of the key GDB/MI data structures. GDB communicates many
 * of it's values to the front end through this key/value data structure.
 *
 * It is basically a list of key/value pairs, where the key is a
 * variable name and the value expands to a string, a tuple of results or
 * a list of values or results.
 *
 * This can be thought of as a custom json object.
 */
struct gdbmi_result {
    /** The key being described by the value. */
    char *variable;
    /** The value describing the key */
    struct gdbmi_value *value;
    /** The next result or NULL if none */
    struct gdbmi_result *next;
};

/** The GDB/MI value kind */
enum gdbmi_value_kind {
    /** The value is a cstring */
    GDBMI_CSTRING,
    /** The value is a tuple */
    GDBMI_TUPLE,
    /** The value is a list */
    GDBMI_LIST
};

/**
 * A GDB/MI value list.
 */
struct gdbmi_value {
    /** The kind of value this represents. */
    enum gdbmi_value_kind kind;

    union {
        /** When kind is GDBMI_CSTRING */
        char *cstring;
        /** When kind is GDBMI_TUPLE */
        struct gdbmi_tuple *tuple;
        /** When kind is GDBMI_LIST */
        struct gdbmi_list *list;
    } variant;

    /** The next value or NULL if none. */
    struct gdbmi_value *next;
};

/**
 * A GDB/MI tuple.
 */
struct gdbmi_tuple {
    /**
     * An optional list of results for this tuple.
     *
     * Will be NULL if there is no results.
     */
    struct gdbmi_result *result;
};

/** The GDB/MI list kind */
enum gdbmi_list_kind {
    /** The list is a list of values */
    GDBMI_VALUE,
    /** The list is a list of results */
    GDBMI_RESULT
};

/**
 * A GDB/MI list.
 */
struct gdbmi_list {
    /** The kind of list this represents. */
    enum gdbmi_list_kind kind;

    union {
        /** When kind is GDBMI_VALUE */
        struct gdbmi_value *value;
        /** When kind is GDBMI_RESULT */
        struct gdbmi_result *result;
    } variant;

    /** The next list item or NULL if none. */
    struct gdbmi_list *next;
};

/**
 * An out of band GDB/MI stream record.
 *
 * A stream record is intended to provide the front end with information
 * from the console, the target or from GDB itself.
 */
struct gdbmi_stream_record {
    /** The kind of stream record. */
    enum gdbmi_stream_record_kind kind;
    /** The buffer provided in this stream record. */
    char *cstring;
};

/* Print result class  */
int print_gdbmi_result_class(enum gdbmi_result_class param);

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

int print_gdbmi_oob_record_kind(enum gdbmi_oob_record_kind param);

/* Creating, Destroying and printing oob_record  */
struct gdbmi_oob_record *create_gdbmi_oob_record(void);
int destroy_gdbmi_oob_record(struct gdbmi_oob_record *param);
struct gdbmi_oob_record *append_gdbmi_oob_record(struct gdbmi_oob_record *list,
        struct gdbmi_oob_record *item);
int print_gdbmi_oob_record(struct gdbmi_oob_record *param);

int print_gdbmi_async_record_kind(enum gdbmi_async_record_kind param);

int print_gdbmi_stream_record_kind(enum gdbmi_stream_record_kind param);

/* Creating, Destroying and printing async_record  */
struct gdbmi_async_record *create_gdbmi_async_record(void);
int destroy_gdbmi_async_record(struct gdbmi_async_record *param);
int print_gdbmi_async_record(struct gdbmi_async_record *param);

/* Creating, Destroying and printing async_output  */
struct gdbmi_async_output *create_gdbmi_async_output(void);
int destroy_gdbmi_async_output(struct gdbmi_async_output *param);
int print_gdbmi_async_output(struct gdbmi_async_output *param);

int print_gdbmi_async_class(enum gdbmi_async_class param);

int print_gdbmi_value_kind(enum gdbmi_value_kind param);

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

int print_gdbmi_list_kind(enum gdbmi_list_kind param);

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

#ifdef __cplusplus 
}
#endif 

#endif
