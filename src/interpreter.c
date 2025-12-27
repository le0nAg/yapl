#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>

static int recursion_depth = 0;
#define MAX_RECURSION_DEPTH 50

static SymbolTable *global_table = NULL;


static Value eval_expression(ASTNode *node, SymbolTable *table);
static void execute_statement(ASTNode *node, SymbolTable *table);

Matrix* create_matrix(int rows, int cols) {
    Matrix *mat = (Matrix*)malloc(sizeof(Matrix));
    mat->rows = rows;
    mat->cols = cols;
    mat->data = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++) {
        mat->data[i] = (double*)calloc(cols, sizeof(double));
    }
    return mat;
}

void free_matrix(Matrix *mat) {
    if (!mat) return;
    for (int i = 0; i < mat->rows; i++) {
        free(mat->data[i]);
    }
    free(mat->data);
    free(mat);
}

Matrix* matrix_multiply(Matrix *a, Matrix *b) {
    if (a->cols != b->rows) {
        fprintf(stderr, "Runtime error: Matrix dimension mismatch for multiplication (%dx%d) @ (%dx%d)\n",
                a->rows, a->cols, b->rows, b->cols);
        exit(1);
    }
    
    Matrix *result = create_matrix(a->rows, b->cols);
    
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[i][k] * b->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }
    
    return result;
}

void print_matrix(Matrix *mat) {
    if (!mat) return;
    
    printf("[\n");
    for (int i = 0; i < mat->rows; i++) {
        printf("  [");
        for (int j = 0; j < mat->cols; j++) {
            if (mat->data[i][j] == (int)mat->data[i][j]) {
                printf("%d", (int)mat->data[i][j]);
            } else {
                printf("%g", mat->data[i][j]);
            }
            if (j < mat->cols - 1) printf(", ");
        }
        printf("]");
        if (i < mat->rows - 1) printf(",");
        printf("\n");
    }
    printf("]\n");
}

Value create_int_value(int val) {
    Value v;
    v.type = VAL_INT;
    v.data.int_val = val;
    return v;
}

Value create_float_value(double val) {
    Value v;
    v.type = VAL_FLOAT;
    v.data.float_val = val;
    return v;
}

Value create_string_value(char *val) {
    Value v;
    v.type = VAL_STRING;
    v.data.string_val = strdup(val);
    return v;
}

Value create_bool_value(int val) {
    Value v;
    v.type = VAL_BOOL;
    v.data.bool_val = val;
    return v;
}

Value create_void_value() {
    Value v;
    v.type = VAL_VOID;
    return v;
}

Value create_matrix_value(int rows, int cols) {
    Value v;
    v.type = VAL_MATRIX;
    v.data.matrix_val = create_matrix(rows, cols);
    return v;
}

void free_value(Value *val) {
    if (val->type == VAL_STRING && val->data.string_val) {
        free(val->data.string_val);
    } else if (val->type == VAL_MATRIX && val->data.matrix_val) {
        free_matrix(val->data.matrix_val);
    }
}

void print_value(Value val) {
    switch (val.type) {
        case VAL_INT:
            printf("%d", val.data.int_val);
            break;
        case VAL_FLOAT:
            printf("%g", val.data.float_val);
            break;
        case VAL_STRING:
            printf("%s", val.data.string_val);
            break;
        case VAL_BOOL:
            printf("%s", val.data.bool_val ? "true" : "false");
            break;
        case VAL_VOID:
            printf("void");
            break;
        case VAL_MATRIX:
            print_matrix(val.data.matrix_val);
            break;
        default:
            printf("(unknown type)");
    }
}

SymbolTable* create_symbol_table(SymbolTable *parent) {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->head = NULL;
    table->parent = parent;
    table->is_returning = 0;
    table->return_value = create_void_value();
    return table;
}

void free_symbol_table(SymbolTable *table) {
    Symbol *current = table->head;
    while (current) {
        Symbol *next = current->next;
        free(current->name);
        free_value(&current->value);
        free(current);
        current = next;
    }
    free(table);
}

void set_symbol(SymbolTable *table, const char *name, Value value) {
    Symbol *current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            free_value(&current->value);
            current->value = value;
            return;
        }
        current = current->next;
    }
    
    Symbol *new_symbol = (Symbol*)malloc(sizeof(Symbol));
    new_symbol->name = strdup(name);
    new_symbol->value = value;
    new_symbol->next = table->head;
    table->head = new_symbol;
}

