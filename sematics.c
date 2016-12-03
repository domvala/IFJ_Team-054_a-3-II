// sematic.c
// 22. 11. 2016
// (c) Jan Oškera
// V rámci projektu do předmětů IFJ a IAL

#include "sematics.h"

int DetectStackError;

//Prototypes
int subtree_int_to_real(tTNodePtr* ptr);
int sematics(tTNodePtr ptr, IAL_HashTable *Table);
int function_control(tTNodePtr ptr, IAL_HashTable *HTable, char *ActClass);
int st_list_control (tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes);
int statement_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes);
int block_list_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes);
int call_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *returns);
int expression_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char typ);
int print_expression_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass);
void load_typ_literal (node_t key, char *typ);
int load_inner (IAL_HashTable *HTable);
int load_inner_function (IAL_HashTable *HTable, char *id, char *t);
int load_static (tTNodePtr ptr, IAL_HashTable *HTable);
int function_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, IAL_HashTable *HTable);
int static_var_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, IAL_HashTable *HTable);
char* add_class_before_name (char *class, char *name, int *error);
void load_typ (node_t key, char *typ);
char* add_typ_before_types (char typ, char *typs);
char *add_char_behind_types (char *typs, char c);
int check_exist_function(IAL_HashTable *HTable, char* ActClass, char *name);
tTNodePtr push_right_go_left (tTNodePtr ptr, tStackPtr *S);

//build structure for call function int to real, repleace int in tree
int subtree_int_to_real(tTNodePtr* ptr){
      tTNodePtr id_f = malloc (sizeof(struct tTNode));
      tTNodePtr par_list = malloc (sizeof(struct tTNode));
      tTNodePtr id_p = malloc (sizeof(struct tTNode));
      
      if (id_f != NULL && par_list != NULL && id_p != NULL){        
            id_f->LPtr = NULL;
            id_f->RPtr = NULL;
            id_f->key = ID;
            id_f->literal = malloc((strlen("IFJ16.IntToReal")+1)*sizeof(char));
            strcpy(id_f->literal,"IFJ16.IntToReal");
            
            par_list->LPtr = id_p;
            par_list->RPtr = NULL;
            par_list->key = PAR_LIST;

            id_p->LPtr = (*ptr)->LPtr;
            id_p->RPtr = (*ptr)->RPtr;
            id_p->key = (*ptr)->key;
            id_p->literal = (*ptr)->literal;
            
            (*ptr)->LPtr = id_f;
            (*ptr)->RPtr = par_list;
            (*ptr)->key = CALL;
            (*ptr)->literal = NULL;
                  
      }
      else{
            free(id_f);
            free(par_list);
            free(id_p);
            return(99);
      }

      return 0;
}

int sematics(tTNodePtr ptr, IAL_HashTable *HTable){
      tStackPtr stack;      
      tStackPtr *S = &stack;      
      char *ActClass = NULL;
      char typ;
      int error = 0;
      
      error = load_inner (HTable);
      if (error != 0){
            return error;                  
      }

      error = load_static (ptr, HTable);
      if (error != 0){
            return error;                  
      }
      
      SInit (S);
      SPush (S, ptr);
      do{
            ptr = STopPop (S);
            //CLASS_LIST
            if (ptr != NULL && ptr->key == CLASS_LIST){
                  ptr = push_right_go_left (ptr, S);
            }
            //CLASS
            if (ptr != NULL && ptr->key == CLASS){
                  ptr = push_right_go_left (ptr, S);
                  ActClass = ptr->literal;
                  continue;          
            }
            //CLASS_ITEM
            if (ptr != NULL && ptr->key == CLASS_ITEM){
                  if (ptr->LPtr != NULL)
                        ptr = push_right_go_left (ptr, S);  
            }
            //STATIC_VAR
            if (ptr != NULL && ptr->key == STATIC_VAR){
                  ptr = push_right_go_left (ptr, S);
                  //DECLARATION
                  ptr = ptr->LPtr;
                  //TYP
                  load_typ (ptr->key, &typ);
                  if ( SEmpty (S) != 1 && STop(S)->key == EXPRESSION ){
                        ptr = STopPop (S);
                        //EXPRESSION
                        error = expression_control(ptr, HTable, NULL, ActClass, typ); 
                        if (error != 0){
                              DStack (S);
                              return error;                        
                        }     
                  }
            }
            //FUNCTION
            if (ptr != NULL && ptr->key == FUNCTION){
                  error = function_control(ptr, HTable, ActClass);
                  if (error != 0){
                        DStack (S);
                        return error;                        
                  }  
            }
      }while (!SEmpty (S));
      
      return 0;
}

