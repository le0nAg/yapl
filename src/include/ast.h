#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

/* Node types */
typedef enum {
    /* Literals */
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BOOL_LITERAL,
    
    /* Identifiers and types */
    NODE_IDENTIFIER,
    NODE_TYPE,
    
    /* Binary operations */
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_MATRIX_MUL,
    
    /* Comparison operations */
    NODE_EQ,
    NODE_NE,
    NODE_LT,
    NODE_GT,
    NODE_LE,
    NODE_GE,
    NODE_PATTERN_MATCH,
    
    /* Logical operations */
    NODE_AND,
    NODE_OR,
    NODE_NOT,
    
    /* Assignment operations */
    NODE_ASSIGN,
    NODE_PLUS_ASSIGN,
    NODE_MINUS_ASSIGN,
    NODE_MUL_ASSIGN,
    NODE_DIV_ASSIGN,
    
    /* Unary operations */
    NODE_UNARY_MINUS,
    NODE_PRE_INC,
    NODE_PRE_DEC,
    NODE_POST_INC,
    NODE_POST_DEC,
    
    /* Range operations */
    NODE_RANGE_INCL,
    NODE_RANGE_EXCL,
    NODE_RANGE_STEP,
    
    /* Statements */
    NODE_COMPOUND,
    NODE_IF,
    NODE_IF_ELSE,
    NODE_WHILE,
    NODE_FOR,
    NODE_FOR_RANGE,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_EXPR_STMT,
    
    /* Declarations */
    NODE_VAR_DECL,
    NODE_ARRAY_DECL,
    NODE_FUNC_DECL,
    NODE_PARAM,
    
    /* Expressions */
    NODE_ARRAY_INDEX,
    NODE_FUNC_CALL,
    NODE_ARRAY_LITERAL,
    
    /* Program */
    NODE_PROGRAM,
    NODE_STMT_LIST,
    NODE_DECL_LIST,
    NODE_PARAM_LIST,
    NODE_ARG_LIST,
    NODE_INIT_LIST
} NodeType;

/* Data type enumeration */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_MATRIX,
    TYPE_ARRAY,
    TYPE_UNKNOWN
} DataType;

/* Forward declaration */
struct ASTNode;

/* Type information */
typedef struct {
    DataType base_type;
    int is_array;
    int array_size;
} TypeInfo;

/* AST Node structure */
typedef struct ASTNode {
    NodeType type;
    TypeInfo data_type;
    int line_number;
    
    union {
        /* Literal values */
        struct {
            int value;
        } int_literal;
        
        struct {
            double value;
        } float_literal;
        
        struct {
            char *value;
        } string_literal;
        
        struct {
            int value;
        } bool_literal;
        
        /* Identifier */
        struct {
            char *name;
        } identifier;
        
        /* Binary operations */
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;
        
        /* Unary operations */
        struct {
            struct ASTNode *operand;
        } unary_op;
        
        /* Range */
        struct {
            struct ASTNode *start;
            struct ASTNode *end;
            struct ASTNode *step;  /* NULL if no step */
        } range;
        
        /* Variable declaration */
        struct {
            TypeInfo type;
            char *name;
            struct ASTNode *initializer;  /* NULL if no initializer */
        } var_decl;
        
        /* Array declaration */
        struct {
            TypeInfo type;
            char *name;
            struct ASTNode *size;
            struct ASTNode *initializer;  /* NULL if no initializer */
        } array_decl;
        
        /* Function declaration */
        struct {
            TypeInfo return_type;
            char *name;
            struct ASTNode *params;  /* Parameter list */
            struct ASTNode *body;    /* Compound statement */
        } func_decl;
        
        /* Parameter */
        struct {
            TypeInfo type;
            char *name;
        } param;
        
        /* If statement */
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_stmt;
            struct ASTNode *else_stmt;  /* NULL if no else */
        } if_stmt;
        
        /* While statement */
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        /* For statement */
        struct {
            struct ASTNode *init;
            struct ASTNode *condition;
            struct ASTNode *increment;
            struct ASTNode *body;
        } for_stmt;
        
        /* Range-based for statement */
        struct {
            char *iterator;
            struct ASTNode *range;
            struct ASTNode *body;
        } for_range;
        
        /* Return statement */
        struct {
            struct ASTNode *value;  /* NULL for void return */
        } return_stmt;
        
        /* Function call */
        struct {
            struct ASTNode *func;  /* Function identifier or expression */
            struct ASTNode *args;  /* Argument list */
        } func_call;
        
        /* Array indexing */
        struct {
            struct ASTNode *array;
            struct ASTNode *index;
        } array_index;
        
        /* Statement/expression list */
        struct {
            struct ASTNode **items;
            int count;
            int capacity;
        } list;
        
    } data;
    
} ASTNode;

/* AST Node creation functions */

/* Literals */
ASTNode* create_int_literal(int value, int line);
ASTNode* create_float_literal(double value, int line);
ASTNode* create_string_literal(char *value, int line);
ASTNode* create_bool_literal(int value, int line);

/* Identifiers */
ASTNode* create_identifier(char *name, int line);

/* Binary operations */
ASTNode* create_binary_op(NodeType type, ASTNode *left, ASTNode *right, int line);

/* Unary operations */
ASTNode* create_unary_op(NodeType type, ASTNode *operand, int line);

/* Range */
ASTNode* create_range(NodeType type, ASTNode *start, ASTNode *end, ASTNode *step, int line);

/* Declarations */
ASTNode* create_var_decl(TypeInfo type, char *name, ASTNode *initializer, int line);
ASTNode* create_array_decl(TypeInfo type, char *name, ASTNode *size, ASTNode *initializer, int line);
ASTNode* create_func_decl(TypeInfo return_type, char *name, ASTNode *params, ASTNode *body, int line);
ASTNode* create_param(TypeInfo type, char *name, int line);

/* Statements */
ASTNode* create_if_stmt(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt, int line);
ASTNode* create_while_stmt(ASTNode *condition, ASTNode *body, int line);
ASTNode* create_for_stmt(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body, int line);
ASTNode* create_for_range(char *iterator, ASTNode *range, ASTNode *body, int line);
ASTNode* create_return_stmt(ASTNode *value, int line);
ASTNode* create_break_stmt(int line);
ASTNode* create_continue_stmt(int line);
ASTNode* create_expr_stmt(ASTNode *expr, int line);

/* Expressions */
ASTNode* create_func_call(ASTNode *func, ASTNode *args, int line);
ASTNode* create_array_index(ASTNode *array, ASTNode *index, int line);

/* Lists */
ASTNode* create_list(NodeType type, int line);
void list_append(ASTNode *list, ASTNode *item);

/* Type creation */
TypeInfo create_type(DataType base_type);
TypeInfo create_array_type(DataType base_type, int size);

/* Utility functions */
void free_ast(ASTNode *node);
void print_ast(ASTNode *node, int indent);
const char* node_type_to_string(NodeType type);
const char* data_type_to_string(DataType type);

#endif /* AST_H */