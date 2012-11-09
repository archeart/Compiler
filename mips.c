#include "asm.h"
#include "arch/mips32.h"


static bool read_used = 0, write_used = 0, called;
static int loop_layer = 0;
static int ir_line, fr_line;
static int trans_line = 0;

static int trans_ss;
static int func_cnt = 1;
static int ps = 0;

FILE *out;
char filename[128];

struct varb vars[VAR_MAX];
int var_reg[VAR_MAX];

struct code_bb cbbs[CODEBLKS_MAX];

#include "fr.c"
#include "codeblk.c"
#include "mips_debug.c"

int push_arg(int vid, int type)
{
    if (type == IR_Const) {
        if (vid == 0)
            return $zero;
        else {
            fr_append(inst_li, $a0, vid, 0);
            return $a0;
        }
    }

    /* push *a */
    if (IR_VAR(type)) {
        if (vars[vid].rid != 0)
            fr_append(inst_lw, $a0, vars[vid].rid, 0);
        else {
            assert(vars[vid].mm_alloc);
            fr_append(inst_lw, $a1, $fp, vars[vid].off);
            fr_append(inst_lw, $a0, $a0, 0);
        }
        /* for left */
    }

    else if (IR_ADDR(type))
        /* must be left */
        fr_append(inst_addi, $a0, $fp, get_off(vid));

    else if (vars[vid].rid != 0)
        return vars[vid].rid;
    
    else {
        assert(vars[vid].mm_alloc);
        fr_append(inst_lw, $a0, $fp, vars[vid].off);
    }

    return $a0;
}
#define get_reg0(x, y) get_reg(x, y, cbbs[bid].env, $a0, 0)
#define get_reg1(x, y) get_reg(x, y, cbbs[bid].env, $a1, 1)
#define get_reg2(x, y) get_reg(x, y, cbbs[bid].env, $a2, 1)
int get_reg(int vid, int type, struct regb *regs, int tempReg, bool left)
{
    int reg = tempReg, reg2;
    int i, j, cand;
    if (type == IR_Const) {
        if (vid == 0)
            return $zero;
        else {
            fr_append(inst_li, reg, vid, 0);
            return reg;
        }
    }

    /* TODO For right */
    /* must solve */
    if (IR_VAR(type)) {
        /* for left */
        reg2 = get_reg(vid, type & IR_mask, regs, tempReg + 1, 1);
        fr_append(inst_lw, reg, reg2, 0);
        return reg;
    }

    if (IR_ADDR(type)) {
        /* must be left */
        fr_append(inst_addi, reg, $fp, get_off(vid));
        return reg;
    }

    if (vars[vid].rid != 0)
        return vars[vid].rid;

    assert(vid != 0);
    /* find unused regs */
    for (i = 0; i < TREG_CNT; i ++) {
        if (regs[i].vid == 0) {
            bind_reg_var(i, vid, left);
            return i + TREG_BEG;
        }
    }
    /* find finished temp regs */
    for (i = 0; i < TREG_CNT; i ++) {
        j = regs[i].vid;
        if (vars[j].type == IR_TempVar && trans_line >= vars[j].last_used) {
            bind_reg_var(i, vid, left);
            vars[j].rid = 0;
            return i + TREG_BEG;
        }
    }

    /* find finished var regs out of loop */
    if (loop_layer == 0) {
        for (i = 0; i < TREG_CNT; i ++) {
            j = regs[i].vid;
            if (vars[j].type == IR_DefVar && trans_line >= vars[j].last_used) {
                bind_reg_var(i, vid, left);
                vars[j].rid = 0;
                return i + TREG_BEG;
            }
        }
    }

    /* find the regs with unset dirty bit*/
    cand = -1;
    for (i = 0; i < TREG_CNT; i ++) {
        if (!regs[i].dirty) {
            if (cand == -1)
                cand = i;
            else if (regs[i].time_stamp < regs[cand].time_stamp)
                cand = i;
        }
    }
    j = regs[cand].vid;
    bind_reg_var(cand, vid, left);
    vars[j].rid = 0;
    return cand + TREG_BEG;

    /* find the regs with mm_alloc set */
    for (i = 0; i < TREG_CNT; i ++) {
        j = regs[i].vid;
        if (vars[j].mm_alloc) {
            fr_append(inst_sw, i + TREG_BEG, $fp, vars[j].off);
            vars[j].rid = 0;
            bind_reg_var(i, vid, left);
            return i + TREG_BEG;
        }
    }

    /* for not mm_alloc set */
    cand = 0;
    for (i = 1; i < TREG_CNT; i ++)
        if (regs[i].time_stamp < regs[cand].time_stamp)
            cand = i;

    j = regs[cand].vid;
    vars[j].mm_alloc = true;
    trans_ss += vars[j].size;
    vars[j].off = trans_ss;
    fr_append(inst_sw, i + TREG_BEG, $fp, vars[j].off);

    bind_reg_var(cand, vid, left);
    vars[j].rid = 0;
    return cand + TREG_BEG;
}

