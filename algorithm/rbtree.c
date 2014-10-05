#include <stdlib.h>
#include "rbtree.h"

/** private assistance functions declaration: **/
void rbt_rotateLeft(RBTree* tree , RBT_node* node);
void rbt_rotateRight(RBTree* tree , RBT_node* node);
void rbt_insertFixUp(RBTree* tree , RBT_node* node);
void rbt_deleteFixUp(RBTree* tree , RBT_node* node);
void rbt_destroyFixUp(RBTree* tree , RBT_node* node);

/** Operation Interface Functions Definitions **/
void rbt_setMemAllocFunc(RBTree* tree, RBT_MemAllocFunc alloc) {
	tree->memAlloc = alloc;
}
void rbt_setMemFreeFunc(RBTree* tree, RBT_MemFreeFunc de_alloc) {
	tree->memFree = de_alloc;
}

int rbt_defaultCmpFunc(const void* key_left, const void* key_right) {
	return *(int*) key_left - *(int*) key_right;
}

RBTree* rbt_initRBTree( RBT_KeyCmpFunc cmpFunc, RBT_MemAllocFunc alloc, RBT_MemFreeFunc de_alloc) {
	RBTree* tree;
	tree = (RBTree*)malloc(sizeof(RBTree));
	if (!tree) {
		return NULL;
	}
	tree->keyCmp = (cmpFunc == NULL) ? rbt_defaultCmpFunc : cmpFunc;
	tree->memAlloc = (alloc == NULL) ? malloc : alloc;
	tree->memFree = (de_alloc == NULL) ? free : de_alloc;

	tree->nil = (RBT_node*)tree->memAlloc(sizeof(RBT_node));
	tree->nil->parent = tree->nil->left = tree->nil->right = tree->nil;
	tree->nil->color = BLACK;
	tree->nil->key = NULL;

	tree->ancestor = (RBT_node*)tree->memAlloc(sizeof(RBT_node));
	tree->ancestor->parent = tree->ancestor->left = tree->ancestor->right = tree->nil;
	tree->ancestor->color = BLACK;
	tree->ancestor->key = NULL;
	return tree;
}

