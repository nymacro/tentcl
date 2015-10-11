/*
 * Tentcl -- Built-in Types
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_VALUE_H
#define TENTCL_VALUE_H

typedef char* TclValue; /* Tcl primitive */

/* Tcl Value Primitive Functions */
void TclValue_new(TclValue*, char*); /* Create Tcl value */
void TclValue_delete(TclValue*); /* Destroy Tcl value */
void TclValue_set(TclValue*, char*); /* Set Tcl value */
void TclValue_replace(TclValue*, TclValue*); /* Replace existing value */
void TclValue_append(TclValue *, char*); /* Append string to end of value */
void TclValue_prepend(TclValue *, char*); /* Prepend string to start of value */

#endif
