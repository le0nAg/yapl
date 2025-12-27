#include "ast.h"
#include <stdio.h>

/* Helper function to create a base node */
static ASTNode* create_node(NodeType type, int line) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    node->type = type;
    node->line_number = line;
    node->data_type.base_type = TYPE_UNKNOWN;
    node->data_type.is_array = 0;
    node->data_type.array_size = 0;
    return node;
}

/* Literals */
ASTNode* create_int_literal(int value, int line) {
    ASTNode *node = create_node(NODE_INT_LITERAL, line);
    node->data.int_literal.value = value;
    node->data_type.base_type = TYPE_INT;
    return node;
}

ASTNode* create_float_literal(double value, int line) {
    ASTNode *node = create_node(NODE_FLOAT_LITERAL, line);
    node->data.float_literal.value = value;
    node->data_type.base_type = TYPE_FLOAT;
    return node;
}

ASTNode* create_string_literal(char *value, int line) {
    ASTNode *node = create_node(NODE_STRING_LITERAL, line);
    node->data.string_literal.value = strdup(value);
    node->data_type.base_type = TYPE_STRING;
    return node;
}

ASTNode* create_bool_literal(int value, int line) {
    ASTNode *node = create_node(NODE_BOOL_LITERAL, line);
    node->data.bool_literal.value = value;
    node->data_type.base_type = TYPE_BOOL;
    return node;
}

/* Identifiers */
ASTNode* create_identifier(char *name, int line) {
    ASTNode *node = create_node(NODE_IDENTIFIER, line);
    node->data.identifier.name = strdup(name);
    return node;
}

