#include <inttypes.h>
#include <stdlib.h>
#include <strings.h>
#ifdef MAIN
#include <stdio.h>
#endif

#define RBBLACK 'b'
#define RBRED   'r'

typedef struct rbnode {
    char color;
    uint64_t key;
    struct rbnode *p;
    struct rbnode *left;
    struct rbnode *right;
} *rbnode_t;

typedef struct rbtree {
    rbnode_t root;
    rbnode_t Tnil;
} *rbtree_t;

rbtree_t rb_create();
void rb_node_destroy(rbnode_t x);
void rb_left_rotate(rbtree_t T, rbnode_t y);
void rb_rotate_right(rbtree_t T, rbnode_t x);
void rb_insert(rbtree_t T, rbnode_t z);
void rb_insert_fixup(rbtree_t T, rbnode_t z);
void rb_transplant(rbtree_t T, rbnode_t u, rbnode_t v);
void rb_delete(rbtree_t T, rbnode_t z);
void rb_delete_fixup(rbtree_t T, rbnode_t x);
rbnode_t rb_node_create(rbnode_t Tnil);
rbnode_t rb_tree_minimum(rbtree_t T, rbnode_t x);
rbnode_t rb_tree_maximum(rbtree_t T, rbnode_t x);
rbnode_t rb_search(rbtree_t T, uint64_t k);
void rb_iterate(rbtree_t T, int(*)(rbnode_t));
void rb_iter(rbnode_t x, rbnode_t Tnil, int(*)(rbnode_t));

void rb_iterate(rbtree_t T, int(*callback)(rbnode_t))
{
    rb_iter(T->root, T->Tnil, callback);
}

void rb_iter(rbnode_t x, rbnode_t Tnil, int(*callback)(rbnode_t))
{
    if (x->left != Tnil)
        rb_iter(x->left, Tnil, callback);
    callback(x);
    if (x->right != Tnil)
        rb_iter(x->right, Tnil, callback);
}

rbnode_t rb_node_create(rbnode_t Tnil)
{
    rbnode_t rbnode = (rbnode_t)malloc(sizeof(struct rbnode));
    bzero(rbnode, sizeof(struct rbnode));
    rbnode->color = RBBLACK;
    rbnode->left = Tnil;
    rbnode->right = Tnil;
    rbnode->p = Tnil;
    return rbnode;
}

void rb_node_destroy(rbnode_t x)
{
    free(x);
}

rbtree_t rb_create() {
    rbtree_t T = (rbtree_t)malloc(sizeof(struct rbtree));
    bzero(T, sizeof(struct rbtree));
    T->Tnil = rb_node_create(NULL);
    T->Tnil->p = T->Tnil;
    T->Tnil->left = T->Tnil;
    T->Tnil->right = T->Tnil;
    T->root = T->Tnil;
    return T;
}

void rb_destroy(rbtree_t T)
{
    while(T->root != T->Tnil) {
        rbnode_t z = T->root;
        rb_delete(T, z);
        rb_node_destroy(z);
        z = T->root;
    }
    free(T->Tnil);
    free(T);
}

void rb_rotate_left(rbtree_t T, rbnode_t x)
{
    rbnode_t y = x->right;
    x->right = y->left;
    if (y->left != T->Tnil)
        y->left->p = x;
    y->p = x->p;
    if (x->p == T->Tnil)
        T->root = y;
    else if (x == x->p->left)
        x->p->left = y;
    else
        x->p->right = y;
    y->left = x;
    x->p = y;
}

void rb_rotate_right(rbtree_t T, rbnode_t x)
{
    rbnode_t y = x->left;
    x->left = y->right;
    if (y->right != T->Tnil)
        y->right->p = x;
    y->p = x->p;
    if (x->p == T->Tnil)
        T->root = y;
    else if (x == x->p->left)
        x->p->right = y;
    else
        x->p->left = y;
    y->right = x;
    x->p = y;
}

void rb_insert(rbtree_t T, rbnode_t z)
{
    rbnode_t y = T->Tnil;
    rbnode_t x = T->root;
    while (x != T->Tnil) {
        y = x;
        if (z->key < x->key)
            x = x->left;
        else
            x = x->right;
    }
    z->p = y;
    if (y == T->Tnil)
        T->root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;
    z->left = T->Tnil;
    z->right = T->Tnil;
    z->color = RBRED;
    rb_insert_fixup(T, z);
}

