%{
#include <stdio.h>
#include "compiler.h"

void iropt();
extern int line_no;
int err = 0;
extern struct irb head;
%}

%union{
    struct syntree *grand;
    int intval;
    float fval;
}


%token INT FLOAT SEMI COMMA ASSIGNOP RELOP 
%token PLUS MINUS STAR DIV AND OR 
%token DOT NOT TYPE LP RP LB 
%token RB LC RC STRUCT RETURN IF 
%token ELSE WHILE ID EXTERN

%token READ WRITE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UNARY_MINUS
%right NOT 
%left LP RP LB RB DOT

%left level0
%left level1
%left level2

%nonassoc IMCP_IF
%nonassoc ELSE

%type <grand> Program
%type <grand> ExtDefList
%type <grand> ExtDef
%type <grand> ExtDec
%type <grand> ExtDecList
%type <grand> Specifier
%type <grand> StructSpecifier
%type <grand> OptTag
%type <grand> Tag
%type <grand> VarDec
%type <grand> FunDec
%type <grand> VarList
%type <grand> ParamDec
%type <grand> CompSt
%type <grand> StmtList
%type <grand> Stmt
%type <grand> DefList
%type <grand> Def
%type <grand> DecList
%type <grand> Dec
%type <grand> Exp
%type <grand> Args
%type <grand> INT
%type <grand> FLOAT
%type <grand> SEMI
%type <grand> COMMA
%type <grand> ASSIGNOP
%type <grand> RELOP
%type <grand> PLUS
%type <grand> MINUS
%type <grand> STAR
%type <grand> DIV
%type <grand> AND
%type <grand> OR
%type <grand> DOT
%type <grand> NOT
%type <grand> TYPE
%type <grand> LP
%type <grand> RP
%type <grand> LB
%type <grand> RB
%type <grand> LC
%type <grand> RC
%type <grand> STRUCT
%type <grand> RETURN
%type <grand> IF
%type <grand> ELSE
%type <grand> WHILE
%type <grand> EXTERN
%type <grand> ID

%type <grand> READ
%type <grand> WRITE


%%
Program     :    ExtDefList 
			{
				$$ = create_syn(term);
				$$->val = TK_Program;
				$$->line = $1->line;
				$$->chld = $1;
                              //  syntree_visit($$, 0);
                                if (!err)
                                    sem_anal($$);
//                                printf("sematics ok\n");
//                                if (!err) ir_print();
			}
            ;


ExtDefList  :    ExtDef ExtDefList 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDefList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |    ExtDec ExtDefList
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDefList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDefList;
			}
            ;


ExtDec      :    EXTERN Specifier ExtDecList SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            |    Specifier FunDec SEMI
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    EXTERN error SEMI
			{
				$$ = create_esyn();
			}
            |    error Specifier SEMI
			{
				$$ = create_esyn();
			}
            |    error FunDec SEMI
			{
				$$ = create_esyn();
			}
            ;


ExtDef      :    Specifier ExtDecList SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDef;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Specifier SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDef;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |    Specifier FunDec CompSt 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDef;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    error SEMI
			{
				$$ = create_esyn();
			}
            |    Specifier ExtDecList error 
			{
				$$ = create_esyn();
			}
            |    Specifier error CompSt
			{
				$$ = create_esyn();
			}
            ;


