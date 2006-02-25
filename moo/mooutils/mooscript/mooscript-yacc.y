%{
#include "mooscript-parser.h"
#include "mooscript-yacc.h"

#define NODE_LIST_ADD(list, node)           _ms_parser_node_list_add (parser, MS_NODE_LIST (list), node)
#define NODE_COMMAND(id, node)              _ms_parser_node_command (parser, id, MS_NODE_LIST (node))
#define NODE_IF_ELSE(cond, then_, else_)    _ms_parser_node_if_else (parser, cond, then_, else_)
#define NODE_WHILE(cond, what)              _ms_parser_node_while (parser, cond, what)
#define NODE_DO_WHILE(cond, what)           _ms_parser_node_do_while (parser, cond, what)
#define NODE_FOR(var, list, what)           _ms_parser_node_for (parser, var, list, what)
#define NODE_ASSIGNMENT(var, val)           _ms_parser_node_assignment (parser, MS_NODE_VAR (var), val)
#define BINARY_OP(op, lval, rval)           _ms_parser_node_binary_op (parser, op, lval, rval)
#define UNARY_OP(op, val)                   _ms_parser_node_unary_op (parser, op, val)
#define NODE_NUMBER(n)                      _ms_parser_node_int (parser, n)
#define NODE_STRING(n)                      _ms_parser_node_string (parser, n)
#define NODE_VALUE_LIST(list)               _ms_parser_node_value_list (parser, MS_NODE_LIST (list))
#define NODE_VALUE_RANGE(first,second)      _ms_parser_node_value_range (parser, first, second)
#define NODE_VAR(string)                    _ms_parser_node_var (parser, string)

#define SET_TOP_NODE(node)                  _ms_parser_set_top_node (parser, node)
%}

%name-prefix="_ms_script_yy"

%union {
    int ival;
    const char *str;
    MSNode *node;
}

%token <str> IDENTIFIER
%token <str> LITERAL
%token <str> VARIABLE
%token <ival> NUMBER

%type <node> program stmt
%type <node> function if_stmt ternary loop assignment
%type <node> simple_expr compound_expr expr variable list_elms

%token IF THEN ELSE FI
%token WHILE DO OD FOR IN
%token EQ NEQ LE GE
%token AND OR NOT
%token UMINUS
%token TWODOTS

%lex-param      {MSParser *parser}
%parse-param    {MSParser *parser}
/* %expect 1 */

%left '-' '+'
%left '*' '/'
%left '%'
%left EQ NEQ '<' '>' GE LE
%left OR
%left AND
%left NOT
%left '#'
%left UMINUS
%right '='

%%

script:   program           { SET_TOP_NODE ($1); }
;

program:  stmt ';'          { $$ = NODE_LIST_ADD (NULL, $1); }
        | program stmt ';'  { $$ = NODE_LIST_ADD ($1, $2); }
;

stmt:   /* empty */         { $$ = NULL; }
        | expr
        | if_stmt
        | loop
;

loop:     WHILE expr DO program OD              { $$ = NODE_WHILE ($2, $4); }
        | DO program WHILE expr                 { $$ = NODE_DO_WHILE ($4, $2); }
        | FOR variable IN expr DO program OD    { $$ = NODE_FOR ($2, $4, $6); }
;

if_stmt:
          IF expr THEN program FI               { $$ = NODE_IF_ELSE ($2, $4, NULL); }
        | IF expr THEN program ELSE program FI  { $$ = NODE_IF_ELSE ($2, $4, $6); }
;

expr:     simple_expr
        | compound_expr
        | assignment
        | function
        | ternary
;

assignment:
          IDENTIFIER '=' expr               { $$ = NODE_ASSIGNMENT ($1, $3); }
;

ternary:  simple_expr '?' simple_expr ':' simple_expr { $$ = NODE_IF_ELSE ($1, $3, $5); }
;

compound_expr:
          expr '+' expr                     { $$ = BINARY_OP (MS_OP_PLUS, $1, $3); }
        | expr '-' expr                     { $$ = BINARY_OP (MS_OP_MINUS, $1, $3); }
        | expr '/' expr                     { $$ = BINARY_OP (MS_OP_DIV, $1, $3); }
        | expr '*' expr                     { $$ = BINARY_OP (MS_OP_MULT, $1, $3); }

        | expr AND expr                     { $$ = BINARY_OP (MS_OP_AND, $1, $3); }
        | expr OR expr                      { $$ = BINARY_OP (MS_OP_OR, $1, $3); }

        | expr EQ expr                      { $$ = BINARY_OP (MS_OP_EQ, $1, $3); }
        | expr NEQ expr                     { $$ = BINARY_OP (MS_OP_NEQ, $1, $3); }
        | expr '<' expr                     { $$ = BINARY_OP (MS_OP_LT, $1, $3); }
        | expr '>' expr                     { $$ = BINARY_OP (MS_OP_GT, $1, $3); }
        | expr LE expr                      { $$ = BINARY_OP (MS_OP_LE, $1, $3); }
        | expr GE expr                      { $$ = BINARY_OP (MS_OP_GE, $1, $3); }
;

simple_expr:
          NUMBER                            { $$ = NODE_NUMBER ($1); }
        | LITERAL                           { $$ = NODE_STRING ($1); }
        | variable
        | '(' stmt ')'                      { $$ = $2; }
        | '[' list_elms ']'                 { $$ = NODE_VALUE_LIST ($2); }
        | '[' expr TWODOTS expr ']'         { $$ = NODE_VALUE_RANGE ($2, $4); }
        | simple_expr '%' simple_expr       { $$ = BINARY_OP (MS_OP_FORMAT, $1, $3); }
        | '#' simple_expr                   { $$ = UNARY_OP (MS_OP_LEN, $2); }
        | NOT simple_expr                   { $$ = UNARY_OP (MS_OP_NOT, $2); }
        | '-' simple_expr %prec UMINUS      { $$ = UNARY_OP (MS_OP_UMINUS, $2); }
;

list_elms: /* empty */                      { $$ = NULL; }
        | expr                              { $$ = NODE_LIST_ADD (NULL, $1); }
        | list_elms ',' expr                { $$ = NODE_LIST_ADD ($1, $3); }
;

variable: IDENTIFIER                        { $$ = NODE_VAR ($1); }
;

function: IDENTIFIER '(' list_elms ')'      { $$ = NODE_COMMAND ($1, $3); }
;

%%
