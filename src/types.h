#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type variable ID and level for generalization
typedef int typevar_id;
typedef int level;

// Forward declarations
typedef struct Type Type;
typedef struct TypeVar TypeVar;
typedef struct TypeEnv TypeEnv;
typedef struct PolyType PolyType;

// Type variable structure
struct TypeVar {
  enum { BOUND,
         UNBOUND } kind;
  union {
    Type *type; // For BOUND
    struct {
      typevar_id id;
      level level;
    } free; // For UNBOUND
  } data;
};

// Type structure
struct Type {
  enum { TYPE_UNIT,
         TYPE_INT,
         TYPE_BOOL,
         TYPE_VAR,
         TYPE_FUNCTION } kind;

  union {
    TypeVar *var; // For TYPE_VAR
    unsigned int *int_val;
    bool *bool_val;
    struct {
      Type *param;
      Type *result;
    } function; // For TYPE_FUNCTION
  } data;
};

Type *new_MT_type(int kind);

// Polymorphic type (forall a1,...,an. T)
struct PolyType {
  unsigned int num_typevars;
  typevar_id *typevars;
  Type *type;
};

// Type environment for type inference
struct TypeEnv {
  char *name;
  PolyType *type;
  TypeEnv *next;
};
// Collect all type variables that should be generalized
typedef struct TVList {
  typevar_id id;
  struct TVList *next;
} TVList;

typedef struct TVMap {
  typevar_id id;
  Type *type;
  struct TVMap *next;
} TVMap;
typedef struct {
  typevar_id id;
  char *name;
} VarNameEntry;

extern level current_level;
extern typevar_id current_typevar;

void enter_level();
void exit_level();
typevar_id new_typevar_id();
Type *new_typevar();
Type *type_function(Type *param, Type *result);
TypeVar *make_typevar(typevar_id id, level level);
PolyType *generalize(Type *type);
PolyType *dont_generalize(Type *type);
Type *instantiate(PolyType *polytype);
void unify(Type *t1, Type *t2);
bool occurs(typevar_id id, level lvl, Type *type);
TypeEnv *extend_type_env(char *name, PolyType *type, TypeEnv *env);
PolyType *lookup_type_env(char *name, TypeEnv *env);
void free_type(Type *type);
void free_polytype(PolyType *polytype);
void free_type_env(TypeEnv *env);
char *type_to_string(Type *type);
