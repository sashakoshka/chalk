#pragma once

#include <stdlib.h>
#include <stdio.h>

typedef enum {
  ObjType_void,
  ObjType_paren,
  ObjType_num,
  ObjType_oper,
  ObjType_var
} ObjType;

typedef enum {
  ObjComputeErr_none = 0,
  ObjComputeErr_syntax,
  ObjComputeErr_divBy0
} ObjComputeErr;

typedef struct {
  double        value;
  ObjComputeErr error;
} ObjComputeRet;

typedef struct Obj {
  ObjType type;
  double  dbvalue;
  char    chvalue;
  struct Obj *prev;
  struct Obj *next;
  struct Obj *parent;
  struct Obj *firstChild;
  struct Obj *lastChild;
} Obj;

void Obj_init (Obj *, ObjType, double, char);
Obj *Obj_new (ObjType, double, char);
void Obj_free (Obj *);
void Obj_adopt (Obj *, Obj *);
ObjComputeRet Obj_compute (Obj *);
void Obj_dump (Obj *, int);
void Obj_nmorph (Obj *, double);
int Obj_isNum (Obj *obj);
