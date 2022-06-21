#include "error.h"

#include <stdio.h> // vsnprintf

DEBUG_ONLY(int error_test_only = 0);

#define ERROR_BUFFER_SIZE 4096
static int error_buffer_index = 0;
// probably only need space for one error buffer, but we do swap them out.
char error_buffers[2][ERROR_BUFFER_SIZE] = {{0}, {0}};
static int current_error_length = 0;

char *error_buffer() {
    return error_buffers[error_buffer_index];
}

void error(const char *format, ...) {
    va_list variable_arguments;
    va_start(variable_arguments, format);
    // i would do `playdate->system->verror(format, variable_arguments);`
    // but playdate->system doesn't expose a verror, so first print to error buffer.
    int delta = vsnprintf(
        error_buffer() + current_error_length,
        current_error_length < ERROR_BUFFER_SIZE
            ?   ERROR_BUFFER_SIZE - current_error_length
            :   0,
        format,
        variable_arguments
    );
    if (delta >= 0) {
        current_error_length += delta;
    } else {
        playdate->system->error("format seems invalid: %s", format);
    }
    va_end(variable_arguments);
    #ifndef NDEBUG
    if (error_test_only) {
        // unfortunately this doesn't stop execution of other stuff,
        // so hopefully your tests don't require stopping immediately at the error.
        // i.e., maybe do a return, too.
        playdate->system->logToConsole(error_buffer());
        return;
    }
    #endif
    playdate->system->error(error_buffer());
}

#ifndef NDEBUG
const char *error_pull() {
    current_error_length = 0;
    const char *result = error_buffer();
    error_buffer_index = 1 - error_buffer_index;
    // new error buffer should be reset:
    error_buffer()[0] = 0;
    return result;
}
#endif