/* Binary operations */
ASTNode* create_binary_op(NodeType type, ASTNode *left, ASTNode *right, int line) {
    ASTNode *node = create_node(type, line);
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

/* Unary operations */
ASTNode* create_unary_op(NodeType type, ASTNode *operand, int line) {
    ASTNode *node = create_node(type, line);
    node->data.unary_op.operand = operand;
    return node;
}

/* Range */
ASTNode* create_range(NodeType type, ASTNode *start, ASTNode *end, ASTNode *step, int line) {
    ASTNode *node = create_node(type, line);
    node->data.range.start = start;
    node->data.range.end = end;
    node->data.range.step = step;
    return node;
}

/* Declarations */
ASTNode* create_var_decl(TypeInfo type, char *name, ASTNode *initializer, int line) {
    ASTNode *node = create_node(NODE_VAR_DECL, line);
    node->data.var_decl.type = type;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.initializer = initializer;
    node->data_type = type;
    return node;
}

ASTNode* create_array_decl(TypeInfo type, char *name, ASTNode *size, ASTNode *initializer, int line) {
    ASTNode *node = create_node(NODE_ARRAY_DECL, line);
    node->data.array_decl.type = type;
    node->data.array_decl.name = strdup(name);
    node->data.array_decl.size = size;
    node->data.array_decl.initializer = initializer;
    node->data_type = type;
    return node;
}

ASTNode* create_func_decl(TypeInfo return_type, char *name, ASTNode *params, ASTNode *body, int line) {
    ASTNode *node = create_node(NODE_FUNC_DECL, line);
    node->data.func_decl.return_type = return_type;
    node->data.func_decl.name = strdup(name);
    node->data.func_decl.params = params;
    node->data.func_decl.body = body;
    node->data_type = return_type;
    return node;
}

ASTNode* create_param(TypeInfo type, char *name, int line) {
    ASTNode *node = create_node(NODE_PARAM, line);
    node->data.param.type = type;
    node->data.param.name = strdup(name);
    node->data_type = type;
    return node;
}

/* Statements */
ASTNode* create_if_stmt(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt, int line) {
    ASTNode *node = create_node(else_stmt ? NODE_IF_ELSE : NODE_IF, line);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;
    return node;
}

ASTNode* create_while_stmt(ASTNode *condition, ASTNode *body, int line) {
    ASTNode *node = create_node(NODE_WHILE, line);
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode* create_for_stmt(ASTNode *init, ASTNode *condition, ASTNode *increment, ASTNode *body, int line) {
    ASTNode *node = create_node(NODE_FOR, line);
    node->data.for_stmt.init = init;
    node->data.for_stmt.condition = condition;
    node->data.for_stmt.increment = increment;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode* create_for_range(char *iterator, ASTNode *range, ASTNode *body, int line) {
    ASTNode *node = create_node(NODE_FOR_RANGE, line);
    node->data.for_range.iterator = strdup(iterator);
    node->data.for_range.range = range;
    node->data.for_range.body = body;
    return node;
}

ASTNode* create_return_stmt(ASTNode *value, int line) {
    ASTNode *node = create_node(NODE_RETURN, line);
    node->data.return_stmt.value = value;
    return node;
}

ASTNode* create_break_stmt(int line) {
    return create_node(NODE_BREAK, line);
}

ASTNode* create_continue_stmt(int line) {
    return create_node(NODE_CONTINUE, line);
}

ASTNode* create_expr_stmt(ASTNode *expr, int line) {
    ASTNode *node = create_node(NODE_EXPR_STMT, line);
    node->data.unary_op.operand = expr;
    return node;
}

/* Expressions */
ASTNode* create_func_call(ASTNode *func, ASTNode *args, int line) {
    ASTNode *node = create_node(NODE_FUNC_CALL, line);
    node->data.func_call.func = func;
    node->data.func_call.args = args;
    return node;
}

ASTNode* create_array_index(ASTNode *array, ASTNode *index, int line) {
    ASTNode *node = create_node(NODE_ARRAY_INDEX, line);
    node->data.array_index.array = array;
    node->data.array_index.index = index;
    return node;
}

/* Lists */
ASTNode* create_list(NodeType type, int line) {
    ASTNode *node = create_node(type, line);
    node->data.list.items = NULL;
    node->data.list.count = 0;
    node->data.list.capacity = 0;
    return node;
}

void list_append(ASTNode *list, ASTNode *item) {
    if (!list || !item) return;
    
    if (list->data.list.count >= list->data.list.capacity) {
        int new_capacity = list->data.list.capacity == 0 ? 8 : list->data.list.capacity * 2;
        list->data.list.items = (ASTNode**)realloc(list->data.list.items, 
                                                     new_capacity * sizeof(ASTNode*));
        if (!list->data.list.items) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(1);
        }
        list->data.list.capacity = new_capacity;
    }
    
    list->data.list.items[list->data.list.count++] = item;
}

/* Type creation */
TypeInfo create_type(DataType base_type) {
    TypeInfo type;
    type.base_type = base_type;
    type.is_array = 0;
    type.array_size = 0;
    return type;
}

TypeInfo create_array_type(DataType base_type, int size) {
    TypeInfo type;
    type.base_type = base_type;
    type.is_array = 1;
    type.array_size = size;
    return type;
}

/* Free AST recursively */
void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_STRING_LITERAL:
            free(node->data.string_literal.value);
            break;
            
        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case NODE_ADD: case NODE_SUB: case NODE_MUL: case NODE_DIV: case NODE_MOD:
        case NODE_MATRIX_MUL: case NODE_EQ: case NODE_NE: case NODE_LT: case NODE_GT:
        case NODE_LE: case NODE_GE: case NODE_PATTERN_MATCH: case NODE_AND: case NODE_OR:
        case NODE_ASSIGN: case NODE_PLUS_ASSIGN: case NODE_MINUS_ASSIGN:
        case NODE_MUL_ASSIGN: case NODE_DIV_ASSIGN:
            free_ast(node->data.binary_op.left);
            free_ast(node->data.binary_op.right);
            break;
            
        case NODE_UNARY_MINUS: case NODE_PRE_INC: case NODE_PRE_DEC:
        case NODE_POST_INC: case NODE_POST_DEC: case NODE_NOT:
            free_ast(node->data.unary_op.operand);
            break;
            
        case NODE_RANGE_INCL: case NODE_RANGE_EXCL: case NODE_RANGE_STEP:
            free_ast(node->data.range.start);
            free_ast(node->data.range.end);
            free_ast(node->data.range.step);
            break;
            
        case NODE_VAR_DECL:
            free(node->data.var_decl.name);
            free_ast(node->data.var_decl.initializer);
            break;
            
        case NODE_ARRAY_DECL:
            free(node->data.array_decl.name);
            free_ast(node->data.array_decl.size);
            free_ast(node->data.array_decl.initializer);
            break;
            
        case NODE_FUNC_DECL:
            free(node->data.func_decl.name);
            free_ast(node->data.func_decl.params);
            free_ast(node->data.func_decl.body);
            break;
            
        case NODE_PARAM:
            free(node->data.param.name);
            break;
            
        case NODE_IF: case NODE_IF_ELSE:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_stmt);
            free_ast(node->data.if_stmt.else_stmt);
            break;
            
        case NODE_WHILE:
            free_ast(node->data.while_stmt.condition);
            free_ast(node->data.while_stmt.body);
            break;
            
        case NODE_FOR:
            free_ast(node->data.for_stmt.init);
            free_ast(node->data.for_stmt.condition);
            free_ast(node->data.for_stmt.increment);
            free_ast(node->data.for_stmt.body);
            break;
            
        case NODE_FOR_RANGE:
            free(node->data.for_range.iterator);
            free_ast(node->data.for_range.range);
            free_ast(node->data.for_range.body);
            break;
            
        case NODE_RETURN:
            free_ast(node->data.return_stmt.value);
            break;
            
        case NODE_FUNC_CALL:
            free_ast(node->data.func_call.func);
            free_ast(node->data.func_call.args);
            break;
            
        case NODE_ARRAY_INDEX:
            free_ast(node->data.array_index.array);
            free_ast(node->data.array_index.index);
            break;
            
        case NODE_STMT_LIST: case NODE_DECL_LIST: case NODE_PARAM_LIST:
        case NODE_ARG_LIST: case NODE_INIT_LIST:
            for (int i = 0; i < node->data.list.count; i++) {
                free_ast(node->data.list.items[i]);
            }
            free(node->data.list.items);
            break;
            
        default:
            break;
    }
    
    free(node);
}

