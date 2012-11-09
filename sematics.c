
#include "compiler.h"
#include "ir.h"
#include "util.h"
#include <assert.h>

char* anony_gen();

#define PER_ADD 0
#define FOR_ADD 1


#define MAX_SYMBOLS 2437
struct hash_item utypes[MAX_SYMBOLS];
struct hash_item ufuncs[MAX_SYMBOLS];

/* env stack */
#define MAX_ENV 10
struct symbols *env[MAX_ENV];
int env_top;
int env_ret;
/* env may contain many other arguments
 * like offsets */


/* basic operations of hash table */
int JShash(char* str)
{
    unsigned int key = 1315423911;

    while (*str) {
       key ^= ((key << 5) + (*str++) + (key >> 2));
    }
    return (key & 0x7FFFFFFF) % MAX_SYMBOLS;
}

void hitem_init (struct hash_item* table, int i, char *name, struct dlink* dp, struct slink* sp, int bw)
{
    table[i].used = true;
    table[i].val.id = name;
    table[i].val.arr_attr = dp;
    table[i].val.str_attr = sp;
    table[i].val.ref = bw;

    if (dp != null) {
        if (bw >= 0) {
            table[i].val.type = table[bw].val.type + 3;
            table[i].val.width = table[bw].val.width * dp->dim;
        } else if (bw == -1) {
            table[i].val.type = TP_INTP;
            table[i].val.width = 4 * dp->dim;
        } else if (bw == -2) {
            table[i].val.type = TP_FLTP;
            table[i].val.width = 4 * dp->dim;
        }
    }
}

bool tyfun_check(struct hash_item* table, char* id, int* pos)
{
    int key = JShash(id);
    int i = key;
    
    while (true) {
        if (table[i].used == false) {
            *pos = i;
            return false;
        }

        else if (strcmp(table[i].val.id, id) == 0) {
            *pos = i;
            return true;
        }

        i = (i + 1) % MAX_SYMBOLS;
    }

}


/* slink */
bool sl_add(struct user_type* utype, struct slink* p)
{
    struct slink *s = utype->str_attr;
    if (s == null) {
        utype->str_attr = p;
        return true;
    }

    while (true) {
        if (strcmp(s->id, p->id) == 0)
            return false;
        if (s->next == null) {
            s->next = p;
            break;
        }
        s = s->next;
    }

    return true;
}


/* maintain for the symbol table */
struct symbols *ty_root, *fun_root;

bool st_insert(struct symbols **cnode, struct symbols *inode)
{
    int cmp;
    if (*cnode == null) {
        *cnode = inode;
        return true;
    }

    cmp = strcmp( (*cnode)->id, inode->id );
    if (cmp > 0)
        return st_insert(&(*cnode)->lchd, inode);
    else if (cmp < 0)
        return st_insert(&(*cnode)->rchd, inode);
    else {
        /* if type not same, mush be wrong */
        /* one or both are declaration, then TRUE */

        if ((*cnode)->hash_key != inode->hash_key)
            return false;
        
        if ((*cnode)->flag && inode->flag)
            return false;

        if (inode->flag == 1)
            (*cnode)->flag = true;

        return true;
    }
}

struct symbols* st_lookup(struct symbols *node, char *id)
{
    int cmp;
    if (node == null)
        return null;
    cmp = strcmp(node->id, id);
    if (cmp == 0)
        return node;
    else if (cmp > 0)
        return st_lookup(node->lchd, id);
    else 
        return st_lookup(node->rchd, id);
}

inline int tyst_add(struct symbols *inode)
{
    inode->var_no = uvar_cnt ++;
    if (inode->hash_key >= 0)
        ir_appenddec(inode->var_no, utypes[inode->hash_key].val.width);
    if (!st_insert(&env[env_top], inode))
        return 0;
    else
        return inode->var_no;
}

inline bool funst_add(struct symbols *inode)
{
    return st_insert(&fun_root, inode);
}

void env_clean(struct symbols *p) 
{
    if (p == null)
        return ;
    env_clean(p->lchd);
    env_clean(p->rchd);
    free(p);
}