int check_exist_function(IAL_HashTable *HTable, char* ActClass, char *name){
      
      int error = 0;
      char *fullname;
      IAL_htItem *item;
      
      fullname = add_class_before_name (ActClass, name, &error);
      if (fullname == NULL){
            return (error == 99)? 99 : 3;
      } 

      item = IAL_htSearch(HTable, fullname);
      if (item != NULL){//Check local var name and function
            if (item->types[0] == 'F'){
                  free(fullname);
                  return 3;
            }
       }

       free(fullname);
       return 0;


}

int function_control(tTNodePtr ptr, IAL_HashTable *HTable, char *ActClass){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      char *name;
      int error = 0;
      char *vartypes;
      char *types;
      int i = 2;
      IAL_htItem *item;
      IAL_HashTable Table;
      IAL_HashTable *LHTable = &Table;
      IAL_htInit(LHTable);

      SInit (S);
      //FUNCTION
      ptr = push_right_go_left (ptr, S);
      //DECLARATION
      ptr = ptr->RPtr;
      //ID
      name = ptr->literal;
      name = add_class_before_name (ActClass, name, &error);
      if (name == NULL){
            DStack (S);   
            IAL_htDispose(LHTable); 
            return 99;       
      }
      item = IAL_htSearch(HTable, name);
      free(name);      
      types = item->types;
      ptr = STopPop (S);
      //FUNCTION 2
      ptr = push_right_go_left (ptr, S);
      //ARG_LIST
      SPush (S, ptr);
      while(SEmpty (S) != 1 && STop(S)->key == ARG_LIST){ //Load params to typs
            ptr = STopPop (S);
            if (ptr->LPtr != NULL){
                  ptr = push_right_go_left (ptr, S);
                  //DECLARATION
                  ptr = ptr->RPtr;
                  //ID
                  name = ptr->literal;

                  error = check_exist_function(HTable, ActClass, name);
                  if (error != 0){
                        DStack (S);
                        IAL_htDispose(LHTable);
                        return error;
                  }

                  if (IAL_htSearch(LHTable, name) == NULL){
                        vartypes = add_char_behind_types ("P", types[i]);
                        error = IAL_htInsert(LHTable, name, 0, vartypes);
                        if (error != 0){
                              DStack (S);
                              IAL_htDispose(LHTable);
                              return 99;
                        }
                        i++;
                        free(vartypes);
                  }
                  else{
                        DStack (S);
                        IAL_htDispose(LHTable);
                        return 3;      
                  }                           
            }
      }
      ptr = STopPop (S);
      
      //ST_LIST       
      if (ptr != NULL && ptr->key == ST_LIST){
            error = st_list_control (ptr, HTable, LHTable, ActClass, types);
            if (error != 0){
                  DStack (S);
                  IAL_htDispose(LHTable);
                  return error;
            }
      }
      
      IAL_htDispose(LHTable);
      return 0;      
}

int st_list_control (tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      char *name;
      int error = 0;
      char typ;
      char *typs;

      SInit (S);
      SPush (S, ptr);
      
      do{
            ptr = STopPop (S);
            //ST_LIST
            if (ptr != NULL && ptr->key == ST_LIST)
                  ptr = push_right_go_left (ptr, S); 
            //LOCAL_VAR 
            if (ptr != NULL && ptr->key == LOCAL_VAR){
                  ptr = push_right_go_left (ptr, S);
                  //DECLARATION
                  ptr = push_right_go_left (ptr, S);
                  //Type
                  load_typ (ptr->key, &typ); //load typ of var {I,D,S} to typ
                  if (typ == 'V'){
                        DStack (S);
                        return 6;                  
                  }
                  typs = add_typ_before_types ('P', &typ);
                  if (typs == NULL){
                        DStack (S);      
                        return 99;
                  }
                  ptr = STopPop (S);
                  //ID
                  name = ptr->literal;

                  error = check_exist_function(HTable, ActClass, name);
                  if (error != 0){
                        DStack (S);
                        return error;
                  }

                  if (IAL_htSearch(LHTable, name) == NULL){
                        error = IAL_htInsert(LHTable, name, 0, typs);
                        free(typs);
                        if (error != 0){
                              DStack (S);
                              return 99;
                        }
                  }
                  else{
                        free(typs);
                        DStack (S);
                        return 3;      
                  }
                 
     
                  //EXPRESSION
                  if (!SEmpty (S) && STop(S)->key == EXPRESSION){
                        ptr = STopPop (S);
                        error = expression_control(ptr, HTable, LHTable, ActClass, typ);
                        if (error != 0){
                              DStack (S);      
                              return error;      
                        }
                  }

            }
            //STATEMENT
            if (ptr != NULL && ptr->key == STATEMENT){
                  error = statement_control(ptr, HTable, LHTable, ActClass, ftypes);
                  if (error != 0){
                        DStack (S);
                        return error;
                  }
                  
            }
              
      }while (!SEmpty (S));
            
      return 0;
}

