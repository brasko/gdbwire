#ifndef GDBWIRE_ANNOTATION_PT_H
#define GDBWIRE_ANNOTATION_PT_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/** The gdbwire_annotation_output kinds.  */
enum gdbwire_annotation_output_kind {
    /**
     * The GDB/Annotation output contains console output.
     *
     * This is console output from gdb, or the inferior if the tty is the same.
     */
    GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT,

    /** 
     * The GDB/Annotation output contains an annotation.
     */
    GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION
};

/** The kind of GDB/Annotation */
enum gdbwire_annotation_kind {
    /**
     * Sent when the breakpoints may have changed.
     *
     * It is recommended to redetermine the breakpoints.
     *
     * breakpoints-invalid
     */
    GDBWIRE_ANNOTATION_BREAKPOINTS_INVALID,

    /**
     * Sent when the source position has changed.
     *
     * It is recommended to show the source location to the user.
     *
     * source
     */
    GDBWIRE_ANNOTATION_SOURCE,

    /**
     * Sent when the frame has changed.
     *
     * It is recommended to show the current source location to the user.
     *
     * frame-end
     */
    GDBWIRE_ANNOTATION_FRAME_END,

    /**
     * Sent when the frames may have changed.
     *
     * It is recommended to redetermine the current source location.
     *
     * frames-invalid
     */
    GDBWIRE_ANNOTATION_FRAMES_INVALID,

    /**
     * GDB is at a commands prompt.
     *
     * The annotations are as follows:
     *   pre-commands
     *   commands
     *   post-commands
     */
    GDBWIRE_ANNOTATION_PRE_COMMANDS,
    GDBWIRE_ANNOTATION_COMMANDS,
    GDBWIRE_ANNOTATION_POST_COMMANDS,

    /**
     * GDB is at an overload choice prompt.
     *
     * The annotations are as follows:
     *   pre-overload-choice
     *   overload-choice
     *   post-overload-choice
     */
    GDBWIRE_ANNOTATION_PRE_OVERLOAD_CHOICE,
    GDBWIRE_ANNOTATION_OVERLOAD_CHOICE,
    GDBWIRE_ANNOTATION_POST_OVERLOAD_CHOICE,

    /**
     * GDB is at an instance prompt.
     *
     * The annotations are as follows:
     *   pre-instance-choice
     *   instance-choice
     *   post-instance-choice
     */
    GDBWIRE_ANNOTATION_PRE_INSTANCE_CHOICE,
    GDBWIRE_ANNOTATION_INSTANCE_CHOICE,
    GDBWIRE_ANNOTATION_POST_INSTANCE_CHOICE,

    /**
     * GDB is at a query prompt.
     *
     * The annotations are as follows:
     *   pre-query
     *   query
     *   post-query
     */
    GDBWIRE_ANNOTATION_PRE_QUERY,
    GDBWIRE_ANNOTATION_QUERY,
    GDBWIRE_ANNOTATION_POST_QUERY,

    /**
     * GDB is at a prompt for continue prompt.
     *
     * You should never see this as it's typical to 'set height 0'
     * when using annotations.
     *
     * The annotations are as follows:
     *   pre-prompt-for-continue
     *   prompt-for-continue
     *   post-prompt-for-continue
     */
    GDBWIRE_ANNOTATION_PRE_PROMPT_FOR_CONTINUE,
    GDBWIRE_ANNOTATION_PROMPT_FOR_CONTINUE,
    GDBWIRE_ANNOTATION_POST_PROMPT_FOR_CONTINUE,

    /**
     * GDB is at a query prompt.
     *
     * The annotations are as follows:
     *   pre-prompt
     *   prompt
     *   post-prompt
     */
    GDBWIRE_ANNOTATION_PRE_PROMPT,
    GDBWIRE_ANNOTATION_PROMPT,
    GDBWIRE_ANNOTATION_POST_PROMPT,

    /**
     * The start of an error message.
     *
     * Any console output messages between this annotation and
     *   GDBWIRE_ANNOTATION_ERROR or GDBWIRE_ANNOTATION_QUIT
     * represents the error message and should be displayed
     * to the user as console output.
     *
     * error-begin
     */
    GDBWIRE_ANNOTATION_ERROR_BEGIN,

    /**
     * Sent when GDB is about to handle an error.
     *
     * error
     */
    GDBWIRE_ANNOTATION_ERROR,

    /**
     * Sent when GDB is about to handle an interrupt.
     *
     * quit
     */
    GDBWIRE_ANNOTATION_QUIT,

    /**
     * Sent when the program has exited.
     *
     * exited
     */
    GDBWIRE_ANNOTATION_EXITED,

    /**
     * Sent when the annotation is unknown.
     *
     * unknown
	 *
	 * The annotation text is still returned, however there is no
	 * enumeration which represents this annotation. Feel free to extend
	 * this enumeration if you would like more annotations supported.
     */
    GDBWIRE_ANNOTATION_UNKNOWN
};

/**
 * The GDB/Annotation output command.
 */
struct gdbwire_annotation_output {
    /// The kind of annotation this output represents
    enum gdbwire_annotation_output_kind kind;

    union {
        /** When kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT */
        struct {
            /**
             * The last annotation provided by gdb before this console output.
             *
             * This allows the front end to handle console output from
             * different annotations in unique ways.
             * 
             * For instance, if gdb just sent the annotation
             * GDBWIRE_ANNOTATION_PRE_PROMPT, then the text until
             * GDBWIRE_ANNOTATION_PROMPT would represent the new prompt.
             *
             * This value will be set to GDBWIRE_ANNOTATION_UNKNOWN
             * before the first annotation is recieved and after an unknown
             * annotation is parsed.
             */
            enum gdbwire_annotation_kind last;

            /** The console output text */
            char *text;
        } console_output;

        /** When kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION */
        struct {
            /** The kind of annotation found */
            enum gdbwire_annotation_kind kind;
            /** The annotation text */
            char *text;
        } annotation;
    } variant;
};

void gdbwire_annotation_output_free(struct gdbwire_annotation_output *param);

#ifdef __cplusplus 
}
#endif 

#endif
