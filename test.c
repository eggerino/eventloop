#include <assert.h>
#include <stdio.h>

#define EL_EVENTLOOP_IMPL
#include "eventloop.h"

void pending_can_be_changed_idependently(void) {
    assert(el_queue.pending_count == 0);
    el_increase_pending_count();
    assert(el_queue.pending_count == 1);
    el_increase_pending_count();
    assert(el_queue.pending_count == 2);
    el_decrease_pending_count();
    assert(el_queue.pending_count == 1);
    el_decrease_pending_count();
    assert(el_queue.pending_count == 0);
}

void pushing_and_poping_the_queue_works_as_expected(void) {
    assert(el_queue.head == NULL);
    assert(el_queue.tail == NULL);

    struct el_task_node *node1 = (struct el_task_node *)malloc(sizeof(struct el_task_node));
    struct el_task_node *node2 = (struct el_task_node *)malloc(sizeof(struct el_task_node));
    struct el_task_node *node3 = (struct el_task_node *)malloc(sizeof(struct el_task_node));

    el_push_task(node1);
    assert(el_queue.head == node1);
    assert(el_queue.tail == node1);

    el_push_task(node2);
    assert(el_queue.head == node1);
    assert(el_queue.tail == node2);

    el_push_task(node3);
    assert(el_queue.head == node1);
    assert(el_queue.tail == node3);

    assert(el_try_pop_task() == node1);
    assert(el_queue.head == node2);
    assert(el_queue.tail == node3);

    assert(el_try_pop_task() == node2);
    assert(el_queue.head == node3);
    assert(el_queue.tail == node3);

    assert(el_try_pop_task() == node3);
    assert(el_queue.head == NULL);
    assert(el_queue.tail == NULL);

    free(node1);
    free(node3);
    free(node2);
}

void enque_new_task_create_task_in_queue(void) {
    assert(el_queue.head == NULL);
    assert(el_queue.tail == NULL);

    assert(el_enque_new_task((enum el_poll (*)(void *))1, (void *)2) == 0);

    assert(el_queue.head == el_queue.tail);
    assert(el_queue.pending_count == 1);
    assert(el_queue.head->next == NULL);
    assert(el_queue.head->routine == (enum el_poll (*)(void *))1);
    assert(el_queue.head->data == (void *)2);

    el_decrease_pending_count();
    el_try_pop_task();
}

enum el_poll set_to_one(int *num) {
    *num = 1;
}

void worker_main_executes_enqued_tasks(void) {
    int *num1 = (int *)calloc(1, sizeof(int));
    int *num2 = (int *)calloc(1, sizeof(int));
    int *num3 = (int *)calloc(1, sizeof(int));

    el_dispatch(set_to_one, num1);
    el_dispatch(set_to_one, num2);
    el_dispatch(set_to_one, num3);

    el_worker_main(NULL);

    assert(el_queue.head == NULL);
    assert(el_queue.tail == NULL);
    assert(*num1 == 1);
    assert(*num2 == 1);
    assert(*num3 == 1);

    free(num1);
    free(num2);
    free(num3);
}

void runtime_executes_enqued_tasks(void) {
    int *num1 = (int *)calloc(1, sizeof(int));
    int *num2 = (int *)calloc(1, sizeof(int));
    int *num3 = (int *)calloc(1, sizeof(int));

    el_dispatch(set_to_one, num1);
    el_dispatch(set_to_one, num2);
    el_dispatch(set_to_one, num3);

    el_run(4);

    assert(el_queue.head == NULL);
    assert(el_queue.tail == NULL);
    assert(*num1 == 1);
    assert(*num2 == 1);
    assert(*num3 == 1);

    free(num1);
    free(num2);
    free(num3);
}

int main(void) {
    pending_can_be_changed_idependently();
    pushing_and_poping_the_queue_works_as_expected();
    enque_new_task_create_task_in_queue();
    worker_main_executes_enqued_tasks();
    runtime_executes_enqued_tasks();

    fprintf(stderr, "All tests passed\n");
    return 0;
}