int statement_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      char *name;
      char *fullname;
      int error = 0;
      char typ;
      char returns;
      IAL_htItem *item;

      SInit (S);

      //STATEMENT
      ptr = ptr->LPtr;
      //ASSIGNMENT
      if (ptr->key == ASSIGNMENT){
            ptr = push_right_go_left (ptr, S);
            //ID
            name = ptr->literal;
            //Try find variable
            if (LHTable != NULL)//Try find local
                  item = IAL_htSearch(LHTable, name);
            else 
                  item = NULL;
                  
            if (item == NULL){//Try find global
                  item = IAL_htSearch(HTable, name);
                  if (item == NULL){
                        fullname = add_class_before_name (ActClass, name, &error);
                        if (error != 0){                              
                              DStack (S);   
                              return error;
                        }                        
                        item = IAL_htSearch(HTable, fullname);
                        if (item == NULL){
                              DStack (S);
                              free(fullname);                     
                              return 3;  
                        }    
                        free(fullname); 
                  }
            }

            typ = item->types[1];
            ptr = STopPop (S);
            //EXPRESSION
            error = expression_control(ptr, HTable, LHTable, ActClass, typ);
            if (error != 0){
                  DStack (S);      
                  return error;      
            }
      }

      //CONDITION
      if (ptr->key == CONDITION){ //if statement
            ptr = push_right_go_left (ptr, S);
            //COMPARISON
            if (ptr->literal == NULL){//check emty condition
                  DStack (S);
                  return 4;
            }
            ptr = push_right_go_left (ptr, S);
            //EXPRESSION
            if (ptr != NULL){
                  error = expression_control(ptr, HTable, LHTable, ActClass, 'D');
                  if (error != 0){
                        DStack (S);
                        return error;
                  }            
            }
            if (STop (S)->key == EXPRESSION){
                  ptr = STopPop (S);
                  //EXPRESSION
                  error = expression_control(ptr, HTable, LHTable, ActClass, 'D');
                  if (error != 0){
                        DStack (S);
                        return error;
                  } 
            }
            ptr = STopPop (S);
            //CONDITION 2
            if (ptr->LPtr != NULL){
                  //BLOCK_LIST
                  error = block_list_control(ptr->LPtr, HTable, LHTable, ActClass, ftypes);
                  if (error != 0){
                        DStack (S);
                        return error;
                  }      
      
                  //BLOCK_LIST
                  if (ptr->RPtr != NULL){
                        error = block_list_control(ptr->RPtr, HTable, LHTable, ActClass, ftypes);
                        if (error != 0){
                              DStack (S);
                              return error;
                        }
                  }
            }       
      }
      //CYCLE
      if (ptr->key == CYCLE){
            ptr = push_right_go_left (ptr, S);
            //COMPARISON
            if (ptr->literal == NULL){//check emty condition
                  DStack (S);
                  return 4;
            }
            ptr = push_right_go_left (ptr, S);
            //EXPRESSION
            if (ptr != NULL){
                  error = expression_control(ptr, HTable, LHTable, ActClass, 'D');
                  if (error != 0){
                        DStack (S);
                        return error;
                  }      
            }
            if (STop (S)->key == EXPRESSION){
                  ptr = STopPop (S);
                  //EXPRESSION
                  error = expression_control(ptr, HTable, LHTable, ActClass, 'D');
                  if (error != 0){
                        DStack (S);
                        return error;
                  } 
            }
            ptr = STopPop (S);
            //BLOCK_LIST
            error = block_list_control(ptr, HTable, LHTable, ActClass, ftypes);
            if (error != 0){
                  DStack (S);
                  return error;
            }
      }

      //CALL
      if (ptr->key == CALL){
            error = call_control(ptr, HTable, LHTable, ActClass, &returns);
            if (error != 0){
                  DStack (S);      
                  return error;      
            }
      }

      //RETURN
      if (ptr->key == RETURN){
            ptr = ptr->LPtr;
            if (ptr != NULL){
                  error = expression_control(ptr, HTable, LHTable, ActClass, ftypes[1]);
                  if (error != 0){
                        DStack (S);      
                        return error;      
                  }         
            }
      } 
      return 0;               
}

