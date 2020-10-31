#include <string.h>

#include "gdbwire_sys.h"
#include "gdbwire_assert.h"
#include "gdbwire_string.h"
#include "gdbwire_annotation_parser.h"
#include "gdbwire_annotation_pt_alloc.h"

/**
 * From the GDB manual.
 *
 * Annotations start with a newline character, two ‘control-z’ characters,
 * and the name of the annotation.
 *
 * If there is no additional information associated with this annotation,
 * the name of the annotation is followed immediately by a newline.
 *
 * If there is additional information, the name of the annotation is
 * followed by a space, the additional information, and a newline.
 *
 * Annotations are of the form
 * '\n\032\032annotation\n'
 * However, on windows \n gets mapped to \r\n so that makes,
 * '\r+\n\032\032annotation\r+\n'
 */

enum gdbwire_annotation_parser_state {
    /**
     * When in this state, characters recieved from GDB are literal
     * GDB output.
     *
     * State transitions:
     * newline -> GDBWIRE_ANNOTATION_NEW_LINE
     * other   -> GDBWIRE_ANNOTATION_GDB_DATA
     */
    GDBWIRE_ANNOTATION_GDB_DATA,
    
    /**
     * When in this state, GDB is either starting an annotation
     * or simply recieving a newline.
     *
     * State transitions:
     * control-z -> GDBWIRE_ANNOTATION_CONTROL_Z
     * newline   -> output newline, stay in same state
     * other     -> output newline, transition to GDBWIRE_ANNOTATION_GDB_DATA
     */
    GDBWIRE_ANNOTATION_NEW_LINE,

    /**
     * When in this state, GDB is either starting an annotation
     * or has recieved a newline followed by a control-z.
     *
     * State transitions:
     * control-z -> GDBWIRE_ANNOTATION_TEXT (an annotation has been found)
     * newline   -> output newline, output control z,
     *              transition to GDBWIRE_ANNOTATION_NEW_LINE
     * other     -> output newline, output control z,
     *              transition to GDBWIRE_ANNOTATION_GDB_DATA
     */
    GDBWIRE_ANNOTATION_CONTROL_Z,

    /**
     * When in this state, GDB has recieved an annotation.
     * It is currently collecting the annotation information.
     *
     * State transitions:
     * other    -> collect annotation information
     * new line -> GDBWIRE_ANNOTATION_GDB_DATA
     */
    GDBWIRE_ANNOTATION_TEXT
};

struct gdbwire_annotation_parser {
    /* The buffer pushed into the parser from the user */
    struct gdbwire_string *buffer;
    /* The client parser callbacks */
    struct gdbwire_annotation_parser_callbacks callbacks;
    /* The annotation parser state */
    enum gdbwire_annotation_parser_state state;
    /* The current annotation text being collected */
    struct gdbwire_string *annotation_text;
    /* The console output from GDB */
    struct gdbwire_string *console_output;
};

struct gdbwire_annotation_parser *
gdbwire_annotation_parser_create(
    struct gdbwire_annotation_parser_callbacks callbacks)
{
    struct gdbwire_annotation_parser *parser;

    if (!callbacks.gdbwire_annotation_output_callback) {
        return NULL;
    }

    parser = (struct gdbwire_annotation_parser *)calloc(1,
        sizeof(struct gdbwire_annotation_parser));
    if (!parser) {
        return NULL;
    }

    parser->buffer = gdbwire_string_create();
    if (!parser->buffer) {
        gdbwire_annotation_parser_destroy(parser);
        return NULL;
    }

    parser->callbacks = callbacks;
    parser->state = GDBWIRE_ANNOTATION_GDB_DATA;

    parser->annotation_text = gdbwire_string_create();
    if (!parser->annotation_text) {
        gdbwire_annotation_parser_destroy(parser);
        return NULL;
    }

    parser->console_output = gdbwire_string_create();
    if (!parser->console_output) {
        gdbwire_annotation_parser_destroy(parser);
        return NULL;
    }

    return parser;
}

void gdbwire_annotation_parser_destroy(struct gdbwire_annotation_parser *parser)
{
    if (parser) {
        if (parser->console_output) {
            gdbwire_string_destroy(parser->console_output);
            parser->console_output = NULL;
        }

        if (parser->annotation_text) {
            gdbwire_string_destroy(parser->annotation_text);
            parser->annotation_text = NULL;
        }

        if (parser->buffer) {
            gdbwire_string_destroy(parser->buffer);
            parser->buffer = NULL;
        }

        free(parser);
        parser = NULL;
    }
}

