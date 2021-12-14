#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "obj.h"
#include "args.h"

int  skipSpace (void);
void getArgs (int, char **);

int main(int argc, char **argv) {
  int ch;

  getArgs(argc, argv);

  if (args.showHelp) {
    printf ("chalk - double precision command line calculator\n");
    printf ("Usage: %s [OPTIONS]\n\n", argv[0]);
    printf ("-t   show tree\n");
    printf ("-h   this\n");
    return EXIT_SUCCESS;
  }
  
  while (1) {
    Obj *root   = Obj_new(ObjType_paren, 0, 0),
        *branch = root;
    
    while ((ch = skipSpace()) != '\n') {
      if (ch == EOF) return EXIT_SUCCESS;

      /* single letter variable name */
      if (isalpha(ch)) {
        Obj_adopt(branch, Obj_new(ObjType_var, 0, ch));

      /* parse double precision numeric value */
      } else if (isdigit(ch) || ch == '.') {
        double value = 0;
        double div   = 1;

        /* get whole number */
        while (isdigit(ch)) {
          value *= 10;
          value += ch - '0';
          ch = getchar();
        }

        /* get decimal, if it exists */
        if (ch == '.') {
          ch = getchar();
          while (isdigit(ch)) {
            div   /= 10;
            value += (ch - '0') * div;
            ch = getchar();
          }
        }

        Obj_adopt(branch, Obj_new(ObjType_num, value, 0));
        ungetc(ch, stdin);

      /* symbol */
      } else switch (ch) {
        case '(': {
          Obj *newBranch = Obj_new(ObjType_paren, 0, 0);
          Obj_adopt(branch, newBranch);
          branch = newBranch;
        } break;
        
        case ')': {
          if (branch->parent == NULL) {
            fprintf(stderr, "%s: ERR unexpected ')'\n", argv[0]);
            goto error;
          }
          branch = branch->parent;
        } break;

        case '^':
        case '*':
        case '/':
        case '+':
        case '-':
        case '!':
        case ',':
        case '%':
          Obj_adopt(branch, Obj_new(ObjType_oper, 0, ch));
          break;
        
        default:
          fprintf(stderr, "%s: ERR unexpected '%c'\n", argv[0], ch);
          goto error;
          break;
      }
    }

    /* show results */ {
      ObjComputeRet result;
    
      if (args.showTree) {
        puts("before calculation:");
        Obj_dump(root, 0);
      }
      
      result = Obj_compute(root);
    
      if (args.showTree) {
        puts("after calculation:");
        Obj_dump(root, 0);
      }

      switch (result.error) {
        case ObjComputeErr_none:
          printf("%f\n", result.value);
          break;
        case ObjComputeErr_syntax:
          fprintf(stderr, "%s: ERR syntax\n", argv[0]);
          break;
        case ObjComputeErr_divBy0:
          fprintf(stderr, "%s: ERR division by zero\n", argv[0]);
          break;
        case ObjComputeErr_badOper:
          fprintf(stderr, "%s: ERR unrecognized operator\n", argv[0]);
          break;
        case ObjComputeErr_facOfNeg:
          fprintf(stderr, "%s: ERR factorial of negative number\n", argv[0]);
          break;
      }
      
      Obj_free(root);
      continue;
    }

    error:
    while (getchar() != '\n');
    Obj_free(root);
  }
  
  return EXIT_SUCCESS;
}

int skipSpace (void) {
  int ch;
  while ((ch = getchar()) == ' ');
  return ch;
}

void getArgs (int argc, char **argv) {
  int getSwitches = 1;
  int i;

  for (i = 1; i < argc; i++) {
    char *ch = argv[i];
    if (*ch == '-' && getSwitches) {
      /* this arg has 1 or more switches */
      while (*(++ch) != 0) switch (*ch) {
        case '-': getSwitches    = 0; break;
        case 't': args.showTree = 1; break;
        case 'h': args.showHelp = 1; break;
      }
    } else {
      /* TODO: be able to read from files */
    }
  }
}
