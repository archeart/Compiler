
void asm_debug()
{
    struct irb *it = head.next;
    struct bbpre_link *p;
    int i;
    extern int uvar_cnt;

    while (it != NULL) {
        if (it->bid != 0) {
            fprintf(stderr, "new block from code_no: %d\n", it->bid);
            i = it->bid;

            if (cbbs[i].loop_lock > 0)
                fprintf(stderr, "loop entry\n");
            if (cbbs[i].loop_lock < 0)
                fprintf(stderr, "loop exit\n");


            fprintf(stderr, "entry: ");
            p = cbbs[i].preb;
            while (p != NULL) {
                fprintf(stderr, "%d ", p->cbb);
                p = p->next;
            }
            fprintf(stderr, "\n");

            fprintf(stderr, "exit: %d %d\n", cbbs[i].exit1, cbbs[i].exit2);
            fprintf(stderr, "ret: %d\n", cbbs[i].exit);

        }
        it = it->next;
    }


    for (i = 0; i < 40; i ++) {
        if (vars[i].type == IR_DefVar)
            fprintf(stderr, "id%d DefVar: line %d, off %d\t", i, vars[i].last_used, vars[i].off);
        else if (vars[i].type == IR_TempVar)
            fprintf(stderr, "id%d TempVar: line %d, off %d\t", i, vars[i].last_used, vars[i].off);
        if (i % 3 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    for (i = 0; i < 40; i ++) {
        if (vars[i].mm_alloc)
            fprintf(stderr, "var%d\t", i);
        if (i % 8 == 0)
            fprintf(stderr, "\n");
    }

    fr_print();
}


