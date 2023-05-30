#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library_compiler.h"

Token *curr_tkn;
Token *consumed_tkn;
int curr_depth = 0;
Symbols symbols;
Symbol *curr_struct;
Symbol *curr_funct;



// ~~~~~~~~ SYNTACTICAL ANALYSIS, COMBINED WITH DOMAIN AND TYPE ANALYSIS FUNCTIONS

int consume(int code) {

    if(curr_tkn->code == code){
        consumed_tkn = curr_tkn;
        curr_tkn = curr_tkn->next;
        return 1;
    }

    return 0;

}



int unit() {

    Symbol *s, *a;
    curr_tkn = tokens;

    init_symbols(&symbols);

    // external functions added at the header of symbolic table
    s = add_ext_func("put_s",create_type(TB_VOID,-1));
    add_func_arg(s,"s",create_type(TB_CHAR,0));

    s = add_ext_func("get_s",create_type(TB_VOID,-1));

    s = add_ext_func("put_i",create_type(TB_VOID,-1));
    add_func_arg(s,"i",create_type(TB_INT,-1));

    s = add_ext_func("get_i",create_type(TB_INT,-1));

    s = add_ext_func("put_d",create_type(TB_VOID,-1));
    add_func_arg(s,"d",create_type(TB_DOUBLE,-1));

    s = add_ext_func("get_d",create_type(TB_DOUBLE,-1));

    s = add_ext_func("put_c",create_type(TB_VOID,-1));
    add_func_arg(s,"c",create_type(TB_CHAR,-1));

    s = add_ext_func("get_c",create_type(TB_CHAR,-1));

    s = add_ext_func("seconds",create_type(TB_DOUBLE,-1));



    while(1) {
        if(decl_struct()) { }
        else if(decl_func()) { }
        else if(decl_var()) { }
        else break;
    }
    if(consume(END))   return 1;
    else token_err(curr_tkn, "Missing END token!");

    return 0;

}



