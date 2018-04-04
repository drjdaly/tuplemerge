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
#ifndef E_INTERVAL_TREE
#define E_INTERVAL_TREE


#include<math.h>
#include<limits.h>
#include<iostream>
#include <vector>
#include "../ElementaryClasses.h"
//  The interval_tree.h and interval_tree.cc files contain code for 
//  interval trees implemented using red-black-trees as described in 
//  the book _Introduction_To_Algorithms_ by Cormen, Leisserson, 
//  and Rivest.  

//  CONVENTIONS:  
//                Function names: Each word in a function name begins with 
//                a capital letter.  An example funcntion name is  
//                CreateRedTree(a,b,c). Furthermore, each function name 
//                should begin with a capital letter to easily distinguish 
//                them from variables. 
//                                                                     
//                Variable names: Each word in a variable name begins with 
//                a capital letter EXCEPT the first letter of the variable 
//                name.  For example, int newLongInt.  Global variables have 
//                names beginning with "g".  An example of a global 
//                variable name is gNewtonsConstant. 


#ifndef MAX_INT
#define MAX_INT UINT_MAX // some architechturs define INT_MAX not MAX_INT
#endif

// The Interval class is an Abstract Base Class.  This means that no
// instance of the Interval class can exist.  Only classes which
// inherit from the Interval class can exist.  Furthermore any class
// which inherits from the Interval class must define the member
// functions GetLowPoint and GetHighPoint.
//
// The GetLowPoint should return the lowest point of the interval and
// the GetHighPoint should return the highest point of the interval.  
/*
class Interval {
public:
	Interval();
	virtual ~Interval();
	virtual unsigned int GetLowPoint() const = 0;
	virtual unsigned int GetHighPoint() const = 0;
	virtual void Print() const;
};
*/
class IntervalTreeNode {
	friend class IntervalTree;
public:
	void Print(IntervalTreeNode*,
			   IntervalTreeNode*) const;
	IntervalTreeNode();
	IntervalTreeNode(Interval *);
	~IntervalTreeNode();
	Interval * GetStoredInterval() { return storedInterval; }

	Interval * storedInterval;
	unsigned int key;
	unsigned int high;
	unsigned int maxHigh;
	bool red; /* if red=0 then the node is black */
	IntervalTreeNode * left;
	IntervalTreeNode * right;
	IntervalTreeNode * parent;
};

struct it_recursion_node {
public:
	/*  this structure stores the information needed when we take the */
	/*  right branch in searching for intervals but possibly come back */
	/*  and check the left branch as well. */

	IntervalTreeNode * start_node;
	unsigned int parentIndex;
	int tryRightBranch;
};


class IntervalTree {
public:
	IntervalTree();
	~IntervalTree();
	void Print() const;
	Interval * DeleteNode(IntervalTreeNode *);
	IntervalTreeNode * Insert(Interval *);
	IntervalTreeNode * GetRootTreeNode() const;
	IntervalTreeNode * GetPredecessorOf(IntervalTreeNode *) const;
	IntervalTreeNode * GetSuccessorOf(IntervalTreeNode *) const;
	IntervalTreeNode * FirstIntervalSearch(unsigned int low, unsigned int high) const;
	Interval * FirstIntervalSearchRetInterval(unsigned int low, unsigned int high) const;
	void CheckAssumptions() const;
protected:
	/*  A sentinel is used for root and for nil.  These sentinels are */
	/*  created when ITTreeCreate is called.  root->left should always */
	/*  point to the node which is the root of the tree.  nil points to a */
	/*  node which should always be black but has aribtrary children and */
	/*  parent and no key or info.  The point of using these sentinels is so */
	/*  that the root and nil nodes do not require special cases in the code */
	IntervalTreeNode * root;
	IntervalTreeNode * nil;
	void LeftRotate(IntervalTreeNode *);
	void RightRotate(IntervalTreeNode *);
	void TreeInsertHelp(IntervalTreeNode *);
	void TreePrintHelper(IntervalTreeNode *) const;
	void FixUpMaxHigh(IntervalTreeNode *);
	void DeleteFixUp(IntervalTreeNode *);
	void CheckMaxHighFields(IntervalTreeNode *) const;
	int CheckMaxHighFieldsHelper(IntervalTreeNode * y,
								 const unsigned int currentHigh,
								 int match) const;
	void RemoveRecursive(IntervalTreeNode* node);

private:
	//unsigned int recursionNodeStackSize;
	////it_recursion_node * recursionNodeStack;
	//unsigned int currentParent;
	//unsigned int recursionNodeStackTop;
};


#endif