void dirty_set(int vid, int bid)
{
    int reg = vars[vid].rid;

    if (reg != 0) {
        assert(reg >= TREG_BEG);
        cbbs[bid].env[reg - TREG_BEG].dirty = 1;

        if (vars[vid].size > 4) {
            fr_append(inst_sw, reg, $fp, vars[vid].off);
            cbbs[bid].env[reg - TREG_BEG].dirty = 0;
        }
    }
}

void save_regs(struct regb *regs, struct irb *p)
{
    int i, s = 4, vid;
    int args = 1;

    while (p->ir_t == ir_arg) {
        args += 1;
        p = p->next;
    }


    for (i = 0; i < TREG_CNT; i ++) {
        vid = regs[i].vid;
        if (vid == 0)
            continue;

        if (loop_layer == 0) {
            if (vars[vid].last_used > trans_line + args) {
                fr_append(inst_sw, i + TREG_BEG, $sp, -s);
                s += 4;
            }
        }
        else {
            if (vars[vid].type == IR_DefVar || 
                (vars[vid].type == IR_TempVar && 
                    vars[vid].last_used > trans_line + args)) {
                fr_append(inst_sw, i + TREG_BEG, $sp, -s);
                s += 4;
            }
        }
    }
    s -= 4;
    if (s != 0)
        fr_append(inst_addi, $sp, $sp, -s);

}

void restore_regs(struct regb* regs)
{
    int i, s = 0, vid;
    for (i = TREG_CNT - 1; i >= 0 ; i --) {
        vid = regs[i].vid;
        if (vid == 0)
            continue;

        if (loop_layer == 0) {
            if (vars[vid].last_used > trans_line) {
                fr_append(inst_lw, i + TREG_BEG, $sp, s);
                s += 4;
            }
        }
        else {
            if (vars[vid].type == IR_DefVar ||
                (vars[vid].type == IR_TempVar &&
                    vars[vid].last_used > trans_line)) {
                fr_append(inst_lw, i + TREG_BEG, $sp, s);
                s += 4;
            }
        }
    }
    if (s != 0)
        fr_append(inst_addi, $sp, $sp, s);
}

inline int get_off(int vid)
{
    if (!vars[vid].mm_alloc)
    assert(vars[vid].mm_alloc);
    return vars[vid].off;
}

void asm_init()
{
    int i;

    fr_init();

/* struct init */
    // init cbbs[0]
    
    memset(vars, 0, sizeof(vars));
    for (i = 0; i < VAR_MAX; i ++) {
        vars[i].size = 4;
    }

    memset(cbbs, 0, sizeof(cbbs));
    cbbs[0].scanned = true;

/* code init */
    /* data */
    fprintf(out, ".data\n");
    fprintf(out, "_ret: .asciiz \"\\n\"\n");
    fprintf(out, "_prompt: .asciiz \"enter a number:\"\n");

    /* globl */
    fprintf(out, ".globl main\n");
    
    /* text */
    fprintf(out, ".text\n");
    /* init read & write */

    fprintf(out, 
            "read:\n\t"
                "li $v0, 4\n\t"
                "la $a0, _prompt\n\t"
                "syscall\n\t"
                "li $v0, 5\n\t"
                "syscall\n\t"
                "jr $ra\n\n"
    );

    fprintf(out, 
            "write:\n\t"
                "li $v0, 1\n\t"
                "syscall\n\t"
                "li $v0, 4\n\t"
                "la $a0, _ret\n\t"
                "syscall\n\t"
                "move $v0, $0\n\t"
                "jr $ra\n\n"
    );
}