Value* get_symbol(SymbolTable *table, const char *name) {
    /* Search in current scope */
    Symbol *current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return &current->value;
        }
        current = current->next;
    }
    
    /* Search in parent scope */
    if (table->parent) {
        return get_symbol(table->parent, name);
    }
    
    return NULL;
}

/* Built-in functions */
static Value builtin_print(ASTNode *args, SymbolTable *table) {
    if (args && args->type == NODE_ARG_LIST) {
        for (int i = 0; i < args->data.list.count; i++) {
            Value val = eval_expression(args->data.list.items[i], table);
            print_value(val);
            if (i < args->data.list.count - 1) {
                printf(" ");
            }
            free_value(&val);
        }
    }
    printf("\n");
    return create_void_value();
}

static Value builtin_printm(ASTNode *args, SymbolTable *table) {
    if (args && args->type == NODE_ARG_LIST) {
        for (int i = 0; i < args->data.list.count; i++) {
            Value val = eval_expression(args->data.list.items[i], table);
            if (val.type == VAL_MATRIX) {
                print_matrix(val.data.matrix_val);
            } else {
                fprintf(stderr, "Runtime error: printm() expects a matrix argument\n");
            }
            free_value(&val);
        }
    }
    return create_void_value();
}

static Value builtin_read(ASTNode *args, SymbolTable *table) {
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        /* Try to parse as integer */
        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (*endptr == '\0') {
            return create_int_value((int)val);
        }
        
        /* Try to parse as float */
        double fval = strtod(buffer, &endptr);
        if (*endptr == '\0') {
            return create_float_value(fval);
        }
        
        /* Return as string */
        return create_string_value(buffer);
    }
    return create_void_value();
}

/* Helper to convert array literal to matrix */
static Value array_literal_to_matrix(ASTNode *node, SymbolTable *table) {
    if (!node || node->type != NODE_ARRAY_LITERAL) {
        fprintf(stderr, "Runtime error: Expected array literal for matrix\n");
        exit(1);
    }
    
    int rows = node->data.list.count;
    if (rows == 0) {
        return create_matrix_value(0, 0);
    }
    
    /* Check first row to get column count */
    ASTNode *first_row = node->data.list.items[0];
    int cols = 0;
    
    if (first_row->type == NODE_ARRAY_LITERAL) {
        cols = first_row->data.list.count;
    } else {
        /* Single row matrix */
        cols = rows;
        rows = 1;
        
        Value mat_val = create_matrix_value(1, cols);
        Matrix *mat = mat_val.data.matrix_val;
        
        for (int j = 0; j < cols; j++) {
            Value elem = eval_expression(node->data.list.items[j], table);
            if (elem.type == VAL_INT) {
                mat->data[0][j] = elem.data.int_val;
            } else if (elem.type == VAL_FLOAT) {
                mat->data[0][j] = elem.data.float_val;
            }
            free_value(&elem);
        }
        return mat_val;
    }
    
    /* Create matrix */
    Value mat_val = create_matrix_value(rows, cols);
    Matrix *mat = mat_val.data.matrix_val;
    
    /* Fill matrix */
    for (int i = 0; i < rows; i++) {
        ASTNode *row = node->data.list.items[i];
        if (row->type != NODE_ARRAY_LITERAL) {
            fprintf(stderr, "Runtime error: Matrix row must be an array\n");
            exit(1);
        }
        if (row->data.list.count != cols) {
            fprintf(stderr, "Runtime error: All matrix rows must have same length\n");
            exit(1);
        }
        
        for (int j = 0; j < cols; j++) {
            Value elem = eval_expression(row->data.list.items[j], table);
            if (elem.type == VAL_INT) {
                mat->data[i][j] = elem.data.int_val;
            } else if (elem.type == VAL_FLOAT) {
                mat->data[i][j] = elem.data.float_val;
            }
            free_value(&elem);
        }
    }
    
    return mat_val;
}