int block_list_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *ftypes){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      int error = 0;

      SInit (S);
      SPush (S, ptr);

      do{
            ptr = STopPop (S);
            //BLOCK_LIST
            ptr = push_right_go_left (ptr, S);
            //STATEMENT
            if (ptr != NULL){
                  error = statement_control(ptr, HTable, LHTable, ActClass, ftypes);
                  if (error != 0){
                        DStack (S);      
                        return error;      
                  }      
            }
            
      }while(!SEmpty (S));

      return 0;
}

int call_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char *returns) {
      char *fullname;
      IAL_htItem *item;
      char *ftypes;      
      char *name;
      int fspecial = 0;
      int error = 0;
      int i = 2;
      tStackPtr stack;      
      tStackPtr *S = &stack;

      SInit (S);
      //CALL 
      ptr = push_right_go_left (ptr, S);
      //ID      
      name = ptr->literal;
      if (strcmp("ifj16.print",name) == 0)
            fspecial = 1;

      //Try find function
      item = IAL_htSearch(HTable, name);
      if (item == NULL){
            fullname = add_class_before_name (ActClass, name, &error);
            if (error != 0){                              
                  DStack (S);                  
                  return error;                  
            }                        
            item = IAL_htSearch(HTable, fullname);
            if (item == NULL){
                  free(fullname);
                  DStack (S);                     
                  return 3;  
            }    
            free(fullname); 
      }

      if ( (item->types)[0] != 'F'){
            DStack (S);
            return 3;
      } 
      ftypes = item->types;
      *returns = ftypes[1];
      
      do{
            ptr = STopPop (S);
            //PAR_LIST
            if (ptr != NULL && ptr->key == PAR_LIST)          
                  ptr = push_right_go_left (ptr, S);
            //ID
            if (ptr != NULL && ptr->key == ID){
                  name = ptr->literal;
                  //Try find variable
                  if (LHTable != NULL)//Try find local
                        item = IAL_htSearch(LHTable, name);
                  else
                        item = NULL;
                  
                  if (item == NULL){//Try find global
                        item = IAL_htSearch(HTable, name);
                        if (item == NULL){
                              fullname = add_class_before_name (ActClass, name, &error);
                              if (error != 0){                              
                                    DStack (S);   
                                    return error;
                              }                        
                              item = IAL_htSearch(HTable, fullname);
                              if (item == NULL){
                                    DStack (S);
                                    free(fullname);                     
                                    return 3;  
                              }    
                              else{
                                    free(ptr->literal);
                                    ptr->literal = fullname; 
                              }
                              fullname = NULL; 
                        }
                  }
                  
                  if ( (item->types)[0] != 'P'){
                        DStack (S);
                        return 3;
                  }

                  if (!fspecial){
                        //Control param type                  
                        if (ftypes[i] != item->types[1]){
                              DStack (S);
                              return 4;
                        }
                  }
                  i++;
            }

            //EXPRESSION
            if (ptr != NULL && ptr->key == EXPRESSION){
                  if (!fspecial){
                        error = expression_control(ptr, HTable, LHTable, ActClass, ftypes[i]);
                  }
                  else{
                        error = print_expression_control(ptr, HTable, LHTable, ActClass);
                  }

                  if (error != 0){
                        DStack (S);                        
                        return error;
                  }
                  i++;       
            }

      }while (!SEmpty (S));
 
      if (!fspecial){
            if (ftypes[i] != '\0')
                  return 4;
      }
      else{
            if (i == 2)
                  return 4;
      }    

        return 0;
}

