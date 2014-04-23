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
     *
     * This output is prepended by the + character.
     */
    GDBMI_STATUS,

    /**
     * The asynchronous exec record kind.
     *
     * Contains asynchronous state change regarding the target:
     *  (stopped, started, disappeared).
     *
     * This output is prepended by the * character.
     */
    GDBMI_EXEC,

    /**
     * The asyncronous notify record kind.
     *
     * Contains supplementary information that the client should handle 
     * (e.g., a new breakpoint information).
     *
     * This output is prepended by the = character.
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
     *
     * This output is prepended by the ~ character.
     */
    GDBMI_CONSOLE,

    /**
     * The target output.
     *
     * Output produced by the target program.
     *
     * This output is prepended by the @ character.
     */
    GDBMI_TARGET,

    /**
     * The GDB log output.
     *
     * Output text coming from GDB's internals. For instance messages 
     * that should be displayed as part of an error log.
     *
     * This output is prepended by the & character.
     */
    GDBMI_LOG
};

/**
 * The GDB/MI asyncronous class.
 *
 * 
 */
enum gdbmi_async_class {
    /**
     * Loading the executable onto the remote target.
     *
     * This occurs if the async record is GDBMI_STATUS.
     */
    GDBMI_ASYNC_DOWNLOAD,

    /**
     * The target has stopped.
     *
     * This occurs if the async record is GDBMI_EXEC.
     */
    GDBMI_ASYNC_STOPPED,

    /**
     * The target is now running.
     *
     * This occurs if the async record is GDBMI_EXEC.
     */
    GDBMI_ASYNC_RUNNING,

    /**
     * Reports that a breakpoint was created.
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * This occurs if the async record is GDBMI_NOTIFY.
     */
    GDBMI_ASYNC_BREAKPOINT_CREATED,

    /// An unsupported async class
    GDBMI_ASYNC_UNSUPPORTED
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
    /** The asynchronous output class */
    enum gdbmi_async_class async_class;

    /**
     * An optional list of results for this async output.
     *
     * Will be NULL if there is no results.
     */
    struct gdbmi_result *result;
};

/** The GDB/MI result kind */
enum gdbmi_result_kind {
    /** The result is a cstring */
    GDBMI_CSTRING,
    /** The result is a tuple */
    GDBMI_TUPLE,
    /** The result is a list */
    GDBMI_LIST
};


/**
 * A GDB/MI result list.
 *
 * This is one of the key GDB/MI data structures. GDB communicates many
 * of it's values to the front end through this key/value data structure.
 *
 * It is basically a list of key/value pairs, where the key is a
 * variable name and the value expands to a string, a tuple of results or
 * a list of results.
 *
 * This can be thought of as a custom json object.
 */
struct gdbmi_result {
    /** The kind of result this represents. */
    enum gdbmi_result_kind kind;

    /** The key being described by the result. */
    char *variable;

    union {
        /** When kind is GDBMI_CSTRING */
        char *cstring;
        /**
         * When kind is GDBMI_TUPLE or GDBMI_LIST.
         *
         * If kind is GDBMI_TUPLE, each result in the tuple should have a
         * valid key according to the GDB/MI specification. That is, for
         * each result, result->variable should not be NULL.
         *
         * If kind is GDBMI_LIST, the GDB/MI specification allows results in
         * this list to not have keys. That is, for each result,
         * result->variable may be NULL.
         *
         * Will be NULL if the tuple or list is empty.
         */
        struct gdbmi_result *result;
    } variant;

    /** The next result or NULL if none */
    struct gdbmi_result *next;
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

void gdbmi_output_free(struct gdbmi_output *param);

struct gdbmi_output *append_gdbmi_output(struct gdbmi_output *list,
        struct gdbmi_output *item);

struct gdbmi_result *append_gdbmi_result(struct gdbmi_result *list,
        struct gdbmi_result *item);

struct gdbmi_oob_record *append_gdbmi_oob_record(struct gdbmi_oob_record *list,
        struct gdbmi_oob_record *item);

#ifdef __cplusplus 
}
#endif 

#endif
