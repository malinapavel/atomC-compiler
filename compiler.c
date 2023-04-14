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
        char *text; // used for ID, CT_CHAR, CT_STRING (dynamically allocated)
        long int i; // used for INT, CT_INT
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
            case CT_CHAR:   printf(":%s", current->type.text);
                            break;
            case CT_STRING: printf(":%s", current->type.text);
                            break;
            case CT_INT:    printf(":%li",current->type.i);
                            break;
            case CT_REAL:   printf(":%f", current->type.r);
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
                start_ch = curr_ch;
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

                else if(ch == ','){
                    curr_ch++;
                    add_token(COMMA);
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

                else if(ch == '/') {
                    curr_ch++;
                    state = 20;
                }

                else if(ch == '\'') {
                    curr_ch++;
                    state = 11;
                }

                else if(ch == '\"') {
                    curr_ch++;
                    state = 15;
                }

                else if (ch == '.') {
                    add_token(DOT);
                    curr_ch++;

                    // checking for numbers like ".123" -> yield an error message
                    if (isdigit(curr_ch[0])) token_err(tkn, "Invalid real number format");
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

                else if (ch == ' ' || ch == '\r' || ch == '\t')   curr_ch++; // consume the character and remains in state 0

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
                    state = 3; // anything that could be a potential CT_INT or CT_REAL is passed in state 3
                    start_ch = curr_ch; // remember the beginning of number
                    curr_ch++;
                }

                else token_err(add_token(END), "Invalid character");
                break;


// ~~~~~~~~ ID

            case 1:
                if (isalnum(ch) || ch == '_') curr_ch++; // after state 0, if we still have letters or digits
                else state = 2; // reached the end of ID
                break;


            case 2:
                n_ch = curr_ch - start_ch; // the ID length
                // keywords tests
                if      (n_ch == 5 && !memcmp(start_ch, "break", 5))    { tkn = add_token(BREAK); state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "char", 4))     { tkn = add_token(CHAR); state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "double", 6))   { tkn = add_token(DOUBLE); state = 0; }
                else if (n_ch == 3 && !memcmp(start_ch, "int", 3))      { tkn = add_token(INT); state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "struct", 6))   { tkn = add_token(STRUCT); state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "void", 4))     { tkn = add_token(VOID); state = 0; }
                else if (n_ch == 2 && !memcmp(start_ch, "if", 2))       { tkn = add_token(IF); state = 0; }
                else if (n_ch == 4 && !memcmp(start_ch, "else", 4))     { tkn = add_token(ELSE); state = 0; }
                else if (n_ch == 5 && !memcmp(start_ch, "while", 5))    { tkn = add_token(WHILE); state = 0; }
                else if (n_ch == 3 && !memcmp(start_ch, "for", 3))      { tkn = add_token(FOR); state = 0; }
                else if (n_ch == 6 && !memcmp(start_ch, "return", 6))   { tkn = add_token(RETURN); state = 0; }

                else { // if no keyword, then it sure is an ID
                    tkn = add_token(ID);
                    tkn->type.text = create_string(start_ch, curr_ch);
                    state = 0;
                }
                break;


// ~~~~~~~~ CT_INT numbers (+separating them from the HEXA/OCTAL INTs AND CT_REAL NUMBERS)

            case 3: // after flushing the first digit
                if (start_ch[0] == '0'){
                    if(ch == 'x' || ch == 'X') { state = 4; curr_ch++; break; } // handle hexadecimal CT_INTs in state 4
                    else if (ch >= '0' && ch <= '7') { state = 5; break; } // handle octal CT_INTs in state 5
                    else if (ch == '8' || ch == '9') token_err(tkn, "Invalid octal format"); // it reached a character from an octal number which happens to start with 8
                    else if (ch == '.') { state = 6; curr_ch++; break; } // if we encounter a real number of the format "0."
                }

                else if (ch == 'e' || ch =='E'){ // if we encounter a real number of the format "[0-9]+ E (+|-)? [0-9]+" (2E+01, for example)
                                                 // a CT_REAL case to be handled directly in state 8
                    state = 8;
                    curr_ch++;
                    break;
                }

                else if (ch == '.'){ // handle CT_REALs in state 6
                    state = 6;
                    curr_ch++;
                    break;
                }

                // handle case where we could have a variable which starts with a number... not acceptable!
                if (isdigit(start_ch[0]) && ch == '=')  token_err(tkn, "A variable must not start with a number!!");

                // this is where we compute our CT_INTs
                if (ch == ';' || ch == ',' || ch == ']' || ch == ')'){ // reached the end of number ; build the number and add the ending tokens ; , ] )
                    tkn = add_token(CT_INT);
                    tkn->type.i = strtol(start_ch, NULL,0);

                    if (ch == ';') add_token(SEMICOLON);
                    else if (ch == ',') add_token(COMMA);
                    else if (ch == ']') add_token(RBRACKET);
                    else if (ch == ')') add_token(RPAR);

                    // if we encounter the format "0x"
                    if ( (curr_ch - start_ch <= 2) && (start_ch[1] == 'x' || start_ch[1] == 'X') )  token_err(tkn, "Invalid hexadecimal format");

                    state = 0;
                }
                curr_ch++;
                break;


            case 4: // hexadecimal
                if ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') ) curr_ch++;
                else if ( (ch >= 'g' && ch <= 'z') || (ch >= 'G' && ch <= 'Z') ) token_err(tkn, "Invalid hexadecimal format");
                else state = 3;
                break;


            case 5: // octal
                if (ch >= '0' && ch <= '7') curr_ch++;
                else if ( ch == '8' || ch == '9') token_err(tkn, "Invalid octal format");
                else state = 3;
                break;