int print_expression_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      int error = 0;
      char returns;
      char *name;
      char *fullname;
      IAL_htItem *item;
      
      SInit (S);
      SPush (S, ptr);
      
      do{
            ptr = STopPop (S);
            //EXPRESSION  
            if (ptr != NULL){
                  while (ptr->key == EXPRESSION){          
                        ptr = push_right_go_left (ptr, S);
                  }           
             }
            
            //TERM
            if (ptr != NULL && ptr->key == TERM)          
                  ptr = push_right_go_left (ptr, S);

            //CALL
            if (ptr != NULL && ptr->key == CALL){
                  error = call_control(ptr, HTable, LHTable, ActClass, &returns);
                  if(error != 0){
                        DStack (S);
                        return error;
                  }                  
            }

            //ID
            if (ptr != NULL && ptr->key == ID){
                  name = ptr->literal;
                  //Try find variable
                  if (LHTable != NULL){//Try find local
                        item = IAL_htSearch(LHTable, name);
                  }
                  else{
                        item = NULL;
                  }

                  if (item == NULL){//Try find global
                        item = IAL_htSearch(HTable, name);
                        if (item == NULL){
                              fullname = add_class_before_name (ActClass, name, &error);
                              if (error != 0){                              
                                    DStack (S);                       
                                    return error;
                              }                        
                              item = IAL_htSearch(HTable, fullname);
                              if (item == NULL){
                                    DStack (S);
                                    free(fullname);                           
                                    return 3;                                    
                              }
                              else{ //full id to tree
                                    free(ptr->literal);
                                    ptr->literal = fullname;      
                              }                     
                              fullname = NULL;
                        }
                  }
                  
                  if ( (item->types)[0] != 'P'){
                        DStack (S);
                        return 3;
                  }      
            }          

      }while (!SEmpty (S));

      return 0;        
}

int expression_control(tTNodePtr ptr, IAL_HashTable *HTable, IAL_HashTable *LHTable, char *ActClass, char typ){
      tStackPtr stack;      
      tStackPtr *S = &stack;
      int error = 0;
      char returns;
      char *name;
      char *fullname;
      IAL_htItem *item;
      char tvar; 
      
      SInit (S);
      SPush (S, ptr);
      
      do{
            ptr = STopPop (S);
            //EXPRESSION  
            if (ptr != NULL){
                  while (ptr->key == EXPRESSION){          
                        ptr = push_right_go_left (ptr, S);
                  }           
             }
         
            
            //TERM
            if (ptr != NULL && ptr->key == TERM)          
                  ptr = push_right_go_left (ptr, S);

            //CALL
            if (ptr != NULL && ptr->key == CALL){
                  error = call_control(ptr, HTable, LHTable, ActClass, &returns);
                  if(error != 0){
                        DStack (S);
                        return error;
                  }

                  //Check return typ of function                  
                  if (returns != typ){
                        if (typ == 'D' && returns == 'I'){
                              error = subtree_int_to_real(&ptr); //repleace int to int_to_real
                              if (error != 0){
                                    DStack (S);
                                    return error;
                              }
                        }
                        else{
                              DStack (S);
                              return 4;
                        }
                  }                   
            }

            //ID
            if (ptr != NULL && ptr->key == ID){
                  name = ptr->literal;
                  //Try find variable
                  if (LHTable != NULL){//Try find local
                        item = IAL_htSearch(LHTable, name);
                  }
                  else{
                        item = NULL;
                  }

                  if (item == NULL){//Try find global
                        item = IAL_htSearch(HTable, name);
                        if (item == NULL){
                              fullname = add_class_before_name (ActClass, name, &error);
                              if (error != 0){                              
                                    DStack (S);                       
                                    return error;
                              }                        
                              item = IAL_htSearch(HTable, fullname);
                              if (item == NULL){
                                    DStack (S);
                                    free(fullname);                           
                                    return 3;                                    
                              }
                              else{ //full id to tree
                                    free(ptr->literal);
                                    ptr->literal = fullname;      
                              }                     
                              fullname = NULL;
                        }
                  }
                  
                  if ( (item->types)[0] != 'P'){
                        DStack (S);
                        return 3;
                  }      
                  tvar = (item->types)[1];
                  if (tvar != typ){
                        if (typ == 'D' && tvar == 'I'){
                              error = subtree_int_to_real(&ptr); //repleace int to int_to_real
                              if (error != 0){
                                    DStack (S);
                                    return error;
                              }
                        }
                        else{
                              DStack (S);
                              return 4;
                        }       
                  }
            }          
                            
            //LITERAL
            if (ptr != NULL && (ptr->key == STRING || ptr->key == INT || ptr->key == DOUBLE) ){
                  load_typ_literal (ptr->key, &tvar);
                  
                  if (tvar != typ){
                        if (typ == 'D' && tvar == 'I'){
                              error = subtree_int_to_real(&ptr); //repleace int to int_to_real
                              if (error != 0){
                                    DStack (S);
                                    return error;
                              }
                        }
                        else{
                              DStack (S);
                              return 4;
                        }  
                  }                          
            }

      }while (!SEmpty (S));

      return 0;            
}

