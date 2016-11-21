#include "sematics.h"

int DetectStackError;

//build structure for call function int to real, repleace int in tree
int subtree_int_to_real(tTNodePtr* ptr){
      tTNodePtr id_f = malloc (struct tTNode);
      tTNodePtr par_list = malloc (struct tTNode);
      tTNodePtr param = malloc (struct tTNode);
      tTNodePtr id_p = malloc (struct tTNode);
      
      if (id_f != NULL && par_list != NULL && param != NULL && id_p != NULL){        
            id_f->LPtr = NULL;
            id_f->RPtr = NULL;
            id_f->key = IDENTIFIER;
            id_f->literal = "IntToReal";
            
            par_list->LPtr = param;
            par_list->RPtr = NULL;
            par_list->key = PAR_LIST;
                  
            param->LPtr = id_p;
            param->RPtr = NULL;
            param->key = PAR_LIST;

            id_p->LPtr = (*ptr)->LPtr;
            id_p->RPtr = (*ptr)->RPtr;
            id_p->key = (*ptr)->key;
            id_p->literal = (*ptr)->literal;
            
            (*ptr)->LPtr = id_f;
            (*ptr)->RPtr = par_list;
            (*ptr)->key = CALL;
            (*ptr)->literal = "";
                  
      }
      else{
            free(id_f);
            free(par_list);
            free(param);
            free(id_p);
            return(99);
      }

      return 0;
}

int sematics(tTNodePtr ptr){
      tStackPtr *S;      
      char *ActClass = NULL;
      char typ;
      int error = 0;

      parsHT_Table *SHTable = malloc(sizeof(parsHT_Table));  //check if tonda want this    
      if (SHTable == NULL)
            return 99;
      
      parsHT_Init(SHTable);
      error = load_static (ptr, SHTable);
      if (error != 0){
            parsHT_Dispose(SHTable);
            free(SHTable);
            return error;                  
      }
      
      SInit (S);
      //CLASS_LIST
      if (ptr != NULL && ptr->key == CLASS_LIST){
            ptr = push_right_go_left (ptr, S);
      }
      //CLASS
      if (ptr != NULL && ptr->key == CLASS){
            ptr = push_right_go_left (ptr, S);
            ActClass = ptr->literal;
            continue;
            //ptr = STopPop (S);            
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
            if ( (STop(S))->item->key == EXPRESSION ){
                  ptr = STopPop (S);
                  //EXPRESSION
                  error = expression_control(ptr, SHTable, ActClass, typ); 
                  if (error != 0){
                        DStack (S);
                        parsHT_Dispose(SHTable);
                        free(SHTable);
                        return error;                        
                  }     
            }
      }
      //FUNCTION
      if (ptr != NULL && ptr->key == FUNCTION){
      //TODO
      }
}

int function_control(tTNodePtr ptr, parsHT_Table *HTable, char *ActClass){
      tStackPtr *S;
      char *name;
      int error = 0;
      char *types;
      int i = 2;
      patsHT_Item *item;
      parsHT_Table *LHTable = malloc(sizeof(parsHT_Table));
      parsHT_Init(LHTable);

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
            free (LHTable);      
            return 99;       
      }
      item = parsHT_Search(HTable, name);
      free(name);      
      types = item->literal;
      ptr = STopPop (S);
      //FUNCTION 2
      ptr = push_right_go_left (ptr, S);
      //ARG_LIST
      SPush (S, ptr);
      while(SEmpty (S) != 1 && (STop(S))->item->key == ARG_LIST){ //Load params to typs
            ptr = STopPop (S);
            if (ptr->LPtr != NULL){
                  ptr = push_right_go_left (ptr, S);
                  //DECLARATION
                  ptr = ptr->RPtr;
                  //ID
                  name = ptr->literal;
                  if (parsHT_Search(LHTable, name) == NULL){
                        parsHT_Insert(LHTable, name, &(typs[i]));
                        i++;
                  }
                  else{
                        DStack (S);
                        parsHT_Dispose(LHTable);
                        free (LHTable);
                        return 3,      
                  }                           
            }
      }
      ptr = STopPop (S);
      //ST_LIST
            
      if (ptr != NULL && ptr->key == ST_LIST){
            error = st_list_control (ptr, HTable, LHTable, ActClass);
            if (error != 0){
                  DStack (S);
                  parsHT_Dispose(LHTable);
                  free(LHTable);
                  return error;
            }
      }
      
      parsHT_Dispose(LHTable);
      free(LHTable);
      return 0;      
}

int st_list_control (tTNodePtr ptr, parsHT_Table *HTable, parsHT_Table *LHTable, char *ActClass){
      tStackPtr *S;
      char *name;
      int error = 0;
      char typ;

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
                        free(typs)
                        DStack (S);
                        return 6;                  
                  }
                  ptr = STopPop (S);
                  //ID
                  name = ptr->literal;
                  if (parsHT_Search(LHTable, name) == NULL){
                        parsHT_Insert(LHTable, name, &typ);
                  }
                  else{
                        DStack (S);
                        return 3;      
                  }
            }
            //STATEMENT
            if (ptr != NULL && ptr->key == STATEMENT){
                  ptr = ptr->LPtr;
                  if (ptr->key == ASSIGNMENT){
                        //TODO
                  }
                  if (ptr->key == CONDITION){
                        //TODO
                  }
                  if (ptr->key == CYCLE){
                        //TODO
                  }
                  if (ptr->key == CALL){
                        //TODO
                  }
                  if (ptr->key == RETURN){
                        //TODO
                  }
                  
            }  

      }while (SEmpty (S));
            
      return 0;
}

