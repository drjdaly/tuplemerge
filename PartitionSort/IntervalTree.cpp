/*
 * MIT License
 *
 * Copyright (c) 2016 by S. Yingchareonthawornchai at Michigan State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "IntervalTree.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <chrono>
// If the symbol CHECK_INTERVAL_TREE_ASSUMPTIONS is defined then the
// code does a lot of extra checking to make sure certain assumptions
// are satisfied.  This only needs to be done if you suspect bugs are
// present or if you make significant changes and want to make sure
// your changes didn't mess anything up.
// #define CHECK_INTERVAL_TREE_ASSUMPTIONS 1


const int MIN_INT = 0;

// define a function to find the maximum of two objects.
#define ITMax(a, b) ( (a > b) ? a : b )

IntervalTreeNode::IntervalTreeNode(){
}

IntervalTreeNode::IntervalTreeNode(Interval * newInterval)
	: storedInterval(newInterval),
	key(newInterval->GetLowPoint()),
	high(newInterval->GetHighPoint()),
	maxHigh(high) {
}
IntervalTreeNode::~IntervalTreeNode(){
	delete storedInterval;
}
Interval::~Interval(){
}
void Interval::Print() const {
	std::cout << "No Print Method defined for instance of Interval" << std::endl;
}

IntervalTree::IntervalTree()
{
	nil = new IntervalTreeNode;
	nil->left = nil->right = nil->parent = nil;
	nil->red = 0;
	nil->key = nil->high = nil->maxHigh = MIN_INT;
	nil->storedInterval = NULL;

	root = new IntervalTreeNode;
	root->parent = root->left = root->right = nil;
	root->key = root->high = root->maxHigh = MAX_INT;
	root->red = 0;
	root->storedInterval = NULL;

	/* the following are used for the Enumerate function */
/*	recursionNodeStackSize = 128000;
	recursionNodeStack = (it_recursion_node *)
		SafeMalloc(recursionNodeStackSize*sizeof(it_recursion_node));
	recursionNodeStackTop = 1;
	recursionNodeStack[0].start_node = NULL;*/

}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  the node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: this, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/

