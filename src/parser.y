%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "interpreter.h"

extern int yylex();
extern int yylineno;
extern FILE *yyin;
void yyerror(const char *s);

ASTNode *root = NULL;  /* Root of the AST */
%}

/* Union for semantic values */
%union {
    int intval;
    double floatval;
    char *strval;
    ASTNode *node;
    TypeInfo type;
}

/* Token declarations */
%token <intval> INT_LITERAL TRUE FALSE
%token <floatval> FLOAT_LITERAL
%token <strval> STRING_LITERAL IDENTIFIER

/* Keywords */
%token IF ELSE WHILE FOR FN RETURN
%token INT FLOAT_TYPE STR BOOL VOID MATRIX
%token BREAK CONTINUE RANGE

/* Operators */
%token MATRIX_MUL PATTERN_MATCH
%token EQ NE LE GE LT GT
%token AND OR NOT
/* todo: working in progress the 2 in 1 Operators */
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN MUL_ASSIGN DIV_ASSIGN
%token PLUS MINUS MUL DIV MOD
%token INC DEC
%token RANGE_OP RANGE_OP_EXCL

/* Delimiters */
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMICOLON COMMA COLON

%token ERROR

/* Non-terminal types */
%type <node> program declaration_list declaration
%type <node> function_decl parameter_list parameter
%type <node> statement_list statement compound_stmt
%type <node> expression_stmt selection_stmt iteration_stmt
%type <node> jump_stmt declaration_stmt
%type <node> expression assignment_expr logical_or_expr logical_and_expr
%type <node> equality_expr relational_expr additive_expr
%type <node> multiplicative_expr matrix_expr unary_expr postfix_expr
%type <node> primary_expr argument_list
%type <type> type_specifier
%type <node> range_expr initializer_list

/* Operator precedence and associativity */
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN MUL_ASSIGN DIV_ASSIGN
%left OR
%left AND
%left EQ NE
%left LT GT LE GE PATTERN_MATCH
%left RANGE_OP RANGE_OP_EXCL
%left PLUS MINUS
%left MUL DIV MOD
%left MATRIX_MUL
%right NOT UMINUS
%left INC DEC
%left LPAREN RPAREN LBRACKET RBRACKET

%%

/* Grammar Rules */

program
    : declaration_list                  { 
        root = $1;
        printf("Parse successful! AST created.\n");
    }
    | /* empty */                       { root = NULL; }
    ;

declaration_list
    : declaration                       { 
        $$ = create_list(NODE_DECL_LIST, yylineno);
        list_append($$, $1);
    }
    | declaration_list declaration      { 
        $$ = $1;
        list_append($$, $2);
    }
    ;

declaration
    : function_decl                     { $$ = $1; }
    | declaration_stmt                  { $$ = $1; }
    | statement                         { $$ = $1; }
    ;

function_decl
    : FN IDENTIFIER LPAREN parameter_list RPAREN type_specifier compound_stmt {
        $$ = create_func_decl($6, $2, $4, $7, yylineno);
        free($2);
    }
    | FN IDENTIFIER LPAREN RPAREN type_specifier compound_stmt {
        $$ = create_func_decl($5, $2, NULL, $6, yylineno);
        free($2);
    }
    ;

parameter_list
    : parameter                         { 
        $$ = create_list(NODE_PARAM_LIST, yylineno);
        list_append($$, $1);
    }
    | parameter_list COMMA parameter    { 
        $$ = $1;
        list_append($$, $3);
    }
    ;

parameter
    : type_specifier IDENTIFIER         { 
        $$ = create_param($1, $2, yylineno);
        free($2);
    }
    ;

type_specifier
    : INT                               { $$ = create_type(TYPE_INT); }
    | FLOAT_TYPE                        { $$ = create_type(TYPE_FLOAT); }
    | STR                               { $$ = create_type(TYPE_STRING); }
    | BOOL                              { $$ = create_type(TYPE_BOOL); }
    | VOID                              { $$ = create_type(TYPE_VOID); }
    | MATRIX                            { $$ = create_type(TYPE_MATRIX); }
    | type_specifier LBRACKET RBRACKET  { 
        $$ = $1;
        $$.is_array = 1;
    }
    ;

compound_stmt
    : LBRACE statement_list RBRACE      { $$ = $2; }
    | LBRACE RBRACE                     { 
        $$ = create_list(NODE_STMT_LIST, yylineno);
    }
    ;

statement_list
    : statement                         { 
        $$ = create_list(NODE_STMT_LIST, yylineno);
        list_append($$, $1);
    }
    | statement_list statement          { 
        $$ = $1;
        list_append($$, $2);
    }
    ;

statement
    : compound_stmt                     { $$ = $1; }
    | expression_stmt                   { $$ = $1; }
    | selection_stmt                    { $$ = $1; }
    | iteration_stmt                    { $$ = $1; }
    | jump_stmt                         { $$ = $1; }
    | declaration_stmt                  { $$ = $1; }
    ;

