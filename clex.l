%{
#include <string.h>

#include "syntax.tab.h"
#include "compiler.h"

void oct_dec(char*, char**);
void hex_dec(char*, char**);

extern int err;
%}

%option noyywrap

ALPHA       [a-zA-Z]
CHARA       [0-9a-zA-Z_]
HCHAR       [0-9a-fA-F]
OCHAR       [0-7]


HEX         0x[1-9a-fA-F]{HCHAR}+
OCT         0[1-7]{OCHAR}*
DEC         0|([1-9][0-9]*)


PFLOAT      {DEC}\.[0-9]*
SFLOAT      ({DEC}|{PFLOAT})[eE]{DEC}
FLOAT       {PFLOAT}|{SFLOAT}

INT         {DEC}|{HEX}|{OCT}

ID          {ALPHA}{CHARA}*

SEMI        ;
COMMA       ,
ASSIGNOP    =
RELOP       >|<|>=|<=|==|!=
PLUS        \+
MINUS       -
STAR        \*
DIV         \/
AND         &&
OR          \|\|
DOT         \.
NOT         !
TYPE        (int|float)
LP          \(
RP          \)
LB          \[
RB          \]
LC          \{
RC          \}
STRUCT      struct
RETURN      return
IF          if
ELSE        else
WHILE       while
EXTERN      extern
READ        read
WRITE       write

    /*  counter */
    int line_no = 1;

