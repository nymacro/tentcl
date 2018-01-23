/*
 * Tentcl -- Regular Expression Function Library
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

#include "regexp.h"

#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

TclReturn TclRegexp_regexp(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    int ret_code = TCL_EXCEPTION;
    char *re_str = TclValue_str(argv[1]);
    char *match_str = TclValue_str(argv[2]);

    pcre2_match_data *match_data = NULL;
    pcre2_code *re = NULL;
    int err = 0;
    PCRE2_SIZE err_offset = 0;
    int rc;

    if (argc < 3) {
        return TCL_EXCEPTION;
    }

    re = pcre2_compile_8((PCRE2_SPTR8)re_str,
                         PCRE2_ZERO_TERMINATED,
                         0,
                         &err,
                         &err_offset,
                         NULL);
    if (re == NULL)
        goto err;

    match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(re,
                     (PCRE2_SPTR8)match_str,
                     strlen(match_str),
                     0,
                     0,
                     match_data,
                     NULL);
    if (rc < 0) {
        switch (rc) {
        case PCRE2_ERROR_NOMATCH:
            ret_code = TCL_OK;
            goto err;
        default:
            ret_code = TCL_EXCEPTION;
            goto err;
        }
    }

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    if (rc > 0) {
        int i = 0;
        size_t substring_length = ovector[2*i+1] - ovector[2*i];
        TclValue_set_raw(ret, match_str + ovector[2*i], substring_length);

        ret_code = TCL_OK;
    }

err:
    if (match_data != NULL)
        pcre2_match_data_free(match_data);
    if (re != NULL)
        pcre2_code_free(re);
    return ret_code;
}

void TclRegexp_register(Tcl *vm) {
    Tcl_register(vm, "regexp", TclRegexp_regexp);
    return;
}