inline void asm_alloc_param(struct irb *it, bool called)
{
    int off;

    off = called ? 8 : 4;
    /* actually the off can be optimized */
    while (it->ir_t == ir_param) {
        vars[it->var1].off = off;
        vars[it->var1].mm_alloc = true;
        off += 4;
        it = it->next;
    }
}

void asm_prepare()
{
    struct irb *iter = head.next;   
    int bid = 0, prebid = 0;
    int off;

    ir_line = 0;

    while (iter->next != NULL) {
        ir_line ++;
//        fprintf(stderr, "iter->ir_t: %d\n", iter->ir_t);
        switch (iter->ir_t) {
            case ir_func:
                
       //         printf("function %d\n", ir_line);
                bb_init_block(label_cnt, 0);
                iter->bid = label_cnt;
                cbbs[iter->bid].ant = iter->bid;

                bid = label_cnt ++;
                break;

            case ir_label:
      //          printf("label %d\n", ir_line);
                bb_init_block(iter->var1, bid);
                cbbs[iter->var1].ant = bid;
                iter->bid = iter->var1;
                bid = iter->var1;
                break;

            case ir_if:
                assert(cbbs[bid].exit1 == 0);

                left_update(iter->var1, iter->vt1);
                left_update(iter->var2, iter->vt2);

                iter = iter->next;
                cbbs[bid].exit1 = iter->var1;
    //            printf("ifgoto %d\n", ir_line);
                bb_add_pre(iter->var1, bid);
                
                if (iter->next->ir_t != ir_goto) {
     //               printf("after if %d\n", ir_line);
                    bb_init_block(label_cnt, bid);
                    cbbs[label_cnt].ant = bid;
                    iter->next->bid = label_cnt;
                    bid = label_cnt ++;
                }
                break;

            case ir_goto:
                assert(iter->next->ir_t != ir_goto && iter->next->ir_t != ir_if);

                bb_add_pre(iter->var1, bid);

                if (iter->next->ir_t != ir_func && iter->next->ir_t != ir_label) {
                    /* unreachable */
//                    fprintf(stderr, "!!!!Unexpect Situation :%d\n", ir_line);
                }
                break;

            case ir_call:
                right_update(iter->var1, iter->vt1);
                break;

            case ir_assign:
                left_update(iter->var2, iter->vt2);
                right_update(iter->var1, iter->vt1);
                break;


            case ir_plus: case ir_minus: case ir_star: case ir_div:
                right_update(iter->var1, iter->vt1);
                left_update(iter->var2, iter->vt2);
                left_update(iter->var3, iter->vt3);
                break;

            case ir_dec:
                vars[iter->var1].size = iter->var2;
                break;

            case ir_write: case ir_arg:
                left_update(iter->var1, iter->vt1);
                break;

            case ir_read:
                right_update(iter->var1, iter->vt1);
                break;


            case ir_return:
                left_update(iter->var1, iter->vt1);
                cbbs[bid].exit = true;
                break;

        }

        iter = iter->next;
    }
}

struct irb *asm_trans_bb(int bid, struct irb *it)
{
    int reg1, reg2;

    bb_entry(bid);
    fr_jump_set();

    if (cbbs[bid].loop_lock > 0)
        loop_layer ++;
    else if (cbbs[bid].loop_lock < 0)
        loop_layer --;