int rbt_insertNode(RBTree* tree , void* key , void* value) {
	if (rbt_searchNodeByKey(tree , key) != NULL)
		return -1;
	RBT_node* y;
	RBT_node* x;
//	RBT_node* newNode;
	x = (RBT_node*)tree->memAlloc(sizeof(RBT_node));
	if (!x) {
		return -2;
	}
	x->key = key;
	x->value = value;

	rbt_insertFixUp(tree , x);
//	newNode = x;
	x->color = RED;
	while ( x->parent->color == RED ) {
		if ( x->parent == x->parent->parent->left ) {
			y = x->parent->parent->right;
			if ( y->color == RED ) {
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			} else {
				if ( x == x->parent->right ) {
					x = x->parent;
					rbt_rotateLeft(tree , x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rbt_rotateRight(tree , x->parent->parent);
			}
		} else {
			y = x->parent->parent->left;
			if ( y->color == RED ) {
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			} else {
				if (x == x->parent->left) {
					x = x->parent;
					rbt_rotateRight(tree , x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rbt_rotateLeft(tree , x->parent->parent);
			}
		}
	}
	tree->ancestor->left->color = BLACK;
	return 0;
}

RBT_node* rbt_searchNodeByKey(RBTree* tree,const void* key) {
	RBT_node* x = rbt_getRoot(tree);
	RBT_node* nil = tree->nil;
	if (x == nil) return NULL;
	int cmpVal = tree->keyCmp(x->key , key);
	while ( cmpVal ) {
		if ( cmpVal > 0) {
			x = x->left;
		} else {
			x = x->right;
		}
		if (x == nil) return NULL;
		cmpVal = tree->keyCmp(x->key , key );
	}
	return x;
}

int rbt_deleteNode(RBTree* tree , void* key) {
	RBT_node* z = rbt_searchNodeByKey(tree , key);
	if (z == NULL) {
		return -1;
	}
	RBT_node* y;
	RBT_node* x;
	RBT_node* nil = tree->nil;
	RBT_node* root = tree->ancestor;

	y = ((z->left == nil) || (z->right == nil)) ? z : rbt_findSuccessor(tree, z);
	x = (y->left == nil) ? y->right : y->left;
	if (root == (x->parent = y->parent)) {
		root->left = x;
	} else {
		if (y == y->parent->left) {
			y->parent->left = x;
		} else {
			y->parent->right = x;
		}
	}
	if (y != z) {
		if (y->color == BLACK)
			rbt_deleteFixUp(tree, x);


		y->left = z->left;
		y->right = z->right;
		y->parent = z->parent;
		y->color = z->color;
		z->left->parent = z->right->parent = y;
		if (z == z->parent->left) {
			z->parent->left = y;
		} else {
			z->parent->right = y;
		}
		tree->memFree(z);
	} else {
		if (y->color == BLACK)
			rbt_deleteFixUp(tree, x);
		tree->memFree(y);
	}
	return 0;
}

void rbt_destroyRBTree(RBTree* tree) {
	rbt_destroyFixUp(tree , tree->ancestor->left);
	tree->memFree(tree->ancestor);
	tree->memFree(tree->nil);
	free(tree);
}






RBT_node* rbt_findSuccessor(RBTree* tree, RBT_node* node) {
	RBT_node* y;
	RBT_node* nil = tree->nil;
	RBT_node* root = tree->ancestor;

	if (nil != (y = node->right)) {
		while (y->left != nil) {
			y = y->left;
		}
		return y;
	} else {
		y = node->parent;
		while (node == y->right) {
			node = y;
			y = y->parent;
		}
		if (y == root)
			return nil;
		return y;
	}
}

RBT_node* rbt_findPredecessor(RBTree* tree, RBT_node* node) {
	RBT_node* y;
	RBT_node* nil = tree->nil;
	RBT_node* root = tree->ancestor;

	if (nil != (y = node->left)) { /* assignment to y is intentional */
		while (y->right != nil) { /* returns the maximum of the left subtree of x */
			y = y->right;
		}
		return y;
	} else {
		y = node->parent;
		while (node == y->left) {
			if (y == root)
				return nil;
			node = y;
			y = y->parent;
		}
		return y;
	}
}

void rbt_rotateLeft(RBTree* tree, RBT_node* node) {
	RBT_node* tmp;
	RBT_node* nil = tree->nil;

	tmp = node->right;
	node->right = tmp->left;

	if (tmp->left != nil)
		tmp->left->parent = node;

	tmp->parent = node->parent;

	if (node == node->parent->left) {
		node->parent->left = tmp;
	} else {
		node->parent->right = tmp;
	}
	tmp->left = node;
	node->parent = tmp;
}

void rbt_rotateRight(RBTree* tree, RBT_node* node) {
	RBT_node* tmp;
	RBT_node* nil = tree->nil;
	tmp = node->left;
	node->left = tmp->right;

	if (nil != tmp->right)
		tmp->right->parent = node;

	tmp->parent = node->parent;
	if (node == node->parent->left) {
		node->parent->left = tmp;
	} else {
		node->parent->right = tmp;
	}
	tmp->right = node;
	node->parent = tmp;
}

void rbt_insertFixUp(RBTree* tree, RBT_node* node) {
	RBT_node* x;
	RBT_node* y;
	RBT_node* nil = tree->nil;

	node->left = node->right = nil;
	y = tree->ancestor;
	x = tree->ancestor->left;
	while (x != nil) {
		y = x;
		if (tree->keyCmp(x->key, node->key) > 0) { /* x.key > node.key */
			x = x->left;
		} else { /* x,key <= node.key */
			x = x->right;
		}
	}
	node->parent = y;
	if ((y == tree->ancestor) || (tree->keyCmp(y->key, node->key) > 0)) { /* y.key > node.key */
		y->left = node;
	} else {
		y->right = node;
	}
}

void rbt_deleteFixUp(RBTree* tree , RBT_node* node) {
	RBT_node* root = tree->ancestor->left;
	RBT_node* w;

	while ( (node->color == BLACK) && (node != root)) {
		if (node == node->parent->left) {
			w = node->parent->right;
			if ( w->color == RED ) {
				w->color = BLACK;
				node->parent->color = RED;
				rbt_rotateLeft(tree , node->parent);
				w = node->parent->right;
			}
			if ( (w->right->color == BLACK) && (w->left->color == BLACK) ) {
				w->color = RED;
				node = node->parent;
			} else {
				if ( w->right->color == BLACK ) {
					w->left->color = BLACK;
					w->color = RED;
					rbt_rotateRight(tree , w);
					w = node->parent->right;
				}
				w->color = node->parent->color;
				node->parent->color = BLACK;
				w->right->color = BLACK;
				rbt_rotateLeft(tree , node->parent);
				node = root;
			}
		} else {
			w = node->parent->left;
			if ( w->color == RED ) {
				w->color = BLACK;
				node->parent->color = RED;
				rbt_rotateRight(tree , node->parent);
				w = node->parent->left;
			}
			if ( (w->right->color == BLACK) && (w->left->color == BLACK) ) {
				w->color = RED;
				node = node->parent;
			} else {
				if ( w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rbt_rotateLeft(tree , w);
					w = node->parent->left;
				}
				w->color = node->parent->color;
				node->parent->color = BLACK;
				w->left->color = BLACK;
				rbt_rotateRight(tree , node->parent);
				node = root;
			}
		}
	}
	node->color = BLACK;
}

void rbt_destroyFixUp(RBTree* tree , RBT_node* node) {
	if ( node != tree->nil ) {
		rbt_destroyFixUp(tree , node->left);
		rbt_destroyFixUp(tree , node->right);
		tree->memFree(node);
	}
}
