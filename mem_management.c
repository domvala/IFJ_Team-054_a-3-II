/*
Projekt IFJ16

xverny00 - Jan Verny
*/

#include "mem_management.h"

int hashCode (char *key, int size) {
	int retval = 1;
	int keylen = strlen(key);
	for ( int i=0; i<keylen; i++ )
		retval += key[i];
	return (retval % size);
}


varTable VTinit(char* c, int type, int size) {
    varTable init = malloc(sizeof(struct tTable));
    init->c = c;
    init->type = type;
    init->array = malloc(sizeof(struct tableElem)*size);
    init->size = size;
    for (int i = 0; i < init->size; i++) {
         (init->array)[i] = NULL;
    }
    return init;
}


tableElemPtr VTsearch(varTable table, char *s) {
  if (table == NULL) {
      return NULL;
  }

  int pos = hashCode(s, table->size);

    if ((table->array)[pos] != NULL) {
        tableElemPtr temp = (table->array)[pos];
	while (temp != NULL) {
	    if (strcmp(temp->id, s)) {
	        temp = temp->next;
	    } else {
		return temp;
            }
       }
    }
  return NULL;
}


int VTinsert(varTable table, char *s, int type, void* value) {
 tableElemPtr temp = VTsearch(table, s);
 if (temp == NULL) {
	 int pos = hashCode(s, table->size);
	 temp = malloc(sizeof(struct tableElem));
	 if (temp == NULL) {
		 exit(1);
	 }
	 temp->next = (table->array)[pos];
         temp->type = type;
         temp->id = malloc(sizeof(char)*(strlen(s)+1));
         if (temp->id == NULL) {
                 exit(1);
	 }
         strcpy(temp->id, s);
         temp->val = value;  
    	 (table->array)[pos] = temp;
 } else {
	 temp->val = value;
 }
}


 void VTdispose(varTable *table) {
 if (table == NULL || *table == NULL) {
     return;
 }
 for (int i = 0; i < (*table)->size; i++) {  
	 tableElemPtr temp = ((*table)->array)[i];
	 ((*table)->array)[i] = NULL;
	 tableElemPtr prev = NULL;
	 while (temp != NULL) {
		 prev = temp;
		 temp = temp->next;
                 free(prev->id);
                 free(prev->val);
		 free(prev);
	 }
 }
 //free((*table)->c);
 free((*table)->array); 
 free(*table);
}

varStack VSinit() {
    varStack init = malloc(sizeof(struct vStack));
    init->top = NULL;
    return init;
}


int VSempty(varStack stack) {
    return (int) (stack->top == NULL);
}


varTable VStop(varStack stack) {
    if (stack->top != NULL) {
        return (stack->top)->item;
    } else {
        return NULL;
    }
}


int VSpush(varStack stack, varTable item) {
    stackElemPtr temp = NULL;
    if (stack->top != NULL) {
        temp = stack->top;
    }
    stack->top = malloc(sizeof(struct stackElem));
    (stack->top)->item = item;
    (stack->top)->next = temp;
    return 0;
}


int VSpop(varStack stack) {
    stackElemPtr temp = stack->top;
    if (stack->top != NULL)
        stack->top = temp->next;
    if (temp != NULL && temp->item != NULL)
        VTdispose(&(temp->item));
    free(temp);
    return 0;
}


void VSdispose(varStack stack) {
    while (!VSempty(stack))
        VSpop(stack);
}


























































