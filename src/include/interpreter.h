#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

/* ValueType enum */
typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING,
    VAL_BOOL,
    VAL_VOID,
    VAL_ARRAY,
    VAL_MATRIX,
    VAL_FUNC
} ValueType;

/* Matrix structure */
typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;

/* Value union */
typedef struct Value {
    ValueType type;
    union {
        int int_val;
        double float_val;
        char *string_val;
        int bool_val;
        struct ASTNode *func_node;
        struct {
            struct Value *elements;
            int size;
        } array_val;
        Matrix *matrix_val;
    } data;
} Value;

/* Symbol table entry */
typedef struct Symbol {
    char *name;
    Value value;
    struct Symbol *next;
} Symbol;

/* SymbolTable struct */
typedef struct SymbolTable {
    Symbol *head;
    struct SymbolTable *parent;
    Value return_value;
    int is_returning;
} SymbolTable;

/* Function to execute the AST */
void execute_program(ASTNode *root);

/* Value operations */
Value create_int_value(int val);
Value create_float_value(double val);
Value create_string_value(char *val);
Value create_bool_value(int val);
Value create_void_value();
Value create_matrix_value(int rows, int cols);
void free_value(Value *val);
void print_value(Value val);

/* Matrix operations */
Matrix* create_matrix(int rows, int cols);
void free_matrix(Matrix *mat);
Matrix* matrix_multiply(Matrix *a, Matrix *b);
void print_matrix(Matrix *mat);

/* Symbol table operations */
SymbolTable* create_symbol_table(SymbolTable *parent);
void free_symbol_table(SymbolTable *table);
void set_symbol(SymbolTable *table, const char *name, Value value);
Value* get_symbol(SymbolTable *table, const char *name);

#endif /* INTERPRETER_H */