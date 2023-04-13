#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define SAFEALLOC(var, Type) if ((var = (Type*)malloc(sizeof(Type))) == NULL) err("not enough memory");



enum {
    ID, END,
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    INT, DOUBLE, CHAR, STRUCT, VOID, IF, ELSE, FOR, WHILE, BREAK, RETURN,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT,
    ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ
}; // tokens codes



char* token_names[]= {
    "ID", "END",
    "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING",
    "INT", "DOUBLE", "CHAR", "STRUCT", "VOID", "IF", "ELSE", "FOR", "WHILE", "BREAK", "RETURN",
    "COMMA", "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC",
    "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT",
    "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER", "GREATEREQ"
}; // tokens names (used mainly for printing the lines and their tokens)



typedef struct _Token {
    int code; // code (name)
    union {
        char *text; // used for ID, CT_STRING (dynamically allocated)
        long int i; // used for INT, CT_INT, CT_CHAR
        double r; // used for DOUBLE, CT_REAL
    } type;
    int line; // the input file line
    struct _Token *next; // link to the next token
} Token;



Token* last_token = NULL;
Token* tokens; // list of all tokens
int line = 1; // auxiliary value used in show_tokens, stored as final value in tkn->line when storing the token



void err(const char *fmt, ...) {

    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);

}


void token_err(const Token *tk, const char *fmt, ...) {

    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "Error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);

}


Token *add_token(int code) {

    Token *tkn;
    SAFEALLOC(tkn, Token);
    tkn->code = code;
    tkn->line = line;
    tkn->next = NULL;

    if (last_token) last_token->next = tkn;
    else tokens = tkn;

    last_token = tkn;
    return tkn;

}


char* create_string(const char* start, const char* end) {

    int length_str = end - start + 1;
    char *str = (char*)malloc(sizeof(char) * length_str);
    snprintf(str, length_str, "%s", start);
    return str;

}


void show_tokens() {

    Token *current = tokens;
    int curr_line = 0;


    while(current != NULL){

        if(curr_line != current->line && strcmp(token_names[current->code], "END") != 0) {
            printf("~Line #%i\n", current->line);
            curr_line = current->line;
        }

        if(strcmp(token_names[current->code], "END") != 0) printf("   %s", token_names[current->code]);
        else printf("END");

        switch(current->code) {

            case ID:
            case CT_CHAR: printf(":%s", current->type.text);
                          break;
            case CT_INT:  printf(":%li",current->type.i);
                          break;
            case CT_REAL: printf(":%f", current->type.r);
                          break;

        }

        printf("\n");
        current = current->next;

    }

}


