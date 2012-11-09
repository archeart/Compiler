enum irtype {
    ir_plus = 0,
    ir_minus,
    ir_star,
    ir_div,
    ir_assign,
    ir_call,
    ir_label,
    ir_goto,
    ir_if,
    ir_return,
    ir_dec,
    ir_arg,
    ir_param,
    ir_read,
    ir_write,
    ir_func,
    ir_program
};

struct irb {
    enum irtype ir_t;
//    char ir_flag;
    int var1, var2, var3;
    char vt1, vt2, vt3;
    int bid;
    struct irb *next, *pre;
};

#define IR_TempVar      1 
#define IR_DefVar       2
#define IR_Const        3
#define IR_StrPtr       4
#define IR_Func         5
#define IR_Read         6
#define IR_Param        4

#define IR_mask         0xf
#define IR_addr_f       0x20
#define IR_val_f        0x10

#define IR_ADDR(x)      (x & IR_addr_f) == IR_addr_f
#define IR_VAR(x)       (x & IR_val_f) == IR_val_f

/* for relops */
#define REL_EQ      0
#define REL_GT      1
#define REL_LT      2
#define REL_GE      3
#define REL_LE      4
#define REL_NE      5

/* for append */
#define ir_appendlabel(x) ir_append(ir_label, x, 0, 0, 0, 0, 0)
#define ir_appendgoto(x) ir_append(ir_goto, x, 0, 0, 0, 0, 0)
#define ir_appenddec(x, y) ir_append(ir_dec, x, IR_DefVar, y, 0, 0, 0)

#define VAR_MAX  10000


/* modify 6-14 */
#define tvar_cnt uvar_cnt 


extern unsigned int label_cnt;
extern struct irb head, *tail;

extern void asm_main();
