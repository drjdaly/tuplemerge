#ifdef DMALLOC
#include <dmalloc.h>
#endif
#ifndef  RBTREE_H
#define  RBTREE_H
#include <functional>
#include"misc.h"
#include"stack.h"
#include "../ElementaryClasses.h"
#include <vector>
#include <stack>
#include <algorithm>
/*  CONVENTIONS:  All data structures for red-black trees have the prefix */
/*                "rb_" to prevent name conflicts. */
/*                                                                      */
/*                Function names: Each word in a function name begins with */
/*                a capital letter.  An example funcntion name is  */
/*                CreateRedTree(a,b,c). Furthermore, each function name */
/*                should begin with a capital letter to easily distinguish */
/*                them from variables. */
/*                                                                     */
/*                Variable names: Each word in a variable name begins with */
/*                a capital letter EXCEPT the first letter of the variable */
/*                name.  For example, int newLongInt.  Global variables have */
/*                names beginning with "g".  An example of a global */
/*                variable name is gNewtonsConstant. */

/* comment out the line below to remove all the debugging assertion */
/* checks from the compiled code.  */
//#define DEBUG_ASSERT 0

static const int LOW = 0, HIGH = 1;
struct rb_red_blk_tree;
typedef std::array<Point, 2>  box; 

//Total 29 bytes per node
typedef struct rb_red_blk_node {
	box key;
	rb_red_blk_node* left;
	rb_red_blk_node* right;
	rb_red_blk_tree * rb_tree_next_level;
	//int priority; /*max_priority of all children*/
	bool red; /* if red=0 then the node is black */
	rb_red_blk_node* parent;

} rb_red_blk_node; 


//Total 
typedef struct rb_red_blk_tree { 
  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when RBTreeCreate is caled.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or info.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  rb_red_blk_node* root;             
  rb_red_blk_node* nil;   
   
  int count = 0;
  std::vector<box> chain_boxes;
  //std::priority_queue<int> pq;
  std::vector<int> priority_list;
  int max_priority_local = -1;

  void PrintKey(const box& b)
  {
	  printf("[%u %u]\n", b[LowDim], b[HighDim]);
  }
  void PushPriority(int p) {
 
	  max_priority_local = std::max(p, max_priority_local);
	  priority_list.push_back(p);
  }
  void PopPriority(int p) {
	  auto result = find(begin(priority_list), end(priority_list),p);
	  priority_list.erase(find(begin(priority_list), end(priority_list), p));
	  if (p == max_priority_local ) {
		  max_priority_local = *std::max_element(begin(priority_list), end(priority_list));
	  }
	  if (priority_list.empty()) max_priority_local = -1;
  }
  void ClearPriority() {
	  max_priority_local = -1;
	  priority_list.clear();
  }
  int GetMaxPriority() const {
	  return max_priority_local;
  }
  int GetSizeList() const{
	  return priority_list.size();
  }
} rb_red_blk_tree;

bool inline Overlap(unsigned int a1, unsigned  int a2, unsigned int b1, unsigned int b2) {
	if (a1 <= b1) {
		return((b1 <= a2));
	} else {
		return((a1 <= b2));
	}
}

/**
FOR RB tree light weight node
**/
rb_red_blk_tree* RBTreeCreate();

rb_red_blk_node * RBTreeInsertWithPathCompression(rb_red_blk_tree* tree, const std::vector<box>& key, unsigned int level, const std::vector<int>& fieldOrder, int priority);
void RBTreeDeleteWithPathCompression(rb_red_blk_tree*& tree, const std::vector<box>& key, int level, const std::vector<int>& fieldOrder, int priority, bool& JustDeletedTree);
std::vector<std::pair<rb_red_blk_tree*, rb_red_blk_node *>> RBFindNodeSequence(rb_red_blk_tree* tree, const std::vector<box>& key, int level, const std::vector<int>& fieldOrder);

bool TreeInsertWithPathCompressionHelp(rb_red_blk_tree* tree, rb_red_blk_node* z, const std::vector<box>& b, int level, const std::vector<int>& fieldOrder, int priority, rb_red_blk_node*& out_ptr);
int RBExactQueryPriority(rb_red_blk_tree*  tree, const Packet& q, int level, const std::vector<int>& fieldOrder, int priority_so_far); 
bool TreeInsertHelp(rb_red_blk_tree* tree, rb_red_blk_node* z, const std::vector<box>& b, int level, const std::vector<int>& fieldOrder, int priority,  rb_red_blk_node*& out_ptr);
rb_red_blk_node * RBTreeInsert(rb_red_blk_tree* tree, const std::vector<box>& key, int level, const std::vector<int>& fieldOrder, int priority=0);
bool RBTreeCanInsert(rb_red_blk_tree* tree, const std::vector<box>& z, int level, const std::vector<int>& fieldOrder);
void RBTreePrint(rb_red_blk_tree*);
void RBDelete(rb_red_blk_tree* , rb_red_blk_node* );
void RBTreeDestroy(rb_red_blk_tree*);
rb_red_blk_node* TreePredecessor(rb_red_blk_tree*,rb_red_blk_node*);
rb_red_blk_node* TreeSuccessor(rb_red_blk_tree*,rb_red_blk_node*);


void RBSerializeIntoRulesRecursion(rb_red_blk_tree * tree, rb_red_blk_node* node, int level, const std::vector<int>& fieldOrder, std::vector<box>& boxes_so_far, std::vector<Rule>& rules_so_far);

std::vector<Rule> RBSerializeIntoRules(rb_red_blk_tree* tree, const std::vector<int>& fieldOrder);

int RBExactQuery(rb_red_blk_tree* tree, const Packet& q, int level, const std::vector<int>& fieldOrder);
stk_stack * RBEnumerate(rb_red_blk_tree* tree,void* low, void* high);
void NullFunction(void*);

int RBExactQueryIterative(rb_red_blk_tree*  tree, const Packet& q, const std::vector<int>& fieldOrder);



int  CalculateMemoryConsumptionRecursion(rb_red_blk_tree * treenode, rb_red_blk_node * node, int level, const std::vector<int>& fieldOrder);
int CalculateMemoryConsumption(rb_red_blk_tree* tree, const std::vector<int>& fieldOrder);

#endif
