#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tcl.h"

void did_fail() {
    /* noop */
    return;
}

#define test_assert(p)                                  \
    do {                                                \
        if (!(p)) {                                     \
            fprintf(stderr, "FAILED " #p "\n");         \
            did_fail();                                 \
            exit(1);                                    \
        }                                               \
    } while (0);

int main(int argc, char *argv[]) {
    TclValue *list;
    TclValue *s1, *s2, *s3;
    TclValue_new(&s1, "a");
    TclValue_new(&s2, "b");
    TclValue_new(&s3, "c");
    TclValue_new_list(&list);

    /* do tests */
    TclValue_list_push(list, s1);
    test_assert(s1->container->ref == 2);

    TclValue_list_push(list, s2);
    TclValue_list_push(list, s3);

    TclValue *r = TclValue_list_pop(list);
    test_assert(r->container == s3->container);
    TclValue_delete(r);

    TclValue_coerce(s1, TCL_VALUE_LIST);
    r = TclValue_list_pop(s1);
    test_assert(TclValue_type(r) == TCL_VALUE_STR);
    test_assert(strcmp(r->container->value, "a") == 0);
    TclValue_delete(r);

    /* cleanup */
    TclValue_delete(s3);
    TclValue_delete(s2);
    TclValue_delete(s1);

    TclValue_delete(list);
    return 0;
}