// ~~~~~~~~ CT_REAL

            case 6:
                if (ch >= '0' && ch <= '9') { curr_ch++; state = 7; }
                else token_err(tkn, "Invalid real number format");
                break;


            case 7:
                if (ch >= '0' && ch <= '9') { curr_ch++; state = 7; }
                else if (ch == 'e' || ch =='E') { curr_ch++; state = 8; }
                else state = 10; // real numbers like '123.67'
                break;


            case 8: // it will jump to the state where we check the digit after E anyway
                if (ch == '+' || ch =='-')  curr_ch++;
                state = 9;
                break;


            case 9: // we encountered a real number of the form '123.67E(+/-)8'
                if (ch >= '0' && ch <= '9') { curr_ch++; state = 10; }
                else token_err(tkn, "Invalid real number format");
                break;


            case 10: // this is where we compute CT_REAL
                if (ch == ';' || ch == ',' || ch == ']' || ch == ')'){ // reached the end of number ; build the number and add the ending tokens ; , ] )
                    tkn = add_token(CT_REAL);
                    tkn->type.r = strtod(start_ch, NULL);

                    if (ch == ';') add_token(SEMICOLON);
                    else if (ch == ',') add_token(COMMA);
                    else if (ch == ']') add_token(RBRACKET);
                    else add_token(RPAR);

                    state = 0;
                }
                curr_ch++;
                break;


// ~~~~~~~~ CT_CHAR

            case 11:
                if (ch == '\\') { curr_ch++; state = 12; } // we have a character like '\n', we entered the ESC case -> go in state 12 to check it out
                else state = 13; // we have a character like 'a', handle that in state 13
                break;


            case 12:
                if(strchr("abfnrtv'?\"\\0", ch)) { curr_ch++; state = 14; }
                else token_err(tkn, "Expected a character from [abfnrtv'?\"\\\\0]");
                break;


            case 13:
                if (isalnum((ch)) || strchr("~`!@#$%^&*()-_=+{[}]|;:,<.>?/", ch) != 0) { curr_ch++; state = 14;}
                else token_err(tkn, "Invalid char format");
                break;


            case 14:
                if(ch == '\''){ // end of character

                    tkn = add_token(CT_CHAR);

                      char aux[2] = {0};
                      if ((curr_ch-2)[0] == '\\'){ // for '\\' [abfnrtv'?"\\0] cases
                          aux[0] = (curr_ch - 2)[0];
                          aux[1] = (curr_ch - 2)[1];
                          aux[2] = '\0';
                      }
                      else if (isalnum(((curr_ch - 1)[0])) || strchr("~`!@#$%^&*()-_=+{[}]|;:,<.>?/", (curr_ch - 1)[0]) != 0) // for 'any_other_character' cases
                      {
                          aux[0] = (curr_ch - 1)[0];
                          aux[1] = '\0';
                          aux[2] = '\0';
                      }

                    tkn->type.text = strdup(aux); // allocate memory for aux string and store it as text for the CT_CHAR token

                    curr_ch++;
                    state = 0;
                }
                else token_err(tkn, "Expected character");
                break;



// ~~~~~~~ CT_STRING

            case 15:
                if (ch == '\\') { curr_ch++; state = 16; } // we have a string like "\n", we entered the ESC case -> go in state 16 to check it out
                else state = 17; // we have a string like "abc", handle that in state 17
                break;


            case 16:
                if(strchr("abfnrtv'?\"\\0", ch)) { curr_ch++; state = 14; }
                else token_err(tkn, "Expected a character from [abfnrtv'?\"\\\\0]");
                break;


            case 17:
                if (ch != '\"' || ch != '\\') { curr_ch++; state = 18; }
                else token_err(tkn, "Invalid char format");
                break;


            case 18: // if we still have some characters in the string before reaching the \"
                if (isalnum((ch)) || strchr("~`!@#$%^&*()-_=+{[}]|;:,<.>?/", ch) != 0) curr_ch++;
                else state = 19;
                break;


            case 19:
                if(ch == '\"'){ // end of string

                    tkn = add_token(CT_STRING);

                    char *aux;
                    aux = create_string(start_ch + 1, curr_ch);
                    aux[strlen(aux)] = '\0';
                    tkn->type.text = strdup(aux); // allocate memory for aux string and store it as text for the CT_STRING token

                    curr_ch++;
                    state = 0;
                }
                else token_err(tkn, "Expected character");
                break;


// ~~~~~~~~ COMMENTS AND OTHER SHENANIGANS

            case 20: // check which cases to handle: COMMENT ('/*') goes in state 21, LINECOMMENT ('//') in state 25 or divide operator will be added, otherwise
                if (ch == '*') { curr_ch++; state = 21; }
                else if (ch == '/') { curr_ch++; state = 23; }
                else { add_token(DIV); state = 0; }
                break;


            case 21: // check whether we have '/**...' or '/*...'
                if (ch == '*') { curr_ch++; state = 22; } // '/**.....'
                else if (ch == '\n') { curr_ch++; line++; } // if we have constructions like '/* .... \n .... */', add each line
                else curr_ch++; // '/*...'
                break;


            case 22:
                if (ch == '/') { curr_ch++; state = 0; } // end of COMMENT
                else if (ch == '*')  curr_ch++;
                else if (isalnum((ch)) || strchr("~`!@#$%^&()-_=+{[}]|;:,<.>? ", ch) != 0) { curr_ch++; state = 21;}
                break;


            case 23:
                if (ch == '\n') { curr_ch++; state = 0; line++; }
                else if (ch == '\r' || ch == '\0') { state = 0; }
                else curr_ch++;
                break;

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