void sem_init()
{
    int i;
    env_top = 0;
    for (i = 0; i < MAX_ENV; i++)
        env[i] = null;

    for (i = 0; i < MAX_SYMBOLS; i++) {
        utypes[i].used = false;
        utypes[i].val.arr_attr = null;
        utypes[i].val.str_attr = null;
        ufuncs[i].used = false;
        ufuncs[i].val.arr_attr = null;
        ufuncs[i].val.str_attr = null;
    }
}


int do_Specifier(char, struct syntree*);

int do_VarDec(struct syntree *p, int ind, char type, int pos, bool def)
{
    struct syntree *s, *q;
    struct dlink *dp1, *dp2;
    struct slink *sp, *sp2;
    struct symbols* nvar;
    char *type_name;
    int ind2, i;

    s = p->chld;
    dp1 = null;

    while (s->val != TK_ID) {
        q = s->sib->sib;

        dp2 = (struct dlink*) malloc (sizeof(struct dlink));
        dp2->dim = atoi(q->cont);

        if (dp1 == null)
            dp1 = dp2;
        else {
            dp2->dim = dp2->dim * dp1->dim;
            dp2->next = dp1;
            dp1 = dp2;
        }

        s = s->chld;
    }
    

    if (dp1 != null) {
        type_name = anony_gen();
        tyfun_check(utypes, type_name, &i);
        utypes[i].val.width = dp1->dim * utypes[ind].val.width;
        hitem_init(utypes, i, type_name, dp1, null, ind);
    } else
        i = ind;


   if (type == 1) {
        nvar = (struct symbols*) malloc (sizeof(struct symbols));
        nvar->id = s->cont;
        nvar->hash_key = i;
        nvar->flag = def;
        nvar->param = false;
        /* tink */
        return tyst_add(nvar);
    }
    else {
        sp = utypes[pos].val.str_attr;

        sp2 = (struct slink*) malloc (sizeof(struct slink));
        sp2->id = s->cont;
        sp2->hash_key = i;
        sp2->next = null;

        
        if (type == 0)
            return sl_add(&utypes[pos].val, sp2);
        else
            return sl_add(&ufuncs[pos].val, sp2);
    }
}

int new_struct(struct syntree *p)
/* OptTag LC DefList RC */
{
    struct syntree *s, *q;
    struct user_type *utype;
    char *name;
    int pos, ind;
    int bw, cnt;
    struct slink* miter;

    if (p->chld == null)
        name = anony_gen();
    else 
        name = p->chld->cont;

    if (tyfun_check(utypes, name, &pos)) {
        printf(sem_err[16], p->line);
        err = 1;
        return -4;
    }

    s = p->sib->sib;

    utypes[pos].used = true;
    utype = &utypes[pos].val;
    utype->width = 0;
    utype->id = name;
    utype->type = TP_STR;

    while (s->chld != null) { //def not null
        q = s->chld->chld;  //specifier
        cnt = 0;
        ind = do_Specifier(FOR_ADD, q);

        if (ind == pos) {
            /* error: inner type same with the outter */
            printf("XError type 1 at line %d: Recursive definition.\n", q->line);
            err = 1;
            return -3;
        }

        if (ind < -2) {
//          printf("type error\n");
            err = 1;
            return -3;
        }

        q = q->sib->chld;     //dec ( comma declist )
        while (true) {
            if (!do_VarDec(q->chld, ind, 0, pos, 1)) {
                printf(sem_err[16], s->line);
                err = 1;
                return -3;
            }

            if (q->chld->sib != null) {
                printf(sem_err[15], q->chld->sib->line);
                err = 1;
            }

            if (q->sib == null)
                break;
            q = q->sib->sib;
            q = q->chld;

        }

        s = s->chld->sib;
    }

    /* calc width */
    miter = utype->str_attr;
    while (miter != null) {
        miter->offset = utype->width;
        if (miter->hash_key < 0)
            utype->width += 4;
        else
            utype->width += utypes[miter->hash_key].val.width;
        miter = miter->next;
    }

    return pos;
}


