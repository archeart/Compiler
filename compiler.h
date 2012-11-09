#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/************
 *  BASICS  *
 ************/

/* useful type */
#define bool unsigned char


/* useful constant */
#define true    1
#define false   0
#define null    0


/* global */
extern int err;

/*********
 * TOKEN *
 *********/
enum token_t {
	TK_INT = 0,
	TK_FLOAT = 1,
	TK_SEMI = 2,
	TK_COMMA = 3,
	TK_ASSIGNOP = 4,
	TK_RELOP = 5,
	TK_PLUS = 6,
	TK_MINUS = 7,
	TK_STAR = 8,
	TK_DIV = 9,
	TK_AND = 10,
	TK_OR = 11,
	TK_DOT = 12,
	TK_NOT = 13,
	TK_TYPE = 14,
	TK_LP = 15,
	TK_RP = 16,
	TK_LB = 17,
	TK_RB = 18,
	TK_LC = 19,
	TK_RC = 20,
	TK_STRUCT = 21,
	TK_RETURN = 22,
	TK_IF = 23,
	TK_ELSE = 24,
	TK_WHILE = 25,
	TK_EXTERN = 26,
	TK_ID = 27,
	TK_Program = 28,
	TK_ExtDefList = 29,
	TK_ExtDec = 30,
	TK_ExtDef = 31,
	TK_ExtDecList = 32,
	TK_Specifier = 33,
	TK_StructSpecifier = 34,
	TK_OptTag = 35,
	TK_Tag = 36,
	TK_VarDec = 37,
	TK_FunDec = 38,
	TK_VarList = 39,
	TK_ParamDec = 40,
	TK_CompSt = 41,
	TK_StmtList = 42,
	TK_Stmt = 43,
	TK_DefList = 44,
	TK_Def = 45,
	TK_DecList = 46,
	TK_Dec = 47,
	TK_Exp = 48,
	TK_Args = 49,
        TK_error = 50,
        TK_READ = 51,
        TK_WRITE = 52
};
extern char* token_str[];


/***********
 * GRAMMAR *
 ***********/

/* struct for syntax tree node */
struct syntree{
    struct syntree *chld, *sib;
    int line;
    char type;
    enum token_t val;
    char *cont;
};

/* for convenience a.gchld   equals to   a.grand.chld */
#define gchld grand->chld
#define gsib grand->sib
#define gtype grand->type
#define gval grand->val
#define glin grand->line
#define gcont grand->cont

#define term    0
#define nterm  1

/* create node */
inline struct syntree* create_syn(char);
inline struct syntree* create_esyn();
void syntree_visit(struct syntree*, int);




/************
 * SEMATICS *
 ************/

/* user defining type table */
struct dlink {
    int dim;
    struct dlink* next;
};

struct slink {
    int hash_key;
    char* id;
    char offset;
    struct slink* next;
};

struct user_type {
    unsigned char type;
    char* id;
    struct dlink* arr_attr;
    struct slink* str_attr;
    int width;
    int ref;
};
#define arg_attr str_attr
#define f_flag width
/* FOR FUNCTION: ref domain means retval type and 
 * stored in the first node of slink when its user type
 * if neccesary. and then slink links all the args
 * f_flag mean implement or not
 * FOR ARRAY: dlink saves all the dimension
 * FOR STRUCTURE: slink saves all the inner type
 */

struct hash_item {
    struct user_type val;
    bool used;
};

extern struct hash_item utypes[];
extern struct hash_item ufuncs[];

#define TP_VOID     0
#define TP_INT      1
#define TP_FLT      2
#define TP_STR      3
#define TP_INTP     4
#define TP_FLTP     5
#define TP_STRP     6
#define TP_FUNC     7


/* Tree Structure for symbol table */
struct symbols {
    char *id;
    int hash_key;
    struct symbols *lchd, *rchd;
    int addr;
    /* at this time addr is not useful */
    bool flag, param;
    /* 0 for only declare; 1 for elaborated definition */
    int var_no;
};

/* struct for exp val */
struct eval {
    int hash_key;
    struct dlink *cur;
    bool left;

    /* addr */
    int var;
    char vt;
    /* label */
    int tlb, flb;
    /* ret val for whether or not need to use label_true, label_false */
    bool goto1, goto0;
    /* args for whether the condition next to true/false label */
    bool nexto;
};

#define EXP_NUMB        0x1
#define EXP_BOOL        0x2
#define EXP_NEXTO       0x4
#define EXP_BOOL_NEXTO  0x6

#define EXP_ISNEXTO(x)  x & 0x4 == 0x4
#define GET_BTYPE_WIDTH(x) (x < 0) ? 4 : utypes[x].val.width

#define LVALUE          1

extern struct symbols *env[];

void sem_anal(struct syntree*);
extern char* sem_err[];

/********
 *  IR  *
 ********/
extern unsigned int tvar_cnt;
extern unsigned int uvar_cnt;
extern unsigned int label_cnt;

extern char filename[];
/**************** 
 * debug switch *
 ****************/

/*
#define LEX_DEBUG
#define LEX_ONLY
*/

