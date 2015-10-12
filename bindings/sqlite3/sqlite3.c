#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "../../src/tcl.h"

static Hash dbs;

TclReturn Tcl_sqopen(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    static int id = 0;
    if (argc != 2) {
        return TCL_EXCEPTION;
    }

    sqlite3 *db;
    int rc;
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        printf("unable to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return TCL_EXCEPTION;
    }

    char tmp[32];
    sprintf(tmp, "__sqlite_%i", id++);

    Hash_get(&dbs, tmp)->data = db;

    TclValue_new(ret, tmp);
    
    return TCL_OK;
}

TclReturn Tcl_sqclose(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc !=2) {
        return TCL_EXCEPTION;
    }

    if (Hash_get(&dbs, argv[1])->data) {
        sqlite3_close(Hash_get(&dbs, argv[1])->data);
        Hash_get(&dbs, argv[1])->data = NULL;
    } else {
        return TCL_EXCEPTION;
    }
    TclValue_new(ret, "closed");
    
    return TCL_OK;
}

static int sq_callback(void *_unused, int argc, char **argv, char **azColName) {
    int i;
    printf("CALLBACK!\n");
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

TclReturn Tcl_sqexec(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }

    char *err = NULL;
    if (sqlite3_exec(Hash_get(&dbs, argv[1])->data, argv[2], NULL,
                     sq_callback, &err) != SQLITE_OK) {
        printf("SQL error: %s\n", err);
        sqlite3_free(err);
    }
    TclValue_new(ret, "queried");
    return TCL_OK;
}

void sq_closedb(BTreeNode *node, void *_unused) {
    sqlite3_close(node->data);
}
void Tcl_cleanup() {
    Hash_map(&dbs, sq_closedb, NULL);
    Hash_delete(&dbs);
}

void Tcl_registerLibrary(Tcl *vm) {
    Hash_new(&dbs);

    Tcl_register(vm, "sqopen", Tcl_sqopen);
    Tcl_register(vm, "sqclose", Tcl_sqclose);
    Tcl_register(vm, "sqexec", Tcl_sqexec);

    atexit(Tcl_cleanup);
}