declaration_stmt
    : type_specifier IDENTIFIER SEMICOLON {
        $$ = create_var_decl($1, $2, NULL, yylineno);
        free($2);
    }
    | type_specifier IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = create_var_decl($1, $2, $4, yylineno);
        free($2);
    }
    | type_specifier IDENTIFIER LBRACKET INT_LITERAL RBRACKET SEMICOLON {
        ASTNode *size = create_int_literal($4, yylineno);
        $$ = create_array_decl($1, $2, size, NULL, yylineno);
        free($2);
    }
    | type_specifier IDENTIFIER LBRACKET INT_LITERAL RBRACKET ASSIGN LBRACE initializer_list RBRACE SEMICOLON {
        ASTNode *size = create_int_literal($4, yylineno);
        $$ = create_array_decl($1, $2, size, $8, yylineno);
        free($2);
    }
    ;

initializer_list
    : expression                        { 
        $$ = create_list(NODE_INIT_LIST, yylineno);
        list_append($$, $1);
    }
    | initializer_list COMMA expression { 
        $$ = $1;
        list_append($$, $3);
    }
    ;

expression_stmt
    : expression SEMICOLON              { $$ = create_expr_stmt($1, yylineno); }
    | SEMICOLON                         { $$ = create_expr_stmt(NULL, yylineno); }
    ;

selection_stmt
    : IF LPAREN expression RPAREN statement {
        $$ = create_if_stmt($3, $5, NULL, yylineno);
    }
    | IF LPAREN expression RPAREN statement ELSE statement {
        $$ = create_if_stmt($3, $5, $7, yylineno);
    }
    ;

iteration_stmt
    : WHILE LPAREN expression RPAREN statement {
        $$ = create_while_stmt($3, $5, yylineno);
    }
    | FOR LPAREN expression_stmt expression_stmt RPAREN statement {
        $$ = create_for_stmt($3, $4, NULL, $6, yylineno);
    }
    | FOR LPAREN expression_stmt expression_stmt expression RPAREN statement {
        $$ = create_for_stmt($3, $4, $5, $7, yylineno);
    }
    | FOR LPAREN IDENTIFIER COLON range_expr RPAREN statement {
        $$ = create_for_range($3, $5, $7, yylineno);
        free($3);
    }
    ;

range_expr
    : expression RANGE_OP expression {
        $$ = create_range(NODE_RANGE_INCL, $1, $3, NULL, yylineno);
    }
    | expression RANGE_OP_EXCL expression {
        $$ = create_range(NODE_RANGE_EXCL, $1, $3, NULL, yylineno);
    }
    | expression RANGE_OP expression COLON expression {
        $$ = create_range(NODE_RANGE_STEP, $1, $3, $5, yylineno);
    }
    | RANGE LPAREN expression COMMA expression RPAREN {
        $$ = create_range(NODE_RANGE_INCL, $3, $5, NULL, yylineno);
    }
    | RANGE LPAREN expression COMMA expression COMMA expression RPAREN {
        $$ = create_range(NODE_RANGE_STEP, $3, $5, $7, yylineno);
    }
    ;

jump_stmt
    : RETURN expression SEMICOLON       { $$ = create_return_stmt($2, yylineno); }
    | RETURN SEMICOLON                  { $$ = create_return_stmt(NULL, yylineno); }
    | BREAK SEMICOLON                   { $$ = create_break_stmt(yylineno); }
    | CONTINUE SEMICOLON                { $$ = create_continue_stmt(yylineno); }
    ;

expression
    : assignment_expr                   { $$ = $1; }
    ;

assignment_expr
    : logical_or_expr                   { $$ = $1; }
    | unary_expr ASSIGN assignment_expr {
        $$ = create_binary_op(NODE_ASSIGN, $1, $3, yylineno);
    }
    | unary_expr PLUS_ASSIGN assignment_expr {
        $$ = create_binary_op(NODE_PLUS_ASSIGN, $1, $3, yylineno);
    }
    | unary_expr MINUS_ASSIGN assignment_expr {
        $$ = create_binary_op(NODE_MINUS_ASSIGN, $1, $3, yylineno);
    }
    | unary_expr MUL_ASSIGN assignment_expr {
        $$ = create_binary_op(NODE_MUL_ASSIGN, $1, $3, yylineno);
    }
    | unary_expr DIV_ASSIGN assignment_expr {
        $$ = create_binary_op(NODE_DIV_ASSIGN, $1, $3, yylineno);
    }
    ;

logical_or_expr
    : logical_and_expr                  { $$ = $1; }
    | logical_or_expr OR logical_and_expr {
        $$ = create_binary_op(NODE_OR, $1, $3, yylineno);
    }
    ;

logical_and_expr
    : equality_expr                     { $$ = $1; }
    | logical_and_expr AND equality_expr {
        $$ = create_binary_op(NODE_AND, $1, $3, yylineno);
    }
    ;