%%
{DEC}		{
#ifdef LEX_DEBUG
			printf("%s: %s\n", "INT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_INT;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return INT;
#endif
		}

{OCT}		{
#ifdef LEX_DEBUG
			printf("%s: %s\n", "INT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_INT;
                        oct_dec(yytext, &(yylval.gcont));
			return INT;
#endif
		}
{HEX}		{
#ifdef LEX_DEBUG
			printf("%s: %s\n", "INT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_INT;
                        hex_dec(yytext, &yylval.gcont);
			return INT;
#endif
		}


{FLOAT}		{
#ifdef LEX_DEBUG
			printf("%s: %s\n", "FLOAT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_FLOAT;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return FLOAT;
#endif
		}

{SEMI}		{
#ifdef DEBUG
			printf("%s: %s\n", "SEMI", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_SEMI;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return SEMI;
#endif
		}


{COMMA}		{
#ifdef DEBUG
			printf("%s: %s\n", "COMMA", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_COMMA;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return COMMA;
#endif
		}


{ASSIGNOP}		{
#ifdef DEBUG
			printf("%s: %s\n", "ASSIGNOP", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_ASSIGNOP;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return ASSIGNOP;
#endif
		}


{RELOP}		{
#ifdef DEBUG
			printf("%s: %s\n", "RELOP", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_RELOP;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return RELOP;
#endif
		}


{PLUS}		{
#ifdef DEBUG
			printf("%s: %s\n", "PLUS", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_PLUS;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return PLUS;
#endif
		}


{MINUS}		{
#ifdef DEBUG
			printf("%s: %s\n", "MINUS", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_MINUS;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return MINUS;
#endif
		}


{STAR}		{
#ifdef DEBUG
			printf("%s: %s\n", "STAR", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_STAR;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return STAR;
#endif
		}


{DIV}		{
#ifdef DEBUG
			printf("%s: %s\n", "DIV", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_DIV;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return DIV;
#endif
		}


{AND}		{
#ifdef DEBUG
			printf("%s: %s\n", "AND", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_AND;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return AND;
#endif
		}


{OR}		{
#ifdef DEBUG
			printf("%s: %s\n", "OR", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_OR;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return OR;
#endif
		}


{DOT}		{
#ifdef DEBUG
			printf("%s: %s\n", "DOT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_DOT;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return DOT;
#endif
		}


{NOT}		{
#ifdef DEBUG
			printf("%s: %s\n", "NOT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_NOT;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return NOT;
#endif
		}


{TYPE}		{
#ifdef DEBUG
			printf("%s: %s\n", "TYPE", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_TYPE;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return TYPE;
#endif
		}


{LP}		{
#ifdef DEBUG
			printf("%s: %s\n", "LP", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_LP;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return LP;
#endif
		}


{RP}		{
#ifdef DEBUG
			printf("%s: %s\n", "RP", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_RP;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return RP;
#endif
		}


{LB}		{
#ifdef DEBUG
			printf("%s: %s\n", "LB", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_LB;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return LB;
#endif
		}


{RB}		{
#ifdef DEBUG
			printf("%s: %s\n", "RB", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_RB;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return RB;
#endif
		}


{LC}		{
#ifdef DEBUG
			printf("%s: %s\n", "LC", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_LC;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return LC;
#endif
		}


{RC}		{
#ifdef DEBUG
			printf("%s: %s\n", "RC", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_RC;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return RC;
#endif
		}


{STRUCT}		{
#ifdef DEBUG
			printf("%s: %s\n", "STRUCT", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_STRUCT;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return STRUCT;
#endif
		}


{RETURN}		{
#ifdef DEBUG
			printf("%s: %s\n", "RETURN", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_RETURN;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return RETURN;
#endif
		}


{IF}		{
#ifdef DEBUG
			printf("%s: %s\n", "IF", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_IF;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return IF;
#endif
		}


{ELSE}		{
#ifdef DEBUG
			printf("%s: %s\n", "ELSE", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_ELSE;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return ELSE;
#endif
		}


{READ}          {
                        yylval.grand = create_syn(nterm);
                        yylval.gval = TK_READ;
                        return READ;
                }

{WRITE}         {
                        yylval.grand = create_syn(nterm);
                        yylval.gval = TK_WRITE;
                        return WRITE;
                }

{WHILE}		{
#ifdef DEBUG
			printf("%s: %s\n", "WHILE", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_WHILE;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return WHILE;
#endif
		}


{EXTERN}		{
#ifdef DEBUG
			printf("%s: %s\n", "EXTERN", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_EXTERN;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return EXTERN;
#endif
		}


{ID}		{
#ifdef DEBUG
			printf("%s: %s\n", "ID", yytext);
#endif

#ifndef LEX_ONLY
			yylval.grand = create_syn(nterm);
			yylval.gval = TK_ID;
			yylval.gcont = (char*)malloc(yyleng+1);
			memcpy(yylval.gcont, yytext, yyleng+1);
			return ID;
#endif
		}

\/\/.*\n        {
                }

\/\*            {
                        char flag = 0;
                        char c = input();
                        while (c != EOF && flag == 0) {
                            if (c == '/') {
                                if (flag == '*')
                                    break;
                                else flag = c;
                            }
                            else if (c == '*' && flag == '/') {
                                printf("Error type 3 at line %d: invalid string in comments\n", line_no);  //ERROR
                                err = 1;
                            }
                            else if (c == '\n')
                                line_no ++;

                            c = input();
                        }

                        if (!flag) {
                            printf("Error type 3 at line %d: comments unfinished\n", line_no);
                            err = 1;
                        }

                }

\n              { line_no ++; }
[ \t] 
.               {
                    printf("Error type 1 at line %d: mysterious input \"%s\"\n", line_no, yytext);
                    err = 1;
                }
%%

void itoa(int num, char* dest)
{
    char str[20];
    int i, len = 0;

    while (num != 0) {
        str[len] = num % 10 + '0';
        len ++;
        num /= 10;
    }

    for (i = 0; i < len; i++)
        dest[i] = str[len-1-i];
    dest[len] = '\0';
}

void oct_dec(char* oct, char** res)
{
    int ans = 0;
    oct ++;
    while (*oct != '\0') {
        ans = ans * 8 + *oct - '0';
        oct ++;
    }
    printf("ans: %d\n", ans);
    *res = (char*)malloc(20);
    itoa(ans, *res);
}

void hex_dec(char* hex, char** res)
{
    int ans = 0;
    hex += 2;
    while (*hex != '\0') {
        if (isdigit(*hex))
            ans = ans * 16 + *hex -'0';
        else
            ans = ans * 16 - *hex -'a';
        hex ++;
    }
    *res = (char*)malloc(24);
    itoa(ans, *res);
}

void sci_float(char* sci, char** res)
{
    float ans = 0;
}