int next_token(char *input) {

    int state = 0, n_ch;
    char ch;
    char *start_ch, *curr_ch = input;
    Token *tkn;


    while (1) {

        ch = *curr_ch;

        switch (state) {

            case 0:
                n_ch = 0;

                if (isalpha(ch) || ch == '_') { // must begin with a letter or _
                    start_ch = curr_ch; // memorizes the beginning of the ID (if it's the case)
                    curr_ch++; // consume the character
                    state = 1; // set the new state
                }

                else if (ch == '=') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '=') {
                        curr_ch++;
                        add_token(ASSIGN);
                    }
                    else add_token(EQUAL);
                }

                else if (ch == '(') {
                    add_token(LPAR);
                    curr_ch++;
                }

                else if (ch == ')') {
                    add_token(RPAR);
                    curr_ch++;
                }

                else if (ch == '[') {
                    add_token(LBRACKET);
                    curr_ch++;
                }

                else if (ch == ']') {
                    add_token(RBRACKET);
                    curr_ch++;
                }

                else if (ch == '{') {
                    add_token(LACC);
                    curr_ch++;
                }

                else if (ch == '}') {
                    add_token(RACC);
                    curr_ch++;
                }

                else if (ch == ';') {
                    add_token(SEMICOLON);
                    curr_ch++;
                }

                else if (ch == '+') {
                    add_token(ADD);
                    curr_ch++;
                }

                else if (ch == '-') {
                    add_token(SUB);
                    curr_ch++;
                }

                else if (ch == '*') {
                    add_token(MUL);
                    curr_ch++;
                }

                else if (ch == '.') {
                    add_token(DOT);
                    curr_ch++;
                }

                else if (ch == '&') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '&') {
                        curr_ch++;
                        add_token(AND);
                    }
                    else token_err(tkn, "Expected binary operator\n");
                }

                else if (ch == '|') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '|') {
                        curr_ch++;
                        add_token(OR);
                    }
                    else token_err(tkn, "Expected binary operator\n");
                }

                else if (ch == '!') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '=') {
                        curr_ch++;
                        add_token(NOTEQ);
                    }
                    else add_token(NOT);
                }

                else if (ch == '<') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '=') {
                        curr_ch++;
                        add_token(LESSEQ);
                    }
                    else add_token(LESS);
                }

                else if (ch == '>') {
                    curr_ch++;
                    ch = *curr_ch;

                    if (ch == '=') {
                        curr_ch++;
                        add_token(GREATEREQ);
                    }
                    else add_token(GREATER);
                }

                else if (ch == ' ' || ch == '\r' || ch == '\t')
                    curr_ch++; // consume the character and remains in state 0

                else if (ch == '\n') { // handled separately in order to update the current line
                    line++;
                    curr_ch++;
                }

                else if (ch == '\0') { // the end of the input string
                    add_token(END);
                    show_tokens();
                    return 0;
                }

                else if (ch >= '0' && ch <= '9') {
                    state = 3; // handle CT_INT or CT_REAL at state 3
                    start_ch = curr_ch; // remember the beginning of number
                    curr_ch++;
                }

                else token_err(add_token(END), "invalid character");
                break;


            case 1:
                if (isalnum(ch) || ch == '_') curr_ch++; // after state 0, if we still have letters or digits
                else state = 2; // reached the end of ID
                break;


            case 2:
                n_ch = curr_ch - start_ch; // the ID length
                // keywords tests
                if      (n_ch == 5 && !memcmp(start_ch, "break", 5))    { tkn = add_token(BREAK); curr_ch++; state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "char", 4))     { tkn = add_token(CHAR); curr_ch++; state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "double", 6))   { tkn = add_token(DOUBLE); curr_ch++; state = 0; }
                else if (n_ch == 3 && !memcmp(start_ch, "int", 3))      { tkn = add_token(INT); curr_ch++; state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "struct", 6))   { tkn = add_token(STRUCT); curr_ch++; state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "void", 4))     { tkn = add_token(VOID); curr_ch++; state = 0; }
                else if (n_ch == 2 && !memcmp(start_ch, "if", 2))       { tkn = add_token(IF); curr_ch++; state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "else", 4))     { tkn = add_token(ELSE); curr_ch++; state = 0; }
                else if (n_ch == 5 && !memcmp(start_ch, "while", 5))    { tkn = add_token(WHILE); curr_ch++; state = 0; }
                else if (n_ch == 3 && !memcmp(start_ch, "for", 3))      { tkn = add_token(FOR); curr_ch++; state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "return", 6))   { tkn = add_token(RETURN); curr_ch++; state = 0; }

                else { // if no keyword, then it sure is an ID
                    tkn = add_token(ID);
                    tkn->type.text = create_string(start_ch, curr_ch);
                    state = 0;
                }
                break;


            case 3:
                if (ch == ';' || ch == ',' || ch == ']' || ch == ')'){ // reached the end of number ; build the number and add the SEMICOLON token
                    tkn = add_token(CT_INT);
                    tkn->type.i = strtol(start_ch, NULL,0);

                    if (ch == ';') add_token(SEMICOLON);
                    else if (ch == ',') add_token(COMMA);
                    else if (ch == ']') add_token(RBRACKET);
                    else add_token(RPAR);

                    state = 0;
                }
                curr_ch++;

        }

    }

}




int main(int argc, char **argv) {

    if (argc != 2){
        printf("Please provide a file name");
        exit(-1);
    }


    int source_fd = open(argv[1], O_RDONLY);
    if (source_fd < 0){
        printf("Could not open file");
        exit(-1);
    }


    struct stat st;
    int file_size;
    if(stat(argv[1], &st)){
        printf("Could not process file stats");
        exit(-1);
    }
    else file_size = st.st_size;


    char buffer[file_size * sizeof(char) - 1];
    int bytes_read;
    if( (bytes_read = read(source_fd, buffer, file_size * sizeof(char))) < 0 ){
        printf("Could not read file contents");
        exit(-1);
    }

    if(bytes_read < file_size)  buffer[bytes_read] = '\0';
    else buffer[file_size] = '\0';

    //printf("%s", buffer);
    next_token(buffer);


    if(close(source_fd) < 0){
        printf("Could not close file");
        exit(-1);
    }

}
