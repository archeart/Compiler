#include <stdlib.h>
#include <stdio.h>
#include "ir.h"
#include "compiler.h"

char ops[] = {'+', '-', '*', '/'};
char* relops[] = {"==", ">", "<", ">=", "<=", "!="};
struct irb head, *tail = &head, param;
//unsigned int tvar_cnt = 1, uvar_cnt = 1, label_cnt = 1;
unsigned int uvar_cnt = 1, label_cnt = 1;


inline bool var_cmp(int var1, char vtype1, int var2, char vtype2)
{
    if (var1 == var2 && vtype1 == vtype2)
        return 1;
    return 0;
}

void ir_init()
{
    head.ir_t = ir_program;
    head.next = NULL;
    head.pre = NULL;
    param.next = NULL;
    param.pre = NULL;
}

inline int reloptoi(char* op)
{
    if (strcmp(op, "==") == 0)
        return REL_EQ;
    else if (strcmp(op, ">") == 0)
        return REL_GT;
    else if (strcmp(op, ">=") == 0)
        return REL_GE;
    else if (strcmp(op, "<=") == 0)
        return REL_LE;
    else if (strcmp(op, "<") == 0)
        return REL_LT;
    else if (strcmp(op, "!=") == 0)
        return REL_NE;

//    printf("[RELOP ERROR]: %s\n", op);
    return REL_EQ;
}

bool iropt_assign1(char type, int var1, char vt1, int var2, char vt2)
{
    /* this optimization is for like this temp = xx op xx; xx = temp */

    if (IR_VAR(vt1))
        return 0;

    if (type != ir_assign)
        return 0;

    if (tail->ir_t <= ir_call) {
        if (var2 == tail->var1 && tail->vt1 == IR_TempVar && vt2 == IR_TempVar) {
            if ((vt1 & IR_mask) == IR_TempVar) {
                tail->var1 = var1;
                tail->vt1 = vt1;
            }
            else if ((vt1 & IR_mask) == IR_DefVar) {
                tvar_cnt --;
                tail->var1 = var1;
                tail->vt1 = vt1;
            }
            return 1;
        }
    }
    return 0;
}

inline bool iropt_return(enum irtype type)
{
    /* the code after return sentence can only be label and func */
    if (tail->ir_t != ir_return)
        return false;
    if (type == ir_label || type == ir_func)
        return false;
    return true;
}

void iropt_func()
{
    while (tail->ir_t == ir_label) {
        tail = tail->pre;
        free(tail->next);
        tail->next = null;
    }
}

bool ir_append(enum irtype type, int var1, char vt1,
        int var2, char vt2, int var3, char vt3)
{
    struct irb *tmp;

    if (iropt_assign1(type, var1, vt1, var2, vt2))
        return true;

    if (iropt_return(type))
        return true;

    tmp = (struct irb*) malloc (sizeof(struct irb));
    tmp->ir_t = type;
    tmp->vt1 = vt1;
    tmp->vt2 = vt2;
    tmp->vt3 = vt3;
    tmp->var1 = var1;
    tmp->var2 = var2;
    tmp->var3 = var3;
    tmp->next = NULL;
    tmp->bid = 0;

    if (type == ir_param && tail->ir_t == ir_dec) {
        if (tail->var1 == var1) {
            tail = tail->pre;
            tail->next = NULL;
        }
    }
/*
    if (type == ir_func)
        iropt_func();
*/

    tail->next = tmp;
    tmp->pre = tail;
    tail = tmp;

    return false;
}


