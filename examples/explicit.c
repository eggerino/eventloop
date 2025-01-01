#include <stdio.h>

#define EL_EVENTLOOP_IMPL
#include "../eventloop.h"

struct io_op_args {
    int state;
    int is_open, done, result;
};
enum el_poll io_op_async(struct io_op_args *args) {
    switch (args->state) {
        case 0:
            // trigger the file opening
            args->is_open = 1;

            args->state = 1;
        case 1:
            if (!args->is_open) return EL_PENDING;

            // Process data
            args->result = 42;
            args->done = 1;
        default:
            break;
    }
    return EL_DONE;
}

struct compute_args {
    int state;
    struct io_op_args *io_args;
};
enum el_poll compute_async(struct compute_args *args) {
    switch (args->state) {
        case 0:
            args->io_args = (struct io_op_args *)calloc(1, sizeof(struct io_op_args));
            el_dispatch(io_op_async, args->io_args);

            args->state = 1;
        case 1:
            if (!args->io_args->done) return EL_PENDING;

            printf("result is %d\n", args->io_args->result);
            free(args->io_args);

        default:
            break;
    }
    return EL_DONE;
}

int main(void) {
    struct compute_args args;
    args.state = 0;
    int error = el_dispatch(compute_async, &args);
    if (error) return error;

    return el_run(1);
}
