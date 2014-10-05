/***************************************
 *           RBTree Lib V4             *
 * Author: House.Lee Date: Jun 28 2012 *
 *----------===============------------*
 ***************************************/

#ifndef RBTREE_H
#define	RBTREE_H


#ifdef	__cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

//Red-Black Color definition
typedef enum COLOR {
	BLACK = 0,
	RED,
} RB_COLOR;

//Search result for specific Red-Black Tree
typedef enum SEARCH_STAT {
	READY_FOR_INSERT = 0,
	VALUE_EXIST,
} SearchStat;

//Memory Allocator and De-allocator
typedef void* (*RBT_MemAllocFunc)(size_t);
typedef void (*RBT_MemFreeFunc)(void*);

/**
 * Function to compare the key,should return:
 * any negative integer if left_key < right_key;
 * 0 if left_key == right_key;
 * any positive integer if left_key > right_key;
 **/
typedef int (*RBT_KeyCmpFunc)(const void*, const void*);

//Definition of Red-Black Tree and its node
typedef struct RBTREE_NODE {
	struct RBTREE_NODE* parent;
	struct RBTREE_NODE* left;
	struct RBTREE_NODE* right;
	void* key;
	void* value;
	RB_COLOR color;
} RBT_node;

typedef struct RBTREE{
	RBT_node* ancestor;
	RBT_node* nil;
	RBT_MemAllocFunc memAlloc;
	RBT_MemFreeFunc memFree;
	RBT_KeyCmpFunc keyCmp;
} RBTree;
typedef RBTree* RBTree_t;

//Frequently Using Functions
static inline RBT_node* rbt_getRoot(RBTree* tree) {
	return tree->ancestor->left;
	return NULL;
}
static inline int rbt_isRoot(RBTree* tree , RBT_node* node) {
	return node == tree->ancestor->left;
}
static inline int rbt_isLeaf(RBTree* tree , RBT_node* node) {
	return node == tree->nil;
}
static inline void* rbt_getKey(RBT_node* node) {
	return node->key;
}
static inline void* rbt_getValue(RBT_node* node) {
	return node->value;
}

RBTree* rbt_initRBTree(RBT_KeyCmpFunc cmpFunc, RBT_MemAllocFunc alloc,
		RBT_MemFreeFunc de_alloc);
int rbt_insertNode(RBTree* tree, void* key, void* value);
RBT_node* rbt_searchNodeByKey(RBTree* tree, const void* key);
int rbt_deleteNode(RBTree* tree, void* key);
void rbt_destroyRBTree(RBTree*);
RBT_node* rbt_findPredecessor(RBTree* tree, RBT_node* node);
RBT_node* rbt_findSuccessor(RBTree* tree, RBT_node* node);

int rbt_defaultCmpFunc(const void* key_left, const void* key_right);
void rbt_setMemAllocFunc(RBTree* tree, RBT_MemAllocFunc alloc);
void rbt_setMemFreeFunc(RBTree* tree, RBT_MemFreeFunc de_alloc);


#ifdef	__cplusplus
}
#endif

#endif	/* RBTREE_H */