int do_Exp(struct syntree*, struct eval*, char flag);
void do_Args(struct syntree* p, struct slink* args)
{
    struct syntree *s = p->chld;
    struct eval exp;

    while (true) {
        do_Exp(s, &exp, EXP_NUMB);
        
        if (args == null || !type_check(args->hash_key, exp.hash_key)) {
            printf(sem_err[9], s->line);
            err = 1;
            return ;
        }

        ir_arg_st(exp.var, exp.vt);
        args = args->next;
        if (s->sib == null)
            break;
        s = s->sib->sib->chld;
    }

    if (args != null){
        printf(sem_err[9], s->line);
        err = 1;
        return ;
    }

    ir_arg_push();
}

int do_Exp(struct syntree *p, struct eval *ret, char flag){
    struct syntree *s = p->chld, *q;
    struct eval exp1, exp2;
    struct symbols *sym;
    struct slink *sp;
    int key, i, j, wid;

    ret->goto0 = false;
    ret->goto1 = false;

    q = s->sib;
    if (s->val == TK_Exp) {
        if (q->val == TK_LB) {      //arrays

            do_Exp(s, &exp1, EXP_NUMB);
            do_Exp(q->sib, &exp2, EXP_NUMB);

            if (exp2.hash_key != -1) {
                printf(sem_err[12], s->line);
                err = 1; return ;
            }

            if (exp1.hash_key == -3) {
                eval_fill(ret, -3, null, true);
                err = 1; return ;
            }
            else if (!is_array(exp1.hash_key)) {
                printf(sem_err[10], s->line);
                eval_fill(ret, -3, null, true);
                err = 1; return ;
            }

            if (exp1.cur->next == null)
                eval_fill(ret, utypes[exp1.hash_key].val.ref, exp1.cur->next, LVALUE);
            else
                eval_fill(ret, exp1.hash_key, exp1.cur->next, LVALUE);

            /* ir start */
            wid = GET_BTYPE_WIDTH(utypes[exp1.hash_key].val.ref);

            wid = (ret->cur == null) ? wid : (wid * ret->cur->dim);
            ir_rexp_append(ir_star, exp2.var, exp2.vt, wid, IR_Const, &i, &j);
            ir_rexp_append(ir_plus, exp1.var, exp1.vt, i, j, &ret->var, &ret->vt);

            if (ret->cur == null && utypes[exp1.hash_key].val.ref < 0) {
                /* modify 6-17 */
                if ( (ret->vt & IR_addr_f) != IR_addr_f )
                    ret->vt = ret->vt | IR_val_f;
                else
                    ret->vt = (ret->vt & IR_mask);
            }

            if (flag == EXP_BOOL)
                trans_exp_bool(ret);
            /* ir ends */
            return ;
        }

        else if (q->val == TK_DOT) {    //structure
            do_Exp(s, &exp1, EXP_NUMB);
            q = q->sib;

            if (exp1.hash_key < 0 || utypes[exp1.hash_key].val.type != TP_STR) {
                printf(sem_err[13], s->line);
                err = 1; i = -3;
                return ;
            }

            sp = str_member(exp1.hash_key, (char*)q->cont);

            if (sp == null) {
                eval_fill(ret, -3, null, true);
                printf(sem_err[14], q->line, q->cont);
                err = 1; return ;
            }
            
            if (sp->hash_key >= 0)
                eval_fill(ret, sp->hash_key, utypes[sp->hash_key].val.arr_attr, true);
            else
                eval_fill(ret, sp->hash_key, null, true);

            ir_rexp_append(ir_plus, exp1.var, exp1.vt, sp->offset, IR_Const, &ret->var, &ret->vt);

            if (sp->hash_key < 0) {
                /* modify 6-17 */
                if ( (ret->vt & IR_addr_f) != IR_addr_f )
                    ret->vt = ret->vt | IR_val_f;
                else
                    ret->vt = (ret->vt & IR_mask);
            }

            if (flag == EXP_BOOL)
                trans_exp_bool(ret);
            /* ir ends */
 
            return ;
        }

        if (q->val != TK_AND && q->val != TK_OR) {
            do_Exp(s, &exp1, EXP_NUMB);
            do_Exp(q->sib, &exp2, EXP_NUMB);
        }

        switch (q->val) {
            case TK_ASSIGNOP:
                if (!typeck_bu(exp1.hash_key, exp2.hash_key)) {
                    printf(sem_err[5], q->line);
                    err = 1;
                }
                if (!exp1.left) {
                    printf(sem_err[6], q->line);
                    err = 1;
                }
                if (exp1.cur != null) {
                    printf("XError type 5 at line %d: Array type can not do assignment.\n", q->line);
                }
 
                eval_fill(ret, exp1.hash_key, null, false);

                /* ir start */
                if (exp2.vt == IR_Read)
                    ir_append(ir_read, exp1.var, exp1.vt, 0, 0, 0, 0);
                else {
                    ret->var = exp1.var;
                    ret->vt = exp1.vt;
                    ir_append(ir_assign, exp1.var, exp1.vt, exp2.var, exp2.vt, 0, 0);
                }
                /* ir end */
                break;

                

            case TK_OR:                                 /* Exp1 OR (new label_false) Exp2 */
                eval_fill(ret, -1, null, false);

                /* ir start */ 
                if (flag == EXP_BOOL) {
                    exp1.tlb = ret->tlb; exp1.flb = label_cnt ++;
                    exp2.tlb = ret->tlb; exp2.flb = ret->flb;
                    exp1.nexto = false;
                    exp2.nexto = ret->nexto;
                    do_Exp(s, &exp1, EXP_BOOL);
                    if (exp1.goto0)
                        ir_appendlabel(exp1.flb);
                    /* when bugs, ir_appendlabel() */
                    do_Exp(q->sib, &exp2, EXP_BOOL);
                    if (exp1.goto1 || exp2.goto1)
                        ret->goto1 = true;
                    if (exp2.goto0)
                        ret->goto0 = true;
                }

/* NUMB */
                else {
                    ret->var = tvar_cnt ++;
                    ret->vt = IR_TempVar;
                    exp1.tlb = label_cnt ++; exp1.flb = label_cnt ++;
                    exp1.nexto = false;
                    exp2.tlb = exp1.tlb; exp2.flb = label_cnt ++;
                    exp2.nexto = true;

                    ir_append(ir_assign, ret->var, ret->vt, 0, IR_Const, 0, 0);
                    do_Exp(s, &exp1, EXP_BOOL);
                    if (exp1.goto0)
                        ir_appendlabel(exp1.flb);
                    do_Exp(q->sib, &exp2, EXP_BOOL);

                    if (exp1.goto1 || exp2.goto1)
                        ir_appendlabel(exp1.tlb);

                    ir_append(ir_assign, ret->var, ret->vt, 1, IR_Const, 0, 0);
                    if (exp2.goto0)
                        ir_appendlabel(exp2.flb);
                }

                if (exp1.hash_key != -1 || exp2.hash_key != -1)
                    printf("Error type 7 at line %d: Only int can do logic operation.\n", q->line);
                /* ir end */
                break;




            case TK_AND:                            /* Exp1 AND (new true_label) Exp2 */
                eval_fill(ret, -1, null, false);

                /* ir start */ 

                if (flag == EXP_BOOL) {
                    exp1.tlb = label_cnt ++; exp1.flb = ret->flb;
                    exp2.tlb = ret->tlb; exp2.flb = ret->flb;
                    exp1.nexto = true;
                    exp2.nexto = ret->nexto;
                    do_Exp(s, &exp1, EXP_BOOL);

                    if (exp1.goto1)
                        ir_appendlabel(exp1.tlb);

                    do_Exp(q->sib, &exp2, EXP_BOOL);

                    if (exp1.goto0 || exp2.goto0)
                        ret->goto0 = true;
                    if (exp2.goto1)
                        ret->goto1 = true;
                }

/* TODO NUMB*/
                else {
                    ret->var = tvar_cnt ++;
                    ret->vt = IR_TempVar;
                    exp1.tlb = label_cnt ++; exp1.flb = label_cnt ++;
                    exp1.nexto = true;
                    exp2.tlb = label_cnt ++; exp2.flb = exp1.flb;
                    exp2.nexto = true;

                    ir_append(ir_assign, ret->var, ret->vt, 0, IR_Const, 0, 0);
                    do_Exp(s, &exp1, EXP_BOOL);
                    if (exp1.goto0)
                        ir_appendlabel(exp1.flb);
                    do_Exp(q->sib, &exp2, EXP_BOOL);

                    if (exp1.goto1 || exp2.goto1)
                        ir_appendlabel(exp1.tlb);

                    ir_append(ir_assign, ret->var, ret->vt, 1, IR_Const, 0, 0);
                    if (exp2.goto0)
                        ir_appendlabel(exp2.flb);

                }
                if (exp1.hash_key != -1 || exp2.hash_key != -1)
                    printf("Error type 7 at line %d: Only int can do logic operation.\n", q->line);

                /* ir end */
                break;






            case TK_RELOP:
                if (!typeck_bu(exp1.hash_key, exp2.hash_key)) {
                    printf(sem_err[7], q->line);
                    printf("err happens at relop\n");
                    err = 1;
                }
                eval_fill(ret, -1, null, false);

                /* ir start */
                if (flag == EXP_BOOL)
                    trans_relop_bool(q->cont, exp1.var, exp1.vt, exp2.var, exp2.vt, ret);
                else
                    trans_relop_numb(q->cont, exp1.var, exp1.vt, exp2.var, exp2.vt, ret);


                /* ir end */
                break;





            case TK_PLUS: case TK_MINUS: case TK_STAR: case TK_DIV:
                if (!typeck_bu(exp1.hash_key, exp2.hash_key)) {
                    printf(sem_err[7], q->line);
                    err = 1; break;
                }

                if (exp1.hash_key >= 0) {
                    printf("Error type 7 at line %d: Only numberic type can do %s operation\n", q->line, token_str[q->val]);
                    err = 1;
                    exp1.hash_key = -1;
                }
                eval_fill(ret, exp1.hash_key, null, false);

                /* ir start */

                ir_rexp_append(q->val-TK_PLUS, exp1.var, exp1.vt, exp2.var, exp2.vt, &ret->var, &ret->vt);

                if (flag == EXP_BOOL) {
                    if (ret->nexto == true) {
                        ir_append(ir_if, ret->var, ret->vt, 0, IR_Const, REL_LE, 0);
                        ir_appendgoto(ret->flb);
                        ret->goto0 = 1;
                    }
                    else {
                        ir_append(ir_if, ret->var, ret->vt, 0, IR_Const, REL_GT, 0);
                        ir_appendgoto(ret->tlb);
                        ret->goto1 = 1;
                    }
                }
                /* ir end */

                break;
        }

    }




    else if (s->val == TK_ID){
        q = s->sib;

        if (q != null) {
            if (!tyfun_check(ufuncs, s->cont, &key)) {
                printf(sem_err[2], s->line, s->cont);
                eval_fill(ret, -3, null, false);
                err = 1; return ;
            }

            else {
                i = ufuncs[key].val.ref;
                if (i < 0)
                    eval_fill(ret, i, null, false);
                else
                    eval_fill(ret, i, utypes[i].val.arr_attr, false);

                q = q->sib;
                if (q->val == TK_Args) {
                    do_Args(q, ufuncs[key].val.str_attr);
                } else if (ufuncs[key].val.str_attr != null) {
                        printf(sem_err[9], q->line);
                        err = 1;
                }

                ret->var = tvar_cnt ++;
                ret->vt = IR_TempVar;
                ir_append(ir_call, ret->var, ret->vt, (int)ufuncs[key].val.id, IR_Func, 0, 0);
            }

            

        }
        else {
            i = env_top;
            while (i >= 0) {
                sym = st_lookup(env[i], s->cont);
                if (sym != null)
                    break;
                i --;
            }

            if (sym == null) {
                printf(sem_err[1], s->line, s->cont);
                err = 1;
                eval_fill(ret, -3, null, true);
            } else {
                if (sym->hash_key < 0)
                    eval_fill(ret, sym->hash_key, null, true);
                else
                    eval_fill(ret, sym->hash_key, utypes[sym->hash_key].val.arr_attr, true);
            }

            ret->var = sym->var_no;
            ret->vt = IR_DefVar;
            if (sym->hash_key >= 0 && !sym->param)
                ret->vt |= IR_addr_f;
            if (flag == EXP_BOOL)
                trans_exp_bool(ret);
        }
    }

    else if (s->val == TK_INT) {
        eval_fill(ret, -1, null, false);
        ret->var = atoi(s->cont);
        ret->vt = IR_Const;
    }

    else if (s->val == TK_FLOAT) {
        eval_fill(ret, -2, null, false);
        ret->var = (int)s->cont;
        ret->vt = IR_StrPtr;
    }

    else {
        if (s->val == TK_NOT) {
            
            if (flag == EXP_BOOL) {
                exp1.nexto = 1 - ret->nexto;
                exp1.tlb = ret->flb;
                exp1.flb = ret->tlb;
//                printf("[Not] nexto: %d\n", ret->nexto);
            }
            else {
                ret->var = tvar_cnt ++;
                ret->vt = IR_TempVar;
                exp1.tlb = label_cnt ++;
                exp1.flb = label_cnt ++;
                exp1.nexto = 0;
                ir_append(ir_assign, ret->var, ret->vt, 0, IR_Const, 0, 0);
            }
            
            do_Exp(q, &exp1, EXP_BOOL);
            if (exp1.hash_key != -1) {
                printf(sem_err[7], q->line);
                err = 1;
                exp1.hash_key = -1;
            }

            else {
                if (flag == EXP_NUMB) {
                    if (exp1.goto0)
                        ir_appendlabel(exp1.flb);
                    ir_append(ir_assign, ret->var, ret->vt, 1, IR_Const, 0, 0);
                    if (exp1.goto1)
                        ir_appendlabel(exp1.tlb);
                }
                else {
                    if (exp1.goto0)
                        ret->goto1 = true;
                    if (exp1.goto1)
                        ret->goto0 = true;
                }
            }

        } 
        else if (s->val == TK_MINUS) {
            do_Exp(q, &exp1, EXP_NUMB);
            if (exp1.hash_key > 0) {
                printf(sem_err[7], q->line);
                exp1.hash_key = -1;
            }
            
            else {
                if (flag == EXP_NUMB) {
                    ret->var = tvar_cnt ++;
                    ret->vt = IR_TempVar;
                    ir_append(ir_minus, ret->var, ret->vt, 0, IR_Const, exp1.var, exp1.vt);
                }
                else {
                    if (ret->nexto == true) {
                        ir_append(ir_if, exp1.var, exp1.vt, 0, IR_Const, REL_GT);
                        ir_appendgoto(ret->flb);
                        ret->goto0 = 1;
                    }
                    else {
                        ir_append(ir_if, exp1.var, exp1.vt, 0, IR_Const, REL_LE);
                        ir_appendgoto(ret->tlb);
                        ret->goto1 = 1;
                    }
 
                }

            }
        } 
        else if (s->val == TK_LP) { /* (exp) */
            exp1.nexto = ret->nexto;
            exp1.tlb = ret->tlb; exp1.flb = ret->flb;
            do_Exp(q, &exp1, flag);
            ret->goto0 = exp1.goto0; ret->goto1 = exp1.goto1;
            ret->var = exp1.var;
            ret->vt = exp1.vt;
        }

        else if (s->val == TK_READ) {
            ret->vt = IR_Read;
            exp1.hash_key = -1;
        }

        else if (s->val == TK_WRITE) {
            do_Exp(s->sib->sib, &exp1, EXP_NUMB);
            ir_append(ir_write, exp1.var, exp1.vt, 0, 0, 0, 0);
        }

        eval_fill(ret, exp1.hash_key, null, false);
    }

    
}

