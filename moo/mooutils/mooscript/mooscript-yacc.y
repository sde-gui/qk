%{
#include "mooscript-parser-priv.h"
#include "mooscript-yacc.h"


static MSNode *
node_list_add (MSParser   *parser,
               MSNodeList *list,
               MSNode     *node)
{
    if (!list)
    {
        list = ms_node_list_new ();
        _ms_parser_add_node (parser, list);
    }

    if (!node)
    {
        MSValue *none = ms_value_none ();
        node = MS_NODE (ms_node_value_new (none));
        ms_value_unref (none);
        _ms_parser_add_node (parser, node);
    }

    ms_node_list_add (list, node);
    return MS_NODE (list);
}


static MSNode *
node_function (MSParser   *parser,
               MSNode     *func,
               MSNodeList *args)
{
    MSNodeFunction *node;

    g_return_val_if_fail (func != NULL, NULL);
    g_return_val_if_fail (!args || MS_IS_NODE_LIST (args), NULL);

    node = ms_node_function_new (func, args);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_if_else (MSParser   *parser,
              MSNode     *condition,
              MSNode     *then_,
              MSNodeList *elif_,
              MSNode     *else_)
{
    MSNodeIfElse *node;

    g_return_val_if_fail (condition && then_, NULL);

    node = ms_node_if_else_new (condition, then_, else_);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}

static MSNode *
node_condition (MSParser   *parser,
                MSNode     *condition,
                MSNode     *then_)
{
    MSNode *tuple;
    tuple = node_list_add (parser, NULL, condition);
    tuple = node_list_add (parser, MS_NODE_LIST (tuple), then_);
    return tuple;
}


static MSNode *
node_while (MSParser   *parser,
            MSCondType  type,
            MSNode     *cond,
            MSNode     *what)
{
    MSNodeWhile *loop;

    g_return_val_if_fail (cond != NULL, NULL);

    loop = ms_node_while_new (type, cond, what);
    _ms_parser_add_node (parser, loop);

    return MS_NODE (loop);
}


static MSNode *
node_for (MSParser   *parser,
          MSNode     *var,
          MSNode     *list,
          MSNode     *what)
{
    MSNodeFor *loop;

    g_return_val_if_fail (var && list, NULL);

    loop = ms_node_for_new (var, list, what);
    _ms_parser_add_node (parser, loop);

    return MS_NODE (loop);
}


static MSNode *
node_var (MSParser   *parser,
          const char *name)
{
    MSNodeVar *node;

    node = ms_node_var_new (name);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_assignment (MSParser   *parser,
                 const char *name,
                 MSNode     *val)
{
    MSNodeAssign *node;
    MSNode *var;

    g_return_val_if_fail (name && name[0], NULL);
    g_return_val_if_fail (val != NULL, NULL);

    var = node_var (parser, name);
    node = ms_node_assign_new (MS_NODE_VAR (var), val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_binary_op (MSParser   *parser,
                MSBinaryOp  op,
                MSNode     *lval,
                MSNode     *rval)
{
    MSNodeFunction *node;

    g_return_val_if_fail (lval && rval, NULL);

    node = ms_node_binary_op_new (op, lval, rval);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_unary_op (MSParser   *parser,
               MSUnaryOp   op,
               MSNode     *val)
{
    MSNodeFunction *node;

    g_return_val_if_fail (val != NULL, NULL);

    node = ms_node_unary_op_new (op, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_int (MSParser   *parser,
          int         n)
{
    MSNodeValue *node;
    MSValue *value;

    value = ms_value_int (n);
    node = ms_node_value_new (value);
    ms_value_unref (value);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_string (MSParser   *parser,
             const char *string)
{
    MSNodeValue *node;
    MSValue *value;

    value = ms_value_string (string);
    node = ms_node_value_new (value);
    ms_value_unref (value);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_python (MSParser   *parser,
             const char *string)
{
    MSNodePython *node;

    node = ms_node_python_new (string);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_list (MSParser   *parser,
                 MSNodeList *list)
{
    MSNodeValList *node;

    node = ms_node_val_list_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_range (MSParser   *parser,
                  MSNode     *first,
                  MSNode     *last)
{
    MSNodeValList *node;

    node = ms_node_val_range_new (first, last);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_get_item (MSParser   *parser,
               MSNode     *obj,
               MSNode     *key)
{
    MSNodeGetItem *node;

    node = ms_node_get_item_new (obj, key);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_set_item (MSParser   *parser,
               MSNode     *obj,
               MSNode     *key,
               MSNode     *val)
{
    MSNodeSetItem *node;

    node = ms_node_set_item_new (obj, key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict (MSParser   *parser,
           MSNodeList *list)
{
    MSNodeDict *node;

    node = ms_node_dict_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_entry (MSParser   *parser,
                 const char *key,
                 MSNode     *val)
{
    MSNodeDictEntry *node;

    node = ms_node_dict_entry_new (key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_return (MSParser   *parser,
             MSNode     *val)
{
    MSNodeReturn *node;

    node = ms_node_return_new (val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_break (MSParser *parser)
{
    MSNodeBreak *node;

    node = ms_node_break_new (MS_BREAK_BREAK);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_continue (MSParser *parser)
{
    MSNodeBreak *node;

    node = ms_node_break_new (MS_BREAK_CONTINUE);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_elm (MSParser   *parser,
               MSNode     *dict,
               const char *key)
{
    MSNodeDictElm *node;

    node = ms_node_dict_elm_new (dict, key);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_assign (MSParser   *parser,
                  MSNode     *dict,
                  const char *key,
                  MSNode     *val)
{
    MSNodeDictAssign *node;

    node = ms_node_dict_assign_new (dict, key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}

%}

%name-prefix="_ms_script_yy"
%error-verbose
%lex-param      {MSParser *parser}
%parse-param    {MSParser *parser}
/* %expect 1 */

%union {
    int ival;
    const char *str;
    MSNode *node;
}

%token <str> IDENTIFIER
%token <str> LITERAL
%token <str> VARIABLE
%token <str> PYTHON
%token <ival> NUMBER

%type <node> program stmt stmt_or_python
%type <node> if_stmt elif_block ternary loop assignment
%type <node> simple_expr compound_expr expr variable
%type <node> list_elms dict_elms dict_entry

%token IF THEN ELSE ELIF FI
%token WHILE DO OD FOR IN
%token CONTINUE BREAK RETURN
%token EQ NEQ LE GE
%token AND OR NOT
%token UMINUS
%token TWODOTS

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

script:   program           { _ms_parser_set_top_node (parser, $1); }
;

program:  stmt_or_python            { $$ = node_list_add (parser, NULL, $1); }
        | program stmt_or_python    { $$ = node_list_add (parser, MS_NODE_LIST ($1), $2); }
;

stmt_or_python:
          error ';'         { $$ = NULL; }
        | stmt ';'          { $$ = $1; }
        | PYTHON            { $$ = node_python (parser, $1); }
;

stmt:   /* empty */         { $$ = NULL; }
        | expr
        | if_stmt
        | loop
        | assignment /* not an expr because otherwise 'var = var + 2' gets parsed as '(var = var) + 2'*/
        | CONTINUE          { $$ = node_continue (parser); }
        | BREAK             { $$ = node_break (parser); }
        | RETURN            { $$ = node_return (parser, NULL); }
        | RETURN expr       { $$ = node_return (parser, $2); }
;

loop:     WHILE expr DO program OD              { $$ = node_while (parser, MS_COND_BEFORE, $2, $4); }
        | DO program WHILE expr                 { $$ = node_while (parser, MS_COND_AFTER, $4, $2); }
        | FOR variable IN expr DO program OD    { $$ = node_for (parser, $2, $4, $6); }
;

if_stmt:
          IF expr THEN program FI               { $$ = node_if_else (parser, $2, $4, NULL, NULL); }
        | IF expr THEN program elif_block FI    { $$ = node_if_else (parser, $2, $4, MS_NODE_LIST ($5), NULL); }
        | IF expr THEN program elif_block ELSE program FI
                                                { $$ = node_if_else (parser, $2, $4, MS_NODE_LIST ($5), $7); }
;

elif_block:
          ELIF expr THEN program                { $$ = node_list_add (parser, NULL, node_condition (parser, $2, $4)); }
        | elif_block ELIF expr THEN program     { $$ = node_list_add (parser, MS_NODE_LIST ($1), node_condition (parser, $3, $5)); }
;

expr:     simple_expr
        | compound_expr
        | ternary
;

assignment:
          IDENTIFIER '=' expr                   { $$ = node_assignment (parser, $1, $3); }
        | simple_expr '[' expr ']' '=' expr     { $$ = node_set_item (parser, $1, $3, $6); }
        | simple_expr '[' error ']' '=' expr    { $$ = NULL; }
        | simple_expr '.' IDENTIFIER '=' expr   { $$ = node_dict_assign (parser, $1, $3, $5); }
;

ternary:  simple_expr '?' simple_expr ':' simple_expr { $$ = node_if_else (parser, $1, $3, NULL, $5); }
;

compound_expr:
          expr '+' expr                     { $$ = node_binary_op (parser, MS_OP_PLUS, $1, $3); }
        | expr '-' expr                     { $$ = node_binary_op (parser, MS_OP_MINUS, $1, $3); }
        | expr '/' expr                     { $$ = node_binary_op (parser, MS_OP_DIV, $1, $3); }
        | expr '*' expr                     { $$ = node_binary_op (parser, MS_OP_MULT, $1, $3); }

        | expr AND expr                     { $$ = node_binary_op (parser, MS_OP_AND, $1, $3); }
        | expr OR expr                      { $$ = node_binary_op (parser, MS_OP_OR, $1, $3); }

        | expr EQ expr                      { $$ = node_binary_op (parser, MS_OP_EQ, $1, $3); }
        | expr NEQ expr                     { $$ = node_binary_op (parser, MS_OP_NEQ, $1, $3); }
        | expr '<' expr                     { $$ = node_binary_op (parser, MS_OP_LT, $1, $3); }
        | expr '>' expr                     { $$ = node_binary_op (parser, MS_OP_GT, $1, $3); }
        | expr LE expr                      { $$ = node_binary_op (parser, MS_OP_LE, $1, $3); }
        | expr GE expr                      { $$ = node_binary_op (parser, MS_OP_GE, $1, $3); }
        | '-' simple_expr %prec UMINUS      { $$ = node_unary_op (parser, MS_OP_UMINUS, $2); }
        | NOT simple_expr                   { $$ = node_unary_op (parser, MS_OP_NOT, $2); }
        | '#' simple_expr                   { $$ = node_unary_op (parser, MS_OP_LEN, $2); }
        | simple_expr '%' simple_expr       { $$ = node_binary_op (parser, MS_OP_FORMAT, $1, $3); }

        | simple_expr IN simple_expr        { $$ = node_binary_op (parser, MS_OP_IN, $1, $3); }
;

simple_expr:
          NUMBER                            { $$ = node_int (parser, $1); }
        | LITERAL                           { $$ = node_string (parser, $1); }
        | variable
        | '(' stmt ')'                      { $$ = $2; }
        | '(' error ')'                     { $$ = NULL; }
        | '[' list_elms ']'                 { $$ = node_value_list (parser, MS_NODE_LIST ($2)); }
        | '[' error ']'                     { $$ = NULL; }
        | '{' dict_elms '}'                 { $$ = node_dict (parser, $2 ? MS_NODE_LIST ($2) : NULL); }
        | '{' error '}'                     { $$ = NULL; }
        | '[' expr TWODOTS expr ']'         { $$ = node_value_range (parser, $2, $4); }
        | simple_expr '(' list_elms ')'     { $$ = node_function (parser, $1, $3 ? MS_NODE_LIST ($3) : NULL); }
        | simple_expr '[' expr ']'          { $$ = node_get_item (parser, $1, $3); }
        | simple_expr '.' IDENTIFIER        { $$ = node_dict_elm (parser, $1, $3); }
;

list_elms: /* empty */                      { $$ = NULL; }
        | expr                              { $$ = node_list_add (parser, NULL, $1); }
        | list_elms ',' expr                { $$ = node_list_add (parser, MS_NODE_LIST ($1), $3); }
;

dict_elms: /* empty */                      { $$ = NULL; }
        | dict_entry                        { $$ = node_list_add (parser, NULL, $1); }
        | dict_elms ',' dict_entry          { $$ = node_list_add (parser, MS_NODE_LIST ($1), $3); }
;

dict_entry: IDENTIFIER '=' expr             { $$ = node_dict_entry (parser, $1, $3); }
;

variable: IDENTIFIER                        { $$ = node_var (parser, $1); }
;

%%