int decl_struct() {

    Token *start_tkn = curr_tkn;
    Token *tkn_name;

    if(consume(STRUCT)) {
        if(consume(ID)) {
            tkn_name = consumed_tkn; // store the consumed token (in this case, the ID) into the tkName
            if(consume(LACC)) {
                if(find_symbol(&symbols,tkn_name->type.text))  token_err(curr_tkn,"Symbol redefinition: %s", tkn_name->type.text);
                curr_struct = add_symbol(&symbols,tkn_name->type.text,CLS_STRUCT);
                init_symbols(&curr_struct->members);
                while(1) {
                    if(decl_var()) { }
                    else break;
                }
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) { curr_struct = NULL; return 1; }
                    else token_err(curr_tkn, "Missing \";\" in structure declaration!");
                } else token_err(curr_tkn, "Missing \"}\" in structure declaration!");
            }
        } else token_err(curr_tkn, "Missing structure name!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int decl_var() {

    Token *start_tkn = curr_tkn;
    Symbol *s;
    Token *tkn_name;
    Type t;

    if(type_base(&t)) {
        if(consume(ID)) {
            tkn_name = consumed_tkn;
            if(!array_decl(&t))  t.n_elements = -1;
            add_var(tkn_name, &t);
            while(1) {
                if(consume(COMMA)) {
                    if (consume(ID)) {
                        tkn_name = consumed_tkn;
                        if(!array_decl(&t))  t.n_elements = -1;
                        add_var(tkn_name, &t);
                    } else token_err(curr_tkn, "ID expected!");
                } else break;
            }
            if(consume(SEMICOLON)) return 1;
            else token_err(curr_tkn, "Missing \";\" in variable declaration!");
        } else token_err(curr_tkn, "ID expected after type base!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int type_base(Type *ret) {

    Token *start_tkn = curr_tkn;
    Token *tkn_name;

    if(consume(INT))    { ret->type_base = TB_INT;    return 1; }
    if(consume(DOUBLE)) { ret->type_base = TB_DOUBLE; return 1; }
    if(consume(CHAR))   { ret->type_base  = TB_CHAR;  return 1; }
    if(consume(STRUCT)) {
        if(consume(ID)) {
            tkn_name = consumed_tkn;
            Symbol *s = find_symbol(&symbols,tkn_name->type.text);
            if(s == NULL)  token_err(curr_tkn,"Undefined symbol: %s", tkn_name->type.text);
            if(s->cls != CLS_STRUCT)   token_err(curr_tkn,"%s is not a \"struct\"", tkn_name->type.text);
            ret->type_base = TB_STRUCT;
            ret->s = s;
            return 1;
        }
        else token_err(curr_tkn, "ID struct name expected after type \"struct\"!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int array_decl(Type *ret) {

    Token *start_tkn = curr_tkn;
    RetVal rv;

    if(consume(LBRACKET)) {
        if(expr(&rv)) {
            if(!rv.is_ctval)  token_err(curr_tkn, "The array size is not a constant!");
            if(rv.type.type_base != TB_INT)   token_err(curr_tkn,"The array size is not an integer!");
            ret->n_elements = rv.ctval.i;
        }
        else ret->n_elements = 0;
        if(consume(RBRACKET))  return 1;
        else token_err(curr_tkn, "Missing \"]\" from array declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int type_name(Type *ret) {

    if(!type_base(ret))   return 0;
    if(!array_decl(ret))  ret->n_elements = -1;
    return 1;

}



int decl_func() {

    Token *start_tkn = curr_tkn;
    Type t;
    Token *tkn_name;

    if(type_base(&t)) {
        if(consume(MUL))   t.n_elements = 0;
        else t.n_elements = -1;
        if(consume(ID)) {
            tkn_name = consumed_tkn;
            if(consume(LPAR)) {
                if(find_symbol(&symbols,tkn_name->type.text)) token_err(curr_tkn,"Symbol redefinition: %s", tkn_name->type.text);
                curr_funct = add_symbol(&symbols,tkn_name->type.text,CLS_FUNC);
                init_symbols(&curr_funct->args);
                curr_funct->type = t;
                curr_depth++;
                if(funct_arg()) {
                    while(1) {
                        if(consume(COMMA)) {
                            if(!funct_arg()) token_err(curr_tkn, "Missing function argument in function declaration!");
                        }
                        else break;
                    }
                }
                if(consume(RPAR)) {
                    curr_depth--;
                    if(stm_compound()) {
                        delete_symbols_after(&symbols, curr_funct);
                        curr_funct = NULL;
                        return 1;
                    } else token_err(curr_tkn, "Missing statement compound in function declaration!");
                } else token_err(curr_tkn, "Missing \")\" in function declaration!");
            }
        } else token_err(curr_tkn, "Missing function name in function declaration!");
    }

    if (consume(VOID)) {
        t.type_base = TB_VOID;
        if(consume(ID)) {
            tkn_name = consumed_tkn;
            if(consume(LPAR)) {
                if(find_symbol(&symbols,tkn_name->type.text)) token_err(curr_tkn,"Symbol redefinition: %s", tkn_name->type.text);
                curr_funct = add_symbol(&symbols,tkn_name->type.text,CLS_FUNC);
                init_symbols(&curr_funct->args);
                curr_funct->type = t;
                curr_depth++;
                if(funct_arg()) {
                    while(1) {
                        if(consume(COMMA)) {
                            if(!funct_arg()) token_err(curr_tkn, "Missing function argument in function declaration!");
                        }
                        else break;
                    }
                }
                if(consume(RPAR)) {
                    curr_depth--;
                    if(stm_compound()) {
                        delete_symbols_after(&symbols, curr_funct);
                        curr_funct = NULL;
                        return 1;
                    } else token_err(curr_tkn, "Missing statement compound in function declaration!");
                } else token_err(curr_tkn, "Missing \")\" in function declaration!");
            }
        } else token_err(curr_tkn, "Missing function name in function declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int funct_arg() {

    Token *start_tkn = curr_tkn;
    Type t;
    Token *tkn_name;

    if(type_base(&t)) {
        if(consume(ID)) {
            tkn_name = consumed_tkn;
            if(!array_decl(&t))   t.n_elements = -1;
            Symbol *s = add_symbol(&symbols,tkn_name->type.text,CLS_VAR);
            s->mem = MEM_ARG;
            s->type = t;
            s = add_symbol(&curr_funct->args,tkn_name->type.text,CLS_VAR);
            s->mem = MEM_ARG;
            s->type = t;
            return 1;
        } else token_err(curr_tkn, "ID missing in function declaration!");
    }

    curr_tkn = start_tkn;
    return 0;

}



int stm() {

    Token *start_tkn = curr_tkn;
    RetVal rv, rv1, rv2, rv3;

    if(stm_compound())  return 1;

    else if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr(&rv)) {
                if(rv.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested!");
                if(consume(RPAR)) {
                    if(stm()) {
                        if(consume(ELSE)) {
                            if(stm()) { }
                            else token_err(curr_tkn, "Expected statement after ELSE!") ;
                        }
                        return 1;
                    } else token_err(curr_tkn, "Expected statement after IF!") ;
                } else token_err(curr_tkn, "Missing \")\" after IF!") ;
            } else token_err(curr_tkn, "Expected expression after \"(\"!");
        } else token_err(curr_tkn, "Missing \"(\" after IF!") ;
    }

    else if(consume(WHILE)) {
        if(consume(LPAR)) {
            if(expr(&rv)) {
                if(rv.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested!");
                if(consume(RPAR)) {
                    if(stm())  return 1;
                    else token_err(curr_tkn, "Expected statement after WHILE!") ;
                } else token_err(curr_tkn, "Missing \")\" after WHILE!") ;
            } else token_err(curr_tkn, "Expected expression after \"(\"!") ;
        } else token_err(curr_tkn, "Missing \"(\" after WHILE!") ;
    }

    else if(consume(FOR)) {
        if(consume(LPAR)) {
            expr(&rv1);
            if(consume(SEMICOLON)) {
                if(expr(&rv2)){
                    if(rv2.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested!");
                }
                if(consume(SEMICOLON)) {
                    expr(&rv3);
                    if(consume(RPAR)) {
                        if(stm()) return 1;
                        else token_err(curr_tkn, "Expected statement after FOR!") ;
                    } else token_err(curr_tkn, "Missing \")\" after FOR!") ;
                } else token_err(curr_tkn, "Missing \";\" in FOR!") ;
            } else token_err(curr_tkn, "Missing \";\" in FOR!") ;
        } else token_err(curr_tkn, "Missing \"(\" after FOR!") ;
    }

    else if(consume(BREAK)) {
        if(consume(SEMICOLON)) return 1;
        else token_err(curr_tkn, "Missing \";\" after BREAK!") ;
    }

    else if(consume(RETURN)) {
        if(expr(&rv)){
            if(curr_funct->type.type_base == TB_VOID) token_err(curr_tkn,"A void function cannot return a value!");
            cast(&curr_funct->type,&rv.type);
        }
        if(consume(SEMICOLON)) return 1;
        else token_err(curr_tkn, "Missing \";\" after RETURN!") ;
    }

    else {
        start_tkn = curr_tkn;
        if(expr(&rv)){
            if(consume(SEMICOLON)) return 1;
            else token_err(curr_tkn,"Missing \";\" after expression in statement!");
        }
    }

    curr_tkn = start_tkn;
    return 0;

}



int stm_compound() {

    Symbol *start = symbols.end[-1];

    if(consume(LACC)) {
        curr_depth++;
        while(1) {
            if(decl_var()) { }
            else if(stm()) { }
            else break;
        }
        if(consume(RACC)) {
            curr_depth--;
            delete_symbols_after(&symbols, start);
            return 1;
        } else token_err(curr_tkn, "Expected \"}\" in compound statement!");
    }

    return 0;

}



int expr(RetVal *rv) {

    if(expr_assign(rv)) return 1;
    return 0;

}



int expr_assign(RetVal *rv) {

    Token *start_tkn = curr_tkn;
    RetVal rve;

    if(expr_unary(rv)) {
        if (consume(ASSIGN)) {
            if (expr_assign(&rve)) {
                if(!rv->is_lval)  token_err(curr_tkn, "Cannot assign to a non-lval!");
                if(rv->type.n_elements > -1 || rve.type.n_elements > -1)  token_err(curr_tkn,"The arrays cannot be assigned!");
                cast(&rv->type,&rve.type);
                rv->is_ctval = rv->is_lval = 0;
                return 1;
            }
        }
        else{
            curr_tkn = start_tkn;

            if(expr_or(rv)) return 1;
            else token_err(curr_tkn, "Expected assign in expression");
        }
    }
    if (expr_or(rv)) return 1;

    curr_tkn = start_tkn;
    return 0;

}



/* Removing left recursion:
     exprOr: exprAnd exprOr1
     exprOr1: OR exprAnd exprOr1 | epsilon
*/
int expr_or(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_and(rv)) {
        if(expr_or_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_or_1(RetVal *rv) {

    RetVal rve;

    if(consume(OR)) {
        if(expr_and(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested!");
            rv->type = create_type(TB_INT, -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_or_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing AND after OR!");
    }

    return 1;

}



/* Removing left recursion:
     exprAnd: exprEq exprAnd1
     exprAnd1: AND exprEq exprAnd1 | epsilon
*/
int expr_and(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_eq(rv)) {
        if(expr_and_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_and_1(RetVal *rv) {

    RetVal rve;

    if(consume(AND)) {
        if(expr_eq(&rve)) {
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT) token_err(curr_tkn,"A structure cannot be logically tested");
            rv->type = create_type(TB_INT, -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_and_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing equality expression after AND!");
    }

    return 1;

}



/* Removing left recursion:
     exprEq: exprRel exprEq1
     exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1 | epsilon
*/
int expr_eq(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_rel(rv)) {
        if(expr_eq_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_eq_1(RetVal *rv) {

    RetVal rve;
    Token *tkn_op;

    if (consume(EQUAL)) {
        tkn_op = consumed_tkn;
        if (expr_rel(&rve)) {
            if(rv->type.type_base == TB_STRUCT||rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested");
            rv->type = create_type(TB_INT, -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_eq_1(rv)) return 1;
        } else token_err(curr_tkn, "Missing relationship expression after EQUAL!");
    }
    if (consume(NOTEQ)) {
        tkn_op = consumed_tkn;
        if (expr_rel(&rve)) {
            if(rv->type.type_base == TB_STRUCT||rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be logically tested");
            rv->type = create_type(TB_INT, -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_eq_1(rv)) return 1;
        }
        else token_err(curr_tkn, "Missing relationship expression after NOTEQ!");
    }

    return 1;

}



/* Removing left recursion:
     exprRel: exprAdd exprRel1
     exprRel1: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1 | epsilon
*/
int expr_rel(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_add(rv)) {
        if(expr_rel_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_rel_1(RetVal *rv) {

    RetVal rve;
    Token *tkn_op;

    if(consume(LESS)) {
        tkn_op = consumed_tkn;
        if(expr_add(&rve)) {
            if(rv->type.n_elements >- 1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be compared!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be compared!");
            rv->type = create_type(TB_INT,  -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_rel_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing addition expression after LESS!");
    }
    if(consume(LESSEQ)) {
        tkn_op = consumed_tkn;
        if(expr_add(&rve)) {
            if(rv->type.n_elements >- 1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be compared!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be compared!");
            rv->type = create_type(TB_INT,  -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_rel_1(rv))  return 1;
        } else token_err(curr_tkn,"Missing addition expression after LESSEQ!");
    }
    if(consume(GREATER)) {
        tkn_op = consumed_tkn;
        if(expr_add(&rve)) {
            if(rv->type.n_elements >- 1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be compared!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be compared!");
            rv->type = create_type(TB_INT,  -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_rel_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing addition expression after GREATER!");
    }
    if(consume(GREATEREQ)) {
        tkn_op = consumed_tkn;
        if(expr_add(&rve)) {
            if(rv->type.n_elements >- 1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be compared!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be compared!");
            rv->type = create_type(TB_INT,  -1);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_rel_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing addition expression after GREATEREQ!");
    }

    return 1;

}



/* Removing left recursion:
     exprAdd: exprMul exprAdd1
     exprAdd1: ( ADD | SUB ) exprMul exprAdd1 | epsilon
*/
int expr_add(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_mul(rv)) {
        if(expr_add_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_add_1(RetVal *rv) {

    RetVal rve;
    Token *tkn_op;

    if(consume(ADD)) {
        tkn_op = consumed_tkn;
        if(expr_mul(&rve)) {
            if(rv->type.n_elements > -1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be added or subtracted!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be added or subtracted!");
            rv->type = get_arith_type(&rv->type,&rve.type);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_add_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing multiplier expression after ADD!");
    }
    if(consume(SUB)) {
        tkn_op = consumed_tkn;
        if(expr_mul(&rve)) {
            if(rv->type.n_elements > -1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be added or subtracted!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be added or subtracted!");
            rv->type = get_arith_type(&rv->type,&rve.type);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_add_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing multiplier expression after SUB!");
    }

    return 1;

}



/* Removing left recursion:
     exprMul: exprCast exprMul1
     exprMul1: ( MUL | DIV ) exprCast exprMul1 | epsilon
*/
int expr_mul(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_cast(rv)) { expr_mul_1(rv); return 1; }

    curr_tkn = start_tkn;
    return 0;

}

int expr_mul_1(RetVal *rv) {

    RetVal rve;
    Token *tkn_op;

    if(consume(MUL)) {
        tkn_op = consumed_tkn;
        if(expr_cast(&rve)) {
            if(rv->type.n_elements > -1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be multiplied or divided!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be multiplied or divided!");
            rv->type = get_arith_type(&rv->type,&rve.type);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_mul_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing casting expression after MUL!");
    }
    if(consume(DIV)) {
        tkn_op = consumed_tkn;
        if(expr_cast(&rve)) {
            if(rv->type.n_elements > -1 || rve.type.n_elements > -1)  token_err(curr_tkn,"An array cannot be multiplied or divided!");
            if(rv->type.type_base == TB_STRUCT || rve.type.type_base == TB_STRUCT)  token_err(curr_tkn,"A structure cannot be multiplied or divided!");
            rv->type = get_arith_type(&rv->type,&rve.type);
            rv->is_ctval = rv->is_lval = 0;

            if(expr_mul_1(rv)) return 1;
        } else token_err(curr_tkn,"Missing casting expression after DIV!");
    }

    return 1;

}



int expr_cast(RetVal *rv){

    Token *start_tkn = curr_tkn;
    RetVal rve;
    Type t;

    if(consume(LPAR)) {
        if(type_name(&t)) {
            if(consume(RPAR)) {
                if(expr_cast(&rve)) {
                    cast(&t,&rve.type);
                    rv->type = t;
                    rv->is_ctval = rv->is_lval=0;
                    return 1;
                }
            } else token_err(curr_tkn,"Missing \")\" in casting expression!");
        } token_err(curr_tkn,"Missing type name in casting expression!");
    }
    if(expr_unary(rv))  return 1;

    curr_tkn = start_tkn;
    return 0;

}



int expr_unary(RetVal *rv) {

    Token *start_tkn = curr_tkn;
    Token *tkn_op;

    if(consume(SUB)) {
        tkn_op = consumed_tkn;
        if(expr_unary(rv)) {
            if(tkn_op->code == SUB) {
                if(rv->type.n_elements >= 0)  token_err(curr_tkn,"Unary '-' cannot be applied to an array!");
                if(rv->type.type_base == TB_STRUCT)  token_err(curr_tkn,"Unary '-' cannot be applied to a struct!");

            }
            rv->is_ctval = rv->is_lval = 0;
            return 1;
        } else token_err(curr_tkn,"Missing unary expression after SUB!");
    }
    if(consume(NOT)) {
        tkn_op = consumed_tkn;
        if(expr_unary(rv)) {
            if (tkn_op->code == NOT) {
                if (rv->type.type_base == TB_STRUCT) token_err(curr_tkn, "\"!\" cannot be applied to a struct!");
                rv->type = create_type(TB_INT, -1);
            }
            rv->is_ctval = rv->is_lval = 0;
            return 1;
        } else token_err(curr_tkn,"Missing unary expression after NOT!");
    }
    if(expr_postfix(rv)) return 1;

    curr_tkn = start_tkn;
    return 0;

}



/* Removing left recursion:
      exprPostfix::= exprPrimary exprPostfix1
      exprPostfix1::= ( LBRACKET expr RBRACKET | DOT ID ) exprPostfix1 | epsilon
*/
int expr_postfix(RetVal *rv) {

    Token *start_tkn = curr_tkn;

    if(expr_primary(rv)) {
        if(expr_postfix_1(rv)) return 1;
    }

    curr_tkn = start_tkn;
    return 0;

}

int expr_postfix_1(RetVal *rv) {

    RetVal rve;
    Token *tkn_name;

    if(consume(LBRACKET)) {
        if(expr(&rve)) {
            if(rv->type.n_elements < 0)  token_err(curr_tkn,"Only an array can be indexed!");
            Type type_int = create_type(TB_INT,-1);
            cast(&type_int,&rve.type);
            rv->type = rv->type;
            rv->type.n_elements = -1;
            rv->is_lval = 1;
            rv->is_ctval = 0;
            if(consume(RBRACKET)) {
                if(expr_postfix_1(rv)) return 1;
            } else token_err(curr_tkn,"Missing right bracket after expression!");
        } else token_err(curr_tkn,"Missing expression in the postfix expression!");
    }
    if(consume(DOT)) {
        if (consume(ID)) {
            tkn_name = consumed_tkn;
            Symbol *s_struct = rv->type.s;
            Symbol *s_member = find_symbol(&s_struct->members, tkn_name->type.text);
            if(!s_member)  token_err(curr_tkn, "Struct %s does not have a member %s", s_struct->name, tkn_name->type.text);
            rv->type = s_member->type;
            rv->is_lval = 1;
            rv->is_ctval = 0;

            if (expr_postfix_1(rv)) return 1;
        } else token_err(curr_tkn, "Missing ID after dot in the postfix expression!");
    }

    return 1;

}



int expr_primary(RetVal *rv) {

    Token *start_tkn = curr_tkn;
    Token *tkn_name = curr_tkn;
    Token *tkn_i, *tkn_r, *tkn_c, *tkn_s;
    RetVal arg;

    if(consume(ID)) {
        Symbol *s = find_symbol(&symbols, tkn_name->type.text);
        if(!s)  token_err(curr_tkn,"Undefined symbol %s", tkn_name->type.text);
        rv->type = s->type;
        rv->is_ctval = 0;
        rv->is_lval = 1;
        if(consume(LPAR)) {
            Symbol **crt_def_arg = s->args.begin;
            if(s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)  token_err(curr_tkn,"Call of the non-function %s", tkn_name->type.text);
            if(expr(&arg)) {
                if(crt_def_arg == s->args.end)  token_err(curr_tkn, "Too many arguments in call!");
                cast(&(*crt_def_arg)->type, &arg.type);
                crt_def_arg++;
                while(1) {
                    if(consume(COMMA)) {
                        if(expr(&arg)) {
                            if(crt_def_arg == s->args.end)  token_err(curr_tkn, "Too many arguments in call!");
                            cast(&(*crt_def_arg)->type, &arg.type);
                            crt_def_arg++;
                        } else token_err(curr_tkn,"Expected expr after COMMA in the primary expression!");
                    } else break;
                }
            }
            if(consume(RPAR)) {
                if(crt_def_arg != s->args.end)  token_err(curr_tkn, "Too few arguments in call!");
                rv->type = s->type;
                rv->is_ctval = rv->is_lval = 0;
                return 1;
            }
            else token_err(curr_tkn,"Missing \")\" in the primary expression!");
        }
        else if(s->cls == CLS_FUNC || s->cls == CLS_EXTFUNC)  token_err(curr_tkn,"Missing call for function %s",tkn_name->type.text);
        return 1;
    }
    if(consume(CT_INT)) {
        tkn_i = consumed_tkn;
        rv->type = create_type(TB_INT, -1);
        rv->ctval.i = tkn_i->type.i;
        rv->is_ctval = 1;
        rv->is_lval = 0;
        return 1;
    }
    if(consume(CT_REAL)) {
        tkn_r = consumed_tkn;
        rv->type = create_type(TB_DOUBLE,-1);
        rv->ctval.d = tkn_r->type.r;
        rv->is_ctval = 1;
        rv->is_lval = 0;
        return 1;
    }
    if(consume(CT_CHAR)) {
        tkn_c = consumed_tkn;
        rv->type = create_type(TB_CHAR,-1);
        rv->ctval.i = tkn_c->type.i;
        rv->is_ctval = 1;
        rv->is_lval = 0;
        return 1;
    }
    if(consume(CT_STRING)) {
        tkn_s = consumed_tkn;
        rv->type = create_type(TB_CHAR,0);
        rv->ctval.str = tkn_s->type.text;
        rv->is_ctval = 1;
        rv->is_lval = 0;
        return 1;
    }
    if(consume(LPAR)) {
        if(expr(rv)) {
            if(consume(RPAR))  return 1;
            else token_err(curr_tkn,"Missing \")\" in the primary expression!");
        }
    }

    curr_tkn = start_tkn;
    return 0;

}





// ~~~~~~~~ DOMAIN ANALYSIS FUNCTIONS

void init_symbols(Symbols *symbols) {

    symbols->begin = NULL;
    symbols->end = NULL;
    symbols->after = NULL;

}



Symbol *add_symbol(Symbols *symbols, const char *name, int cls) {

    Symbol *s;

    if(symbols->end == symbols->after) { // create more room

        int count = symbols->after - symbols->begin;
        int n = count * 2; // double the room
        if(n == 0)  n = 1; // needed for the initial case
        symbols->begin = (Symbol**)realloc(symbols->begin, n * sizeof(Symbol*));
        if(symbols->begin == NULL)  err("not enough memory");

        symbols->end = symbols->begin + count;
        symbols->after = symbols->begin + n;

    }

    SAFEALLOC(s,Symbol)
    *symbols->end++ = s;
    s->name = name;
    s->cls = cls;
    s->depth = curr_depth;
    return s;

}



Symbol *find_symbol(Symbols *symbols, const char *name) {

    int size = symbols->end - symbols->begin - 1;

    while(size >= 0) {
        if(strcmp(symbols->begin[size]->name, name) == 0)   return symbols->begin[size];
        size--;
    }
    return NULL;

}



void add_var(Token *tkName,Type *t) {

    Symbol *s;

    if(curr_struct) {
        if(find_symbol(&curr_struct->members,tkName->type.text)) token_err(curr_tkn,"Symbol redefinition: %s",tkName->type.text);
        s = add_symbol(&curr_struct->members,tkName->type.text,CLS_VAR);
    }
    else if(curr_funct) {
        s = find_symbol(&symbols,tkName->type.text);
        if(s && s->depth == curr_depth) token_err(curr_tkn,"Symbol redefinition: %s",tkName->type.text);
        s = add_symbol(&symbols,tkName->type.text,CLS_VAR);
        s->mem = MEM_LOCAL;
    }
    else {
        if(find_symbol(&symbols,tkName->type.text)) token_err(curr_tkn,"Symbol redefinition: %s",tkName->type.text);
        s = add_symbol(&symbols,tkName->type.text,CLS_VAR);
        s->mem = MEM_GLOBAL;
    }

    s->type = *t;

}



void delete_symbols_after(Symbols *symbols, Symbol *symbol) {

    int size = symbols->end - symbols->begin;
    int found;


    for(int cnt = 0; cnt < size ; cnt++) {
        if(symbols->begin[cnt] == symbol) {
            found = cnt;
            cnt++;
            while(cnt < size)   {
                free(symbols->begin[cnt]);
                cnt++;
            }
            symbols->end = symbols->begin + found + 1;
        }
    }

}





// ~~~~~~~~ TYPE ANALYSIS FUNCTIONS

Type create_type(int type_base, int n_elements) {

    Type t;
    t.type_base = type_base;
    t.n_elements = n_elements;
    return t;

}



void cast(Type *dst, Type *src) {

    if(src->n_elements > -1) {
        if(dst->n_elements > -1) {
            if(src->type_base != dst->type_base) token_err(curr_tkn,"An array cannot be converted to an array of another type!");
        } else token_err(curr_tkn,"An array cannot be converted to a non-array!");
    } else if(dst->n_elements > -1)  token_err(curr_tkn,"A non-array cannot be converted to an array!");

    switch(src->type_base) {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            switch(dst->type_base) {
                case TB_CHAR:
                case TB_INT:
                case TB_DOUBLE: return;
            }
        case TB_STRUCT:
            if(dst->type_base == TB_STRUCT) {
                if(src->s != dst->s)  token_err(curr_tkn,"A structure cannot be converted to another one!");
                return;
            }
    }
    token_err(curr_tkn,"Incompatible types!");

}



Type get_arith_type(Type *s1, Type *s2) {

    switch (s1->type_base) {
        case TB_STRUCT:
        case TB_CHAR: return *s2;
        case TB_INT: if (s2->type_base == TB_DOUBLE)   return create_type(TB_DOUBLE, -1);
                     else return create_type(TB_INT, -1); // for INTs and CHARs
        case TB_DOUBLE: return create_type(TB_DOUBLE, -1);
    }

}



Symbol *add_ext_func(const char *name, Type type) {

    Symbol *s = add_symbol(&symbols,name,CLS_EXTFUNC);
    s->type = type;
    init_symbols(&s->args);
    return s;

}



Symbol *add_func_arg(Symbol *func, const char *name, Type type) {

    Symbol *a = add_symbol(&func->args,name,CLS_VAR);
    a->type = type;
    return a;

}










































