/* Print AST for debugging */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    printf("%s", node_type_to_string(node->type));
    
    switch (node->type) {
        case NODE_INT_LITERAL:
            printf(": %d\n", node->data.int_literal.value);
            break;
            
        case NODE_FLOAT_LITERAL:
            printf(": %f\n", node->data.float_literal.value);
            break;
            
        case NODE_STRING_LITERAL:
            printf(": \"%s\"\n", node->data.string_literal.value);
            break;
            
        case NODE_BOOL_LITERAL:
            printf(": %s\n", node->data.bool_literal.value ? "true" : "false");
            break;
            
        case NODE_IDENTIFIER:
            printf(": %s\n", node->data.identifier.name);
            break;
            
        case NODE_VAR_DECL:
            printf(": %s %s\n", data_type_to_string(node->data.var_decl.type.base_type),
                   node->data.var_decl.name);
            if (node->data.var_decl.initializer) {
                print_ast(node->data.var_decl.initializer, indent + 1);
            }
            break;
            
        case NODE_ARRAY_DECL:
            printf(": %s %s[", data_type_to_string(node->data.array_decl.type.base_type),
                   node->data.array_decl.name);
            if (node->data.array_decl.size) {
                if (node->data.array_decl.size->type == NODE_INT_LITERAL) {
                    printf("%d", node->data.array_decl.size->data.int_literal.value);
                }
            }
            printf("]\n");
            if (node->data.array_decl.initializer) {
                print_ast(node->data.array_decl.initializer, indent + 1);
            }
            break;
            
        case NODE_FUNC_DECL:
            printf(": %s %s\n", data_type_to_string(node->data.func_decl.return_type.base_type),
                   node->data.func_decl.name);
            if (node->data.func_decl.params) {
                print_ast(node->data.func_decl.params, indent + 1);
            }
            print_ast(node->data.func_decl.body, indent + 1);
            break;
            
        case NODE_PARAM:
            printf(": %s %s\n", data_type_to_string(node->data.param.type.base_type),
                   node->data.param.name);
            break;
            
        case NODE_IF:
        case NODE_IF_ELSE:
            printf("\n");
            print_indent(indent + 1);
            printf("condition:\n");
            print_ast(node->data.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("then:\n");
            print_ast(node->data.if_stmt.then_stmt, indent + 2);
            if (node->data.if_stmt.else_stmt) {
                print_indent(indent + 1);
                printf("else:\n");
                print_ast(node->data.if_stmt.else_stmt, indent + 2);
            }
            break;
            
        case NODE_WHILE:
            printf("\n");
            print_indent(indent + 1);
            printf("condition:\n");
            print_ast(node->data.while_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            print_ast(node->data.while_stmt.body, indent + 2);
            break;
            
        case NODE_FOR:
            printf("\n");
            if (node->data.for_stmt.init) {
                print_indent(indent + 1);
                printf("init:\n");
                print_ast(node->data.for_stmt.init, indent + 2);
            }
            if (node->data.for_stmt.condition) {
                print_indent(indent + 1);
                printf("condition:\n");
                print_ast(node->data.for_stmt.condition, indent + 2);
            }
            if (node->data.for_stmt.increment) {
                print_indent(indent + 1);
                printf("increment:\n");
                print_ast(node->data.for_stmt.increment, indent + 2);
            }
            print_indent(indent + 1);
            printf("body:\n");
            print_ast(node->data.for_stmt.body, indent + 2);
            break;
            
        case NODE_FOR_RANGE:
            printf(": %s\n", node->data.for_range.iterator);
            print_indent(indent + 1);
            printf("range:\n");
            print_ast(node->data.for_range.range, indent + 2);
            print_indent(indent + 1);
            printf("body:\n");
            print_ast(node->data.for_range.body, indent + 2);
            break;
            
        case NODE_RETURN:
            printf("\n");
            if (node->data.return_stmt.value) {
                print_ast(node->data.return_stmt.value, indent + 1);
            }
            break;
            
        case NODE_BREAK:
        case NODE_CONTINUE:
            printf("\n");
            break;
            
        case NODE_EXPR_STMT:
            printf("\n");
            if (node->data.unary_op.operand) {
                print_ast(node->data.unary_op.operand, indent + 1);
            }
            break;
            
        case NODE_FUNC_CALL:
            printf("\n");
            print_indent(indent + 1);
            printf("function:\n");
            print_ast(node->data.func_call.func, indent + 2);
            if (node->data.func_call.args) {
                print_indent(indent + 1);
                printf("arguments:\n");
                print_ast(node->data.func_call.args, indent + 2);
            }
            break;
            
        case NODE_ARRAY_INDEX:
            printf("\n");
            print_indent(indent + 1);
            printf("array:\n");
            print_ast(node->data.array_index.array, indent + 2);
            print_indent(indent + 1);
            printf("index:\n");
            print_ast(node->data.array_index.index, indent + 2);
            break;
            
        case NODE_RANGE_INCL:
        case NODE_RANGE_EXCL:
        case NODE_RANGE_STEP:
            printf("\n");
            print_indent(indent + 1);
            printf("start:\n");
            print_ast(node->data.range.start, indent + 2);
            print_indent(indent + 1);
            printf("end:\n");
            print_ast(node->data.range.end, indent + 2);
            if (node->data.range.step) {
                print_indent(indent + 1);
                printf("step:\n");
                print_ast(node->data.range.step, indent + 2);
            }
            break;
            
        case NODE_STMT_LIST:
        case NODE_DECL_LIST:
        case NODE_PARAM_LIST:
        case NODE_ARG_LIST:
        case NODE_INIT_LIST:
        case NODE_ARRAY_LITERAL:
            printf(" (%d items)\n", node->data.list.count);
            for (int i = 0; i < node->data.list.count; i++) {
                print_ast(node->data.list.items[i], indent + 1);
            }
            break;
            
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD:
        case NODE_MATRIX_MUL:
        case NODE_EQ:
        case NODE_NE:
        case NODE_LT:
        case NODE_GT:
        case NODE_LE:
        case NODE_GE:
        case NODE_PATTERN_MATCH:
        case NODE_AND:
        case NODE_OR:
        case NODE_ASSIGN:
        case NODE_PLUS_ASSIGN:
        case NODE_MINUS_ASSIGN:
        case NODE_MUL_ASSIGN:
        case NODE_DIV_ASSIGN:
            printf("\n");
            print_indent(indent + 1);
            printf("left:\n");
            print_ast(node->data.binary_op.left, indent + 2);
            print_indent(indent + 1);
            printf("right:\n");
            print_ast(node->data.binary_op.right, indent + 2);
            break;
            
        case NODE_UNARY_MINUS:
        case NODE_PRE_INC:
        case NODE_PRE_DEC:
        case NODE_POST_INC:
        case NODE_POST_DEC:
        case NODE_NOT:
            printf("\n");
            print_ast(node->data.unary_op.operand, indent + 1);
            break;
            
        default:
            printf(" (unhandled node type)\n");
            break;
    }
}

/* Utility functions */
const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_INT_LITERAL: return "INT_LITERAL";
        case NODE_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case NODE_STRING_LITERAL: return "STRING_LITERAL";
        case NODE_BOOL_LITERAL: return "BOOL_LITERAL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_TYPE: return "TYPE";
        case NODE_ADD: return "ADD";
        case NODE_SUB: return "SUB";
        case NODE_MUL: return "MUL";
        case NODE_DIV: return "DIV";
        case NODE_MOD: return "MOD";
        case NODE_MATRIX_MUL: return "MATRIX_MUL";
        case NODE_EQ: return "EQ";
        case NODE_NE: return "NE";
        case NODE_LT: return "LT";
        case NODE_GT: return "GT";
        case NODE_LE: return "LE";
        case NODE_GE: return "GE";
        case NODE_PATTERN_MATCH: return "PATTERN_MATCH";
        case NODE_AND: return "AND";
        case NODE_OR: return "OR";
        case NODE_NOT: return "NOT";
        case NODE_ASSIGN: return "ASSIGN";
        case NODE_PLUS_ASSIGN: return "PLUS_ASSIGN";
        case NODE_MINUS_ASSIGN: return "MINUS_ASSIGN";
        case NODE_MUL_ASSIGN: return "MUL_ASSIGN";
        case NODE_DIV_ASSIGN: return "DIV_ASSIGN";
        case NODE_UNARY_MINUS: return "UNARY_MINUS";
        case NODE_PRE_INC: return "PRE_INC";
        case NODE_PRE_DEC: return "PRE_DEC";
        case NODE_POST_INC: return "POST_INC";
        case NODE_POST_DEC: return "POST_DEC";
        case NODE_RANGE_INCL: return "RANGE_INCL";
        case NODE_RANGE_EXCL: return "RANGE_EXCL";
        case NODE_RANGE_STEP: return "RANGE_STEP";
        case NODE_COMPOUND: return "COMPOUND";
        case NODE_VAR_DECL: return "VAR_DECL";
        case NODE_ARRAY_DECL: return "ARRAY_DECL";
        case NODE_FUNC_DECL: return "FUNC_DECL";
        case NODE_PARAM: return "PARAM";
        case NODE_IF: return "IF";
        case NODE_IF_ELSE: return "IF_ELSE";
        case NODE_WHILE: return "WHILE";
        case NODE_FOR: return "FOR";
        case NODE_FOR_RANGE: return "FOR_RANGE";
        case NODE_RETURN: return "RETURN";
        case NODE_BREAK: return "BREAK";
        case NODE_CONTINUE: return "CONTINUE";
        case NODE_EXPR_STMT: return "EXPR_STMT";
        case NODE_ARRAY_INDEX: return "ARRAY_INDEX";
        case NODE_FUNC_CALL: return "FUNC_CALL";
        case NODE_ARRAY_LITERAL: return "ARRAY_LITERAL";
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_STMT_LIST: return "STMT_LIST";
        case NODE_DECL_LIST: return "DECL_LIST";
        case NODE_PARAM_LIST: return "PARAM_LIST";
        case NODE_ARG_LIST: return "ARG_LIST";
        case NODE_INIT_LIST: return "INIT_LIST";
        default: return "UNKNOWN";
    }
}

const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "str";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_MATRIX: return "matrix";
        case TYPE_ARRAY: return "array";
        default: return "unknown";
    }
}