/*
 * linked_list_test_program.c
 * Cross-platform linked list test harness.
 * 
 * ✅ Infinite loop detection using alarm()/SIGALRM on Linux/macOS.
 * ✅ Disabled on Windows for compatibility.
 * ✅ Uses instrumented_malloc for controlled allocation testing.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <unistd.h>
#endif

#include "linked_list.h"

#define TEST(x) printf("Running test " #x "\n"); fflush(stdout);

#ifdef _WIN32
#define SUBTEST(x) printf("    Executing subtest " #x "\n"); fflush(stdout);
#define PASS(x) printf("PASS!\n");
#else
#define SUBTEST(x) printf("    Executing subtest " #x "\n"); fflush(stdout); alarm(1);
#define PASS(x) printf("PASS!\n"); alarm(0);
#endif

#define FAIL(cond, msg) if (cond) {\
    printf("    FAIL! "); \
    printf(#msg "\n"); \
    exit(-1);\
}

bool instrumented_malloc_fail_next             = false;
bool instrumented_malloc_last_alloc_successful = false;

#ifndef _WIN32
void gracefully_exit_on_suspected_infinite_loop(int signal_number) {
    const char* err_msg = "        Likely stuck in infinite loop! Exiting.\n";
    ssize_t retval = write(STDOUT_FILENO, err_msg, strlen(err_msg));
    fflush(stdout);
    (void)retval;
    (void)signal_number;
    exit(1);
}
#endif

void * instrumented_malloc(size_t size) {
    if (instrumented_malloc_fail_next) {
        instrumented_malloc_fail_next = false;
        instrumented_malloc_last_alloc_successful = false;
        return NULL;
    }
    void * ptr = malloc(size);
    instrumented_malloc_last_alloc_successful = (ptr != NULL);
    return ptr;
}

/* ------------------------ TEST FUNCTIONS ------------------------ */

void check_null_handling(void) {
    TEST(check_null_handling)

    SUBTEST(linked_list_delete)
    bool status = linked_list_delete(NULL);
    FAIL(status != false, "linked_list_delete(NULL) did not return false")

    SUBTEST(linked_list_insert_front)
    status = linked_list_insert_front(NULL, 0);
    FAIL(status != false, "linked_list_insert_front(NULL, 0) did not return false")

    SUBTEST(linked_list_insert_end)
    status = linked_list_insert_end(NULL, 0);
    FAIL(status != false, "linked_list_insert_end(NULL, 0) did not return false")

    SUBTEST(linked_list_insert)
    status = linked_list_insert(NULL, 0, 0);
    FAIL(status != false, "linked_list_insert(NULL, 0, 0) did not return false")

    SUBTEST(linked_list_create_iterator)
    struct iterator *iter = linked_list_create_iterator(NULL, 0);
    FAIL(iter != NULL, "linked_list_create_iterator(NULL, 0) did not return NULL")

    SUBTEST(linked_list_delete_iterator)
    status = linked_list_delete_iterator(NULL);
    FAIL(status != false, "linked_list_delete_iterator(NULL) did not return false")

    SUBTEST(linked_list_iterate)
    status = linked_list_iterate(NULL);
    FAIL(status != false, "linked_list_iterate(NULL) did not return false")

    SUBTEST(linked_list_find)
    size_t index = linked_list_find(NULL, 0);
    FAIL(index != SIZE_MAX, "linked_list_find(NULL, 0) did not return SIZE_MAX")

    PASS(check_null_handling)
}

void check_empty_list_properties(void) {
    TEST(check_empty_list_properties)

    SUBTEST(linked_list_create)
    struct linked_list *ll = linked_list_create();
    FAIL(ll == NULL, "linked_list_create() returned NULL on allocation success")
    FAIL(ll->head != NULL, "ll->head is non-null in empty linked_list")

    linked_list_delete(ll);

    SUBTEST(linked_list_memory_alloc_fail)
    instrumented_malloc_fail_next = true;
    ll = linked_list_create();
    FAIL(ll != NULL, "linked_list_create() returns non-null pointer on allocation failure")

    SUBTEST(empty_linked_list_iterator)
    ll = linked_list_create();
    struct iterator *iter = linked_list_create_iterator(ll, 0);
    FAIL(iter != NULL, "linked_list_create_iterator returned an iterator for an empty linked_list")

    linked_list_delete_iterator(iter);
    linked_list_delete(ll);

    PASS(check_empty_list_properties)
}