equality_expr
    : relational_expr                   { $$ = $1; }
    | equality_expr EQ relational_expr {
        $$ = create_binary_op(NODE_EQ, $1, $3, yylineno);
    }
    | equality_expr NE relational_expr {
        $$ = create_binary_op(NODE_NE, $1, $3, yylineno);
    }
    ;

relational_expr
    : additive_expr                     { $$ = $1; }
    | relational_expr LT additive_expr {
        $$ = create_binary_op(NODE_LT, $1, $3, yylineno);
    }
    | relational_expr GT additive_expr {
        $$ = create_binary_op(NODE_GT, $1, $3, yylineno);
    }
    | relational_expr LE additive_expr {
        $$ = create_binary_op(NODE_LE, $1, $3, yylineno);
    }
    | relational_expr GE additive_expr {
        $$ = create_binary_op(NODE_GE, $1, $3, yylineno);
    }
    | relational_expr PATTERN_MATCH additive_expr {
        $$ = create_binary_op(NODE_PATTERN_MATCH, $1, $3, yylineno);
    }
    ;

additive_expr
    : multiplicative_expr               { $$ = $1; }
    | additive_expr PLUS multiplicative_expr {
        $$ = create_binary_op(NODE_ADD, $1, $3, yylineno);
    }
    | additive_expr MINUS multiplicative_expr {
        $$ = create_binary_op(NODE_SUB, $1, $3, yylineno);
    }
    ;

multiplicative_expr
    : matrix_expr                       { $$ = $1; }
    | multiplicative_expr MUL matrix_expr {
        $$ = create_binary_op(NODE_MUL, $1, $3, yylineno);
    }
    | multiplicative_expr DIV matrix_expr {
        $$ = create_binary_op(NODE_DIV, $1, $3, yylineno);
    }
    | multiplicative_expr MOD matrix_expr {
        $$ = create_binary_op(NODE_MOD, $1, $3, yylineno);
    }
    ;

matrix_expr
    : unary_expr                        { $$ = $1; }
    | matrix_expr MATRIX_MUL unary_expr {
        $$ = create_binary_op(NODE_MATRIX_MUL, $1, $3, yylineno);
    }
    ;

unary_expr
    : postfix_expr                      { $$ = $1; }
    | INC unary_expr                    { $$ = create_unary_op(NODE_PRE_INC, $2, yylineno); }
    | DEC unary_expr                    { $$ = create_unary_op(NODE_PRE_DEC, $2, yylineno); }
    | PLUS unary_expr                   { $$ = $2; }
    | MINUS unary_expr %prec UMINUS     { $$ = create_unary_op(NODE_UNARY_MINUS, $2, yylineno); }
    | NOT unary_expr                    { $$ = create_unary_op(NODE_NOT, $2, yylineno); }
    ;

postfix_expr
    : primary_expr                      { $$ = $1; }
    | postfix_expr LBRACKET expression RBRACKET {
        $$ = create_array_index($1, $3, yylineno);
    }
    | postfix_expr LPAREN argument_list RPAREN {
        $$ = create_func_call($1, $3, yylineno);
    }
    | postfix_expr LPAREN RPAREN {
        $$ = create_func_call($1, NULL, yylineno);
    }
    | postfix_expr INC                  { $$ = create_unary_op(NODE_POST_INC, $1, yylineno); }
    | postfix_expr DEC                  { $$ = create_unary_op(NODE_POST_DEC, $1, yylineno); }
    ;

argument_list
    : expression                        { 
        $$ = create_list(NODE_ARG_LIST, yylineno);
        list_append($$, $1);
    }
    | argument_list COMMA expression    { 
        $$ = $1;
        list_append($$, $3);
    }
    ;

primary_expr
    : IDENTIFIER                        { 
        $$ = create_identifier($1, yylineno);
        free($1);
    }
    | INT_LITERAL                       { $$ = create_int_literal($1, yylineno); }
    | FLOAT_LITERAL                     { $$ = create_float_literal($1, yylineno); }
    | STRING_LITERAL                    { 
        $$ = create_string_literal($1, yylineno);
        free($1);
    }
    | TRUE                              { $$ = create_bool_literal(1, yylineno); }
    | FALSE                             { $$ = create_bool_literal(0, yylineno); }
    | LPAREN expression RPAREN          { $$ = $2; }
    | LBRACKET initializer_list RBRACKET {
        $$ = $2;
        $$->type = NODE_ARRAY_LITERAL;
    }
    | range_expr                        { $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Error opening file");
            return 1;
        }
    }
    
    int result = yyparse();
    
    if (result == 0 && root) {
        printf("\n=== Abstract Syntax Tree ===\n");
        print_ast(root, 0);
        
        printf("\n=== Program Execution ===\n");
        execute_program(root);
        
        free_ast(root);
    }
    
    if (yyin != stdin) {
        fclose(yyin);
    }
    
    return result;
}