static void
gdbwire_send_console_output_if_available(
        struct gdbwire_annotation_parser *parser)
{
    if (gdbwire_string_size(parser->console_output) != 0) {
        struct gdbwire_annotation_output * output =
                gdbwire_annotation_output_alloc();
        output->kind = GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT;
        gdbwire_string_append_char(parser->console_output, 0);
        output->variant.console_output = gdbwire_strndup(
                gdbwire_string_data(parser->console_output),
                gdbwire_string_size(parser->console_output));
        parser->callbacks.gdbwire_annotation_output_callback(
            parser->callbacks.context, output);

        gdbwire_string_clear(parser->console_output);
    }
}

static void
gdbwire_annotation_parser_send_callback(
        struct gdbwire_annotation_parser *parser,
        enum gdbwire_annotation_kind kind)
{

    struct gdbwire_annotation_output *output =
            gdbwire_annotation_output_alloc();

    output->kind = GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION;
    output->variant.annotation.kind = kind;

    gdbwire_string_append_char(parser->annotation_text, 0);
    output->variant.annotation.text = gdbwire_strndup(
            gdbwire_string_data(parser->annotation_text),
            gdbwire_string_size(parser->annotation_text));

    // Send any console output before the annotation is sent
    gdbwire_send_console_output_if_available(parser);

    parser->callbacks.gdbwire_annotation_output_callback(
        parser->callbacks.context, output);
}

/**
 * The annotations.
 */
static struct gdbwire_annotation {
    /** The name of the annotation. */
    const char *name;

    /** The annotation kind */
    enum gdbwire_annotation_kind kind;

} gdbwire_annotations[] = {
    {
        "breakpoints-invalid",
        GDBWIRE_ANNOTATION_BREAKPOINTS_INVALID
    }, {
        "source",
        GDBWIRE_ANNOTATION_SOURCE
    }, {
        "frame-end",
        GDBWIRE_ANNOTATION_FRAME_END
    }, {
        "frames-invalid",
        GDBWIRE_ANNOTATION_FRAMES_INVALID
    }, {
        "pre-commands",
        GDBWIRE_ANNOTATION_PRE_COMMANDS
    }, {
        "commands",
        GDBWIRE_ANNOTATION_COMMANDS
    }, {
        "post-commands",
        GDBWIRE_ANNOTATION_POST_COMMANDS
    }, {
        "pre-overload-choice",
        GDBWIRE_ANNOTATION_PRE_OVERLOAD_CHOICE
    }, {
        "overload-choice",
        GDBWIRE_ANNOTATION_OVERLOAD_CHOICE
    }, {
        "post-overload-choice",
        GDBWIRE_ANNOTATION_POST_OVERLOAD_CHOICE
    }, {
        "pre-instance-choice",
        GDBWIRE_ANNOTATION_PRE_INSTANCE_CHOICE
    }, {
        "instance-choice",
        GDBWIRE_ANNOTATION_INSTANCE_CHOICE
    }, {
        "post-instance-choice",
        GDBWIRE_ANNOTATION_POST_INSTANCE_CHOICE
    }, {
        "pre-query",
        GDBWIRE_ANNOTATION_PRE_QUERY
    }, {
        "query",
        GDBWIRE_ANNOTATION_QUERY
    }, {
        "post-query",
        GDBWIRE_ANNOTATION_POST_QUERY
    }, {
        "pre-prompt-for-continue",
        GDBWIRE_ANNOTATION_PRE_PROMPT_FOR_CONTINUE
    }, {
        "prompt-for-continue",
        GDBWIRE_ANNOTATION_PROMPT_FOR_CONTINUE
    }, {
        "post-prompt-for-continue",
        GDBWIRE_ANNOTATION_POST_PROMPT_FOR_CONTINUE
    }, {
        "pre-prompt",
        GDBWIRE_ANNOTATION_PRE_PROMPT
    }, {
        "prompt",
        GDBWIRE_ANNOTATION_PROMPT
    }, {
        "post-prompt",
        GDBWIRE_ANNOTATION_POST_PROMPT
    }, {
        "error-begin",
        GDBWIRE_ANNOTATION_ERROR_BEGIN
    }, {
        "error",
        GDBWIRE_ANNOTATION_ERROR
    }, {
        "quit",
        GDBWIRE_ANNOTATION_QUIT
    }, {
        "exited",
        GDBWIRE_ANNOTATION_EXITED
    }, {
        NULL,
        GDBWIRE_ANNOTATION_UNKNOWN
    }
};