inline void var_gen(int var, int vt, char *res)
{
    int type = vt & IR_mask;

    if (IR_ADDR(vt)) {
        assert (!(IR_VAR(vt)));
        if (IR_VAR(vt)) {
//            printf("!!!VAR %d\n");
            sprintf(res, "&*Var%d", var);
        }else
            sprintf(res, "&Var%d", var);
    }
    else if (IR_VAR(vt)) {
        if (type == IR_TempVar)
            sprintf(res, "*Temp%d", var);
        else if (type == IR_DefVar)
            sprintf(res, "*Var%d", var);
    }
    else {
        assert(type != IR_Func);
        switch (type) {
            case IR_TempVar:
                sprintf(res, "Temp%d", var);
                break;
            case IR_DefVar:
                sprintf(res, "Var%d", var);
                break;
            case IR_Const:
                sprintf(res, "#%d", var);
                break;
            case IR_Func:
                sprintf(res, "CALL %s", (char*)var);
                break;
            case IR_StrPtr:
                sprintf(res, "%s", (char*)var);
                break;

        }
    }
        
}

#define active_right(id, type) \
    if (type != IR_Const) left[id] = false
#define active_left(id, type) \
    if (type != IR_Const) left[id] = true

#define active_ts(id) \
    if (!left[id]) { \
        it->pre->next = it->next; \
        it->next->pre = it->pre;  \
        p = it; \
        it = it->next; \
        free(p);    \
    }

void iropt_deadcode()
{
    struct irb *it, *p, *q;
    bool *left = (bool*)malloc(VAR_MAX);

    while (tail->ir_t <= ir_assign) {
        p = tail;
        tail = tail->pre;
        tail->next = NULL;
        free(p);
    }

    it = tail;
        
    memset(left, 0, sizeof(left));
    while (it->pre != NULL) {
//        printf("type: %d\n", it->ir_t);
        switch (it->ir_t) {

            case ir_assign:
                active_ts(it->var1)
                left[it->var1] = false;
                assert(it->vt2 != IR_Func);
                active_left(it->var2, it->vt2);
                break;

            case ir_plus: case ir_minus: case ir_star: case ir_div:
                active_ts(it->var1)
//                printf("out of ts\n");
                active_left(it->var2, it->vt2);
                active_left(it->var3, it->vt3);
                break;
                
            case ir_call:
                left[it->var1] = false;
                break;

            case ir_read: 
                left[it->var1] = false;
                break;

            case ir_write: case ir_return: case ir_arg:
                active_left(it->var1, it->vt1);
                break;

            case ir_if:
                active_left(it->var1, it->vt1);
                active_left(it->var2, it->vt2);
                break;

            default:
                break;
        }

        it = it->pre;
        
    }
}

void iropt_unreachable()
{
    struct irb *it = head.next, *p, *q;
    while (it != NULL) {
        if (it->ir_t == ir_return || it->ir_t == ir_goto) {
            p = it->next;
            while (p != NULL && p->ir_t != ir_func && p->ir_t != ir_label) {
                p->pre->next = p->next;
                p->next->pre = p->pre;
                q = p;
                p = q->next;
                free(q);
            }
        }
        else if (it->ir_t == ir_if)
            it = it->next;
        it = it->next;
    }
}

