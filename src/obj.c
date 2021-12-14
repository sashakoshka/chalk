#include "obj.h"
#include <math.h>

void Obj_init (Obj *obj, ObjType type, double dbvalue, char chvalue) {  
  obj->type       = type;
  obj->dbvalue    = dbvalue;
  obj->chvalue    = chvalue;
  obj->prev       = NULL;
  obj->next       = NULL;
  obj->parent     = NULL;
  obj->firstChild = NULL;
  obj->lastChild  = NULL;
}

Obj *Obj_new (ObjType type, double dbvalue, char chvalue) {
  Obj *ret = malloc(sizeof(Obj));
  if (ret == NULL) return NULL;
  Obj_init(ret, type, dbvalue, chvalue);
  return ret;
}

void Obj_free (Obj *obj) {
  Obj *child = obj->firstChild;
  while (child) {
    Obj *next = child->next;
    Obj_free(child);
    child = next;
  }

  if (obj->parent) {
    if (obj->prev == NULL)
      obj->parent->firstChild = obj->next;
    else
      obj->prev->next = obj->next;

    if (obj->next == NULL)
      obj->parent->lastChild = obj->prev;
    else
      obj->next->prev = obj->prev;
    free(obj);
  }
}

void Obj_adopt (Obj *parent, Obj *child) {
  child->next = NULL;
  if (parent->lastChild == NULL) {
    child->prev = NULL;
    parent->firstChild = child;
  } else {
    child->prev = parent->lastChild;
    parent->lastChild->next = child;
  }
  parent->lastChild = child;
  child->parent = parent; 
}

#define throw(err) {result.error = err; return result;}

ObjComputeRet Obj_compute (Obj *obj) {
  Obj *child;
  ObjComputeRet result = {0, ObjComputeErr_none};

  /* beginning negative */ {
    if (obj->firstChild != NULL
     && obj->firstChild->type == ObjType_oper
     && obj->firstChild->chvalue == '-'
    ) {
      Obj_free(obj->firstChild);
      if (obj->firstChild != NULL)
        obj->firstChild->dbvalue *= -1;
    }
  }

  /* validation pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      ObjType nextt = ObjType_void,
              prevt = ObjType_void;
      if (child->next != NULL) nextt = child->next->type;
      if (child->prev != NULL) prevt = child->prev->type;
      /* printf("%i %i %i\n", prevt, child->type, nextt); */
      switch (child->type) {
        /* this is going to change when functions and implicit multiplicative
           syntax like 5(3) is added */
        case ObjType_var:
        case ObjType_num:
        case ObjType_paren:
          if (prevt == ObjType_num
           || prevt == ObjType_var
           || prevt == ObjType_paren
           || nextt == ObjType_num
           || nextt == ObjType_var
           || nextt == ObjType_paren
          ) throw(ObjComputeErr_syntax);
          break;
        case ObjType_oper:
          switch (child->chvalue) {
            case '^':
            case '*':
            case '/':
            case '+':
            case '-':
            case ',':
            case '%':
              if (prevt == ObjType_void
               || prevt == ObjType_oper
               || nextt == ObjType_oper
               || nextt == ObjType_void
              ) throw(ObjComputeErr_syntax);
              break;
            case '!':
              if (prevt == ObjType_oper || prevt == ObjType_void)
                throw(ObjComputeErr_syntax);
              break;
            default:
              throw(ObjComputeErr_badOper);
          }
        case ObjType_void: break;
      }
      prevt = child->type;
      child = child->next;
    }
  }

  /* single (left hand) input operator pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      switch (child->chvalue) {
        case '!':
          if (child->prev->dbvalue < 0)
            throw(ObjComputeErr_facOfNeg);
          Obj_nmorph(child, factorial(child->prev->dbvalue));
          Obj_free(child->prev);
          break;
        default:
          goto singleLOperNext;
      }
      singleLOperNext:
      child = child->next;
    }
  }

  /* parentheses pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      if (child->type == ObjType_paren) {
        ObjComputeRet childResult = Obj_compute(child);
        if (childResult.error != ObjComputeErr_none)
          return childResult;
        Obj_nmorph(child, childResult.value);
      }
      child = child->next;
    }
  }

  /* exponent pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      if (child->type == ObjType_oper && child->chvalue == '^') {
        Obj_nmorph(child, pow(child->prev->dbvalue, child->next->dbvalue));
        Obj_free(child->prev);
        Obj_free(child->next);
      }
      
      child = child->next;
    }
  }

  /* multiplication pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      if (child->type == ObjType_oper) {
        switch (child->chvalue) {
          case '*':
            Obj_nmorph(child, child->prev->dbvalue * child->next->dbvalue);
            break;
          case '/':
            if (child->next->dbvalue == 0)
              throw(ObjComputeErr_divBy0);
            Obj_nmorph(child, child->prev->dbvalue / child->next->dbvalue);
            break;
          case '%':
            Obj_nmorph(child, fmod(child->prev->dbvalue, child->next->dbvalue));
            break;
          default:
            goto multiplyNext;
        }
        
        Obj_free(child->prev);
        Obj_free(child->next);
      }

      multiplyNext:
      child = child->next;
    }
  }

  /* addition  pass */ {
    child = obj->firstChild;
    while (child != NULL) {
      if (child->type == ObjType_oper) {
        if (child->chvalue == '+') {
          Obj_nmorph(child, child->prev->dbvalue + child->next->dbvalue);
        } else if (child->chvalue == '-') {
          Obj_nmorph(child, child->prev->dbvalue - child->next->dbvalue);
        } else goto addNext;
        
        Obj_free(child->prev);
        Obj_free(child->next);
      }

      addNext:
      child = child->next;
    }
  }

  result.value = obj->firstChild->dbvalue;
  return result;
}

#undef throw

void Obj_nmorph (Obj *obj, double value) {
  obj->dbvalue = value;
  obj->type = ObjType_num;
}

int Obj_isNum (Obj *obj) {
  return obj->type = ObjType_num;
}

void Obj_dump (Obj *obj, int indent) {
  int i = 0;
  Obj *child = obj->firstChild;

  switch (obj->type) {
    case ObjType_paren:
      printf("(       ");
      break;
    case ObjType_num:
      printf("N%7.1f",    obj->dbvalue);
      break;
    case ObjType_oper:
    case ObjType_var:
      printf("%c       ", obj->chvalue);
      break;
    case ObjType_void:
      printf("void    ");
      break;
  }

  while (child != NULL) {
    int j = indent + 8;
    
    if (i ++ > 0)
      while (j --> 0)
        putchar(' ');
    
    Obj_dump(child, indent + 8);
    child = child->next;
  }

  if (i == 0) putchar('\n');
}
