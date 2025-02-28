#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

level current_level = 0;
typevar_id current_typevar = 0;

void enter_level() {
  current_level++;
}
void exit_level() {
  current_level--;
}

typevar_id new_typevar_id() {
  return ++current_typevar;
}

Type *new_typevar() {
  Type *type = (Type *)malloc(sizeof(
      Type)); // create wrapper for error checking in case of failed malloc
  type->kind = TYPE_VAR;
  type->data.var = (TypeVar *)malloc(sizeof(TypeVar));
  type->data.var->kind = UNBOUND;
  type->data.var->data.free.id = new_typevar_id();
  type->data.var->data.free.level = current_level;
  return type;
}

Type *new_MT_type(int kind) {
  Type *type = (Type *)malloc(sizeof(Type));
  type->kind = kind;
  return type;
}

Type *type_function(Type *param, Type *result) {
  Type *type = (Type *)malloc(sizeof(Type));
  type->kind = TYPE_FUNCTION;
  type->data.function.param = param;
  type->data.function.result = result;
  return type;
}

TypeVar *make_typevar(typevar_id id, level level) {
  TypeVar *var = (TypeVar *)malloc(sizeof(TypeVar));
  var->kind = UNBOUND;
  var->data.free.id = id;
  var->data.free.level = level;
  return var;
}

// Check if type variable occurs in a type (occurs check)
bool occurs(typevar_id id, level lvl, Type *type) {
  switch (type->kind) {
  case TYPE_INT:
  case TYPE_BOOL:
  case TYPE_UNIT:
    return false;
  case TYPE_VAR:
    if (type->data.var->kind == BOUND) {
      return occurs(id, lvl, type->data.var->data.type);
    } else {
      // Unbound type variable
      if (type->data.var->data.free.id == id) {
        return true;
      }
      // Update the level to the minimum of the two
      if (type->data.var->data.free.level > lvl) {
        type->data.var->data.free.level = lvl;
      }
      return false;
    }

  case TYPE_FUNCTION:
    return occurs(id, lvl, type->data.function.param) ||
           occurs(id, lvl, type->data.function.result);
  }
  // Should never reach here
  return false;
}

// Unification algorithm
void unify(Type *t1, Type *t2) {
  // First check if types are already bound
  if (t1->kind == TYPE_VAR && t1->data.var->kind == BOUND) {
    unify(t1->data.var->data.type, t2);
    return;
  }

  if (t2->kind == TYPE_VAR && t2->data.var->kind == BOUND) {
    unify(t1, t2->data.var->data.type);
    return;
  }

  // Main unification logic
  if (t1->kind == TYPE_INT && t2->kind == TYPE_INT) return;
  else if (t1->kind == TYPE_BOOL && t2->kind == TYPE_BOOL) return;
  else if (t1->kind == TYPE_UNIT && t2->kind == TYPE_UNIT) return;
  else if (t1->kind == TYPE_VAR && t2->kind == TYPE_VAR &&
             t1->data.var->kind == UNBOUND && t2->data.var->kind == UNBOUND &&
             t1->data.var->data.free.id == t2->data.var->data.free.id) {
    return; // Same type variable, already unified
  } else if (t1->kind == TYPE_VAR && t1->data.var->kind == UNBOUND) {
    // Check occurs
    typevar_id id = t1->data.var->data.free.id;
    level lvl = t1->data.var->data.free.level;
    if (occurs(id, lvl, t2)) {
      fprintf(stderr, "Type error: recursive type detected\n");
      exit(1);
    }
    // Bind t1 to t2
    t1->data.var->kind = BOUND;
    t1->data.var->data.type = t2;
  } else if (t2->kind == TYPE_VAR && t2->data.var->kind == UNBOUND) {
    // Check occurs
    typevar_id id = t2->data.var->data.free.id;
    level lvl = t2->data.var->data.free.level;
    if (occurs(id, lvl, t1)) {
      fprintf(stderr, "Type error: recursive type detected\n");
      exit(1);
    }
    // Bind t2 to t1
    t2->data.var->kind = BOUND;
    t2->data.var->data.type = t1;
  } else if (t1->kind == TYPE_FUNCTION && t2->kind == TYPE_FUNCTION) {
    // Both are function types, unify parameter and result
    unify(t1->data.function.param, t2->data.function.param);
    unify(t1->data.function.result, t2->data.function.result);
  } else {
    fprintf(stderr, "Type error: cannot unify types\n");
    exit(1);
  }
}
void collect_typevars(Type *t, TVList *tvs, unsigned int *count) {
  switch (t->kind) {
  case TYPE_INT:
  case TYPE_BOOL:
  case TYPE_UNIT:
    break;
  case TYPE_VAR:
    if (t->data.var->kind == BOUND) {
      collect_typevars(t->data.var->data.type, tvs, count);
    } else {
      // Check if level is deeper than current level
      if (t->data.var->data.free.level > current_level) {
        // Check if already in list
        bool found = false;
        TVList *cur = tvs;
        while (cur != NULL) {
          if (cur->id == t->data.var->data.free.id) {
            found = true;
            break;
          }
          cur = cur->next;
        }

        if (!found) {
          // Add to list
          TVList *new_tv = (TVList *)malloc(sizeof(TVList));
          new_tv->id = t->data.var->data.free.id;
          new_tv->next = tvs;
          tvs = new_tv;
          (*count)++;
        }
      }
    }
    break;

  case TYPE_FUNCTION:
    collect_typevars(t->data.function.param, tvs, count);
    collect_typevars(t->data.function.result, tvs, count);
    break;
  }
}

