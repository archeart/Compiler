#include "compiler.h"

#define MAX_ANONYFUNC 99

char* anony_gen()
{
    static int id = 0;
    int x, i;
    char *prefix;

    prefix = (char*) malloc (9);
    id ++;
    x = id; i = 0;
    if (id > MAX_ANONYFUNC)
        return 0;

    while (x) {
        prefix[i] = (x % 10) + '0';
        i ++;
        x = x / 10;
    }
    strcpy(&prefix[i], "anony");  
    return prefix;
}

bool dlink_cmp(struct dlink*, struct dlink*);
bool type_check(int key1, int key2)
{
    struct user_type *t1, *t2;
    if (key1 == key2)
        return true;

    else if (key1 >= 0 && key2 >= 0) {
        t1 = &utypes[key1].val;
        t2 = &utypes[key2].val;

        if (t1->type == t2->type && t1->type > 3)
            if (t1->ref == t2->ref) 
                if (dlink_cmp(t1->arr_attr, t2->arr_attr))
                    return true;
    }

    return false;
}

bool dlink_cmp(struct dlink *dp1, struct dlink *dp2)
{
    while (dp1 != null && dp2 != null) {
        dp1 = dp1->next;
        dp2 = dp2->next;
    }
    if (dp1 != null || dp2 != null)
        return false;

    return true;
}

void slink_free(struct slink *dp)
{
    struct slink *p;
    while (dp != null) {
        p = dp;
        dp = dp->next;
        free(p);
    }
}

bool slink_cmp(struct slink *sp1, struct slink *sp2)
{
//    printf("slink cmp:\n");
    while (sp1 != null && sp2 != null) {
//     printf(" %d %d\n", sp1->hash_key, sp2->hash_key);
        if (!type_check(sp1->hash_key, sp2->hash_key))
            return false;
        sp1 = sp1->next;
        sp2 = sp2->next;
    }

    if (sp1 != null || sp2 != null)
        return false;
    return true;
}

void slink_vis(struct slink *sp)
{
    printf("slink_walk: ");
    while (sp != null) {
        printf( "%d. ", sp->hash_key);
        sp = sp->next;
    }
    printf("\n");
}

void dlink_vis(struct dlink *dp)
{
    printf("dlink_walk: ");
    while (dp != null) {
        printf("[%d]", dp->dim);
        dp = dp->next;
    }

    printf("\n");


}

void display_symbols(struct symbols *x)
{
    printf("id: %s; hash_key: %d; imp: %d\n", x->id, x->hash_key, x->flag);
}

void display_type(int ind, int blank)
{
    int i;
    struct user_type *utype;
    struct slink* sp;
    struct dlink* ap;

    for (i = 0; i < blank; i++) printf(" ");
   
    if (ind == -1) {
        printf("int\n");
        return ;
    }
    else if (ind == -2) {
        printf("float\n");
        return ;
    }

    utype = &utypes[ind].val;
    printf("id: %s; type: %d\n", utype->id, utype->type);

    sp = utype->str_attr;
    while (sp != null) {
        for (i = 0; i < blank; i++) printf(" ");
        printf("%s\n", sp->id);
        display_type(sp->hash_key, blank + 2);
        sp = sp->next;
    }

    ap = utype->arr_attr;
    if (ap != null) {
        for (i = 0; i < blank; i++) printf(" ");
        while (ap != null) {
            printf("[%d]", ap->dim);
            ap = ap->next;
        }
        printf("\n");
        display_type(utype->ref, blank + 2);
    }
}

void st_visit(struct symbols *node)
{
    if (node == null)
        return;
    
    printf("symbol id: %s\n", node->id);
    printf("  symbols type: \n");
    display_type(node->hash_key, 4);
    st_visit(node->lchd);
    st_visit(node->rchd);
}


inline void eval_fill(struct eval* ret, int key, struct dlink *dp, bool left)
{
    ret->hash_key = key;
    ret->cur = dp;
    ret->left = left;
}

inline bool typeck_bu(int key1, int key2) 
{
//    printf("@typeck_bu, %d %d\n", key1, key2);
    if (key1 == -3 || key2 == -3)
        return true;
//    if (key1 < 0 && key2 < 0)
 //       return true;
    else if (key1 == key2)
        return true;

    return false;
}

inline bool is_array(int key)
{
    if (key < 0)
        return false;
    if (utypes[key].val.arr_attr == null)
        return false;

    return true;
}

struct slink* str_member(int key, char *id)
{
    struct slink* sp;
    sp = utypes[key].val.str_attr;

    while (sp != null) {
        if (strcmp(sp->id, id) == 0)
            break;
        sp = sp->next;
    }

    return sp;
}