    while (it != NULL) {
        trans_line ++;
//        fprintf(stderr, "it->ir_t: %d\n", it->ir_t);
        switch (it->ir_t) {
            case ir_assign:
                /* *a = b */
                if ((it->vt1 & IR_val_f) == IR_val_f) {
                    assert( (it->vt1 & IR_mask) < IR_Const );
                    assert( (it->vt1 & IR_addr_f) != IR_addr_f );

                    fr_append(inst_sw, get_reg1(it->var2, it->vt2),
                            get_reg2(it->var1, it->vt1 & IR_mask), 0);
                }
                /* a = &b */
                else if ((it->vt2 & IR_addr_f) == IR_addr_f)
                    fr_append(inst_addi, get_reg0(it->var1, it->vt1), $fp, get_off(it->var2));

                /* a = *b */
                else if ((it->vt2 & IR_val_f) == IR_addr_f)
                    fr_append(inst_lw, get_reg0(it->var1, it->vt1), 
                            get_reg1(it->var2, it->vt2 & IR_mask), 0);

                /* a = b */
                else if (it->vt2 != IR_Const)
                    fr_append(inst_move, get_reg0(it->var1, it->vt1), 
                        get_reg1(it->var2, it->vt2), 0);

                else
                    fr_append(inst_li, get_reg0(it->var1, it->vt1), it->var2, 0);

                if (!(IR_VAR(it->vt1)))
                    dirty_set(it->var1, bid);
                break;

            case ir_plus: case ir_minus: case ir_star:
                fr_append(inst_add + it->ir_t, get_reg0(it->var1, it->vt1),
                    get_reg1(it->var2, it->vt2), get_reg2(it->var3, it->vt3));

                if ( IR_VAR(it->vt1) )
                    fr_append(inst_sw, $a0, 
                            get_reg1(it->var1, it->vt1 & IR_mask), 0);
                else
                    dirty_set(it->var1, bid);
                break;

            case ir_div:
                fr_append(inst_div, get_reg1(it->var2, it->vt2),
                    get_reg2(it->var3, it->vt3), 0);
                fr_append(inst_mflo, get_reg0(it->var1, it->vt1), 0, 0);

                if ( IR_VAR(it->vt1) )
                    fr_append(inst_sw, $a0, 
                            get_reg1(it->var1, it->vt1 & IR_mask), 0);
                else
                    dirty_set(it->var1, bid);
                break;


            case ir_arg:
                if (ps == 0)
                    save_regs(cbbs[bid].env, it);
                ps += 4;
                fr_append(inst_sw, push_arg(it->var1, it->vt1), $sp, -ps);
                break;

            case ir_call:
                if (ps == 0)
                    save_regs(cbbs[bid].env, it);
                else
                    fr_append(inst_addi, $sp, $sp, -ps);

                fr_append(inst_jal, 0, 0, it->var2);
                if (ps != 0)
                    fr_append(inst_addi, $sp, $sp, ps);

                restore_regs(cbbs[bid].env);

                fr_append(inst_move, get_reg0(it->var1, it->vt1), $v0, 0);
                ps = 0;
                break;

            case ir_goto:
                fr_jump_append(inst_j, it->var1, 0, 0);
                break;

            case ir_label:
                fr_append(inst_label, it->var1, 0, 0);
                break;

            case ir_return:
                if ( (it->vt1 & IR_val_f) == IR_val_f )
                    fr_append(inst_lw, $v0, get_reg1(it->var1, it->vt1 & IR_mask), 0);
                else if (it->vt1 == IR_Const)
                    fr_append(inst_li, $v0, it->var1, 0);
                else
                    fr_append(inst_move, $v0, get_reg0(it->var1, it->vt1), 0);

                fr_jump_append(inst_j, func_cnt, 0, (int)"exit");
                break;

            case ir_read:
                fr_append(inst_jal, 0, 0, (int)"read");
                if ( (it->vt1 & IR_val_f) == IR_val_f )
                    fr_append(inst_sw, $v0, 
                            get_reg1(it->var1, it->vt1 & IR_mask), 0);
                else {
                    fr_append(inst_move, get_reg0(it->var1, it->vt1), $v0, 0);
                    dirty_set(it->var1, bid);
                }


                break;

            case ir_write:
                if ( (it->vt1 & IR_val_f) == IR_val_f )
                    fr_append(inst_lw, $a0, get_reg1(it->var1, it->vt1 & IR_mask), 0);
                else
                    fr_append(inst_move, $a0, get_reg1(it->var1, it->vt1), 0);
                fr_append(inst_jal, 0, 0, (int)"write");
                break;

            case ir_if:
                reg1 = get_reg1(it->var1, it->vt1);
                reg2 = get_reg2(it->var2, it->vt2);
                fr_jump_append(inst_beq + it->var3, reg1, reg2, it->next->var1);
                it = it->next;
                if (it->next->ir_t == ir_goto) {
                    fr_jump_append(inst_j, it->next->var1, 0, 0);
                    trans_line ++;
                    it = it->next;
                }

                break;

        }

        it = it->next;
        if (it == NULL)
            break;
        if (it->bid != 0)
            break;
    }

