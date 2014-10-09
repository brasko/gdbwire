#ifndef __GDBMI_PT_H__
#define __GDBMI_PT_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * The position of a token in a GDB/MI line.
 *
 * Note that a string in C is zero based and the token column
 * position is 1 based. For example,
 *   char *str = "hello world";
 * The "hello" token would have a start_column as 1 and an end
 * column as 5.
 *
 * The start_column and end_column will be the same column number for
 * a token of size 1.
 */
struct gdbmi_position {
    /// The starting column position of the token
    int start_column;
    /// The ending column position of the token
    int end_column;
};

/** The gdbmi output kinds. */
enum gdbmi_output_kind {
    /**
     * The GDB/MI output contains an out of band record.
     *
     * The out of band record is not necessarily associated with any
     * particular GDB/MI input command.
     */
    GDBMI_OUTPUT_OOB,

    /**
     * The GDB/MI output contains a gdbmi result record.
     *
     * This record typically contains the result data from a request
     * made by the client in a previous GDB/MI input command.
     */
    GDBMI_OUTPUT_RESULT,

    /**
     * The GDB/MI output represents a prompt. (ie. (gdb) )
     * 
     * TODO: Document when GDB is ready to receive a command. Only if
     * the prompt is received and at *stopped?
     */
    GDBMI_OUTPUT_PROMPT,

    /**
     * A parse error occurred.
     */
    GDBMI_OUTPUT_PARSE_ERROR
};

/**
 * The GDB/MI output command.
 *
 * A GDB/MI output command is the main mechanism in which GDB
 * corresponds with a front end.
 */
struct gdbmi_output {
    enum gdbmi_output_kind kind;

    union {
        /** When kind == GDBMI_OUTPUT_OOB, never NULL. */
        struct gdbmi_oob_record *oob_record;
        /** When kind == GDBMI_OUTPUT_RESULT, never NULL. */
        struct gdbmi_result_record *result_record;
        /** When kind == GDBMI_OUTPUT_PARSE_ERROR, never NULL. */
        struct {
            /** The token the error occurred on */
            char *token;
            /** The position of the token where the error occurred. */
            struct gdbmi_position pos;
        } error;
    } variant;

    /**
     * The GDB/MI output line that was used to create this output instance.
     *
     * Each gdbmi output structure is created from exactly one line of
     * MI output from GDB. This field represents the line that created 
     * this particular output structure.
     *
     * This field is always available and never NULL, even for a parse error.
     */
    char *line;

    /** The next GDB/MI output command or NULL if none */
    struct gdbmi_output *next;
};

/**
 * A GDB/MI token.
 *
 * A string made up of one or more digits.
 * The regular expression [0-9]+ will match this types contents.
 */
typedef char *gdbmi_token_t;

/**
 * A GDB/MI output command may contain one of the following result indications.
 */
enum gdbmi_result_class {
    /**
     * The synchronous operation was successful (^done).
     */
    GDBMI_DONE,

    /**
     * Equivalent to GDBMI_DONE (^running).
     *
     * Historically, was output by GDB instead of ^done if the command
     * resumed the target.
     *
     * Do not rely on or use this result class in the front end to determine
     * the state of the target. Use the async *running output record to
     * determine which threads have resumed running.
     *
     * TODO: Ensure that early versions of GDB can depend on the async
     * *running or if front ends DO have to rely on ^running.
     */
    GDBMI_RUNNING,

    /**
     * GDB has connected to a remote target (^connected).
     *
     * This is in response to the -target-select command.
     *
     * A comment in the GDB source code says,
     *   There's no particularly good reason why target-connect results
     *   in not ^done.  Should kill ^connected for MI3.
     *
     * With this in mind, it makes sense to assume that GDBMI_CONNECTED and
     * GDBMI_DONE are equivalent.
     */
    GDBMI_CONNECTED,

    /**
     * An error has occurred (^error).
     *
     * This can occur if the user provides an improper command to GDB.
     * In this case, the user will be provided the standard error output but
     * the front end will also be provided this information independently.
     */
    GDBMI_ERROR,

    /**
     * GDB has terminated (^exit).
     *
     * When GDB knows it is about to exit, it provides this notification
     * in the GDB/MI output command. However, on all other circumstances,
     * the front end should be prepared to have GDB exit and not provide
     * this information.
     */
    GDBMI_EXIT,

    /// An unsupported result class
    GDBMI_UNSUPPORTED
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
     * The token associated with the corresponding GDB/MI input command.
     *
     * The client may provide a unique string of digits at the beginning of a
     * GDB/MI input command. For example,
     *   0000-foo
     * When GDB finally gets around to responding to the GDB/MI input command,
     * it takes the token provided in the input command and puts it into the
     * result record of the corresponding GDB/MI output command. For
     * example, the output commmand associated with the above input command is,
     *   0000^error,msg="Undefined MI command: foo",code="undefined-command"
     * and the result record would have the below token field set to "0000".
     *
     * This is intended to allow the front end to correlate the GDB/MI input
     * command it sent with the GDB/MI output command GDB responded with.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
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
     * This was undocumented in the GDB manual as far as GDB 7.7.
     *
     * This occurs if the async record is GDBMI_STATUS as +download.
     */
    GDBMI_ASYNC_DOWNLOAD,