void ir_print()
{
    char n1[20], n2[20], n3[20];
    struct irb *iter = head.next;


    iropt_unreachable();
//    iropt_deadcode();

    while (iter != NULL) {
       switch (iter->ir_t) {
           case ir_label:
               fprintf(stderr, "LABEL label%d :\n", iter->var1);
               break;

           case ir_assign:
               var_gen(iter->var1, iter->vt1, n1);
               var_gen(iter->var2, iter->vt2, n2);
               fprintf(stderr, "%s := %s\n", n1, n2);
               break;
            
            case ir_plus: case ir_minus: case ir_star: case ir_div:
               var_gen(iter->var1, iter->vt1, n1);
               var_gen(iter->var2, iter->vt2, n2);
               var_gen(iter->var3, iter->vt3, n3);
               fprintf(stderr, "%s := %s %c %s\n", n1, n2, ops[iter->ir_t], n3);
               break;

            case ir_goto:
               fprintf(stderr, "GOTO label%d\n", iter->var1);
               break;

            case ir_if:
               var_gen(iter->var1, iter->vt1, n1);
               var_gen(iter->var2, iter->vt2, n2);
               fprintf(stderr, "IF %s %s %s ", n1, relops[iter->var3], n2);
               break;

            case ir_dec:
               fprintf(stderr, "DEC Var%d %d\n", iter->var1, iter->var2);
               break;

            case ir_param:
               fprintf(stderr, "PARAM Var%d\n", iter->var1);
               break;


            case ir_return:
               var_gen(iter->var1, iter->vt1, n1);
               fprintf(stderr, "RETURN %s\n\n", n1);
               break;

            case ir_arg:
               var_gen(iter->var1, iter->vt1, n1);
               fprintf(stderr, "ARG %s\n", n1);
               break;

            case ir_func:
               fprintf(stderr, "FUNCTION %s :\n", (char*)iter->var1);
               break;

            case ir_call:
               var_gen(iter->var1, iter->vt1, n1);
               fprintf(stderr, "%s := CALL %s\n", n1, (char*)iter->var2);
               break;

            case ir_read:
               var_gen(iter->var1, iter->vt1, n1);
               fprintf(stderr, "READ %s\n", n1);
               break;

            case ir_write:
               var_gen(iter->var1, iter->vt1, n1);
               fprintf(stderr, "WRITE %s\n", n1);
               break;

            default:
               fprintf(stderr, "Undesigned %d\n", iter->ir_t);
               
       }

       iter = iter->next;
    }

    fclose(stderr);

    asm_main();
}

char iropt_calc1(int optype, int var1, char vtype1, int var2, char vtype2, int *ret)
{
    /* for 2 consts */
    if (vtype1 != IR_Const  || vtype2 != IR_Const)
        return 0;

    switch (optype) {
        case ir_plus:
            *ret = var1 + var2;
            break;
        case ir_minus:
            *ret = var1 - var2;
            break;
        case ir_star:
            *ret = var1 * var2;
            break;
        case ir_div:
            *ret = var1 / var2;
            break;
        default:
            return 0;
    }

    return 1;
}

char iropt_calc2(int optype, int var1, char vtype1, int var2, char vtype2)
{
    /* trivial calc */
    if (vtype1 != IR_Const && vtype2 != IR_Const)
        return 0;

    switch (optype) {
        case ir_plus:
            if (var1 == 0)
                return 1;
            if (var2 == 0)
                return 2;
            break;

        case ir_minus:
            if (var2 == 0)
                return 2;
            break;

        case ir_star:
            if (var1 == 1 && vtype1 == IR_Const)
                return 1;
            if (var2 == 1 && vtype2 == IR_Const)
                return 2;
            break;

        case ir_div:
            if (var2 == 1 && vtype2 == IR_Const)
                return 2;
            break;
    }

    return 0;
}

char ir_rexp_append(char type, int var1, char vtype1, int var2, char vtype2, int *lvar, char *lvtype)
{
    int i;

    if (iropt_calc1(type, var1, vtype1, var2, vtype2, lvar)) {
        *lvtype = IR_Const;
//        var_gen(*lvar, *lvtype, deb);
        return 1;
    }

    i = iropt_calc2(type, var1, vtype1, var2, vtype2);
    if (i == 1) {
        *lvar = var2;
        *lvtype = vtype2;
//       var_gen(*lvar, *lvtype, deb);
        return 1;
    }
    else if (i == 2) {
        *lvar = var1;
        *lvtype = vtype1;
//        var_gen(*lvar, *lvtype, deb);
        return 1;
    }

    *lvar = tvar_cnt ++;
    *lvtype = IR_TempVar;
    ir_append(type, *lvar, *lvtype, var1, vtype1, var2, vtype2);
//    var_gen(*lvar, *lvtype, deb);
    return 0;
}