void load_typ_literal (node_t key, char *typ){
      if (key == STRING)
            *typ = 'S';
      else
            if (key == INT)
                  *typ = 'I';
            else
                  *typ = 'D';                                     
}

int load_inner (IAL_HashTable *HTable){
      int error = 0;

      error = load_inner_function (HTable, "ifj16.readInt", "FI");
      if (error != 0)
            return 99;       
      error = load_inner_function (HTable, "ifj16.readDouble", "FD");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.readString", "FS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.print", "FVS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.length", "FIS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.substr", "FSSII");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.compare", "FISS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.find", "FISS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.sort", "FSS");
      if (error != 0)
            return 99;
      error = load_inner_function (HTable, "ifj16.IntToReal", "FDI");
      if (error != 0)
            return 99;

      return 0;
}

int load_inner_function (IAL_HashTable *HTable, char *id, char *t){
      char *name;
      char *types;
      size_t size;
      int error = 0;

      size = strlen (id) + 1;
      name = malloc(size); 
      name = strcpy(name, id);
      
      size = strlen (t) + 1;
      types = malloc(size);
      types = strcpy(types, t);

      error = IAL_htInsert(HTable, name, 0, types);
      free(name);
      free(types);
      
      if (error != 0)
            return error;     
      else
            return 0;
}

int load_static (tTNodePtr ptr, IAL_HashTable *HTable){      
      tStackPtr stack;      
      tStackPtr *S = &stack;
      char *ActClass = NULL;
      int error = 0;
      IAL_HashTable Table;
      IAL_HashTable *CHTable = &Table; //Class table
 
      IAL_htInit(CHTable);
      IAL_htInsert(CHTable, "ifj16", 0, "C"); //not redefinition ifj16 class
      SInit (S);
      SPush (S, ptr);
      do{
            ptr = STopPop (S);
            //CLASS_LIST
            if (ptr != NULL && ptr->key == CLASS_LIST){
                  ptr = push_right_go_left (ptr, S);
            }
            
            //CLASS
            if (ptr != NULL && ptr->key == CLASS){
                  ptr = push_right_go_left (ptr, S);
                  ActClass = ptr->literal;  
                  if (error = IAL_htInsert(CHTable, ActClass, 0, "C") != 0){ //Control class redefinition
                        DStack (S);
                        IAL_htDispose(CHTable);
                        return (error == 99)? 99 : 3;
                  }
                  continue;            
            }
            //CLASS_ITEM
            if (ptr != NULL && ptr->key == CLASS_ITEM){
                  if (ptr->LPtr != NULL)
                        ptr = push_right_go_left (ptr, S);
            }
            //STATIC_VAR
            if (ptr != NULL && ptr->key == STATIC_VAR){
                  error = static_var_htinsert(&ptr, S, ActClass, HTable);
                  if (error != 0){
                        IAL_htDispose(CHTable);
                        DStack (S);
                        return error;
                  }      
            }
            //FUNCTION
            if (ptr != NULL && ptr->key == FUNCTION){  
                  error = function_htinsert(&ptr, S, ActClass, HTable);
                  if (error != 0){
                        IAL_htDispose(CHTable);
                        DStack (S);
                        return error;
                  }                        
            }
      }while (!SEmpty (S));
      
      IAL_htDispose(CHTable);
      //Check exist Main.run in file
      return (IAL_htSearch(HTable, "Main.run") != NULL)? 0 : 3;     
}      