int do_Specifier(char per, struct syntree *p)
/* retval == -3 || -4 means erorr */
{
    struct syntree *s = p->chld, *q;
    int pos;

    if (s->val == TK_TYPE) {
        if (strcmp(s->cont, "int") == 0)
            return -1;
        else if (strcmp(s->cont, "float") == 0)
            return -2;
        else {
            printf("Inner Error of Parser: TYPE not right");
            exit(1);
        }
    } 
    /* struct specifier */
    s = s->chld->sib;
    if (s->val == TK_Tag) {     /* struct Tag */

        if (tyfun_check(utypes, s->chld->cont, &pos)) {

            return pos;
        }
        else {
            printf(sem_err[17], s->line);
            err = 1;
            return -3;
        }
    }

    else {
        if (per == FOR_ADD) {
            printf("XError type 17 at line %d: Struct definition is forbidden in such scope.\n", s->line);
            return -4;
        }

        return new_struct(s);
    }
}

int do_FunDec(struct syntree *p, int key, bool def)
{
    struct syntree *s = p->chld, *q;
    struct slink *sp1 = null, *sp2;
    struct symbols *inode;
    int ind;
    int spec;
    bool his;
    
    q = s->sib->sib;
    his = tyfun_check(ufuncs, s->cont, &ind);

    if (his) {
        if (ufuncs[ind].val.ref != key){
            printf(sem_err[19], q->line);
            err = 1;
            return -1;
        }
        if (ufuncs[ind].val.f_flag && def) {
            printf(sem_err[4], s->line, s->cont);
            err = 1;
            return -1;
        }

        sp2 = ufuncs[ind].val.str_attr;
        ufuncs[ind].val.str_attr = null;
    }

    while (q->val == TK_VarList) {
        q = q->chld;
        spec = do_Specifier(FOR_ADD, q->chld);
        if (!do_VarDec(q->chld->sib, spec, 3, ind, def))
            printf("rename\n");

        if (q->sib != null)
            q = q->sib->sib;
    }


    if (his) {
        sp1 = ufuncs[ind].val.str_attr;
        if (!slink_cmp(sp1, sp2)){
            printf(sem_err[19], q->line);
            err = 1;
            if (!def) {
                ufuncs[ind].val.str_attr = sp2;
                slink_free(sp1);
            }
            return -1;
        }

    } else {
        ufuncs[ind].used = true;
        ufuncs[ind].val.id = s->cont;
        ufuncs[ind].val.ref = key;
    }

    if (def) {
        env_top ++;

        ir_append(ir_func, (int)ufuncs[ind].val.id, 0, 0, 0, 0, 0);

        ufuncs[ind].val.f_flag = true;
        sp2 = ufuncs[ind].val.str_attr;
        while (sp2 != null) {
            inode = (struct symbols*) malloc (sizeof(struct symbols));
            inode->id = sp2->id;
            inode->hash_key = sp2->hash_key;
            inode->flag = true;
            inode->param = true;
            if (!tyst_add(inode))
                break;
            ir_append(ir_param, inode->var_no, 0, 0, 0, 0, 0);
            sp2 = sp2->next;
        }
        env_top --;

    }
}