int iropt_relop1(int reltype, int var1, char vtype1, int var2, char vtype2)
{
    /* this is for if (const relop const) */
    if (vtype1 != IR_Const || vtype2 != IR_Const)
        return 0;

    switch (reltype) {
        case REL_EQ:
            if (var1 == var2)
                return 1;
            else
                return -1;
            break;

        case REL_GT:
            if (var1 > var2)
                return 1;
            else
                return -1;
            break;

        case REL_LT:
            if (var1 < var2)
                return 1;
            else
                return -1;
            break;
    
        case REL_GE:
            if (var1 >= var2)
                return 1;
            else
                return -1;
            break;

        case REL_LE:
            if (var1 <= var2)
                return 1;
            else
                return -1;
            break;

        case REL_NE:
            if (var1 != var2)
                return 1;
            else
                return -1;
            break;
            
        default :
            return 0;
    }
}

void trans_relop_bool(char* op, int var1, char vtype1, int var2, char vtype2, struct eval *ret)
{
    int i, j;
    i = reloptoi(op);
    j = iropt_relop1(i, var1, vtype1, var2, vtype2);
    if (j != 0) {
        if (j == 1 && ret->nexto == false) {
            ir_appendgoto(ret->tlb);
            ret->goto1 = true;
        }
        else if (j == -1 && ret->nexto == true) {
            ir_appendgoto(ret->flb);
            ret->goto0 = true;
        }
        return ;
    }

    if (ret->nexto == false) {
        ir_append(ir_if, var1, vtype1, var2, vtype2, i, 0);
        ir_appendgoto(ret->tlb);
        ret->goto1 = 1;
    }
    else {
        ir_append(ir_if, var1, vtype1, var2, vtype2, 5-i, 0);
        ir_appendgoto(ret->flb);
        ret->goto0 = 1;
    }
}

void trans_relop_numb(char* op, int var1, char vtype1, int var2, char vtype2, struct eval *ret)
{
    int i, j, label;
    i = reloptoi(op);
    ret->var = tvar_cnt ++;
    ret->vt = IR_TempVar;
    
    j = iropt_relop1(i, var1, vtype1, var2, vtype2);

    if (j != 0){
        if (j == 1)
            ir_append(ir_assign, ret->var, ret->vt, 1, IR_Const, 0, 0);
        else
            ir_append(ir_assign, ret->var, ret->vt, 0, IR_Const, 0, 0);
    }
    else {
        ir_append(ir_assign, ret->var, ret->vt, 1, IR_Const, 0, 0);
        ir_append(ir_if, var1, vtype1, var2, vtype2, i, 0);
        label = label_cnt ++;
        ir_appendgoto(label);
        ir_append(ir_assign, ret->var, ret->vt, 0, IR_Const, 0, 0);
        ir_appendlabel(label);
    }
}

void trans_exp_bool(struct eval *ret)
{
    if (ret->nexto) {
        ir_append(ir_if, ret->var, ret->vt, 0, IR_Const, REL_LE, 0);
        ir_appendgoto(ret->flb);
        ret->goto0 = true;
    }
    else {
        ir_append(ir_if, ret->var, ret->vt, 0, IR_Const, REL_GT, 0);
        ir_appendgoto(ret->tlb);
        ret->goto1 = true;
    }
}

void ir_arg_st(int var, char vt)
{
    struct irb *node;
    node = (struct irb*)malloc(sizeof(struct irb));
    node->ir_t = ir_arg;
    node->var1 = var;
    node->vt1 = vt;

    node->next = param.next;
    node->pre = &param;
    if (param.next != NULL)
        param.next->pre = node;
    param.next = node;
}

void ir_arg_push()
{
    tail->next = param.next;
    if (param.next != NULL)
        param.next->pre = tail;
    while (tail->next != NULL)
        tail = tail->next;
    param.next = NULL;
}

void iropt(){
    iropt_unreachable();
}