void check_insertion_functionality(void) {
    TEST(check_insertion_functionality)

    SUBTEST(check_insert_end)
    struct linked_list *ll = linked_list_create();
    for (size_t i = 1; i <= 4; i++) {
        bool status = linked_list_insert_end(ll, i);
        FAIL(status == false, "Failed to insert at end")
    }
    struct iterator *iter = linked_list_create_iterator(ll, 0);
    for (size_t i = 1; i <= 4; i++) {
        FAIL(iter->data != i, "Incorrect data during iteration at end")
        FAIL(iter->current_index != (i - 1), "Incorrect index during iteration at end")
        linked_list_iterate(iter);
    }
    linked_list_delete(ll);
    linked_list_delete_iterator(iter);

    SUBTEST(check_insert_front)
    ll = linked_list_create();
    for (size_t i = 4; i != 0; i--) {
        bool status = linked_list_insert_front(ll, i);
        FAIL(status == false, "Failed to insert at front")
    }
    iter = linked_list_create_iterator(ll, 0);
    for (size_t i = 1; i <= 4; i++) {
        FAIL(iter->data != i, "Incorrect data during iteration at front")
        FAIL(iter->current_index != (i - 1), "Incorrect index during iteration at front")
        linked_list_iterate(iter);
    }
    linked_list_delete(ll);
    linked_list_delete_iterator(iter);

    SUBTEST(check_insert_at_indices)
    ll = linked_list_create();
    FAIL(linked_list_insert(ll, 1, 0) != false, "Insert at invalid index did not fail")
    for (size_t i = 0; i < 4; i++) {
        bool status = linked_list_insert(ll, i, i + 1);
        FAIL(status != true, "Insert at valid index failed")
    }
    iter = linked_list_create_iterator(ll, 0);
    for (size_t i = 1; i <= 4; i++) {
        FAIL(iter->data != i, "Incorrect data during index insert iteration")
        FAIL(iter->current_index != (i - 1), "Incorrect index during index insert iteration")
        linked_list_iterate(iter);
    }
    linked_list_delete(ll);
    linked_list_delete_iterator(iter);

    PASS(check_insertion_functionality)
}

void check_find_functionality(void) {
    TEST(check_find_functionality)

    struct linked_list *ll = linked_list_create();
    for (size_t i = 1; i <= 10; i++) {
        linked_list_insert_end(ll, i);
    }

    SUBTEST(find_beginning)
    size_t index = linked_list_find(ll, 1);
    FAIL(index != 0, "Failed to find element at beginning")

    SUBTEST(find_end)
    index = linked_list_find(ll, 10);
    FAIL(index != 9, "Failed to find element at end")

    SUBTEST(find_middle)
    index = linked_list_find(ll, 5);
    FAIL(index != 4, "Failed to find element in middle")

    SUBTEST(find_nonexistent)
    index = linked_list_find(ll, 42);
    FAIL(index != SIZE_MAX, "Incorrectly found nonexistent element")

    linked_list_delete(ll);

    PASS(check_find_functionality)
}

/* ------------------------ MAIN ------------------------ */

int main(void) {
#ifndef _WIN32
    signal(SIGALRM, gracefully_exit_on_suspected_infinite_loop);
#endif

    linked_list_register_malloc(instrumented_malloc);
    linked_list_register_free(free);

    check_null_handling();
    check_empty_list_properties();
    check_insertion_functionality();
    check_find_functionality();

    printf("All tests passed!\n");
    return 0;
}