ExtDecList  :    VarDec 
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDecList;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    VarDec COMMA ExtDecList
			{
				$$ = create_syn(term);
				$$->val = TK_ExtDecList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            ;


Specifier   :    TYPE 
			{
				$$ = create_syn(term);
				$$->val = TK_Specifier;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    StructSpecifier 
			{
				$$ = create_syn(term);
				$$->val = TK_Specifier;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            ;


StructSpecifier :    STRUCT OptTag LC DefList RC 
			{
				$$ = create_syn(term);
				$$->val = TK_StructSpecifier;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; $4->sib = $5; 
			}
            |    STRUCT Tag 
			{
				$$ = create_syn(term);
				$$->val = TK_StructSpecifier;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            ;


OptTag      :    ID 
			{
				$$ = create_syn(term);
				$$->val = TK_OptTag;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |     
			{
				$$ = create_syn(term);
				$$->val = TK_OptTag;
			}
            ;


Tag         :    ID 
			{
				$$ = create_syn(term);
				$$->val = TK_Tag;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            ;


VarDec      :    ID 
			{
				$$ = create_syn(term);
				$$->val = TK_VarDec;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    VarDec LB INT RB 
			{
				$$ = create_syn(term);
				$$->val = TK_VarDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            ;


FunDec      :    ID LP VarList RP 
			{
				$$ = create_syn(term);
				$$->val = TK_FunDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            |    ID LP RP 
			{
				$$ = create_syn(term);
				$$->val = TK_FunDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            ;


VarList     :    ParamDec COMMA VarList 
			{
				$$ = create_syn(term);
				$$->val = TK_VarList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    ParamDec 
			{
				$$ = create_syn(term);
				$$->val = TK_VarList;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            ;


ParamDec    :    Specifier VarDec 
			{
				$$ = create_syn(term);
				$$->val = TK_ParamDec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            ;


CompSt      :    LC DefList StmtList RC 
			{
				$$ = create_syn(term);
				$$->val = TK_CompSt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            |    error DefList StmtList RC
			{
				$$ = create_esyn();
			}
            |    LC DefList StmtList error
			{
				$$ = create_esyn();
			}
            |    error DefList StmtList error
			{
				$$ = create_esyn();
			}
            ;


StmtList    :    Stmt StmtList 
			{
				$$ = create_syn(term);
				$$->val = TK_StmtList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |     
			{
				$$ = create_syn(term);
				$$->val = TK_StmtList;
			}
            ;


Stmt        :    Exp SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |    CompSt 
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    RETURN Exp SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    IF LP Exp RP Stmt 
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; $4->sib = $5; 
			}
            |    IF LP Exp RP Stmt ELSE Stmt
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; $4->sib = $5; $5->sib = $6; $6->sib = $7; 
			}
            |    WHILE LP Exp RP Stmt 
			{
				$$ = create_syn(term);
				$$->val = TK_Stmt;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; $4->sib = $5; 
			}
            |    error SEMI
			{
				$$ = create_esyn();
			}
            |    IF error Stmt  %prec IMCP_IF
			{
				$$ = create_esyn();
			}
            |    IF error Stmt ELSE Stmt 
			{
				$$ = create_esyn();
			}
            |    WHILE error Stmt 
			{
				$$ = create_esyn();
			}
            |    error RP Stmt
			{
				$$ = create_esyn();
			}
            |    error Exp Stmt
			{
				$$ = create_esyn();
			}
            ;


DefList     :    Def DefList 
			{
				$$ = create_syn(term);
				$$->val = TK_DefList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |     
			{
				$$ = create_syn(term);
				$$->val = TK_DefList;
			}
            ;


Def         :    Specifier DecList SEMI 
			{
				$$ = create_syn(term);
				$$->val = TK_Def;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    error SEMI
			{
				$$ = create_esyn();
			}
            |    Specifier error SEMI
			{
				$$ = create_esyn();
			}
            |    Specifier DecList error
			{
				$$ = create_esyn();
			}
            ;


DecList     :    Dec 
			{
				$$ = create_syn(term);
				$$->val = TK_DecList;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    Dec COMMA DecList 
			{
				$$ = create_syn(term);
				$$->val = TK_DecList;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    error DefList
			{
				$$ = create_esyn();
			}
            ;


Dec         :    VarDec 
			{
				$$ = create_syn(term);
				$$->val = TK_Dec;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    VarDec ASSIGNOP Exp
			{
				$$ = create_syn(term);
				$$->val = TK_Dec;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            ;


Exp         :    Exp ASSIGNOP Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp AND Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp OR Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp RELOP Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp PLUS Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp MINUS Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp STAR Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp DIV Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    LP Exp RP 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    MINUS Exp          %prec UNARY_MINUS
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |    NOT Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; 
			}
            |    ID LP Args RP 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            |    ID LP RP 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp LB Exp RB 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; $3->sib = $4; 
			}
            |    Exp DOT ID 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    ID 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    INT 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    FLOAT 
			{
				$$ = create_syn(term);
				$$->val = TK_Exp;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            |    READ LP RP
                        {
                                $$ = create_syn(term);
                                $$->val = TK_Exp;
                                $$->line = $1->line;
                                $$->chld = $1;
                                $1->sib = $2; $2->sib = $3;
                        }
            |    WRITE LP Exp RP
                        {
                                $$ = create_syn(term);
                                $$->val = TK_Exp;
                                $$->line = $1->line;
                                $$->chld = $1;
                                $1->sib = $2; $2->sib = $3; $3->sib = $4;
                        }
            ;


Args        :    Exp COMMA Args 
			{
				$$ = create_syn(term);
				$$->val = TK_Args;
				$$->line = $1->line;
				$$->chld = $1;
				$1->sib = $2; $2->sib = $3; 
			}
            |    Exp 
			{
				$$ = create_syn(term);
				$$->val = TK_Args;
				$$->line = $1->line;
				$$->chld = $1;
				
			}
            ;
%%

int main(int argc, char** argv)
{
    int i;
    if (argc < 2) {
        printf("no input files\n");
        return 1;
    }
    if (argc < 3) {
        printf("no output files\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    strcpy(filename, argv[2]);
    if (f == null) {
        printf("error when open file\n");
        return 1;
    }
    sem_init();
    ir_init();

    yyrestart(f);
    yyparse();

    if (!err) {
        iropt();
        asm_main();
    }
//    st_visit(env[0]);
}

yyerror(char *s)
{
    err = 1;
    fprintf(stderr, "Error type 2 at line %d: %s\n", line_no, s);
}

inline struct syntree* create_syn(char type)
{
    struct syntree* node;
    node = (struct syntree*)malloc(sizeof(struct syntree));
    node->chld = null; node->sib = null;
    node->line = line_no;
    node->type = type;
    return node;
}

inline struct syntree* create_esyn()
{
    struct syntree* node;
    node = (struct syntree*)malloc(sizeof(struct syntree));
    node->chld = null; node->sib = null;
    node->line = line_no;
    node->val = TK_error;
    node->type = nterm;
    err = 1;
    return node;
}

void syntree_visit(struct syntree *p, int blank)
{
    struct syntree *q;
    int i;


    //printf("%s\n", p->val);
    
    if (p->type == term) {
        if (p->chld != null) {
            for (i = 0; i < blank; i++) printf("%c", ' ');
            printf("%s (%d)\n", token_str[p->val], p->line);
            q = p->chld;

            while (q != null) {
                syntree_visit(q, blank+2);
                q = q->sib;
            }
        }

    } else {
        for (i = 0; i < blank; i++) printf("%c", ' ');
        printf("%s", token_str[p->val]);
        if (p->val == TK_TYPE || p->val == TK_INT ||
            p->val == TK_FLOAT || p->val == TK_ID)
            printf(": %s", p->cont);
        printf("\n");
    } 
}