void IntervalTree::LeftRotate(IntervalTreeNode* x) {
	IntervalTreeNode* y;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls LeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when DeleteFixUP */
	/*  calls LeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	y = x->right;
	x->right = y->left;

	if (y->left != nil) y->left->parent = x; /* used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	y->parent = x->parent;

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}
	y->left = x;
	x->parent = y;

	x->maxHigh = ITMax(x->left->maxHigh, ITMax(x->right->maxHigh, x->high));
	y->maxHigh = ITMax(x->maxHigh, ITMax(y->right->maxHigh, y->high));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red, "nil not red in ITLeftRotate");
	Assert((nil->maxHigh = MIN_INT),
		   "nil->maxHigh != MIN_INT in ITLeftRotate");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  node to rotate on */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: this, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. Also updates the maxHigh fields of x and y */
/*            after rotation. */
/***********************************************************************/


void IntervalTree::RightRotate(IntervalTreeNode* y) {
	IntervalTreeNode* x;

	/*  I originally wrote this function to use the sentinel for */
	/*  nil to avoid checking for nil.  However this introduces a */
	/*  very subtle bug because sometimes this function modifies */
	/*  the parent pointer of nil.  This can be a problem if a */
	/*  function which calls LeftRotate also uses the nil sentinel */
	/*  and expects the nil sentinel's parent pointer to be unchanged */
	/*  after calling this function.  For example, when DeleteFixUP */
	/*  calls LeftRotate it expects the parent pointer of nil to be */
	/*  unchanged. */

	x = y->left;
	y->left = x->right;

	if (nil != x->right)  x->right->parent = y; /*used to use sentinel here */
	/* and do an unconditional assignment instead of testing for nil */

	/* instead of checking if x->parent is the root as in the book, we */
	/* count on the root sentinel to implicitly take care of this case */
	x->parent = y->parent;
	if (y == y->parent->left) {
		y->parent->left = x;
	} else {
		y->parent->right = x;
	}
	x->right = y;
	y->parent = x;

	y->maxHigh = ITMax(y->left->maxHigh, ITMax(y->right->maxHigh, y->high));
	x->maxHigh = ITMax(x->left->maxHigh, ITMax(y->maxHigh, x->high));
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red, "nil not red in ITRightRotate");
	Assert((nil->maxHigh = MIN_INT),
		   "nil->maxHigh != MIN_INT in ITRightRotate");
#endif
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the InsertTree function and not by the user */
/***********************************************************************/

void IntervalTree::TreeInsertHelp(IntervalTreeNode* z) {
	/*  This function should only be called by InsertITTree (see above) */
	IntervalTreeNode* x;
	IntervalTreeNode* y;

	z->left = z->right = nil;
	y = root;
	x = root->left;
	while (x != nil) {
		y = x;
		if (x->key > z->key) {
			x = x->left;
		} else { /* x->key <= z->key */
			x = x->right;
		}
	}
	z->parent = y;
	if ((y == root) ||
		(y->key > z->key)) {
		y->left = z;
	} else {
		y->right = z;
	}


#if defined(DEBUG_ASSERT)
	Assert(!nil->red, "nil not red in ITTreeInsertHelp");
	Assert((nil->maxHigh = MIN_INT),
		   "nil->maxHigh != MIN_INT in ITTreeInsertHelp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  FixUpMaxHigh  */
/**/
/*  INPUTS:  x is the node to start from*/
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  this */
/**/
/*  EFFECTS:  Travels up to the root fixing the maxHigh fields after */
/*            an insertion or deletion */
/***********************************************************************/

void IntervalTree::FixUpMaxHigh(IntervalTreeNode * x) {
	while (x != root) {
		x->maxHigh = ITMax(x->high, ITMax(x->left->maxHigh, x->right->maxHigh));
		x = x->parent;
	}
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
	CheckAssumptions();
#endif
}

/*  Before calling InsertNode  the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  InsertNode */
/**/
/*  INPUTS:  newInterval is the interval to insert*/
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/

IntervalTreeNode * IntervalTree::Insert(Interval * newInterval)
{
	IntervalTreeNode * y;
	IntervalTreeNode * x;
	IntervalTreeNode * newNode;

	x = new IntervalTreeNode(newInterval);
	TreeInsertHelp(x);
	FixUpMaxHigh(x->parent);
	newNode = x;
	x->red = 1;
	while (x->parent->red) { /* use sentinel instead of checking for root */
		if (x->parent == x->parent->parent->left) {
			y = x->parent->parent->right;
			if (y->red) {
				x->parent->red = 0;
				y->red = 0;
				x->parent->parent->red = 1;
				x = x->parent->parent;
			} else {
				if (x == x->parent->right) {
					x = x->parent;
					LeftRotate(x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				RightRotate(x->parent->parent);
			}
		} else { /* case for x->parent == x->parent->parent->right */
			/* this part is just like the section above with */
			/* left and right interchanged */
			y = x->parent->parent->left;
			if (y->red) {
				x->parent->red = 0;
				y->red = 0;
				x->parent->parent->red = 1;
				x = x->parent->parent;
			} else {
				if (x == x->parent->left) {
					x = x->parent;
					RightRotate(x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				LeftRotate(x->parent->parent);
			}
		}
	}
	root->left->red = 0;
	return(newNode);

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red, "nil not red in ITTreeInsert");
	Assert(!root->red, "root not red in ITTreeInsert");
	Assert((nil->maxHigh = MIN_INT),
		   "nil->maxHigh != MIN_INT in ITTreeInsert");
#endif
}

/***********************************************************************/
/*  FUNCTION:  GetSuccessorOf  */
/**/
/*    INPUTS:  x is the node we want the succesor of */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

IntervalTreeNode * IntervalTree::GetSuccessorOf(IntervalTreeNode * x) const
{
	IntervalTreeNode* y;

	if (nil != (y = x->right)) { /* assignment to y is intentional */
		while (y->left != nil) { /* returns the minium of the right subtree of x */
			y = y->left;
		}
		return(y);
	} else {
		y = x->parent;
		while (x == y->right) { /* sentinel used instead of checking for nil */
			x = y;
			y = y->parent;
		}
		if (y == root) return(nil);
		return(y);
	}
}

/***********************************************************************/
/*  FUNCTION:  GetPredecessorOf  */
/**/
/*    INPUTS:  x is the node to get predecessor of */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

IntervalTreeNode * IntervalTree::GetPredecessorOf(IntervalTreeNode * x) const {
	IntervalTreeNode* y;

	if (nil != (y = x->left)) { /* assignment to y is intentional */
		while (y->right != nil) { /* returns the maximum of the left subtree of x */
			y = y->right;
		}
		return(y);
	} else {
		y = x->parent;
		while (x == y->left) {
			if (y == root) return(nil);
			x = y;
			y = y->parent;
		}
		return(y);
	}
}

/***********************************************************************/
/*  FUNCTION:  Print */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively prints the nodes of the tree */
/*              inorder. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:    This function should only be called from ITTreePrint */
/***********************************************************************/

void IntervalTreeNode::Print(IntervalTreeNode * nil,
							 IntervalTreeNode * root) const {
	storedInterval->Print();
	//printf(", k=%i, h=%i, mH=%i", key, high, maxHigh);
	/*printf("  l->key=");
	if (left == nil) printf("NULL"); else printf("%i", left->key);
	printf("  r->key=");
	if (right == nil) printf("NULL"); else printf("%i", right->key);
	printf("  p->key=");
	if (parent == root) printf("NULL"); else printf("%i", parent->key);
	printf("  red=%i\n", red);*/
}

void IntervalTree::TreePrintHelper(IntervalTreeNode* x) const {

	if (x != nil) {
		TreePrintHelper(x->left);
		x->Print(nil, root);
		TreePrintHelper(x->right);
	}
}

void IntervalTree::RemoveRecursive(IntervalTreeNode* node)
{
	if (node == nil) return;
	RemoveRecursive(node->left);
	RemoveRecursive(node->right);
	delete node;
}

IntervalTree::~IntervalTree() {
	RemoveRecursive(root);
	delete nil;
}


/***********************************************************************/
/*  FUNCTION:  Print */
/**/
/*    INPUTS:  none */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  This function recursively prints the nodes of the tree */
/*             inorder. */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/

void IntervalTree::Print() const {
	TreePrintHelper(root->left);
}

/***********************************************************************/
/*  FUNCTION:  DeleteFixUp */
/**/
/*    INPUTS:  x is the child of the spliced */
/*             out node in DeleteNode. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: this, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

void IntervalTree::DeleteFixUp(IntervalTreeNode* x) {
	IntervalTreeNode * w;
	IntervalTreeNode * rootLeft = root->left;

	while ((!x->red) && (rootLeft != x)) {
	
		if (x == x->parent->left) {
			w = x->parent->right;
			if (w->red) {
				w->red = 0;
				x->parent->red = 1;
				LeftRotate(x->parent);
				w = x->parent->right;
			}
			if ((!w->right->red) && (!w->left->red)) {
				w->red = 1;
				x = x->parent;
			} else {
				if (!w->right->red) {
					w->left->red = 0;
					w->red = 1;
					RightRotate(w);
					w = x->parent->right;
				}
				w->red = x->parent->red;
				x->parent->red = 0;
				w->right->red = 0;
				LeftRotate(x->parent);
				x = rootLeft; /* this is to exit while loop */
			}
		} else { /* the code below is has left and right switched from above */
			w = x->parent->left;
			if (w->red) {
				w->red = 0;
				x->parent->red = 1;
				RightRotate(x->parent);
				w = x->parent->left;
			}
			if ((!w->right->red) && (!w->left->red)) {
				w->red = 1;
				x = x->parent;
			} else {
				if (!w->left->red) {
					w->right->red = 0;
					w->red = 1;
					LeftRotate(w);
					w = x->parent->left;
				}
				w->red = x->parent->red;
				x->parent->red = 0;
				w->left->red = 0;
				RightRotate(x->parent);
				x = rootLeft; /* this is to exit while loop */
			}
		}
	}
	x->red = 0;

#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
	CheckAssumptions();
#elif defined(DEBUG_ASSERT)
	Assert(!nil->red, "nil not black in ITDeleteFixUp");
	Assert((nil->maxHigh = MIN_INT),
		   "nil->maxHigh != MIN_INT in ITDeleteFixUp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  DeleteNode */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  returns the Interval stored at deleted node */
/**/
/*    EFFECT:  Deletes z from tree and but don't call destructor */
/*             Then calls FixUpMaxHigh to fix maxHigh fields then calls */
/*             ITDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input:  z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

Interval * IntervalTree::DeleteNode(IntervalTreeNode * z){
	IntervalTreeNode* y;
	IntervalTreeNode* x;
	Interval * returnValue = z->storedInterval;

	y = ((z->left == nil) || (z->right == nil)) ? z : GetSuccessorOf(z);
	x = (y->left == nil) ? y->right : y->left;
	if (root == (x->parent = y->parent)) { /* assignment of y->p to x->p is intentional */
		root->left = x;
	} else {
		if (y == y->parent->left) {
			y->parent->left = x;
		} else {
			y->parent->right = x;
		}
	}
	if (y != z) { /* y should not be nil in this case */

#ifdef DEBUG_ASSERT
		Assert((y != nil), "y is nil in DeleteNode \n");
#endif
		/* y is the node to splice out and x is its child */

		y->maxHigh = MIN_INT;
		y->left = z->left;
		y->right = z->right;
		y->parent = z->parent;
		z->left->parent = z->right->parent = y;
		if (z == z->parent->left) {
			z->parent->left = y;
		} else {
			z->parent->right = y;
		}
		FixUpMaxHigh(x->parent);
		if (!(y->red)) {
			y->red = z->red;
			DeleteFixUp(x);
		} else
			y->red = z->red;
		delete z;
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
		CheckAssumptions();
#elif defined(DEBUG_ASSERT)
		Assert(!nil->red, "nil not black in ITDelete");
		Assert((nil->maxHigh = MIN_INT), "nil->maxHigh != MIN_INT in ITDelete");
#endif
	} else {
		FixUpMaxHigh(x->parent);
		if (!(y->red)) DeleteFixUp(x);
		delete y;
#ifdef CHECK_INTERVAL_TREE_ASSUMPTIONS
		CheckAssumptions();
#elif defined(DEBUG_ASSERT)
		Assert(!nil->red, "nil not black in ITDelete");
		Assert((nil->maxHigh = MIN_INT), "nil->maxHigh != MIN_INT in ITDelete");
#endif
	}

	return returnValue;
}


/***********************************************************************/
/*  FUNCTION:  Overlap */
/**/
/*    INPUTS:  [a1,a2] and [b1,b2] are the low and high endpoints of two */
/*             closed intervals.  */
/**/
/*    OUTPUT:  stack containing pointers to the nodes between [low,high] */
/**/
/*    Modifies Input: none */
/**/
/*    EFFECT:  returns 1 if the intervals overlap, and 0 otherwise */
/***********************************************************************/

bool inline Overlap(unsigned int a1, unsigned  int a2, unsigned int b1, unsigned int b2) {
	if (a1 <= b1) {
		return((b1 <= a2));
	} else {
		return((a1 <= b2));
	}
}



/*FirstInteralSearch: follow Interval-Search from CLRS book*/

IntervalTreeNode * IntervalTree::FirstIntervalSearch(unsigned int low, unsigned int high) const {

	IntervalTreeNode* x = root->left;
	int i = 0;
	while ((x != nil) && !Overlap(low, high, x->GetStoredInterval()->GetLowPoint(), x->GetStoredInterval()->GetHighPoint())) {
		i++;
		if (x->left != nil && x->left->maxHigh >= low) {
			x = x->left;
		} else {
			x = x->right;
		}
	}
	//printf("search cost= %d \n", i);
	return x; 

}

Interval * IntervalTree::FirstIntervalSearchRetInterval(unsigned int low, unsigned int high) const {

	IntervalTreeNode* x = root->left;
	int i = 0;
	auto  intervalx = x->storedInterval;
	while ((x != nil) && !Overlap(low, high, intervalx->GetLowPoint(), intervalx->GetHighPoint())) {
		i++;
		if (x->left != nil && x->left->maxHigh >= low) {
			x = x->left;
		} else {
			x = x->right;
		}
		intervalx = x->storedInterval;
	}
	//printf("search cost= %d \n", i);
	return x->storedInterval;

}

IntervalTreeNode * IntervalTree::GetRootTreeNode() const {
	if (root->left == nullptr) printf("Warning: GetRootTreeNode return nullptr;\n");
	return root->left;
}
