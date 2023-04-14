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


void err(const char *fmt, ...);
void token_err(const Token *tk, const char *fmt, ...);
Token *add_token(int code);
char* create_string(const char* start, const char* end);
void show_tokens();
int next_token(char *input);


#endif //LABCT_LIBRARY_COMPILER_H
