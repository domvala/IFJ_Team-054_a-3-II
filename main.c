/*
Projekt IFJ16
xverny00 - Jan Verny
xosker02 - Jan Oskera
xvlach16 - Antonin Vlach
*/

#include "ial.h"
#include "sematics.h"
#include "lex_anal.h"
#include "interpret.h"



int main(int argc, char *argv[]){
      int ret;

      FILE *inOrig = stdin;
      if (argc > 1) {
          stdin = fopen(argv[1], "r");
          if (stdin == NULL) {
              printf("Error!\n");
              exit(99); 
          }
      }

      beginLexicalAnalysis();

      derivationTree = malloc(sizeof(struct tTNode));
      ret = file(derivationTree);
      if (ret == 0) {
          IAL_HashTable HTable;
          IAL_htItem *item = NULL;
          IAL_htInit(&HTable);

          stdin = inOrig;

          ret = sematics(derivationTree, &HTable);
          if (ret == 0) {
              //printTree(derivationTree);
              ret = interpret();
          }
          IAL_htDispose(&HTable); 
      }
      freeTree(&derivationTree);
      return ret;
}