void rb_insert_fixup(rbtree_t T, rbnode_t z)
{
    while(z->p->color == RBRED) {
        if (z->p == z->p->p->left) {
            rbnode_t y = z->p->p->right;
            if (y->color == RBRED) {
                z->p->color = RBBLACK;
                y->color = RBBLACK;
                z->p->p->color = RBRED;
                z = z->p->p;
            } else {
                if (z == z->p->right) {
                    z = z->p;
                    rb_rotate_left(T, z);
                }
                z->p->color = RBBLACK;
                z->p->p->color = RBRED;
                rb_rotate_right(T, z->p->p);
            }
        } else {
            rbnode_t y = z->p->p->left;
            if (y->color == RBRED) {
                z->p->color = RBBLACK;
                y->color = RBBLACK;
                z->p->p->color = RBRED;
                z = z->p->p;
            } else {
                if (z == z->p->left) {
                    z = z->p;
                    rb_rotate_right(T, z);
                }
                z->p->color = RBBLACK;
                z->p->p->color = RBRED;
                rb_rotate_left(T, z->p->p);
            }
        }
    }
    
    T->root->color = RBBLACK;
}


rbnode_t rb_tree_minimum(rbtree_t T, rbnode_t x) {
    while(x->left != T->Tnil)
        x = x->left;
    return x;
}

rbnode_t rb_tree_maximum(rbtree_t T, rbnode_t x) {
    while(x->right != T->Tnil)
        x = x->right;
    return x;
}

void rb_transplant(rbtree_t T, rbnode_t u, rbnode_t v)
{
    if (u->p == T->Tnil)
        T->root = v;
    else if (u == u->p->left)
        u->p->left = v;
    else
        u->p->right = v;
    v->p = u->p;
}

void rb_delete(rbtree_t T, rbnode_t z)
{
    char y_original_color;
    rbnode_t x, y;
    
    y = z;
    y_original_color = y->color;
    if (z->left == T->Tnil) {
        x = z->right;
        rb_transplant(T, z, z->right);
    } else if (z->right == T->Tnil) {
        x = z->left;
        rb_transplant(T, z, z->left);
    } else {
        y = rb_tree_minimum(T, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->p == z) {
            x->p = y;
        } else {
            rb_transplant(T, y, y->right);
            y->right = z->right;
            y->right->p = y;
        }
        rb_transplant(T, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
        if (y->left == y->right)
            y->right = T->Tnil;
    }
    if (y_original_color == RBBLACK)
        rb_delete_fixup(T, x);
}

void rb_delete_fixup(rbtree_t T, rbnode_t x)
{
    while(x != T->root && x->color == RBBLACK && x != T->Tnil)
    {
        if (x == x->p->left) {
            rbnode_t w = x->p->right;
            if (w->color == RBRED) {
                w->color = RBBLACK;
                x->p->color = RBRED;
                rb_rotate_left(T, x->p);
                w = x->p->right;
            }
            if (w->left->color == RBBLACK && w->right->color == RBBLACK) {
                w->color = RBRED;
                x = x->p;
            }
            else {
                if (w->right->color == RBBLACK) {
                    w->left->color = RBBLACK;
                    w->color = RBRED;
                    rb_rotate_right(T, w);
                    w = x->p->right;
                }
                w->color = x->p->color;
                x->p->color = RBBLACK;
                w->right->color = RBBLACK;
                rb_rotate_left(T, x->p);
                x = T->root;
            }
        } else {
            rbnode_t w = x->p->left;
            if (w->color == RBRED) {
                w->color = RBBLACK;
                x->p->color = RBRED;
                rb_rotate_right(T, x->p);
                w = x->p->left;
            }
            if (w->right->color == RBBLACK && w->left->color == RBBLACK) {
                w->color = RBRED;
                x = x->p;
            }
            else {
                if (w->left->color == RBBLACK) {
                    w->right->color = RBBLACK;
                    w->color = RBRED;
                    rb_rotate_left(T, w);
                    w = x->p->left;
                }
                w->color = x->p->color;
                x->p->color = RBBLACK;
                w->left->color = RBBLACK;
                rb_rotate_right(T, x->p);
                x = T->root;
            }
        }
    }
    x->color = RBBLACK;
}

rbnode_t rb_search(rbtree_t T, uint64_t k)
{
    rbnode_t x = T->root;
    while (x != T->Tnil && k != x->key) {
        printf("search: %llx\n", x->key);
        printf("\t left: %llx\n", x->left->key);
        printf("\tright: %llx\n", x->right->key); 
        if (k < x->key)
            x = x->left;
        else
            x = x->right;
    }
    
    return x;
}

#ifdef MAIN

int iter_callback(rbnode_t x) {
    printf("key: %llx\n", x->key);
    return 0;
}

int main(int argc, char *argv[]) {
    rbtree_t t;
    rbnode_t s;
    uint64_t i;

    t = rb_create();
    for(i=0;i<0xffff;++i) {
        rbnode_t x = rb_node_create(t->Tnil);
        x->key = i;
        rb_insert(t, x);
    }

    s = rb_search(t, arc4random() % 0xfffe);
    if (s) {
        printf("found: %llx\n", s->key);
    } else {
        printf("not found! boo!\n");
    }
    rb_iterate(t, iter_callback);
    rb_destroy(t);
}

#endif