void do_ExtDef(struct syntree *p)
{
    struct syntree *s = p->chld, *q;
    int key = do_Specifier(PER_ADD, s);
    int ind;

    s = s->sib;
    if (s->val == TK_ExtDecList) {
        s = s->chld;
        while (true) {
            if (!do_VarDec(s, key, 1, 0, true)) {
                printf("variable error\n");
                err = 1;
            }

            if (s->sib == null)
                break;
            s = s->sib->sib->chld;
        }
    }
    else if (s->val == TK_FunDec) {
        env_ret = key;
        ind = do_FunDec(s, key, true);
        /* push the variables */

        if (ind != -1)
            sem_anal(s->sib);
    }
}

void do_ExtDec(struct syntree *p)
{
    struct syntree *s = p->chld, *q;
    int key, ind;
    
    if (s->val == TK_EXTERN)
        s = s->sib;
    key = do_Specifier(PER_ADD, s);

    s = s->sib;
    if (s->val == TK_ExtDecList) {
        s = s->chld; q = s->chld;
        while (true) {
            if (!do_VarDec(s, key, 1, 0, false))
                printf("XError type 3 at line %d: Conflicting declaration of variable \"%s\".\n", q->line, q->cont);
            if (s->sib == null)
                break;
            s = s->sib->sib->chld;
        }
    }
    else if (s->val == TK_FunDec) {
        ind = do_FunDec(s, key, false);
    }
}

