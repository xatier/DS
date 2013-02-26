#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// data node
struct node {
    int index;
    int weight;
    struct node *l;
    struct node *r;
    int code;
};

// record field frome the Huft.txt file
struct record {
    int index;
    int weight;
    int code;
    double prob;
    char *str;
};

// data node constructure
struct node *new_node (const int w, const int i,
                       struct node *l, struct node *r) {
    struct node *n = (struct node *)malloc(sizeof(struct node));
    n->weight = w;
    n->index = i;
    n->l = l;
    n->r = r;
    n->code = 0;
    return n;
}

// data node destructure
void del_node (struct node *n) {
    if (n) {
        del_node(n->l);
        del_node(n->r);
        free(n);
    }
}

char *bit2str (int n, const int depth) {
    char *str = (char *)malloc(depth + 1);
    memset(str, '0', depth);
    for (int i = depth-1; i >= 0; i--) {
        if (n & 1)
            str[i] = '1';
        n >>= 1;
    }

    str[depth] = '\0';
    return str;
}

void print_node (const struct node *n, const int depth) {
    if (n != NULL) {
        if (n->index != -1) {
            char *str = bit2str(n->code, depth);
            printf("[%d]: %d(%02x|%d)[c:%s(%d)]\n", depth, n->weight, n->index, n->index, str, n->code);
            free(str);
        }
        print_node(n->l, depth+1);
        print_node(n->r, depth+1);
    }
}

void set_node_code (struct node *n, const int code, struct record *r, const int depth) {
    if (n != NULL) {
        n->code = code;

        if (n->index != -1) {
            r[n->index].index = n->index;
            r[n->index].weight = n->weight;
            r[n->index].code = n->code;
            r[n->index].str = bit2str(n->code, depth);
        }

        set_node_code(n->l, code * 2 + 0, r, depth +1);
        set_node_code(n->r, code * 2 + 1, r, depth +1);
    }
}

// just another heap structure (X
struct heap {
    struct node **arr;
    int nodes;
    int size;
};

// heap constructure
struct heap *init_heap (void) {
    struct heap *h = (struct heap *)malloc(sizeof(struct heap));
    h->size = 1000000;
    h->arr = (struct node **)malloc(sizeof(struct node *) * h->size);
    h->nodes = 0;
    return h;
}

// heap destructure
void del_heap (struct heap *h) {
    for (int i = 0; i < h->nodes; i++)
        del_node(h->arr[i]);
    free(h->arr);
    free(h);
}

void print_heap (const struct heap *h) {
    for (int i = 0; i < h->nodes; i++)
        printf("%d, ", h->arr[i]->weight);
    printf("=> %d nodes\n", h->nodes);
}

// make a heap from an array data
void mk_heap (struct heap *h, const int *arr, const int n) {
    for (int i = 0; i < n; i++) {
        h->arr[h->nodes++] = new_node(arr[i], i, NULL, NULL);
        int l = h->nodes - 1;
        int p = (l-1) / 2;
        while (l >= 0 && h->arr[l]->weight < h->arr[p]->weight) {
            struct node *tmp = h->arr[l];
            h->arr[l] = h->arr[p];
            h->arr[p] = tmp;
            l = p;
            p = (l-1) / 2;
        }
    }
}

// remove the root node from the heap
struct node *rm_root (struct heap *h) {
    struct node *r = h->arr[0];
    int l = --(h->nodes);
    h->arr[0] = h->arr[l];
    l = 0;
    struct node *c1 = h->arr[l*2 + 1];
    struct node *c2 = h->arr[l*2 + 2];
    int c = c1->weight <= (c2->weight) ?  l*2 + 1 : l*2 +2;

    while (c < h->nodes && h->arr[c]->weight < h->arr[l]->weight) {
        struct node *tmp = h->arr[l];
        h->arr[l] = h->arr[c];
        h->arr[c] = tmp;
        l = c;
        if (l*2 +1 >= h->nodes)
            break;
        c1 = h->arr[l*2 + 1];
        c2 = h->arr[l*2 + 2];
        c = c1->weight <= (c2->weight) ?  l*2 + 1 : l*2 +2;
    }
    return r;
}

// insert a node from the end of the heap
void insert_end (struct heap *h, struct node *n) {
    h->arr[h->nodes++] = n;
    int l = h->nodes - 1;
    int p = (l-1) / 2;
    while (l >= 0 && h->arr[l]->weight < h->arr[p]->weight) {
        struct node *tmp = h->arr[l];
        h->arr[l] = h->arr[p];
        h->arr[p] = tmp;
        l = p;
        p = (l-1) / 2;
    }
}

