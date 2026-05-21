#include "red_and_black_tree.h"
#include <stdlib.h>

enum Color { RED, BLACK };

typedef struct RBNode {
    Process* process;
    enum Color color;
    struct RBNode *left, *right, *parent;
} RBNode;

static RBNode* root;
static RBNode* TNULL;

// Inicializa a árvore vazia
void rb_init(void) {
    TNULL = (RBNode*)malloc(sizeof(RBNode));
    TNULL->color = BLACK;
    TNULL->left = NULL;
    TNULL->right = NULL;
    root = TNULL;
}

void left_rotate(RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left != TNULL) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL) root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void right_rotate(RBNode* x) {
    RBNode* y = x->left;
    x->left = y->right;
    if (y->right != TNULL) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL) root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

// Corrige as propriedades da árvore após inserção
void rb_insert_fix(RBNode* k) {
    RBNode* u;
    while (k->parent != NULL && k->parent->color == RED) {
        if (k->parent == k->parent->parent->right) {
            u = k->parent->parent->left;
            if (u->color == RED) {
                u->color = BLACK;
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->left) {
                    k = k->parent;
                    right_rotate(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                left_rotate(k->parent->parent);
            }
        } else {
            u = k->parent->parent->right;
            if (u->color == RED) {
                u->color = BLACK;
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->right) {
                    k = k->parent;
                    left_rotate(k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                right_rotate(k->parent->parent);
            }
        }
        if (k == root) break;
    }
    root->color = BLACK;
}

// Insere um processo na Árvore indexado pelo vruntime
void rb_insert(Process* p) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    node->process = p;
    node->color = RED;
    node->left = TNULL;
    node->right = TNULL;
    node->parent = NULL;

    RBNode* y = NULL;
    RBNode* x = root;

    while (x != TNULL) {
        y = x;
        // Ordena pelo vruntime. Em caso de empate, usa o PID para estabilidade
        if (node->process->vruntime < x->process->vruntime || 
           (node->process->vruntime == x->process->vruntime && node->process->pid < x->process->pid)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    node->parent = y;
    if (y == NULL) {
        root = node;
    } else if (node->process->vruntime < y->process->vruntime || 
              (node->process->vruntime == y->process->vruntime && node->process->pid < y->process->pid)) {
        y->left = node;
    } else {
        y->right = node;
    }

    if (node->parent == NULL) {
        node->color = BLACK;
        return;
    }
    if (node->parent->parent == NULL) return;

    rb_insert_fix(node);
}

void rb_transplant(RBNode* u, RBNode* v) {
    if (u->parent == NULL) root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

static RBNode* rb_minimum(RBNode* node) {
    while (node->left != TNULL) node = node->left;
    return node;
}

// Corrige as propriedades da árvore após deletar
void rb_delete_fix(RBNode* x) {
    RBNode* s;
    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            s = x->parent->right;
            if (s->color == RED) {
                s->color = BLACK;
                x->parent->color = RED;
                left_rotate(x->parent);
                s = x->parent->right;
            }
            if (s->left->color == BLACK && s->right->color == BLACK) {
                s->color = RED;
                x = x->parent;
            } else {
                if (s->right->color == BLACK) {
                    s->left->color = BLACK;
                    s->color = RED;
                    right_rotate(s);
                    s = x->parent->right;
                }
                s->color = x->parent->color;
                x->parent->color = BLACK;
                s->right->color = BLACK;
                left_rotate(x->parent);
                x = root;
            }
        } else {
            s = x->parent->left;
            if (s->color == RED) {
                s->color = BLACK;
                x->parent->color = RED;
                right_rotate(x->parent);
                s = x->parent->left;
            }
            if (s->right->color == BLACK && s->left->color == BLACK) {
                s->color = RED;
                x = x->parent;
            } else {
                if (s->left->color == BLACK) {
                    s->right->color = BLACK;
                    s->color = RED;
                    left_rotate(s);
                    s = x->parent->left;
                }
                s->color = x->parent->color;
                x->parent->color = BLACK;
                s->left->color = BLACK;
                right_rotate(x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}

// Remove um nó da árvore (usado para retirar o processo selecionado)
static void rb_delete(RBNode* z) {
    RBNode* y = z;
    RBNode* x;
    enum Color y_original_color = y->color;
    
    if (z->left == TNULL) {
        x = z->right;
        rb_transplant(z, z->right);
    } else if (z->right == TNULL) {
        x = z->left;
        rb_transplant(z, z->left);
    } else {
        y = rb_minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rb_transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rb_transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    if (y_original_color == BLACK) rb_delete_fix(x);
    free(z);
}

Process *rb_minimum_process(void) {
    if (root == TNULL) {
        return NULL;
    }
    return rb_minimum(root)->process;
}

static void rb_free_subtree(RBNode *node) {
    if (node == NULL || node == TNULL) {
        return;
    }
    rb_free_subtree(node->left);
    rb_free_subtree(node->right);
    free(node);
}

void rb_destroy(void) {
    rb_free_subtree(root);
    free(TNULL);
    root = NULL;
    TNULL = NULL;
}