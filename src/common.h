/*
 * Tentcl -- Common
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_COMMON_H
#define TENTCL_COMMON_H

#ifdef LEAK_CHECK
# define GC_DEBUG
# include <gc.h>
# define malloc(n) GC_MALLOC(n)
# define calloc(m,n) GC_MALLOC((m)*(n))
# define free(p) GC_FREE(p)
# define realloc(p,n) GC_REALLOC((p),(n))
# define strdup(s) GC_STRDUP((s))
# define CHECK_LEAKS() GC_gcollect()
#endif

#endif