    /**
     * The target has stopped.
     *
     * This occurs if the async record is GDBMI_EXEC as *stopped.
     */
    GDBMI_ASYNC_STOPPED,

    /**
     * The target is now running.
     *
     * This occurs if the async record is GDBMI_EXEC as *running.
     */
    GDBMI_ASYNC_RUNNING,

    /**
     * Reports that a thread group was added.
     *
     * When a thread group is added, it generally might not be associated
     * with a running process.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-group-added.
     */
    GDBMI_ASYNC_THREAD_GROUP_ADDED,

    /**
     * Reports that a thread group was removed.
     *
     * When a thread group is removed, its id becomes invalid and cannot be
     * used in any way. 
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-group-removed.
     */
    GDBMI_ASYNC_THREAD_GROUP_REMOVED,

    /**
     * Reports that a thread group was started.
     *
     * A thread group became associated with a running program.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-group-started.
     */
    GDBMI_ASYNC_THREAD_GROUP_STARTED,

    /**
     * Reports that a thread group was exited.
     *
     * A thread group is no longer associated with a running program.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-group-exited.
     */
    GDBMI_ASYNC_THREAD_GROUP_EXITED,

    /**
     * Reports that a thread was created.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-created.
     */
    GDBMI_ASYNC_THREAD_CREATED,

    /**
     * Reports that a thread was exited.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-exited.
     */
    GDBMI_ASYNC_THREAD_EXITED,

    /**
     * Reports that a thread was selected.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =thread-selected.
     */
    GDBMI_ASYNC_THREAD_SELECTED,

    /**
     * Reports that a new library was loaded.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =library-loaded.
     */
    GDBMI_ASYNC_LIBRARY_LOADED,

    /**
     * Reports that a new library was unloaded.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =library-unloaded.
     */
    GDBMI_ASYNC_LIBRARY_UNLOADED,

    /**
     * Reports that a trace frame was changed.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =traceframe-changed.
     */
    GDBMI_ASYNC_TRACEFRAME_CHANGED,

    /**
     * Reports that a trace state variable was created.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =tsv-created.
     */
    GDBMI_ASYNC_TSV_CREATED,

    /**
     * Reports that a trace state variable was modified.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =tsv-modified.
     */
    GDBMI_ASYNC_TSV_MODIFIED,

    /**
     * Reports that a trace state variable was deleted.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =tsv-deleted.
     */
    GDBMI_ASYNC_TSV_DELETED,

    /**
     * Reports that a breakpoint was created.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBMI_NOTIFY as =breakpoint-created.
     */
    GDBMI_ASYNC_BREAKPOINT_CREATED,

    /**
     * Reports that a breakpoint was modified.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBMI_NOTIFY as =breakpoint-modified.
     */
    GDBMI_ASYNC_BREAKPOINT_MODIFIED,

    /**
     * Reports that a breakpoint was deleted.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBMI_NOTIFY as =breakpoint-deleted.
     */
    GDBMI_ASYNC_BREAKPOINT_DELETED,

    /**
     * Reports that execution log recording was started on an inferior.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =record-started.
     */
    GDBMI_ASYNC_RECORD_STARTED,

    /**
     * Reports that execution log recording was stopped on an inferior.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =record-stopped.
     */
    GDBMI_ASYNC_RECORD_STOPPED,

    /**
     * Reports that a parameter of the command set param is changed to value.
     *
     * For example, when the user runs a command like 'set print pretty on',
     * this async command will be invoked with the parameter reported as
     * 'print pretty' and the value as 'on'.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =cmd-param-changed.
     */
    GDBMI_ASYNC_CMD_PARAM_CHANGED,

    /**
     * Reports that bytes from addr to data + len were written in an inferior.
     *
     * This occurs if the async record is GDBMI_NOTIFY as =memory-changed.
     */
    GDBMI_ASYNC_MEMORY_CHANGED,

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
     * Please note that the GDB/MI manual says that asyncronous records
     * do not currently populate this token on output but reserve the right
     * to do so. For that reason, token here should always be NULL.
     *
     * From the GDB documentation:
     *   Note that for all async output, while the token is allowed by the
     *   grammar and may be output by future versions of gdb for select async
     *   output messages, it is generally omitted. Frontends should treat all
     *   async output as reporting general changes in the state of the target
     *   and there should be no need to associate async output to any prior
     *   command. 
     *
     * After further investigation, I determined that newer GDB's will no
     * longer ever output this information. Older GDB's will. The commit
     * that made this change in GDB is 721c02de on April 24th, 2008.
     * The next GDB that was released was on October 6th, 2009, version 7.0.
     *
     * Before the above mentioned commit async *stopped commands would
     * sometimes output the token associated with the last token provided in
     * a GDB/MI input command. After that change, the token is never
     * associated with an async output command, even though the
     * documentation says it might be.
     *
     * Finally, even before that change when the token was output in the
     * async *stopped command, the developers of GDB felt that it was not
     * useful and should be avoided by front ends.
     *
     * With this information, I've determined that front ends should never
     * use this value to determine logic. However, the value is parsed in
     * order to accurately handle and represent the cases where this value
     * occurs.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
     */
    gdbmi_token_t token;

    /** The kind of asynchronous record. */
    enum gdbmi_async_record_kind kind;

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
 * This is one of the important GDB/MI data structures. GDB communicates many
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

#ifdef __cplusplus 
}
#endif 

#endif
