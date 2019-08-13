/*
 * Tentcl -- Core
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_TCL_H
#define TENTCL_TCL_H

#include "List.h"
#include "Hash.h"
#include "value.h"

#define TENTCL_VERSION "0.9.0-tentcl"

extern List mem_list;

/* Tcl Namespace */
typedef List TclNamespaceList;
typedef Hash TclNamespace;

struct Tcl {
    TclNamespaceList *namespace;

    Hash *variables;
    Hash *functions;
};
typedef struct Tcl Tcl;

enum TclReturn {
    TCL_OK = 0,
    TCL_RETURN,
    TCL_BREAK,
    TCL_CONTINUE,
    TCL_EXCEPTION,
    TCL_EXIT,
    TCL_BADCMD,
    TCL_OOM,
    TCL_INTERRUPT,
    TCL_COUNT
};
typedef enum TclReturn TclReturn;

enum TclReturnInfoType {
    TCL_RI_INT = 0,
    TCL_RI_STRING,
    TCL_RI_USERDATA
};
typedef enum TclReturnInfoType TclReturnInfoType;

struct TclReturnInfo {
    TclReturnInfoType type;
    union {
	int i;
	char *s;
	void *u;
    };
};
typedef struct TclReturnInfo TclReturnInfo;

typedef TclReturn (*TclFunction)(Tcl*, int, TclValue*[], TclValue*);

/* Tcl Init/Free functions */
void Tcl_new(Tcl*);
void Tcl_delete(Tcl*);
Tcl *Tcl_malloc(void);
void Tcl_free(Tcl *self);

/* Tcl Functions */
TclReturn Tcl_evalExpression(Tcl*, char*, TclValue*);
TclReturn Tcl_eval(Tcl*, char*, TclValue*);
TclReturn Tcl_funcall(Tcl*, char*, int, TclValue*[], TclValue*);
TclReturn Tcl_expand(Tcl *vm, char *value, TclValue *result);
void Tcl_split(Tcl*, char*, char*, List*);
void Tcl_register(Tcl*, char*, TclFunction);
void Tcl_addVariable(Tcl *self, char *name, char *value);
void Tcl_addVariable_(Tcl *self, char *name, TclValue *val);
void Tcl_pushNamespace(Tcl *self);
void Tcl_popNamespace(Tcl *self);
TclValue *Tcl_getVariable(Tcl *self, char *name);
TclValue *Tcl_getVariableUp(Tcl *self, char *name, int level);
TclValue *Tcl_getFunction(Tcl *self, char *name);
int Tcl_isComplete(Tcl *self, char *expr);

int Tcl_statusToCode(TclReturn);

const char *Tcl_returnString(TclReturn status);

TclReturnInfo Tcl_getReturnInfo(void);
void Tcl_setReturnInfo(TclReturnInfo);

#endif
