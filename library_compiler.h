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



typedef struct _Symbol Symbol;
typedef struct{
    Symbol **begin; // the beginning of the symbols, or NULL
    Symbol **end; // the position after the last symbol
    Symbol **after; // the position after the allocated space
}Symbols;

enum{TB_INT,TB_DOUBLE,TB_CHAR,TB_STRUCT,TB_VOID};
typedef struct{
    int type_base; // TB_*
    Symbol *s; // struct definition for TB_STRUCT
    int n_elements; // >0 array of given size, 0=array without size, <0 non array
} Type;

enum{CLS_VAR,CLS_FUNC,CLS_EXTFUNC,CLS_STRUCT};
enum{MEM_GLOBAL,MEM_ARG,MEM_LOCAL};
typedef struct _Symbol{
    const char *name; // a reference to the name stored in a token
    int cls; // CLS_*
    int mem; // MEM_*
    Type type;
    int depth; // 0-global, 1-in function, 2... - nested blocks in function
    union{
        Symbols args; // used only of functions
        Symbols members; // used only for structs
    };
} Symbol;



typedef union{
    long int i; // int, char
    double d; // double
    const char *str; // char[]
} CtVal;
typedef struct{
    Type type; // type of the result
    int is_lval; // if it is a LVal
    int is_ctval; // if it is a constant value (int, real, char, char[])
    CtVal ctval; // the constat value
} RetVal;



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
int type_base(Type *ret);
int array_decl(Type *ret);
int type_name(Type *ret);
int decl_func();
int funct_arg();
int stm();
int stm_compound();
int expr(RetVal *rv);
int expr_assign(RetVal *rv);
int expr_or(RetVal *rv);
int expr_or_1(RetVal *rv);
int expr_and(RetVal *rv);
int expr_and_1(RetVal *rv);
int expr_eq(RetVal *rv);
int expr_eq_1(RetVal *rv);
int expr_rel(RetVal *rv);
int expr_rel_1(RetVal *rv);
int expr_add(RetVal *rv);
int expr_add_1(RetVal *rv);
int expr_mul(RetVal *rv);
int expr_mul_1(RetVal *rv);
int expr_cast(RetVal *rv);
int expr_unary(RetVal *rv);
int expr_postfix(RetVal *rv);
int expr_postfix_1(RetVal *rv);
int expr_primary(RetVal *rv);
void init_symbols(Symbols *symbols);
Symbol *add_symbol(Symbols *symbols, const char *name, int cls);
Symbol *find_symbol(Symbols *symbols, const char *name);
void add_var(Token *tkName, Type *t);
void delete_symbols_after(Symbols *symbols, Symbol *symbol);
Type create_type(int type_base, int n_elements);
void cast(Type *dst,Type *src);
Type get_arith_type(Type *s1, Type *s2);
Symbol *add_ext_func(const char *name, Type type);
Symbol *add_func_arg(Symbol *func,const char *name, Type type);


#endif //LABCT_LIBRARY_COMPILER_H
