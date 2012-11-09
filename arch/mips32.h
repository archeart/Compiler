/* mips registers */
#define $zero	 0
#define $at	 1
#define $v0	 2
#define $v1	 3
#define $a0	 4
#define $a1	 5
#define $a2	 6
#define $a3	 7
#define $t0	 8
#define $t1	 9
#define $t2	 10
#define $t3	 11
#define $t4	 12
#define $t5	 13
#define $t6	 14
#define $t7	 15
#define $s0	 16
#define $s1	 17
#define $s2	 18
#define $s3	 19
#define $s4	 20
#define $s5	 21
#define $s6	 22
#define $s7	 23
#define $t8	 24
#define $t9	 25
#define $k0	 26
#define $k1	 27
#define $gp	 28
#define $sp	 29
#define $fp	 30
#define $ra	 31

#define reg_zero "$zero"
#define reg_at	 "$at"
#define reg_v0	 "$v0"
#define reg_v1	 "$v1"
#define reg_a0	 "$a0"
#define reg_a1	 "$a1"
#define reg_a2	 "$a2"
#define reg_a3	 "$a3"
#define reg_t0	 "$t0"
#define reg_t1	 "$t1"
#define reg_t2	 "$t2"
#define reg_t3	 "$t3"
#define reg_t4	 "$t4"
#define reg_t5	 "$t5"
#define reg_t6	 "$t6"
#define reg_t7	 "$t7"
#define reg_s0	 "$s0"
#define reg_s1	 "$s1"
#define reg_s2	 "$s2"
#define reg_s3	 "$s3"
#define reg_s4	 "$s4"
#define reg_s5	 "$s5"
#define reg_s6	 "$s6"
#define reg_s7	 "$s7"
#define reg_t8	 "$t8"
#define reg_t9	 "$t9"
#define reg_k0	 "$k0"
#define reg_k1	 "$k1"
#define reg_gp	 "$gp"
#define reg_sp	 "$sp"
#define reg_fp	 "$fp"
#define reg_ra	 "$ra"

const char* regs_str[] = {
	"$zero", 
	"$at", 
	"$v0", 
	"$v1", 
	"$a0", 
	"$a1", 
	"$a2", 
	"$a3", 
	"$t0", 
	"$t1", 
	"$t2", 
	"$t3", 
	"$t4", 
	"$t5", 
	"$t6", 
	"$t7", 
	"$s0", 
	"$s1", 
	"$s2", 
	"$s3", 
	"$s4", 
	"$s5", 
	"$s6", 
	"$s7", 
	"$t8", 
	"$t9", 
	"$k0", 
	"$k1", 
	"$gp", 
	"$sp", 
	"$fp", 
	"$ra"
};


/* mips32 instruction */
enum fr_type {
	inst_addi, 
	inst_add, 
	inst_sub, 
	inst_mul, 
	inst_div,
        inst_move, 
	inst_mflo, 
	inst_mfhi, 
	inst_lw, 
	inst_li, 
	inst_la, 
	inst_sw, 
	inst_j, 
	inst_jr, 
	inst_jal, 
	inst_beq, 
	inst_bgt, 
	inst_blt, 
	inst_bge, 
	inst_ble, 
	inst_bne, 
	inst_label, 
	inst_syscall
};

const char *inst_str[] = {
	"addi", 
	"add", 
	"sub", 
	"mul", 
	"div",
        "move", 
	"mflo", 
	"mfhi", 
	"lw", 
	"li", 
	"la", 
	"sw", 
	"j", 
	"jr", 
	"jal", 
	"beq", 
	"bgt", 
	"blt", 
	"bge", 
	"ble", 
	"bne", 
	"label", 
	"syscall"
};


struct varb {
    int rid;
    int off;
    int last_used;
    int size;
    bool mm_alloc;
    char type;

//    struct varb *sr_pre, *sr_next;    /* varbs share the same reg */
};

struct regb {
    int vid;
    bool dirty;
    int time_stamp;
};

struct bbpre_link {
    int cbb;
    struct bbpre_link *next;
};


#define TREG_BEG 8
#define TREG_CNT 16
#define CODEBLKS_MAX 1000
struct code_bb {
    struct regb env[TREG_CNT];
    int loop_lock;

    int exit1, exit2;
    int ant;
    struct bbpre_link *preb;
    bool scanned, transed;
    bool exit;
};


#define left_update(vid, type) \
    if ((type & IR_mask) != IR_Const && \
            ((type & IR_addr_f) != IR_addr_f)) \
        vars[vid].last_used = ir_line

#define right_update(vid, type) \
    if ((type & IR_val_f) == IR_val_f) \
        vars[vid].last_used = ir_line

#define bind_reg_var(x, y, left)  \
    regs[x].dirty = 0; regs[x].vid = y; \
    vars[y].rid = x + TREG_BEG; regs[x].time_stamp = trans_line; \
    if (left) fr_append(inst_lw, x+TREG_BEG, $fp, vars[y].off)


#define stack_alloc(vid)    \
    vars[vid].mm_alloc = true;  trans_ss += vars[vid].size; \
    vars[vid].off = -trans_ss 

struct frb {
    enum fr_type type;
    int op1;
    int op2;
    union {
        char* name;
        int op3;
    };

    struct frb *next;
};

