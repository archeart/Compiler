#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
extern struct irb head, *tail;

#define bool unsigned char 
#define true    1
#define false   0

void bb_add_pre(int, int);
void bb_init_block(int, int);