// Generalize a type to a polymorphic type
PolyType *generalize(Type *type) {
  TVList *tvs = NULL;
  unsigned int count = 0;
  // Collect type variables
  collect_typevars(type, tvs, &count);

  // Create polytype
  PolyType *polytype = (PolyType *)malloc(sizeof(PolyType));
  polytype->num_typevars = count;
  polytype->type = type;

  if (count > 0) {
    // Fill typevar array from the list
    polytype->typevars = (typevar_id *)malloc(sizeof(typevar_id) * count);
    TVList *cur = tvs;
    for (int i = 0; i < count; i++) {
      polytype->typevars[i] = cur->id;
      TVList *tmp = cur;
      cur = cur->next;
      free(tmp);
    }
  } else {
    polytype->typevars = NULL;
  }

  return polytype;
}

// Create a non-generalized polytype (for lambda parameters)
PolyType *dont_generalize(Type *type) {
  PolyType *polytype = (PolyType *)malloc(sizeof(PolyType));
  polytype->num_typevars = 0;
  polytype->typevars = NULL;
  polytype->type = type;
  return polytype;
}

Type *instantiate_type(Type *t, TVMap *map) {
  switch (t->kind) {
  case TYPE_UNIT:
  case TYPE_INT:
  case TYPE_BOOL:
    return new_MT_type(t->kind);
  case TYPE_VAR:
    if (t->data.var->kind == BOUND) {
      return instantiate_type(t->data.var->data.type, map);
    } else {
      // Check if this type variable is in the map
      TVMap *cur = map;
      while (cur != NULL) {
        if (cur->id == t->data.var->data.free.id) {
          return cur->type;
        }
        cur = cur->next;
      }

      // Not in map, create a copy
      Type *new_t = (Type *)malloc(sizeof(Type));
      new_t->kind = TYPE_VAR;
      new_t->data.var = (TypeVar *)malloc(sizeof(TypeVar));
      new_t->data.var->kind = UNBOUND;
      new_t->data.var->data.free.id = t->data.var->data.free.id;
      new_t->data.var->data.free.level = t->data.var->data.free.level;
      return new_t;
    }

  case TYPE_FUNCTION:
    return type_function(instantiate_type(t->data.function.param, map),
                         instantiate_type(t->data.function.result, map));
  default:
    fprintf(stderr, "001 Not implemented");
  }
  // Should never reach here
  return NULL;
}

// Instantiate a polymorphic type into a monomorphic type
Type *instantiate(PolyType *polytype) {
  // Create a mapping from the generalized type variables to new types

  TVMap *map = NULL;

  // Create new type variables for each generalized type variable
  for (int i = 0; i < polytype->num_typevars; i++) {
    TVMap *entry = (TVMap *)malloc(sizeof(TVMap));
    entry->id = polytype->typevars[i];
    entry->type = new_typevar();
    entry->next = map;
    map = entry;
  }
  // Instantiate the type
  Type *result = instantiate_type(polytype->type, map);

  // Free the map
  while (map != NULL) {
    TVMap *tmp = map;
    map = map->next;
    free(tmp);
  }

  return result;
}

