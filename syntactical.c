#include <stdio.h>
#include <stdlib.h>
#include "library_compiler.h"

Token *curr_tkn;
Token *consumed_tkn;



int consume(int code){

    if(curr_tkn->code == code){
        consumed_tkn = curr_tkn;
        curr_tkn = curr_tkn->next;
        return 1;
    }

    return 0;

}



int unit() {

    curr_tkn = tokens;

    while(1){
        if(decl_struct()) {}
        else if(decl_func()) {}
        else if(decl_var()) {}
        else break;
    }
    if(consume(END)) return 1;
    else token_err(curr_tkn, "Missing END token!");

    return 0;

}



int decl_struct() {

    Token *start_tkn = curr_tkn;

    if(consume(STRUCT)){
        if(consume(ID)){
            if(consume(LACC)){
                while(1){
                    if(decl_var()) {}
                    else break;
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)) return 1;
                    else token_err(curr_tkn, "Missing semicolon in structure declaration!");
                } else token_err(curr_tkn, "Missing right accolade in structure declaration!");
            } else token_err(curr_tkn, "Missing left accolade in structure declaration!");
        } else token_err(curr_tkn, "Missing structure name!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int decl_var() {

    Token *start_tkn = curr_tkn;

    if(type_base()) {
        if(consume(ID)) {
            array_decl();
            while(1){
                if(consume(COMMA)){
                    if (consume(ID)){
                        array_decl();
                    } else token_err(curr_tkn, "ID expected!");
                } else break;
            }
            if(consume(SEMICOLON)) return 1;
            else token_err(curr_tkn, "Missing ; in variable declaration!");
        } else token_err(curr_tkn, "ID expected after type base!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int type_base() {

    Token *start_tkn = curr_tkn;

    if(consume(INT))          return 1;
    else if(consume(DOUBLE))  return 1;
    else if(consume(CHAR))    return 1;
    else if(consume(STRUCT)) {
        if(consume(ID))  return 1;
        else token_err(curr_tkn, "ID struct name expected after type \"struct\"!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int array_decl() {

    Token *start_tkn = curr_tkn;

    if(consume(LBRACKET)) {
        expr();
        if(consume(RBRACKET))  return 1;
        else token_err(curr_tkn, "Missing right bracket from array declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int type_name() {

    if(type_base())return 0;
    array_decl();
    return 1;

}



int decl_func() {

    Token *start_tkn = curr_tkn;

    if(type_base()) {
        if(consume(MUL))  { }
            if(consume(ID)) {
                if(consume(LPAR)){
                    if(funct_arg()){
                        while(1) {
                            if(consume(COMMA)){
                                if(!funct_arg()) token_err(curr_tkn, "Missing function argument in function declaration!");
                            }
                            else break;
                        }
                    }
                    if(consume(RPAR)) {
                        if(stm_compound()) return 1;
                        else token_err(curr_tkn, "Missing statement compound in function declaration!");
                    } else token_err(curr_tkn, "Missing right parenthesis in function declaration!");
                } else token_err(curr_tkn, "Missing left parenthesis in function declaration!");
            } else token_err(curr_tkn, "Missing function name in function declaration!");
    }

    else if (consume(VOID)) {
        if(consume(ID)) {
            if(consume(LPAR)){
                if(funct_arg()){
                    while(1) {
                        if(consume(COMMA)){
                            if(!funct_arg()) token_err(curr_tkn, "Missing function argument in function declaration!");
                        }
                        else break;
                    }
                }
                if(consume(RPAR)) {
                    if(stm_compound()) return 1;
                    else token_err(curr_tkn, "Missing statement compound in function declaration!");
                } else token_err(curr_tkn, "Missing right parenthesis in function declaration!");
            } else token_err(curr_tkn, "Missing left parenthesis in function declaration!");
        } else token_err(curr_tkn, "Missing function name in function declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int funct_arg() {

    Token *start_tkn = curr_tkn;

    if(type_base()){
        if(consume(ID)) {
            if(array_decl()) { }
            return 1;
        }
        else token_err(curr_tkn, "ID missing in function declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int stm() {

    Token *start_tkn = curr_tkn;

    if(stm_compound())  return 1;

    else if(consume(IF)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(!stm()) token_err(curr_tkn, "Expected statement after ELSE!") ;
                        }
                        return 1;
                    } else token_err(curr_tkn, "Expected statement after IF!") ;
                } else token_err(curr_tkn, "Missing right parenthesis after IF") ;
            } else token_err(curr_tkn, "Expected expression after left parenthesis!");
        } else token_err(curr_tkn, "Missing left parenthesis after IF!") ;
    }

    else if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr()) {
                if(consume(RPAR)){
                    if(stm())  return 1;
                    else token_err(curr_tkn, "Expected statement after WHILE!") ;
                } else token_err(curr_tkn, "Missing left parenthesis after WHILE!") ;
            } else token_err(curr_tkn, "Expected expression after left parenthesis!") ;
        } else token_err(curr_tkn, "Missing left parenthesis after WHILE!") ;
    }

    else if(consume(FOR)){
        if(consume(LPAR)){
            expr();
            if(consume(SEMICOLON)){
                expr();
                if(consume(SEMICOLON)){
                    expr();
                    if(consume(RPAR)) {
                        if(stm()) return 1;
                        else token_err(curr_tkn, "Expected statement after FOR!") ;
                    } else token_err(curr_tkn, "missing right parenthesis after FOR!") ;
                } else token_err(curr_tkn, "Missing ; in FOR!") ;
            } else token_err(curr_tkn, "Missing ; in FOR!") ;
        } else token_err(curr_tkn, "Missing left parenthesis after FOR!") ;
    }

    else if(consume(BREAK)) {
        if(consume(SEMICOLON)) return 1;
        else token_err(curr_tkn, "Missing ; after BREAK!") ;
    }

    else if(consume(RETURN)) {
        expr();
        if(consume(SEMICOLON)) return 1;
        else token_err(curr_tkn, "Missing ; after RETURN!") ;
    }

    else if(expr()) {
        if(consume(SEMICOLON)) return 1;
        else token_err(curr_tkn,"Missing ; after expression in statement!");
    }
    else if(consume(SEMICOLON)) return 1;

    curr_tkn = start_tkn;
    return 0;

}


int stm_compound() {

    if(consume(LACC)){
        while(1) {
            if(decl_var()) { }
            else if(stm()) { }
            else break;
        }
        if(consume(RACC)) return 1;
        else token_err(curr_tkn, "Expected right accolade in compound statement!");
    }

    return 0;

}



int expr() {

    if(expr_assign()) return 1;

    curr_tkn = curr_tkn->next;
    return 0;

}



int expr_assign() {

    Token *start_tkn = curr_tkn;

    if(expr_unary()) {
        if (consume(ASSIGN)) {
            if (expr_assign()) return 1;
            else token_err(curr_tkn, "Expected assign in expression");
        }
    }
    else if (expr_or()) return 1;

    curr_tkn = start_tkn;
    return 0;

}



/* Removing left recursion:
     exprOr: exprAnd exprOr1
     exprOr1: OR exprAnd exprOr1 | epsilon
*/
int expr_or() {

    Token *start_tkn = curr_tkn;

    if(expr_and()) { expr_or_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

void expr_or_1() {

    if(consume(OR)) {
        if(expr_and()) expr_or_1();
        else token_err(curr_tkn,"Missing \'and\' expression after OR!");
    }

}



/* Removing left recursion:
     exprAnd: exprEq exprAnd1
     exprAnd1: AND exprEq exprAnd1 | epsilon
*/
int expr_and() {

    Token *start_tkn = curr_tkn;

    if(expr_eq()) { expr_and_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

void expr_and_1() {

    if(consume(AND)) {
        if(expr_eq()) expr_and_1();
        else token_err(curr_tkn,"Missing equality expression after AND!");
    }

}




/* Removing left recursion:
     exprEq: exprRel exprEq1
     exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1 | epsilon
*/
int expr_eq() {

    Token *start_tkn = curr_tkn;

    if(expr_rel()) { expr_eq_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

void expr_eq_1() {
    if (consume(EQUAL)) {
        if (expr_rel()) expr_eq_1();
        else token_err(curr_tkn, "Missing relationship expression after EQUAL!");
    }
    else if (consume(NOTEQ)) {
        if (expr_rel()) expr_eq_1();
        else token_err(curr_tkn, "Missing relationship expression after NOTEQ!");
    }

}



/* Removing left recursion:
     exprRel: exprAdd exprRel1
     exprRel1: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1 | epsilon
*/
int expr_rel() {

    Token *start_tkn = curr_tkn;

    if(expr_add()) { expr_rel_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

void expr_rel_1() {

    if(consume(LESS)) {
        if(expr_add())  expr_rel_1();
        else token_err(curr_tkn,"Missing addition expression after LESS!");
    }
    else if(consume(LESSEQ)) {
        if(expr_add())  expr_rel_1();
        else token_err(curr_tkn,"Missing addition expression after LESSEQ!");
    }
    else if(consume(GREATER)) {
        if(expr_add())  expr_rel_1();
        else token_err(curr_tkn,"Missing addition expression after GREATER!");
    }
    else if(consume(GREATEREQ)) {
        if(expr_add())  expr_rel_1();
        else token_err(curr_tkn,"Missing addition expression after GREATEREQ!");
    }

}



/* Removing left recursion:
     exprAdd: exprMul exprAdd1
     exprAdd1: ( ADD | SUB ) exprMul exprAdd1 | epsilon
*/
void expr_add_1() {
    if(consume(ADD)) {
        if(expr_mul())  expr_add_1();
        else token_err(curr_tkn,"Missing multiplier expression after ADD!");
    }
    else if(consume(SUB)) {
        if(expr_mul())  expr_add_1();
        else token_err(curr_tkn,"Missing multiplier expression after SUB!");
    }

}

int expr_add() {

    Token *start_tkn = curr_tkn;

    if(expr_mul()) { expr_add_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}



/* Removing left recursion:
     exprMul: exprCast exprMul1
     exprMul1: ( MUL | DIV ) exprCast exprMul1 | epsilon
*/
int expr_mul() {

    Token *start_tkn = curr_tkn;

    if(expr_cast()) { expr_mul_1(); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

void expr_mul_1() {

    if(consume(MUL)) {
        if(expr_cast()) expr_mul_1();
        else token_err(curr_tkn,"Missing casting expression after MUL!");
    }
    else if(consume(DIV)) {
        if(expr_cast()) expr_mul_1();
        else token_err(curr_tkn,"Missing casting expression after DIV!");
    }

}



int expr_cast(){

    Token *start_tkn = curr_tkn;

    if(consume(LPAR)){
        if(type_name()) {
            if(consume(RPAR)) {
                if(expr_cast()) return 1;
            }
        }
    }
    else if(expr_unary())  return 1;

    curr_tkn = start_tkn;
    return 0;

}



int expr_unary() {

    Token *start_tkn = curr_tkn;

    if(consume(SUB)) {
        if(expr_unary())  return 1;
        else token_err(curr_tkn,"Missing unary expression after SUB!");
    }
    else if(consume(NOT)) {
        if(expr_unary())  return 1;
        else token_err(curr_tkn,"Missing unary expression after NOT!");
    }
    else if(expr_postfix()) return 1;

    curr_tkn = start_tkn;
    return 0;

}



/* Removing left recursion:
      exprPostfix::= exprPrimary exprPostfix1
      exprPostfix1::= ( LBRACKET expr RBRACKET | DOT ID ) exprPostfix1 | epsilon
*/
int expr_postfix() {

    Token *start_tkn = curr_tkn;

    if(expr_primary()) { expr_postfix_1(); return 1;}

    curr_tkn = start_tkn;
    return 0;

}

void expr_postfix_1() {

    if(consume(LBRACKET)) {
        if(expr()){
            if(consume(RBRACKET))  expr_postfix_1();
            else token_err(curr_tkn,"Missing right bracket after expression!");
        }
        else token_err(curr_tkn,"Missing expression after left bracket in the postfix expression!");
    }
    else if(consume(DOT)) {
        if(consume(ID)) { expr_postfix_1(); }
        else token_err(curr_tkn,"Missing ID after dot in the postfix expression!");
    }

}



int expr_primary() {

    Token *start_tkn = curr_tkn;

    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                while(1){
                    if(!consume(COMMA)) break;
                    if(!expr()) token_err(curr_tkn,"Missing expr after COMMA in the primary expression!");
                }
            }
            if(consume(RPAR))  return 1;
            else token_err(curr_tkn,"Right parenthesis missing in the primary expression!");
        }
    }
    else if(consume(CT_INT))     return 1;
    else if(consume(CT_REAL))    return 1;
    else if(consume(CT_CHAR))    return 1;
    else if(consume(CT_STRING))  return 1;
    else if(consume(LPAR)){
        if(expr()) {
            if(consume(RPAR))  return 1;
            else token_err(curr_tkn,"Right parenthesis missing after expression!");
        }
    }

    curr_tkn = start_tkn;
    return 0;

}










































































