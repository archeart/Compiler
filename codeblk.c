
void bb_entry(int bid)
{
    struct bbpre_link *p;
    int i, j, k;

    p = cbbs[bid].preb;
    if (p == NULL)
        return ;

    while (p != NULL) {
        j = p->cbb;
        if (!cbbs[j].transed) {
            return;
        }
        p = p->next;
    }

    p = cbbs[bid].preb;
    memcpy(cbbs[bid].env, cbbs[p->cbb].env, TREG_CNT * sizeof(struct regb));

    p = p->next;
    while (p != NULL) {
        j = p->cbb;
        for (i = 0; i < TREG_CNT; i ++)
            if (cbbs[bid].env[i].vid == cbbs[j].env[i].vid)
                cbbs[bid].env[i].dirty |= cbbs[j].env[i].dirty;
            else
                cbbs[bid].env[i].vid = 0;
        p = p->next;
    }

    for (i = 0; i < TREG_CNT; i++) {
        j = cbbs[bid].env[i].vid;
        if (j != 0)
            vars[j].rid = i + TREG_BEG;
    }

    cbbs[bid].transed = true;
}

int bb_exit(int bid, int line)
{
    int i, vid, off;
    int extra = 0;

    /*
     *  this may add to reduce the store operation
     
    if (cbbs[bid].exit)
        goto store;

    if (cbbs[bid].exit2 == 0 && cbbs[bid].exit1 != 0) {
        i = cbbs[bid].exit1;
        if (cbbs[
    }
    */

store:

    for (i = 0; i < TREG_CNT; i ++) {
        vid = cbbs[bid].env[i].vid;
        if (vid == 0)
            continue;
        vars[vid].rid = 0;

        if (cbbs[bid].env[i].dirty) {

            if (vars[vid].type == IR_TempVar) {
                if (line >= vars[vid].last_used)
                    continue;

                if (!vars[vid].mm_alloc)
                    stack_alloc(vid);
            }

            fr_append(inst_sw, i + TREG_BEG, $fp, vars[vid].off);
            cbbs[bid].env[i].dirty = 0;
        }
    }
/*
    if (cbbs[bid].exit)
        fr_append(inst_j, func_cnt, 0, (int)"exit");
        */
        
}

void bb_add_pre(int bid, int fid)
{
    struct bbpre_link *p, *q;
    int i, j;

    /* father contains return */
    if (fid == 0 || cbbs[fid].exit)
        return ;

//    fprintf(stderr, "[[add_pre bid]]: %d, fid: %d\n", bid, fid);

    if (cbbs[bid].scanned == true) {
        i = cbbs[bid].exit1;
        if (cbbs[i].scanned == true)
            i = cbbs[bid].exit1;
//        printf("[loop] %d %d \n", bid, i);
        cbbs[i].loop_lock = -bid;
        cbbs[bid].loop_lock = i;
    }

    /* TODO */
    p = (struct bbpre_link*) malloc (sizeof(struct bbpre_link));
    p->cbb = fid;

    p->next = cbbs[bid].preb;
    cbbs[bid].preb = p;
}

void bb_init_block(int bid, int fid)
{

    static blk_cnt = 1;
//    fprintf(stderr, "[%d]init block%d \n", blk_cnt ++, bid);
    assert(!cbbs[bid].scanned);

    if (cbbs[fid].exit1 == 0)
        cbbs[fid].exit1 = bid;
    else
        cbbs[fid].exit2 = bid;

    bb_add_pre(bid, fid);

    cbbs[bid].scanned = true;
}