/* Evaluate expressions */
static Value eval_expression(ASTNode *node, SymbolTable *table) {
    if (!node) return create_void_value();
    
    switch (node->type) {
        case NODE_INT_LITERAL:
            return create_int_value(node->data.int_literal.value);
            
        case NODE_FLOAT_LITERAL:
            return create_float_value(node->data.float_literal.value);
            
        case NODE_STRING_LITERAL:
            return create_string_value(node->data.string_literal.value);
            
        case NODE_BOOL_LITERAL:
            return create_bool_value(node->data.bool_literal.value);
            
        case NODE_ARRAY_LITERAL:
            return array_literal_to_matrix(node, table);
            
        case NODE_IDENTIFIER: {
            Value *val = get_symbol(table, node->data.identifier.name);
            if (val) {
                /* Return a copy */
                Value copy = *val;
                if (copy.type == VAL_STRING) {
                    copy.data.string_val = strdup(val->data.string_val);
                } else if (copy.type == VAL_MATRIX) {
                    /* Deep copy matrix */
                    Matrix *orig = val->data.matrix_val;
                    Matrix *new_mat = create_matrix(orig->rows, orig->cols);
                    for (int i = 0; i < orig->rows; i++) {
                        for (int j = 0; j < orig->cols; j++) {
                            new_mat->data[i][j] = orig->data[i][j];
                        }
                    }
                    copy.data.matrix_val = new_mat;
                }
                return copy;
            }
            fprintf(stderr, "Runtime error: Undefined variable '%s'\n", 
                    node->data.identifier.name);
            exit(1);
        }
        
        case NODE_ADD: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result;
            
            if (left.type == VAL_INT && right.type == VAL_INT) {
                result = create_int_value(left.data.int_val + right.data.int_val);
            } else {
                double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
                double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                result = create_float_value(l + r);
            }
            
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_SUB: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result;
            
            if (left.type == VAL_INT && right.type == VAL_INT) {
                result = create_int_value(left.data.int_val - right.data.int_val);
            } else {
                double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
                double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                result = create_float_value(l - r);
            }
            
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_MUL: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result;
            
            if (left.type == VAL_INT && right.type == VAL_INT) {
                result = create_int_value(left.data.int_val * right.data.int_val);
            } else {
                double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
                double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                result = create_float_value(l * r);
            }
            
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_DIV: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result;
            
            double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
            double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
            
            if (r == 0) {
                fprintf(stderr, "Runtime error: Division by zero\n");
                exit(1);
            }
            
            result = create_float_value(l / r);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_MOD: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result;
            
            if (left.type != VAL_INT || right.type != VAL_INT) {
                fprintf(stderr, "Runtime error: Modulo operator requires integer operands\n");
                exit(1);
            }
            
            if (right.data.int_val == 0) {
                fprintf(stderr, "Runtime error: Modulo by zero\n");
                exit(1);
            }
            
            result = create_int_value(left.data.int_val % right.data.int_val);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_MATRIX_MUL: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            if (left.type != VAL_MATRIX || right.type != VAL_MATRIX) {
                fprintf(stderr, "Runtime error: Matrix multiplication requires matrix operands\n");
                exit(1);
            }
            
            Matrix *result_mat = matrix_multiply(left.data.matrix_val, right.data.matrix_val);
            Value result;
            result.type = VAL_MATRIX;
            result.data.matrix_val = result_mat;
            
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_LT: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
            double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
            
            Value result = create_bool_value(l < r);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_GT: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
            double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
            
            Value result = create_bool_value(l > r);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_LE: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
            double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
            
            Value result = create_bool_value(l <= r);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_GE: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            double l = (left.type == VAL_INT) ? left.data.int_val : left.data.float_val;
            double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
            
            Value result = create_bool_value(l >= r);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_EQ: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            int equal = 0;
            if (left.type == VAL_INT && right.type == VAL_INT) {
                equal = (left.data.int_val == right.data.int_val);
            } else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
                equal = (left.data.bool_val == right.data.bool_val);
            } else if (left.type == VAL_STRING && right.type == VAL_STRING) {
                equal = (strcmp(left.data.string_val, right.data.string_val) == 0);
            }
            
            Value result = create_bool_value(equal);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_NE: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            int equal = 0;
            if (left.type == VAL_INT && right.type == VAL_INT) {
                equal = (left.data.int_val == right.data.int_val);
            } else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
                equal = (left.data.bool_val == right.data.bool_val);
            }
            
            Value result = create_bool_value(!equal);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_PATTERN_MATCH: {
            Value left = eval_expression(node->data.binary_op.left, table);
            Value right = eval_expression(node->data.binary_op.right, table);
            
            int matches = 0;
            
            if (left.type == VAL_STRING && right.type == VAL_STRING) {
                regex_t regex;
                int reti = regcomp(&regex, right.data.string_val, REG_EXTENDED);
                
                if (reti == 0) {
                    reti = regexec(&regex, left.data.string_val, 0, NULL, 0);
                    matches = (reti == 0);
                    regfree(&regex);
                } else {
                    char error_buf[100];
                    regerror(reti, &regex, error_buf, sizeof(error_buf));
                    fprintf(stderr, "Runtime error: Invalid regex pattern: %s\n", error_buf);
                    regfree(&regex);
                }
            }
            
            Value result = create_bool_value(matches);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_AND: {
            Value left = eval_expression(node->data.binary_op.left, table);
            if (!left.data.bool_val) {
                return create_bool_value(0);
            }
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result = create_bool_value(right.data.bool_val);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_OR: {
            Value left = eval_expression(node->data.binary_op.left, table);
            if (left.data.bool_val) {
                return create_bool_value(1);
            }
            Value right = eval_expression(node->data.binary_op.right, table);
            Value result = create_bool_value(right.data.bool_val);
            free_value(&left);
            free_value(&right);
            return result;
        }
        
        case NODE_NOT: {
            Value operand = eval_expression(node->data.unary_op.operand, table);
            Value result = create_bool_value(!operand.data.bool_val);
            free_value(&operand);
            return result;
        }
        
        case NODE_UNARY_MINUS: {
            Value operand = eval_expression(node->data.unary_op.operand, table);
            Value result;
            if (operand.type == VAL_INT) {
                result = create_int_value(-operand.data.int_val);
            } else {
                result = create_float_value(-operand.data.float_val);
            }
            free_value(&operand);
            return result;
        }
        
        case NODE_PRE_INC: {
            if (node->data.unary_op.operand->type == NODE_IDENTIFIER) {
                char *name = node->data.unary_op.operand->data.identifier.name;
                Value *current = get_symbol(table, name);
                if (!current) {
                    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
                    exit(1);
                }
                
                Value result;
                if (current->type == VAL_INT) {
                    result = create_int_value(current->data.int_val + 1);
                } else if (current->type == VAL_FLOAT) {
                    result = create_float_value(current->data.float_val + 1.0);
                } else {
                    fprintf(stderr, "Runtime error: Cannot increment non-numeric type\n");
                    exit(1);
                }
                
                set_symbol(table, name, result);
                
                /* Return updated value (copy for safety) */
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            fprintf(stderr, "Runtime error: Pre-increment requires lvalue\n");
            exit(1);
        }
        
        case NODE_PRE_DEC: {
            if (node->data.unary_op.operand->type == NODE_IDENTIFIER) {
                char *name = node->data.unary_op.operand->data.identifier.name;
                Value *current = get_symbol(table, name);
                if (!current) {
                    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
                    exit(1);
                }
                
                Value result;
                if (current->type == VAL_INT) {
                    result = create_int_value(current->data.int_val - 1);
                } else if (current->type == VAL_FLOAT) {
                    result = create_float_value(current->data.float_val - 1.0);
                } else {
                    fprintf(stderr, "Runtime error: Cannot decrement non-numeric type\n");
                    exit(1);
                }
                
                set_symbol(table, name, result);
                
                /* Return updated value (copy for safety) */
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            fprintf(stderr, "Runtime error: Pre-decrement requires lvalue\n");
            exit(1);
        }
        
        case NODE_POST_INC: {
            if (node->data.unary_op.operand->type == NODE_IDENTIFIER) {
                char *name = node->data.unary_op.operand->data.identifier.name;
                Value *current = get_symbol(table, name);
                if (!current) {
                    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
                    exit(1);
                }
                
                /* Save old value to return */
                Value old_val;
                if (current->type == VAL_INT) {
                    old_val = create_int_value(current->data.int_val);
                } else if (current->type == VAL_FLOAT) {
                    old_val = create_float_value(current->data.float_val);
                } else {
                    fprintf(stderr, "Runtime error: Cannot increment non-numeric type\n");
                    exit(1);
                }
                
                /* Increment variable */
                Value new_val;
                if (current->type == VAL_INT) {
                    new_val = create_int_value(current->data.int_val + 1);
                } else {
                    new_val = create_float_value(current->data.float_val + 1.0);
                }
                set_symbol(table, name, new_val);
                
                /* Return old value */
                return old_val;
            }
            fprintf(stderr, "Runtime error: Post-increment requires lvalue\n");
            exit(1);
        }
        
        case NODE_POST_DEC: {
            if (node->data.unary_op.operand->type == NODE_IDENTIFIER) {
                char *name = node->data.unary_op.operand->data.identifier.name;
                Value *current = get_symbol(table, name);
                if (!current) {
                    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
                    exit(1);
                }
                
                /* Save old value to return */
                Value old_val;
                if (current->type == VAL_INT) {
                    old_val = create_int_value(current->data.int_val);
                } else if (current->type == VAL_FLOAT) {
                    old_val = create_float_value(current->data.float_val);
                } else {
                    fprintf(stderr, "Runtime error: Cannot decrement non-numeric type\n");
                    exit(1);
                }
                
                /* Decrement variable */
                Value new_val;
                if (current->type == VAL_INT) {
                    new_val = create_int_value(current->data.int_val - 1);
                } else {
                    new_val = create_float_value(current->data.float_val - 1.0);
                }
                set_symbol(table, name, new_val);
                
                /* Return old value */
                return old_val;
            }
            fprintf(stderr, "Runtime error: Post-decrement requires lvalue\n");
            exit(1);
        }
        
        case NODE_ASSIGN: {
            Value val = eval_expression(node->data.binary_op.right, table);
            
            if (node->data.binary_op.left->type == NODE_IDENTIFIER) {
                char *name = node->data.binary_op.left->data.identifier.name;
                /* Make a copy for storage */
                Value store_val = val;
                if (val.type == VAL_STRING) {
                    store_val.data.string_val = strdup(val.data.string_val);
                } else if (val.type == VAL_MATRIX) {
                    /* Deep copy matrix */
                    Matrix *orig = val.data.matrix_val;
                    Matrix *new_mat = create_matrix(orig->rows, orig->cols);
                    for (int i = 0; i < orig->rows; i++) {
                        for (int j = 0; j < orig->cols; j++) {
                            new_mat->data[i][j] = orig->data[i][j];
                        }
                    }
                    store_val.data.matrix_val = new_mat;
                }
                set_symbol(table, name, store_val);
            }
            
            return val;
        }
        
        case NODE_PLUS_ASSIGN: {
            if (node->data.binary_op.left->type == NODE_IDENTIFIER) {
                char *name = node->data.binary_op.left->data.identifier.name;
                Value *current = get_symbol(table, name);
                Value right = eval_expression(node->data.binary_op.right, table);
                
                Value result;
                if (current->type == VAL_INT && right.type == VAL_INT) {
                    result = create_int_value(current->data.int_val + right.data.int_val);
                } else {
                    double l = (current->type == VAL_INT) ? current->data.int_val : current->data.float_val;
                    double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                    result = create_float_value(l + r);
                }
                
                set_symbol(table, name, result);
                free_value(&right);
                
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            return create_void_value();
        }
        
        case NODE_MINUS_ASSIGN: {
            if (node->data.binary_op.left->type == NODE_IDENTIFIER) {
                char *name = node->data.binary_op.left->data.identifier.name;
                Value *current = get_symbol(table, name);
                Value right = eval_expression(node->data.binary_op.right, table);
                
                Value result;
                if (current->type == VAL_INT && right.type == VAL_INT) {
                    result = create_int_value(current->data.int_val - right.data.int_val);
                } else {
                    double l = (current->type == VAL_INT) ? current->data.int_val : current->data.float_val;
                    double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                    result = create_float_value(l - r);
                }
                
                set_symbol(table, name, result);
                free_value(&right);
                
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            return create_void_value();
        }
        
        case NODE_MUL_ASSIGN: {
            if (node->data.binary_op.left->type == NODE_IDENTIFIER) {
                char *name = node->data.binary_op.left->data.identifier.name;
                Value *current = get_symbol(table, name);
                Value right = eval_expression(node->data.binary_op.right, table);
                
                Value result;
                if (current->type == VAL_INT && right.type == VAL_INT) {
                    result = create_int_value(current->data.int_val * right.data.int_val);
                } else {
                    double l = (current->type == VAL_INT) ? current->data.int_val : current->data.float_val;
                    double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                    result = create_float_value(l * r);
                }
                
                set_symbol(table, name, result);
                free_value(&right);
                
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            return create_void_value();
        }
        
        case NODE_DIV_ASSIGN: {
            if (node->data.binary_op.left->type == NODE_IDENTIFIER) {
                char *name = node->data.binary_op.left->data.identifier.name;
                Value *current = get_symbol(table, name);
                Value right = eval_expression(node->data.binary_op.right, table);
                
                double l = (current->type == VAL_INT) ? current->data.int_val : current->data.float_val;
                double r = (right.type == VAL_INT) ? right.data.int_val : right.data.float_val;
                
                if (r == 0) {
                    fprintf(stderr, "Runtime error: Division by zero\n");
                    exit(1);
                }
                
                Value result = create_float_value(l / r);
                set_symbol(table, name, result);
                free_value(&right);
                
                Value ret = result;
                if (ret.type == VAL_STRING) {
                    ret.data.string_val = strdup(result.data.string_val);
                }
                return ret;
            }
            return create_void_value();
        }
        
        case NODE_FUNC_CALL: {
            if (node->data.func_call.func->type == NODE_IDENTIFIER) {
                char *func_name = node->data.func_call.func->data.identifier.name;
                
                /* Check built-ins first */
                if (strcmp(func_name, "print") == 0) return builtin_print(node->data.func_call.args, table);
                if (strcmp(func_name, "printm") == 0) return builtin_printm(node->data.func_call.args, table);
                if (strcmp(func_name, "read") == 0) return builtin_read(node->data.func_call.args, table);

                /* Lookup user-defined function */
                Value *val = get_symbol(table, func_name);
                if (!val || val->type != VAL_FUNC) {
                    fprintf(stderr, "Runtime error: Undefined function '%s'\n", func_name);
                    exit(1);
                }

                /* Check recursion limit */
                if (recursion_depth >= MAX_RECURSION_DEPTH) {
                    fprintf(stderr, "Runtime error: Max recursion depth (%d) exceeded\n", MAX_RECURSION_DEPTH);
                    exit(1);
                }

                ASTNode *func_decl = val->data.func_node;
                ASTNode *params = func_decl->data.func_decl.params;
                ASTNode *args = node->data.func_call.args;
                
                /* Create new scope */
                SymbolTable *func_scope = create_symbol_table(global_table);
                recursion_depth++;

                /* Bind arguments to parameters */
                if (params && args) {
                    for (int i = 0; i < params->data.list.count && i < args->data.list.count; i++) {
                        Value arg_val = eval_expression(args->data.list.items[i], table);
                        set_symbol(func_scope, params->data.list.items[i]->data.param.name, arg_val);
                    }
                }

                /* Execute body */
                execute_statement(func_decl->data.func_decl.body, func_scope);
                
                Value result = func_scope->return_value;
                /* Copy string/matrix result if necessary */
                if (result.type == VAL_STRING) {
                    result.data.string_val = strdup(result.data.string_val);
                } else if (result.type == VAL_MATRIX) {
                    Matrix *orig = result.data.matrix_val;
                    Matrix *new_mat = create_matrix(orig->rows, orig->cols);
                    for (int i = 0; i < orig->rows; i++) {
                        for (int j = 0; j < orig->cols; j++) {
                            new_mat->data[i][j] = orig->data[i][j];
                        }
                    }
                    result.data.matrix_val = new_mat;
                }
                
                free_symbol_table(func_scope);
                recursion_depth--;
                return result;
            }
        }
        
        default:
            fprintf(stderr, "Runtime error: Unhandled expression type %d\n", node->type);
            return create_void_value();
    }
}

static void execute_statement(ASTNode *node, SymbolTable *table) {
    if (!node || table->is_returning) return;
    
    switch (node->type) {
        case NODE_EXPR_STMT:
            if (node->data.unary_op.operand) {
                Value v = eval_expression(node->data.unary_op.operand, table);
                free_value(&v);
            }
            break;
            
        case NODE_VAR_DECL: {
            Value val;
            if (node->data.var_decl.initializer) {
                val = eval_expression(node->data.var_decl.initializer, table);
            } else {
                // Default initialization
                switch (node->data.var_decl.type.base_type) {
                    case TYPE_INT:
                        val = create_int_value(0);
                        break;
                    case TYPE_FLOAT:
                        val = create_float_value(0.0);
                        break;
                    case TYPE_BOOL:
                        val = create_bool_value(0);
                        break;
                    case TYPE_STRING:
                        val = create_string_value("");
                        break;
                    case TYPE_MATRIX:
                        val = create_matrix_value(0, 0);
                        break;
                    default:
                        val = create_void_value();
                }
            }
            set_symbol(table, node->data.var_decl.name, val);
            break;
        }
        
        case NODE_IF:
        case NODE_IF_ELSE: {
            Value cond = eval_expression(node->data.if_stmt.condition, table);
            if (cond.data.bool_val || cond.data.int_val) {
                execute_statement(node->data.if_stmt.then_stmt, table);
            } else if (node->data.if_stmt.else_stmt) {
                execute_statement(node->data.if_stmt.else_stmt, table);
            }
            free_value(&cond);
            break;
        }
        
        case NODE_WHILE: {
            while (1) {
                Value cond = eval_expression(node->data.while_stmt.condition, table);
                int should_continue = cond.data.bool_val || cond.data.int_val;
                free_value(&cond);
                
                if (!should_continue) break;
                
                execute_statement(node->data.while_stmt.body, table);
            }
            break;
        }
        
        case NODE_FOR_RANGE: {
            /* Evaluate range */
            ASTNode *range = node->data.for_range.range;
            Value start_val = eval_expression(range->data.range.start, table);
            Value end_val = eval_expression(range->data.range.end, table);
            
            int start = (start_val.type == VAL_INT) ? start_val.data.int_val : (int)start_val.data.float_val;
            int end = (end_val.type == VAL_INT) ? end_val.data.int_val : (int)end_val.data.float_val;
            int step = 1;
            
            if (range->data.range.step) {
                Value step_val = eval_expression(range->data.range.step, table);
                step = (step_val.type == VAL_INT) ? step_val.data.int_val : (int)step_val.data.float_val;
                free_value(&step_val);
            }
            
            free_value(&start_val);
            free_value(&end_val);
            
            /* Determine if inclusive or exclusive */
            int inclusive = (range->type == NODE_RANGE_INCL || range->type == NODE_RANGE_STEP);
            int limit = inclusive ? end : end - 1;
            
            /* Execute loop */
            char *iterator = node->data.for_range.iterator;
            for (int i = start; (step > 0 ? i <= limit : i >= limit); i += step) {
                set_symbol(table, iterator, create_int_value(i));
                execute_statement(node->data.for_range.body, table);
            }
            break;
        }
        
        case NODE_RETURN: {
            if (node->data.return_stmt.value) {
                table->return_value = eval_expression(node->data.return_stmt.value, table);
            } else {
                table->return_value = create_void_value();
            }
            table->is_returning = 1;
            break;
        }
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.list.count && !table->is_returning; i++) {
                execute_statement(node->data.list.items[i], table);
            }
            break;
            
        default:
            break;
    }
}

/* Execute program */
void execute_program(ASTNode *root) {
    if (!root) return;
    
    global_table = create_symbol_table(NULL);
    
    if (root->type == NODE_DECL_LIST) {
        for (int i = 0; i < root->data.list.count; i++) {
            ASTNode *decl = root->data.list.items[i];
            
            if (decl->type == NODE_FUNC_DECL) {
                /* Store function in symbol table */
                Value func_val;
                func_val.type = VAL_FUNC;
                func_val.data.func_node = decl;
                set_symbol(global_table, decl->data.func_decl.name, func_val);
            } else {
                /* Execute top-level statements */
                execute_statement(decl, global_table);
            }
        }
        
        /* If main function exists, execute it */
        Value *main_val = get_symbol(global_table, "main");
        if (main_val && main_val->type == VAL_FUNC) {
            ASTNode *main_func = main_val->data.func_node;
            execute_statement(main_func->data.func_decl.body, global_table);
        }
    }
    
    free_symbol_table(global_table);
}