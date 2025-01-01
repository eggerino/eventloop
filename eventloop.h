#ifndef EL_EVENTLOOP_H_
#define EL_EVENTLOOP_H_ 1

#include <stddef.h>

// Struct member for the state machine of an asyncronous task.
#define EL_STATE_MEMBER int __state

// Initialize the state of the state machine of an asyncronous task.
#define el_init_state(data) (*(data)).__state = 0

// Ergonomics macro for enqueing a new task.
#define el_dispatch(routine, data) el_enque_new_task((enum el_poll (*)(void *))(routine), (void *)(data))

// Declare an the function of an asyncronous task.
#define EL_ASYNC_TASK_DECL(name, state_type) enum el_poll name(state_type *args);

// Start the body of an function of an asyncronous task.
#define EL_ASYNC_TASK(name, state_type) enum el_poll name(state_type *args) { switch (args->__state) { case 0:

// Interrupt the current task, if expr is false and defer the task until expr is becomes true.
#define el_await(expr) args->__state = __LINE__; case __LINE__: do { if(!(expr)) { return EL_PENDING; } } while(0)

// End the body of a function of an asyncronous task.
#define EL_ASYNC_TASK_END default: break; } return EL_DONE; }

// State of a polled asyncronous task.
enum el_poll {
    // The task is still not finished
    EL_PENDING = 0,

    // The task has finished.
    EL_DONE,
};

// Add a new asyncronous task to the queue. It gets executed when it is its turn.
int el_enque_new_task(enum el_poll (*routine)(void *), void *data);

// Manually run the event loop. The function blocks until all pending tasks are resolved. 
int el_run(size_t num_threads);

#ifdef EL_EVENTLOOP_IMPL

#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define EL_ERR_OK 0
#define EL_ERR_NOALLOC 90

struct el_task_node {
    struct el_task_node *next;
    enum el_poll (*routine)(void *);
    void *data;
};

struct el_task_queue {
    struct el_task_node *head;
    struct el_task_node *tail;
    size_t pending_count;
};

static void el_increase_pending_count();
static void el_decrease_pending_count();
static void el_push_task(struct el_task_node *node);
static struct el_task_node *el_try_pop_task();
static void *el_worker_main(void *);

static struct el_task_queue el_queue = {0};
static pthread_mutex_t el_queue_lock = {0};

int el_enque_new_task(enum el_poll (*routine)(void *), void *data) {
    struct el_task_node *node = (struct el_task_node *)malloc(sizeof(*node));
    if (!node) {
        return EL_ERR_NOALLOC;
    }

    node->next = NULL;
    node->routine = routine;
    node->data = data;

    el_increase_pending_count();
    el_push_task(node);
    return EL_ERR_OK;
}

int el_run(size_t num_threads) {
    pthread_t *worker_pids = (pthread_t *)malloc(num_threads * sizeof(*worker_pids));
    if (!worker_pids) {
        return EL_ERR_NOALLOC;
    }

    for (size_t i = 0; i < num_threads; ++i) {
        int error = pthread_create(&worker_pids[i], NULL, el_worker_main, NULL);
        if (error) {
            // Cancel all started threads
            for (size_t i_cancel = 0; i_cancel < i; ++i_cancel) {
                pthread_cancel(worker_pids[i_cancel]);
            }

            free(worker_pids);
            return error;
        }
    }

    for (size_t i = 0; i < num_threads; ++i) {
        int error = pthread_join(worker_pids[i], NULL);
        if (error) {
            // Cancel all remaining (and not joined) threads    
            for (size_t i_cancel = i; i_cancel < num_threads; ++i_cancel) {
                pthread_cancel(worker_pids[i_cancel]);
            }

            free(worker_pids);
            return error;
        }
    }

    free(worker_pids);
    return EL_ERR_OK;
}

void el_increase_pending_count() {
    pthread_mutex_lock(&el_queue_lock);
    ++el_queue.pending_count;
    pthread_mutex_unlock(&el_queue_lock);
}

void el_decrease_pending_count() {
    pthread_mutex_lock(&el_queue_lock);
    --el_queue.pending_count;
    pthread_mutex_unlock(&el_queue_lock);
}

void el_push_task(struct el_task_node *node) {
    node->next = NULL;

    pthread_mutex_lock(&el_queue_lock);
    if (!el_queue.head) {
        el_queue.head = node;
    } else {
        el_queue.tail->next = node;
    }
    el_queue.tail = node;
    pthread_mutex_unlock(&el_queue_lock);
}

struct el_task_node *el_try_pop_task() {
    struct el_task_node *node = NULL;

    pthread_mutex_lock(&el_queue_lock);
    // Only pop if there is at least one node in the queue
    if (el_queue.head) {
        node = el_queue.head;
        el_queue.head = el_queue.head->next;
        if (node == el_queue.tail) {
            el_queue.tail = NULL;
        }
        node->next = NULL;
    }
    pthread_mutex_unlock(&el_queue_lock);
    return node;
}

void *el_worker_main(void *) {
    while (el_queue.pending_count) {
        struct el_task_node *current = el_try_pop_task();

        if (!current) {
            // Worker thread is not needed to handle the current task queue in parallel
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 100 * 1000000};
            nanosleep(&ts, NULL); 

            continue;
        }

        // Continue on the current task
        enum el_poll state = current->routine(current->data);
        if (state == EL_PENDING) {
            el_push_task(current);
        } else {
            // Task is completed
            el_decrease_pending_count();
            free(current);
        }
    }

    return NULL;
}

#endif // EL_EVENTLOOP_IMPL

#endif //  EL_EVENTLOOP_H_