void do_Def(struct syntree *p)
{
    struct syntree *s = p->chld, *q;
    struct eval exp;
    int key = do_Specifier(FOR_ADD, s);
    int ret;

    if (key < -2) {
//        printf("type error\n");
//        return ;
    }

    s = s->sib->chld;   
    while (true) {
        ret = do_VarDec(s->chld, key, 1, 0, true);

        q = s->chld;
        if (!ret) {
            printf(sem_err[3], q->chld->line, q->chld->cont);
            err = 1; return ;
        }
        q = q->sib;

        if (q != null) {
            do_Exp(q->sib, &exp, EXP_NUMB);
            if (!typeck_bu(key, exp.hash_key)) {
                printf(sem_err[5], q->line);
                err = 1; return ;
            }
            ir_append(ir_assign, ret, IR_DefVar, exp.var, exp.vt, 0, 0);
        }

        if (s->sib != null)
            s = s->sib->sib->chld;
        else
            break;
    }
}


void do_Stmt(struct syntree *p)
{
    struct syntree *s = p->chld, *q;
    struct eval exp;
    int label;

    if (s->val == TK_Exp)
        do_Exp(s, &exp, EXP_NUMB);

    else if (s->val == TK_IF) {
        q = s->sib->sib;

        exp.tlb = label_cnt ++;
        exp.flb = label_cnt ++;

        exp.nexto = true;
        do_Exp(q, &exp, EXP_BOOL);

        if (exp.hash_key != -1) {
            printf(sem_err[20], q->line);
        }

        if (exp.goto1)
            ir_appendlabel(exp.tlb);
        
        q = q->sib->sib;
        do_Stmt(q);

        /* TK_ELSE */
        q = q->sib;
        if (q != NULL) {
            label = label_cnt ++;
            ir_appendgoto(label);
            if (exp.goto0)
                ir_appendlabel(exp.flb);
            do_Stmt(q->sib);
            ir_appendlabel(label);
        }
        else if (exp.goto0)
            ir_appendlabel(exp.flb);
    }

    else if (s->val == TK_WHILE) {
        /* label start */
        label = label_cnt ++;
        ir_appendlabel(label);

        exp.tlb = label_cnt ++;
        exp.flb = label_cnt ++;
        exp.nexto = true;

        q = s->sib->sib;
        do_Exp(q, &exp, EXP_BOOL);

        if (exp.goto1)
            ir_append(exp.tlb);
        do_Stmt(q->sib->sib);
        ir_appendgoto(label);
        ir_appendlabel(exp.flb);
    }

    else if (s->val == TK_RETURN) {
        do_Exp(s->sib, &exp, EXP_NUMB);
        if (exp.hash_key != env_ret) {
            printf(sem_err[8], p->line);
            err = 1; return ;
        }
        ir_append(ir_return, exp.var, exp.vt, 0, 0, 0, 0);
    }

    else if (s->val == TK_CompSt)
        sem_anal(s);
}

void sem_anal(struct syntree *p)
{
    struct syntree *s;
    struct eval exp;
    
    switch (p->val) {
        case TK_ExtDef:
            /* global definition */
            do_ExtDef(p);
            break;

        case TK_Def:
            /* local definition */
            do_Def(p);
            break;

        case TK_ExtDec:
            /* global declaration */
            do_ExtDec(p);
            break;

        case TK_CompSt:
            env_top ++;
            s = p->chld;
            while (s != null) {
                sem_anal(s);
                s = s->sib;
            }

//            env_clean(env[env_top]);
            env[env_top] = null;
            env_top --;
            break;


        case TK_Stmt:
            do_Stmt(p);
            break;

        default:
            s = p->chld;
            while (s != null) {
                sem_anal(s);
                s = s->sib;
            }
    }
}