int function_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, IAL_HashTable *HTable){
      char typ;
      char *typs = NULL;
      char *name = NULL; 
      char *help = NULL;    
      int error = 0;  
      tTNodePtr ptr;
   
      ptr = *original;
      SPush (S, ptr->RPtr);
      ptr = ptr->LPtr;
      //DECLARATION
      SPush (S, ptr->RPtr);      
      ptr = ptr->LPtr;
      //Typ
      load_typ (ptr->key, &typ);
      typs = add_typ_before_types ('F', &typ);
      ptr = STopPop (S);
      //ID
      name = ptr->literal;
      name = add_class_before_name (ActClass, name, &error);
      if (name == NULL){
            free(typs);
            DStack (S);      
            return (error == 99) ? 99 : 3;       
      }
      ptr = STopPop (S);
      //FUNCTION 2
      ptr = ptr->LPtr;
      //ARG_LIST
      SPush (S, ptr);
      while(SEmpty (S) != 1 && (STop(S))->key == ARG_LIST){ //Load params to typs
            ptr = STopPop (S);
            if (ptr->LPtr != NULL){
                  ptr = push_right_go_left (ptr, S);
                  //DECLARATION
                  ptr = ptr->LPtr;
                  //TYP
                  load_typ (ptr->key, &typ);
                  help = add_char_behind_types (typs, typ);
                  free(typs);
                  if (help == NULL){
                        free(name);
                        DStack (S);      
                        return 99;                             
                  }
                  typs = help;           
            }
      }
      if (IAL_htSearch(HTable, name) == NULL){
            error = IAL_htInsert(HTable, name, 0, typs);
            free(name);
            free(typs);
            if (error != 0){
                  DStack (S);
                  return 99;
            }           
      }
      else{
            free(name);
            free(typs);
            DStack (S);
            return 3;      
      }

      *original = ptr;
      return 0;
         
}

int static_var_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, IAL_HashTable *HTable){
      char typ;
      char *typs = NULL;
      char *name = NULL;
      int i = 0;    
      int error = 0;  
      tTNodePtr ptr; 

      ptr = *original;
      ptr = ptr->LPtr;
      //DECLARATION      
      SPush (S, ptr->RPtr);      
      ptr = ptr->LPtr;
      //Type
      load_typ (ptr->key, &typ); //load typ of var {I,D,S} to typ
      if (typ == 'V'){
            free(typs);
            DStack (S);
            return 6;                  
      }
      typs = add_typ_before_types ('P', &typ);
      if (typs == NULL){
            DStack (S);      
            return 99;
      }
      ptr = STopPop (S);
      //ID
      name = ptr->literal;
      name = add_class_before_name (ActClass, name, &error);
      if (name == NULL){
            free(typs);
            DStack (S);      
            return (error == 99) ? 99 : 3;       
      }
      if (IAL_htSearch(HTable, name) == NULL){
            error = IAL_htInsert(HTable, name, i++, typs);
            free(name);
            free(typs);           
            if (error != 0){
                  DStack (S);
                  return 99;
            }
      }
      else{
            free(name);
            free(typs);
            DStack (S);
            return 3;      
      }
      *original = ptr;
      return 0; 
}


tTNodePtr push_right_go_left (tTNodePtr ptr, tStackPtr *S){
       if (ptr->RPtr != NULL)
            SPush (S, ptr->RPtr);
       return ptr->LPtr;
}

char* add_class_before_name (char *class, char *name, int *error){
      size_t size;
      char* new;

      if (strchr(name, (int)'.') != NULL){
            return NULL;      
      }

      size = strlen(class) + strlen(name) + 2;
      new = malloc (size);
      if (new != NULL){
            new = strcpy(new, class);
            new = strcat(new, ".");
            new = strcat(new, name);
      }
      else
            *error = 99;

      return new;          
}

void load_typ (node_t key, char *typ){
      if (key == STRING_DATA)
            *typ = 'S';
      else{
            if (key == INT_DATA)
                  *typ = 'I';
            else
                  if (key == DOUBLE_DATA)
                        *typ = 'D';
                  else                  
                        *typ = 'V'; 
      }                       
}

char* add_typ_before_types (char typ, char *typs){
      size_t size;
      char* new;
      
      size = strlen(typs) + 2;
      new = malloc (size);
      if (new != NULL){
            new[0] = typ;
            new[1] = '\0';
            new = strcat(new, typs);
      }
      return new;
}

char *add_char_behind_types (char *typs, char c){
      size_t size;
      char* new;
      
      size = strlen(typs);
      new = malloc (size + 2);
      if (new != NULL){
            new = strcpy(new, typs);
            new[size] = c;
            new[size+1] = '\0';                 
      }

      return new;
}

