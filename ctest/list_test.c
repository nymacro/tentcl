#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tcl.h"

void did_fail() {
    /* noop */
    return;
}

#define test_assert(p)						\
    do {							\
        if (!(p)) {						\
	    fprintf(stderr, "%i:FAILED " #p "\n", __LINE__);	\
            did_fail();						\
            exit(1);						\
        }							\
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
    test_assert(s3->container->ref == 2);

    TclValue *r = TclValue_list_pop(list);
    test_assert(r->container == s3->container);
    test_assert(s3->container->ref == 2);
    TclValue_delete(r);

    TclValue *old_s1;
    TclValue_new_ref(&old_s1, s1);
    test_assert(s1->container->ref == 3);

    TclValue_coerce(s1, TCL_VALUE_LIST);
    test_assert(s1->container->ref == 1);
    test_assert(old_s1->container->ref == 2);
    TclValue_delete(old_s1);

    r = TclValue_list_pop(s1);
    test_assert(TclValue_type(r) == TCL_VALUE_STR);
    test_assert(strcmp(r->container->value, "a") == 0);
    TclValue_delete(r);

    /* shift test */
    TclValue *list2;
    TclValue_new_list(&list2);
    TclValue_list_push(list2, s2);
    TclValue_list_push(list2, s3);

    r = TclValue_list_shift(list2);
    test_assert(r->container == s2->container);
    TclValue_delete(r);

    /* cleanup */
    TclValue_delete(s3);
    TclValue_delete(s2);
    TclValue_delete(s1);

    TclValue_delete(list2);
    TclValue_delete(list);
    return 0;
}
