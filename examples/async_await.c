#include <stdio.h>

#define EL_EVENTLOOP_IMPL
#include "../eventloop.h"

struct io_op_args {
    EL_STATE_MEMBER;
    int is_open, done, result;
};
EL_ASYNC_TASK(io_op_async, struct io_op_args)
    // Trigger file opening
    args->is_open = 1;
    el_await(args->is_open);

    // Process data
    args->result = 42;
    args->done = 1;
EL_ASYNC_TASK_END

struct compute_args {
    EL_STATE_MEMBER;
    struct io_op_args *io_args;
};
EL_ASYNC_TASK(compute_async, struct compute_args)
    args->io_args = (struct io_op_args *)calloc(1, sizeof(struct io_op_args));
    el_dispatch(io_op_async, args->io_args);
    el_await(args->io_args->done);

    printf("result is %d\n", args->io_args->result);
    free(args->io_args);
EL_ASYNC_TASK_END

int main(void) {
    struct compute_args args;
    el_init_state(&args);
    int error = el_dispatch(compute_async, &args);
    if (error) return error;

    return el_run(1);
}
