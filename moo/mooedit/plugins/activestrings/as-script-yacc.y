%{
#include "as-script-parser.h"
#include "as-script-yacc.h"

#define NODE_LIST_ADD(list, node)           _as_parser_node_list_add (parser, AS_NODE_LIST (list), node)
#define NODE_COMMAND(id, node)              _as_parser_node_command (parser, id, AS_NODE_LIST (node))
#define NODE_IF_ELSE(cond, then_, else_)    _as_parser_node_if_else (parser, cond, then_, else_)
#define NODE_REPEAT(times, what)            _as_parser_node_repeat (parser, times, what)
#define NODE_WHILE(cond, what)              _as_parser_node_while (parser, cond, what)
#define NODE_ASSIGNMENT(var, val)           _as_parser_node_assignment (parser, AS_NODE_VAR (var), val)
#define BINARY_OP(op, lval, rval)           _as_parser_node_binary_op (parser, op, lval, rval)
#define UNARY_OP(op, val)                   _as_parser_node_unary_op (parser, op, val)
#define NODE_NUMBER(n)                      _as_parser_node_int (parser, n)
#define NODE_STRING(n)                      _as_parser_node_string (parser, n)
#define NODE_VALUE_LIST(list)               _as_parser_node_value_list (parser, AS_NODE_LIST (list))
#define VAR_POSITIONAL(n)                   _as_parser_node_var_pos (parser, n)
#define VAR_NAMED(string)                   _as_parser_node_var_named (parser, string)

#define SET_TOP_NODE(node)                  _as_parser_set_top_node (parser, node)
%}

%name-prefix="_as_script_yy"

%union {
    int ival;
    const char *str;
    ASNode *node;
}

%token <str> IDENTIFIER
%token <str> LITERAL
%token <str> VARIABLE
%token <ival> NUMBER

%type <node> program stmt non_empty_stmt stmt_or_program
%type <node> command if_stmt ternary loop assignment
%type <node> expr_list simple_expr expr variable list_elms

%token IF THEN ELSE ELIF WHILE REPEAT
%token EQ NEQ LE GE
%token AND OR NOT
%token UMINUS

%lex-param      {ASParser *parser}
%parse-param    {ASParser *parser}
%expect 1

%left '-' '+'
%left '*' '/'
%left '%'
%left EQ NEQ '<' '>' GE LE
%left OR
%left AND
%left NOT
%left '#'
%left UMINUS

%%

script:   program           { SET_TOP_NODE ($1); }

program:  stmt ';'          { $$ = NODE_LIST_ADD (NULL, $1); }
        | program stmt ';'  { $$ = NODE_LIST_ADD ($1, $2); }
;

non_empty_stmt:
          '{' stmt_or_program '}'   { $$ = $2; }
        | command
        | expr
        | if_stmt
        | ternary
        | loop
        | assignment
;

stmt:     /* empty */       { $$ = NULL; }
        | non_empty_stmt
;

stmt_or_program:                            { $$ = NULL; }
        | program
        | non_empty_stmt
        | program non_empty_stmt            { $$ = NODE_LIST_ADD ($1, $2); }
;

command: IDENTIFIER expr_list               { $$ = NODE_COMMAND ($1, $2); }
;

expr_list: /* empty */                      { $$ = NULL; }
        | expr_list simple_expr             { $$ = NODE_LIST_ADD ($1, $2); }
;

if_stmt:  IF expr THEN non_empty_stmt       { $$ = NODE_IF_ELSE ($2, $4, NULL); }
        | IF expr THEN non_empty_stmt ELSE non_empty_stmt
                                            { $$ = NODE_IF_ELSE ($2, $4, $6); }
;

ternary:  expr '?' expr ':' expr            { $$ = NODE_IF_ELSE ($1, $3, $5); }
;

loop:     REPEAT simple_expr non_empty_stmt { $$ = NODE_REPEAT ($2, $3); }
        | WHILE simple_expr non_empty_stmt  { $$ = NODE_WHILE ($2, $3); }
;

assignment:
        variable '=' expr                   { $$ = NODE_ASSIGNMENT ($1, $3); }
;

expr:     simple_expr
        | expr '+' expr                     { $$ = BINARY_OP (AS_OP_PLUS, $1, $3); }
        | expr '-' expr                     { $$ = BINARY_OP (AS_OP_MINUS, $1, $3); }
        | expr '/' expr                     { $$ = BINARY_OP (AS_OP_DIV, $1, $3); }
        | expr '*' expr                     { $$ = BINARY_OP (AS_OP_MULT, $1, $3); }

        | expr AND expr                     { $$ = BINARY_OP (AS_OP_AND, $1, $3); }
        | expr OR expr                      { $$ = BINARY_OP (AS_OP_OR, $1, $3); }

        | expr EQ expr                      { $$ = BINARY_OP (AS_OP_EQ, $1, $3); }
        | expr NEQ expr                     { $$ = BINARY_OP (AS_OP_NEQ, $1, $3); }
        | expr '<' expr                     { $$ = BINARY_OP (AS_OP_LT, $1, $3); }
        | expr '>' expr                     { $$ = BINARY_OP (AS_OP_GT, $1, $3); }
        | expr LE expr                      { $$ = BINARY_OP (AS_OP_LE, $1, $3); }
        | expr GE expr                      { $$ = BINARY_OP (AS_OP_GE, $1, $3); }
;

simple_expr:
          NUMBER                            { $$ = NODE_NUMBER ($1); }
        | LITERAL                           { $$ = NODE_STRING ($1); }
        | variable
        | '(' stmt_or_program ')'           { $$ = $2; }
        | '[' list_elms ']'                 { $$ = NODE_VALUE_LIST ($2); }
        | simple_expr '%' simple_expr       { $$ = BINARY_OP (AS_OP_FORMAT, $1, $3); }
        | '#' simple_expr                   { $$ = UNARY_OP (AS_OP_LEN, $2); }
        | NOT simple_expr                   { $$ = UNARY_OP (AS_OP_NOT, $2); }
        | '-' simple_expr %prec UMINUS      { $$ = UNARY_OP (AS_OP_UMINUS, $2); }
;

list_elms: /* empty */                      { $$ = NULL; }
        | expr                              { $$ = NODE_LIST_ADD (NULL, $1); }
        | list_elms ',' expr                { $$ = NODE_LIST_ADD ($1, $3); }
;

variable: '$' NUMBER                        { $$ = VAR_POSITIONAL ($2); }
        | '$' IDENTIFIER                    { $$ = VAR_NAMED ($2); }
;

%%
