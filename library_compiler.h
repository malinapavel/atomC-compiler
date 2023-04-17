#ifndef LABCT_LIBRARY_COMPILER_H
#define LABCT_LIBRARY_COMPILER_H

#define SAFEALLOC(var, Type) if ((var = (Type*)malloc(sizeof(Type))) == NULL) err("not enough memory");

enum {
    ID, END,
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    INT, DOUBLE, CHAR, STRUCT, VOID, IF, ELSE, FOR, WHILE, BREAK, RETURN,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT,
    ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ
}; // tokens codes


typedef struct _Token {
    int code; // code (name)
    union {
        char *text; // used for ID, CT_CHAR, CT_STRING
        long int i; // used for INT, CT_INT
        double r; // used for DOUBLE, CT_REAL
    } type;
    int line; // the input file line
    struct _Token *next; // link to the next token
} Token;

extern Token *tokens;
void err(const char *fmt, ...);
void token_err(const Token *tk, const char *fmt, ...);
Token *add_token(int code);
char* create_string(const char* start, const char* end);
void show_tokens();
int next_token(char *input);
int consume();
int unit();
int decl_struct();
int decl_var();
int type_base();
int array_decl();
int type_name();
int decl_func();
int funct_arg();
int stm();
int stm_compound();
int expr();
int expr_assign();
int expr_or();
void expr_or_1();
int expr_and();
void expr_and_1();
int expr_eq();
void expr_eq_1();
int expr_rel();
void expr_rel_1();
int expr_add();
void expr_add_1();
int expr_mul();
void expr_mul_1();
int expr_cast();
int expr_unary();
int expr_postfix();
void expr_postfix_1();
int expr_primary();

#endif //LABCT_LIBRARY_COMPILER_H