int call_control(tTNodePtr ptr, parsHT_Table *HTable, char *ActClass, char *returns){
      char *fullname;
      patsHT_Item *item;
      char *ftypes;      
      char *name;
      char typ;
      int error = 0;
      int i = 2;
      tStackPtr *S;

      SInit (S);
      //CALL 
      ptr = push_right_go_left (ptr, S);
      //ID      
      name = ptr->literal;

      //Try find function
      item = parsHT_Search(HTable, name);
      if (item == NULL){
            fullname = add_class_before_name (ActClass, name, &error);
            if (error != 0){                              
                  DStack (S);                  
                  return error;                  
            }                        
            item = parsHT_Search(HTable, fullname);
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
                  item = parsHT_Search(HTable, name);
                  if (item == NULL){
                        fullname = add_class_before_name (ActClass, name, &error);
                        if (error != 0){                              
                              DStack (S);   
                              return error;
                        }                        
                        item = parsHT_Search(HTable, fullname);
                        if (item == NULL){
                              DStack (S);
                              free(fullname);                     
                              return 3;  
                        }    
                  free(fullname); 
                  }
                  
                  if ( (item->types)[0] != 'P'){
                        DStack (S);
                        return 3;
                  }

                  //Control param type                  
                  if (ftypes[i] != item->types[1]){
                        DStack (S);
                        return 4;
                  }
                  else
                        i++;
            }

            //EXPRESSION
            if (ptr != NULL && ptr->key == EXPRESSION){
                  error = expression_control(ptr, HTable, ActClass, ftypes[i]);
                  if (error != 0){
                        DStack (S);                        
                        return error;
                  }
                  else
                        i++;       
            }

      }while (SEmpty (S));
 
      if (ftypes[i] != '\0')
            return 4;
      
      return 0;
}



int expression_control(tTNodePtr ptr, parsHT_Table *HTable, char *ActClass, char typ){
      tStackPtr *S; 
      int error = 0;
      char returns;
      char *name;
      char *fullname;
      patsHT_Item *item;
      char tvar; 
      
      SInit (S);
      SPush (S, ptr);
      
      do{
            ptr = STopPop (S);
            //EXPRESSION  
            if (ptr != NULL && ptr->key == EXPRESSION)          
                  ptr = push_right_go_left (ptr, S);
            
            //TERM
            if (ptr != NULL && ptr->key == TERM)          
                  ptr = push_right_go_left (ptr, S);

            //ID
            if (ptr != NULL && ptr->key == ID){
                  name = ptr->literal;
                  //Try find variable
                  item = parsHT_Search(HTable, name);
                  if (item == NULL){
                        fullname = add_class_before_name (ActClass, name, &error);
                        if (error != 0){                              
                              DStack (S);                        
                              return error;
                        }                        
                        item = parsHT_Search(HTable, fullname);
                        if (item == NULL){
                              DStack (S);
                              free(fullname);                              
                              return 3;                                    
                        }                  
                        free(fullname);                  
                  }
                  
                  if ( (item->types)[0] != 'P'){
                        DStack (S);
                        return 3;
                  }      
                  tvar = (item->types)[1];
                  if (tvar != typ)
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
            
            //CALL
            if (ptr != NULL && ptr->key == CALL){
                  call_control(ptr, HTable, ActClass, &returns);
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

      }while (SEmpty (S));

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

int load_static (tTNodePtr ptr, parsHT_Table *HTable){      
      tStackPtr *S;
      char *ActClass = NULL;
      int error = 0;

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
                  //ptr = STopPop (S);            
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
                        DStack (S);
                        return error;
                  }      
            }
            //FUNCTION
            if (ptr != NULL && ptr->key == FUNCTION){  
                  error = function_htinsert(&ptr, S, ActClass, HTable);
                  if (error != 0){
                        DStack (S);
                        return error;
                  }                        
            }
      }while (SEmpty (S));
      
      return 0;     
}      

int function_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, parsHT_Table *HTable){
      char typ = '';
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
            return (error == 99) ? 99 : 6;       
      }
      ptr = STopPop (S);
      //FUNCTION 2
      ptr = ptr->LPtr;
      //ARG_LIST
      SPush (S, ptr);
      while(SEmpty (S) != 1 && (STop(S))->item->key == ARG_LIST){ //Load params to typs
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
      if (parsHT_Search(HTable, name) == NULL){
            parsHT_Insert(HTable, name, typs);
            free(name);
            free(typs);           
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

int static_var_htinsert(tTNodePtr *original, tStackPtr *S, char *ActClass, parsHT_Table *HTable){
      char typ = '';
      char *typs = NULL;
      char *name = NULL;    
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
            free(typs)
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
            return (error == 99) ? 99 : 6;       
      }
      if (parsHT_Search(HTable, name) == NULL){
            parsHT_Insert(HTable, name, typs);
            free(name);
            free(typs);           
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

char* add_typ_before_types (char typ, char *typs){
      size_t size;
      char* new;
      
      size = strlen(typs) + 2;
      new = malloc (size);
      if (new != NULL){
            new = strcpy(new, &typ);
            new = strcat(new, typs);
      }
      return new;
}

char *add_char_behind_types (char *typs, char c){
      size_t size;
      char* new;
      
      size = strlen(typs) + 2;
      new = malloc (size);
      if (new != NULL){
            new = strcpy(new, typs);
            new = strcat(new, &c);
      }

      return new;
}