enum gdbwire_result
gdbwire_annotation_parser_push(struct gdbwire_annotation_parser *parser,
        const char *data)
{
    return gdbwire_annotation_parser_push_data(parser, data, strlen(data));
}

static void
gdbwire_annotation_parser_parse_annotation(
        struct gdbwire_annotation_parser *parser)
{
    int i;
    size_t annotation_text_length =
            gdbwire_string_find_first_of(parser->annotation_text, " ");
    char *annotation = gdbwire_string_data(parser->annotation_text);
    enum gdbwire_annotation_kind kind = GDBWIRE_ANNOTATION_UNKNOWN;

    for (i = 0; annotation && gdbwire_annotations[i].name != NULL; ++i) {
        size_t length = strlen(gdbwire_annotations[i].name);

        if (annotation_text_length == length &&
            strncmp(gdbwire_annotations[i].name, annotation, length) == 0) {
            kind = gdbwire_annotations[i].kind;
            break;
        }
    }
    gdbwire_annotation_parser_send_callback(parser, kind);
}

static void
gdbwire_annotation_parser_process_char(
        struct gdbwire_annotation_parser *parser, char c)
{
    gdbwire_string_append_char(parser->console_output, c);

    // Arbitrary, but send a line at a time for now
    if (c == '\n') {
        gdbwire_send_console_output_if_available(parser);
    }
}

int
gdbwire_annotation_parser_parse(
        struct gdbwire_annotation_parser *parser, const char *str, size_t size)
{
    int result = 0;
    size_t i;

    for (i = 0; i < size; ++i) {
        /* Ignore all car returns outputted by gdb */
        if (str[i] == '\r')
            continue;

        switch (parser->state) {
            case GDBWIRE_ANNOTATION_GDB_DATA:
                if (str[i] == '\n') {
                    parser->state = GDBWIRE_ANNOTATION_NEW_LINE;
                } else {
                    gdbwire_annotation_parser_process_char(parser, str[i]);
                }
                break;
            case GDBWIRE_ANNOTATION_NEW_LINE:
                if (str[i] == '\032') {
                    parser->state = GDBWIRE_ANNOTATION_CONTROL_Z;
                }  else {
                    gdbwire_annotation_parser_process_char(parser, '\n');
                    if (str[i] == '\n') {
                        // Transition to GDBWIRE_ANNOTATION_NEW_LINE; do nothing
                    } else {
                        gdbwire_annotation_parser_process_char(parser, str[i]);
                        parser->state = GDBWIRE_ANNOTATION_GDB_DATA;
                    }
                }
                break;
            case GDBWIRE_ANNOTATION_CONTROL_Z:
                if (str[i] == '\032') {
                    parser->state = GDBWIRE_ANNOTATION_TEXT;
                }  else {
                    gdbwire_annotation_parser_process_char(parser, '\n');
                    gdbwire_annotation_parser_process_char(parser, '\032');

                    if (str[i] == '\n') {
                        parser->state = GDBWIRE_ANNOTATION_NEW_LINE;
                    } else {
                        gdbwire_annotation_parser_process_char(parser, str[i]);
                        parser->state = GDBWIRE_ANNOTATION_GDB_DATA;
                    }
                }
                break;
            case GDBWIRE_ANNOTATION_TEXT:
                if (str[i] == '\n') {
                    gdbwire_annotation_parser_parse_annotation(parser);
                    parser->state = GDBWIRE_ANNOTATION_GDB_DATA;
                    gdbwire_string_clear(parser->annotation_text);

                } else {
                    gdbwire_string_append_char(parser->annotation_text, str[i]);
                }
                break;
        }
    }

    gdbwire_send_console_output_if_available(parser);

    return result;
}

enum gdbwire_result
gdbwire_annotation_parser_push_data(struct gdbwire_annotation_parser *parser,
        const char *data, size_t size)
{
    enum gdbwire_result result = GDBWIRE_OK;

    GDBWIRE_ASSERT(parser && data);

    // TODO: Handle error? was never handled before
    gdbwire_annotation_parser_parse(parser, data, size);

    return result;
}