    bb_exit(bid, trans_line);
    fr_jump_link();

    return it;
}

struct irb *asm_trans_func(struct irb *entry, int det_ss, bool called)
{
    struct frb *sp;
    struct irb *it;
    int ps;

    trans_ss = det_ss;
//    fprintf(stderr, "func ss: %d\n", det_ss);

    fr_append(inst_label, 0, 0, entry->var1);
    /* func entry */
    if (called) {
        fr_append(inst_addi, $sp, $sp, -8);
        fr_append(inst_sw, $ra, $sp, 0);
        fr_append(inst_sw, $fp, $sp, 4);
        fr_append(inst_move, $fp, $sp, 0);
    }
    else {
        fr_append(inst_addi, $sp, $sp, -4);
        fr_append(inst_sw, $fp, $sp, 0);
        fr_append(inst_move, $fp, $sp, 0);
    }
    fr_append(inst_addi, $sp, $sp, -trans_ss);
    sp = fr_getlast();

    /* calc off for param */
    asm_alloc_param(entry->next, called);
    
    /* func core */
    it = entry;
    while (it != NULL) {
        it = asm_trans_bb(it->bid, it);
        if (it == NULL)
            break;
        if (it->ir_t == ir_func)
            break;
    }

    /* func exit */
    fr_append(inst_label, func_cnt++, 0, (int)"exit");
    sp->op3 = -trans_ss;
    fr_append(inst_addi, $sp, $sp, trans_ss);

    if (called) {
        fr_append(inst_lw, $fp, $sp, 4);
        fr_append(inst_lw, $ra, $sp, 0);
        fr_append(inst_addi, $sp, $sp, 8);
    }
    else {
        fr_append(inst_lw, $fp, $sp, 0);
        fr_append(inst_addi, $sp, $sp, 4);
    }
    fr_append(inst_jr, $ra, 0, 0);
}

/* calc off for DefVar  &  reserve the type for var descriptor */
void off_ts(int vid, char type, int *ss)
{
    if ( (type & IR_mask) == IR_DefVar ) {
        if ( !vars[vid].mm_alloc ) {
            vars[vid].mm_alloc = true;
            *ss += vars[vid].size;
            vars[vid].off = -*ss;
        }
    }

//    printf("[OFF_TS] id: %d;  type: %d\n", vid, type);
    if (type != IR_Const)
        vars[vid].type = type & IR_mask;
}
void asm_ckvar(struct irb *p, int *ss)
{
    off_ts(p->var1, p->vt1, ss);
    off_ts(p->var2, p->vt2, ss);
    if (p->ir_t != ir_assign)
        off_ts(p->var3, p->vt3, ss);
}
void asm_funcprg()
{
    /* the task is 
     * 1. determine whether call any subroutine
     * 2. alloc stack for vars
     * */

    struct irb *it = head.next;
    struct irb *entry = NULL;

    bool called = false;
    int ss = 0;

    while (it != NULL) {
        switch (it->ir_t) {
            case ir_func:

                if (entry != NULL)
                    asm_trans_func(entry, ss, called);

                entry = it;
                called = false;
                ss = 0;
                break;

            case ir_param:
                vars[it->var1].mm_alloc = true;
                break;

            case ir_call: case ir_arg:
                off_ts(it->var1, it->vt1, &ss);
                called = true;
                break;

            case ir_assign: case ir_plus: case ir_minus: case ir_star: case ir_div:
                asm_ckvar(it, &ss);
                break;

            case ir_read: case ir_write:
                off_ts(it->var1, it->vt1, &ss);
                called = true;
                break;
        }
        it = it->next;
    }

    assert(entry != NULL);
    asm_trans_func(entry, ss, called);
}

void asm_main()
{
//    fprintf(stderr, "ASM START\n");
    out = fopen(filename, "w");
    asm_init();
    asm_prepare();
    asm_funcprg();
//    asm_debug();
    fr_print();
    fclose(out);
}