// Environment operations
TypeEnv *extend_type_env(char *name, PolyType *type, TypeEnv *env) {
  TypeEnv *new_env = (TypeEnv *)malloc(sizeof(TypeEnv));
  new_env->name = strdup(name);
  new_env->type = type;
  new_env->next = env;
  return new_env;
}

PolyType *lookup_type_env(char *name, TypeEnv *env) {
  while (env != NULL) {
    if (strcmp(env->name, name) == 0) {
      return env->type;
    }
    env = env->next;
  }
  return NULL;
}

// Memory management
void free_type(Type *type) {
  if (type == NULL)
    return;

  switch (type->kind) {
  case TYPE_UNIT:
  case TYPE_INT:
  case TYPE_BOOL:
    break;

  case TYPE_VAR:
    if (type->data.var->kind == BOUND) {
      // Don't free the bound type here to avoid cycles
      // It will be freed elsewhere
    }
    free(type->data.var);
    break;

  case TYPE_FUNCTION:
    free_type(type->data.function.param);
    free_type(type->data.function.result);
    break;
  }

  free(type);
}

void free_polytype(PolyType *polytype) {
  if (polytype == NULL)
    return;

  if (polytype->typevars != NULL) {
    free(polytype->typevars);
  }

  // Don't free the type here, it will be freed elsewhere

  free(polytype);
}

void free_type_env(TypeEnv *env) {
  while (env != NULL) {
    TypeEnv *next = env->next;
    free(env->name);
    free_polytype(env->type);
    free(env);
    env = next;
  }
}

char *get_var_name(typevar_id id, VarNameEntry *var_names, int *var_count) {
  // Check if already in map
  for (int i = 0; i < *var_count; i++) {
    if (var_names[i].id == id) {
      return var_names[i].name;
    }
  }

  // Create new name
  char *name = (char *)malloc(10);
  sprintf(name, "'%c", 'a' + *var_count);

  // Add to map
  var_names[*var_count].id = id;
  var_names[*var_count].name = name;
  (*var_count)++;
  return name;
}
// Helper: should we add parentheses around this type in a function context?
bool needs_parens(Type *t) {
  if (t->kind == TYPE_FUNCTION) {
    return true;
  }
  if (t->kind == TYPE_VAR && t->data.var->kind == BOUND) {
    return needs_parens(t->data.var->data.type);
  }
  return false;
}

char *type_to_string_rec(Type *t, bool is_function_param,
                         VarNameEntry *var_names, int *var_count) {
  char buffer[1024];

  switch (t->kind) {
  case TYPE_UNIT:
    return strdup("unit");
  case TYPE_INT:
    return strdup("int");
  case TYPE_BOOL:
    return strdup("bool");

  case TYPE_VAR:
    if (t->data.var->kind == BOUND) {
      return type_to_string_rec(t->data.var->data.type, is_function_param,
                                var_names, var_count);
    } else {
      return strdup(
          get_var_name(t->data.var->data.free.id, var_names, var_count));
    }

  case TYPE_FUNCTION: {
    char *param_str =
        type_to_string_rec(t->data.function.param, true, var_names, var_count);
    char *result_str = type_to_string_rec(t->data.function.result, false,
                                          var_names, var_count);

    if (needs_parens(t->data.function.param) && is_function_param) {
      sprintf(buffer, "(%s) -> %s", param_str, result_str);
    } else {
      sprintf(buffer, "%s -> %s", param_str, result_str);
    }

    free(param_str);
    free(result_str);
    return strdup(buffer);
  }
  }
  return strdup("unknown");
}

char *type_to_string(Type *type) {
  VarNameEntry var_names[256]; 
  int var_count = 0;
  char *result = type_to_string_rec(type, false, var_names, &var_count);
  for (int i = 0; i < var_count; i++) {
    free(var_names[i].name);
  }
  return result;
}
