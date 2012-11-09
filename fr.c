

struct frb fr_head, *fr_tail;
struct frb *fr_jump;


void fr_init() 
{
    fr_head.next = NULL;
    fr_tail = NULL;
}

void fr_append(enum fr_type type, int op1, int op2, int op3)
{
    struct frb *p;

    p = (struct frb*) malloc (sizeof(struct frb));
    p->type = type;
    p->op1 = op1;
    p->op2 = op2;
    p->op3 = op3;
    p->next = NULL;

    if (fr_tail == NULL) {
        fr_tail = p;
        fr_head.next = p;
    }
    else {
        fr_tail->next = p;
        fr_tail = p;
    }
}

void fr_jump_set()
{
    fr_jump = NULL;
}

void fr_jump_append(enum fr_type type, int op1, int op2, int op3)
{
    struct frb *p;

    p = (struct frb*) malloc (sizeof(struct frb));
    p->type = type;
    p->op1 = op1;
    p->op2 = op2;
    p->op3 = op3;
    p->next = NULL;

    if (fr_jump == NULL)
        fr_jump = p;
    else {
        assert(fr_jump->next == NULL);
        fr_jump->next = p;
    }
}

void fr_jump_link()
{
    fr_tail->next = fr_jump;
    while (fr_tail->next != NULL)
        fr_tail = fr_tail->next;
}

struct frb* fr_getlast()
{
    return fr_tail;
}

void fr_print()
{
    struct frb *it;
    it = fr_head.next;

    while (it != NULL) {
        switch (it->type) {
            case inst_add: case inst_sub: case inst_mul:
                fprintf(out, "\t%s %s, %s, %s\n", inst_str[it->type],
                    regs_str[it->op1], regs_str[it->op2], regs_str[it->op3]);
                break;

            case inst_addi:
                fprintf(out, "\t%s %s, %s, %d\n", "addi",
                    regs_str[it->op1], regs_str[it->op2], it->op3);
                break;

            case inst_div: case inst_move:
                fprintf(out, "\t%s %s, %s\n", inst_str[it->type],
                    regs_str[it->op1], regs_str[it->op2]);
                break;

            case inst_mflo: case inst_mfhi:
            case inst_jr:
                fprintf(out, "\t%s %s\n", inst_str[it->type],
                    regs_str[it->op1]);
                break;

            case inst_label:
                if (it->name != NULL) {
                    if (it->op1 != 0)
                        fprintf(out, "%s%d:\n", it->name, it->op1);
                    else
                        fprintf(out, "%s:\n", it->name);
                }
                else
                    fprintf(out, "label%d:\n", it->op1);
                break;
            
            case inst_lw: case inst_sw:
                fprintf(out, "\t%s %s, %d(%s)\n", inst_str[it->type],
                    regs_str[it->op1], it->op3, regs_str[it->op2]);
                break;

            case inst_li:
                fprintf(out, "\tli %s, %d\n", regs_str[it->op1], it->op2);
                break;

            case inst_j:
                if (it->name != NULL)
                    fprintf(out, "\tj %s%d\n", it->name, it->op1);
                else
                    fprintf(out, "\tj label%d\n", it->op1);
                break;

            case inst_jal:
                fprintf(out, "\tjal %s\n", it->name);
                break;

            case inst_beq: case inst_bne: case inst_bgt:
            case inst_blt: case inst_bge: case inst_ble:
                fprintf(out, "\t%s %s, %s, label%d\n", inst_str[it->type],
                    regs_str[it->op1], regs_str[it->op2], it->op3);
                break;

            default:
                fprintf(out, "\t%s\n", inst_str[it->type]);
        }

        it = it->next;
    }

    printf("\n");
}
