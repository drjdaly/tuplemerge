#include "red_black_tree.h"

int CompareBox(const box& a, const box& b ) {
	int compare_size = a.size();
	const int LOW = 0, HIGH = 1;
	if (a[LOW] == b[LOW] && a[HIGH] == b[HIGH])  return 0;
	if (Overlap(a[LOW], a[HIGH], b[LOW], b[HIGH])) return 2;
	if (a[HIGH] < b[LOW]) {
		return -1;
	} else if (a[LOW] > b[HIGH]) {
		return 1;
	}
	
	return 0;
}
int inline CompareQuery(const box& a, const Packet& q, int level, const std::vector<int>& fieldOrder) {

	const int LOW = 0, HIGH = 1;
	if (a[HIGH] < q[fieldOrder[level]]) {
		return -1;
	} else if (a[LOW] > q[fieldOrder[level]]) {
		return 1;
	}
	
	return 0;
}

/***********************************************************************/
/*  FUNCTION:  RBTreeCreate */
/**/
/*  INPUTS:  All the inputs are names of functions.  CompFunc takes to */
/*  void pointers to keys and returns 1 if the first arguement is */
/*  "greater than" the second.   DestFunc takes a pointer to a key and */
/*  destroys it in the appropriate manner when the node containing that */
/*  key is deleted.  InfoDestFunc is similiar to DestFunc except it */
/*  recieves a pointer to the info of a node and destroys it. */
/*  PrintFunc recieves a pointer to the key of a node and prints it. */
/*  PrintInfo recieves a pointer to the info of a node and prints it. */
/*  If RBTreePrint is never called the print functions don't have to be */
/*  defined and NullFunction can be used.  */
/**/
/*  OUTPUT:  This function returns a pointer to the newly created */
/*  red-black tree. */
/**/
/*  Modifies Input: none */
/***********************************************************************/

rb_red_blk_tree* RBTreeCreate() {

  rb_red_blk_tree* newTree;
  rb_red_blk_node* temp;

  newTree = new rb_red_blk_tree;//(rb_red_blk_tree*) SafeMalloc(sizeof(rb_red_blk_tree));
  /*  see the comment in the rb_red_blk_tree structure in red_black_tree.h */
  /*  for information on nil and root */
  temp=newTree->nil= (rb_red_blk_node*) SafeMalloc(sizeof(rb_red_blk_node));
  temp->parent=temp->left=temp->right=temp;
  temp->red=0;
  temp->key = { { 1111, 1111 } };
  temp=newTree->root= (rb_red_blk_node*) SafeMalloc(sizeof(rb_red_blk_node));
  temp->parent=temp->left=temp->right=newTree->nil;
  temp->key = { { 2222, 2222 } };
  temp->red=0; 

  return(newTree);
}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: tree, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */
/***********************************************************************/

void LeftRotate(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;

  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when RBDeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  y=x->right;
  x->right=y->left;

  if (y->left != nil) y->left->parent=x; /* used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */
  
  y->parent=x->parent;   

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  if( x == x->parent->left) {
    x->parent->left=y;
  } else {
    x->parent->right=y;
  }
  y->left=x;
  x->parent=y;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in LeftRotate");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: tree, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */
/***********************************************************************/

void RightRotate(rb_red_blk_tree* tree, rb_red_blk_node* y) {
  rb_red_blk_node* x;
  rb_red_blk_node* nil=tree->nil;

  /*  I originally wrote this function to use the sentinel for */
  /*  nil to avoid checking for nil.  However this introduces a */
  /*  very subtle bug because sometimes this function modifies */
  /*  the parent pointer of nil.  This can be a problem if a */
  /*  function which calls LeftRotate also uses the nil sentinel */
  /*  and expects the nil sentinel's parent pointer to be unchanged */
  /*  after calling this function.  For example, when RBDeleteFixUP */
  /*  calls LeftRotate it expects the parent pointer of nil to be */
  /*  unchanged. */

  x=y->left;
  y->left=x->right;

  if (nil != x->right)  x->right->parent=y; /*used to use sentinel here */
  /* and do an unconditional assignment instead of testing for nil */

  /* instead of checking if x->parent is the root as in the book, we */
  /* count on the root sentinel to implicitly take care of this case */
  x->parent=y->parent;
  if( y == y->parent->left) {
    y->parent->left=x;
  } else {
    y->parent->right=x;
  }
  x->right=y;
  y->parent=x;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in RightRotate");
#endif
}
bool inline IsIntersect(unsigned a1, unsigned b1, unsigned a2, unsigned b2) {
	return std::max(a1, a2) <= std::min(b1, b2);
}
bool inline IsIdentical(unsigned a1, unsigned b1, unsigned a2, unsigned b2) {
	return a1 == a2  && b1 == b2;
}
bool RBTreeCanInsert(rb_red_blk_tree* tree, const std::vector<box>& z, int level, const std::vector<int>& fieldOrder) {
	 
	if (tree == nullptr) return true;
	
	if (level == fieldOrder.size()) {
		return true; 
	} else if (tree->count == 1) { 
		auto chain_boxes = tree->chain_boxes;
		for (size_t i = level; i < fieldOrder.size(); i++) {
			if (IsIdentical(z[fieldOrder[i]][0], z[fieldOrder[i]][1], chain_boxes[i - level][0], chain_boxes[i - level][1])) continue;
			if (IsIntersect(z[fieldOrder[i]][0], z[fieldOrder[i]][1], chain_boxes[i - level][0], chain_boxes[i - level][1])) return false;
			else return true;
		}
		return true;
	}

	rb_red_blk_node* x;
	rb_red_blk_node* y;
	rb_red_blk_node* nil = tree->nil;

	y = tree->root;
	x = tree->root->left;
	while (x != nil) {
		y = x;
		int compare_result = CompareBox(x->key, z[fieldOrder[level]]);
		if (compare_result == 1) { /* x.key > z.key */
			x = x->left;
		} else if (compare_result == -1) { /* x.key < z.key */
			x = x->right;
		} else if (compare_result == 0) {  /* x.key = z.key */
			/*printf("TreeInsertHelp:: Exact Match!\n");
			return true;
			x = x->right;*/
			return level == z.size() - 1 ? true : RBTreeCanInsert(x->rb_tree_next_level, z, level + 1,fieldOrder);
		} else {  /* x.key || z.key */
			return false;
		}
	}
	return true;
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  tree is the tree to insert into and z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  tree, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the RBTreeInsert function and not by the user */
/***********************************************************************/


bool TreeInsertWithPathCompressionHelp(rb_red_blk_tree* tree, rb_red_blk_node* z, const std::vector<box>& b, int level, const std::vector<int>& fieldOrder, int priority, rb_red_blk_node*& out_ptr) {
  /*  This function should only be called by InsertRBTree (see above) */
  rb_red_blk_node* x;
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  
  z->left=z->right=nil;
  y=tree->root;
  x=tree->root->left;
  while( x != nil) {
    y=x;
	int compare_result =  CompareBox(x->key, z->key);
	if (compare_result == 1) { /* x.key > z.key */
      x=x->left;
    } else if (compare_result ==-1) { /* x.key < z.key */
      x=x->right;
	} else if (compare_result == 0) {  /* x.key = z.key */
		
		if (level != fieldOrder.size() - 1) {
			/*if (x->rb_tree_next_level->count == 1) {
				//path compression

				auto temp_chain_boxes = x->rb_tree_next_level->chain_boxes;
				int xpriority = x->rb_tree_next_level->GetMaxPriority();

				free(x->rb_tree_next_level);
				//unzipping the next level
				int run = 1;
		
				std::vector<int> naturalFieldOrder(fieldOrder.size());
				std::iota(begin(naturalFieldOrder), end(naturalFieldOrder), 0);
				while ( (temp_chain_boxes[run][0] == b[fieldOrder[level + run]][0] && temp_chain_boxes[run][1] == b[fieldOrder[level + run]][1])) {
					x->rb_tree_next_level = RBTreeCreate();
					x->rb_tree_next_level->count = 1;
					x = RBTreeInsert(x->rb_tree_next_level, temp_chain_boxes, level + run, naturalFieldOrder, priority);
					run++;
					if (level + run >= fieldOrder.size()) break;
				}

				if (level + run  >= fieldOrder.size()) {
					x->rb_tree_next_level = RBTreeCreate();
					x->rb_tree_next_level->pq.push(priority);
					x->rb_tree_next_level->pq.push(xpriority);
					out_ptr = x;
				}
				else if (!(temp_chain_boxes[run][0] == b[fieldOrder[level + run]][0] && temp_chain_boxes[run][1] == b[fieldOrder[level + run]][1])) {
					//split into z and x node
					x->rb_tree_next_level = RBTreeCreate();
					RBTreeInsert(x->rb_tree_next_level, temp_chain_boxes, level + run, naturalFieldOrder, xpriority);
					RBTreeInsert(x->rb_tree_next_level, b, level + run, fieldOrder, priority);
				}
			
			}
			else {*/
			RBTreeInsertWithPathCompression(x->rb_tree_next_level, b, level + 1, fieldOrder, priority);
			//}
		
		}
		else {
			if ( x->rb_tree_next_level == nullptr)
				x->rb_tree_next_level = RBTreeCreate();
			x->rb_tree_next_level->count++;
			x->rb_tree_next_level->PushPriority(priority); 
			out_ptr = x;
		}
		free(z);
		return true;
	} else {  /* x.key || z.key */
		printf("Warning TreeInsertPathcompressionHelp : x.key || z.key\n");
	}
  }
  z->parent=y;
  if ( (y == tree->root) ||
	  (1 ==  CompareBox(y->key, z->key))) { /* y.key > z.key */
    y->left=z;
  } else {
    y->right=z;
  }
  //found new one to insert to
  //need to create a tree first then followed by insertion
  //we use path-compression here since this tree contains a single rule
  z->rb_tree_next_level = RBTreeCreate(); 
  RBTreeInsertWithPathCompression(z->rb_tree_next_level, b, level  +1, fieldOrder, priority);

  

  return false;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in TreeInsertHelp");
#endif
}


/*  Before calling Insert RBTree the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  RBTreeInsert */
/**/
/*  INPUTS:  tree is the red-black tree to insert a node which has a key */
/*           pointed to by key and info pointed to by info.  */
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

rb_red_blk_node * RBTreeInsertWithPathCompression(rb_red_blk_tree* tree, const std::vector<box>& key, unsigned int level, const std::vector<int>& fieldOrder,int priority) {

	
  rb_red_blk_node * y;
  rb_red_blk_node * x;
  rb_red_blk_node * newNode;
 
  if (level == fieldOrder.size()) {  
	

	  tree->count++;
	  tree->PushPriority(priority);   
	  return nullptr;
  } 

  if (tree->count == 0) { 
	  //no need to create a node yet; just compress the path
	  tree->count++;
	  tree->PushPriority(priority);
	  //level <= b.size() -1 
	  for (size_t i = level; i < fieldOrder.size(); i++)  
		 tree->chain_boxes.push_back(key[fieldOrder[i]]);
	  return nullptr;
  }
  if (tree->count == 1) {
	  //path compression
	  auto temp_chain_boxes = tree->chain_boxes;
	  int xpriority =  tree->GetMaxPriority();
	
	//  tree->pq = std::priority_queue<int>();
	  tree->count++;
	//  tree->PushPriority(priority);
	 
	 /* bool is_identical = 1;
	  for (int i = level; i < fieldOrder.size(); i++) {
		  if (tree->chain_boxes[i - level] != key[fieldOrder[i]]){
			  is_identical = 0;
			  break;
		  }
	  }
	  if (is_identical) {
		  //TODO:: move tree->pq.push(priority); to this line
		  tree->count = 1;
		  return nullptr;
	  }*/
	  //quick check if identical just reset count = 1;
	  
	  tree->chain_boxes.clear();

	  //unzipping the next level 

	  std::vector<int> naturalFieldOrder(fieldOrder.size());
	  std::iota(begin(naturalFieldOrder), end(naturalFieldOrder), 0);
	  size_t run = 0; 
	  if (temp_chain_boxes[run][0] == key[fieldOrder[level + run]][0] && temp_chain_boxes[run][1] == key[fieldOrder[level + run]][1]) {
		//  printf("[%u %u] vs. [%u %u]\n", temp_chain_boxes[run][0], temp_chain_boxes[run][1], key[fieldOrder[level + run]][0], key[fieldOrder[level + run]][1]);
	 
		  x = RBTreeInsert(tree, key, level + run++, fieldOrder, xpriority);
		  if (level + run < fieldOrder.size()) {
			  while ((temp_chain_boxes[run][0] == key[fieldOrder[level + run]][0] && temp_chain_boxes[run][1] == key[fieldOrder[level + run]][1])) {

				  x->rb_tree_next_level = RBTreeCreate();

				  //  x->rb_tree_next_level->PushPriority(xpriority);
				  // x->rb_tree_next_level->PushPriority(priority);
				  x->rb_tree_next_level->count = 2;
				  x = RBTreeInsert(x->rb_tree_next_level, key, level + run, fieldOrder, priority);


				  run++;


				  if (level + run >= fieldOrder.size()) break;
			  }
		  }
		  if (level + run >= fieldOrder.size()) {
			  x->rb_tree_next_level = RBTreeCreate(); 
			  x->rb_tree_next_level->count++;
			  x->rb_tree_next_level->PushPriority(priority);
			  x->rb_tree_next_level->count++;
			  x->rb_tree_next_level->PushPriority(xpriority);
		  } else if (!(temp_chain_boxes[run][0] == key[fieldOrder[level + run]][0] && temp_chain_boxes[run][1] == key[fieldOrder[level + run]][1])) {
			  if (IsIntersect(temp_chain_boxes[run][0], temp_chain_boxes[run][1], key[fieldOrder[level + run]][0], key[fieldOrder[level + run]][1])) {
				  printf("Warning not intersect?\n");
				  printf("[%lu %lu] vs. [%lu %lu]\n", temp_chain_boxes[run][LowDim], temp_chain_boxes[run][HighDim], key[fieldOrder[level + run]][LowDim], key[fieldOrder[level + run]][HighDim]);
				  printf("chain_boxes:\n");
				  for (auto e : temp_chain_boxes)
					  printf("[%u %u] ", e[LowDim], e[HighDim]);
				  printf("\n boxes:\n");
				  for (size_t i = 0; i < key.size(); i++) {
					  printf("[%u %u] ", key[fieldOrder[i]][LowDim], key[fieldOrder[i]][HighDim]);
				  }
				  printf("\n");
				  exit(0);
			  }
			  //split into z and x node
			  x->rb_tree_next_level = RBTreeCreate(); 
			//  x->rb_tree_next_level->PushPriority(priority);
			 // x->rb_tree_next_level->PushPriority(xpriority);
			  auto PrependChainbox = [](std::vector<box>& cb, int n_prepend) {
				  std::vector<box> t;
				  for (int i = 0; i < n_prepend; i++) t.push_back({ { 999, 100020 } });
				  t.insert(end(t), begin(cb), end(cb));
				  return t;
			  };
			  auto z1 = RBTreeInsert(x->rb_tree_next_level, PrependChainbox(temp_chain_boxes, level), level + run, naturalFieldOrder, xpriority); 
			  auto z2 = RBTreeInsert(x->rb_tree_next_level, key, level + run, fieldOrder, priority); 

			  x->rb_tree_next_level->count = 2;

			  z1->rb_tree_next_level = RBTreeCreate(); 
			  z2->rb_tree_next_level = RBTreeCreate(); 
			  RBTreeInsertWithPathCompression(z1->rb_tree_next_level, PrependChainbox(temp_chain_boxes, level), level + run + 1, naturalFieldOrder, xpriority);
			  RBTreeInsertWithPathCompression(z2->rb_tree_next_level, key, level + run + 1, fieldOrder, priority);

		  }
	  } else { 

		  auto PrependChainbox = [](std::vector<box>& cb, int n_prepend) {
			  std::vector<box> t;
			  for (int i = 0; i < n_prepend; i++) t.push_back({ { 0, 10000000 } });
			  t.insert(end(t), begin(cb), end(cb));
			  return t;
		  };

		  auto z1 = RBTreeInsert(tree, PrependChainbox(temp_chain_boxes, level), level + run, naturalFieldOrder, xpriority);
		  auto z2 = RBTreeInsert(tree, key, level + run, fieldOrder, priority);
		  tree->count = 2;

		  z1->rb_tree_next_level = RBTreeCreate(); 
		  z2->rb_tree_next_level = RBTreeCreate();

		  RBTreeInsertWithPathCompression(z1->rb_tree_next_level, PrependChainbox(temp_chain_boxes, level), level + run + 1, naturalFieldOrder, xpriority);
		  RBTreeInsertWithPathCompression(z2->rb_tree_next_level, key, level + run + 1, fieldOrder, priority);

	  }
	  return nullptr;
  } 

  tree->count++;
 /* tree->PushPriority(priority);
  int maxpri = tree->GetMaxPriority();
  tree->priority_list.clear();
  tree->PushPriority(maxpri);*/


  x = (rb_red_blk_node*)SafeMalloc(sizeof(rb_red_blk_node));
  x->key = key[fieldOrder[level]];
  rb_red_blk_node * out_ptr;

  if (TreeInsertWithPathCompressionHelp(tree, x, key, level, fieldOrder, priority, out_ptr)){
	  
	  //insertion finds identical box.
	  //do nothing for now
	  return out_ptr;
  }

  newNode=x;
  x->red=1;
  while(x->parent->red) { /* use sentinel instead of checking for root */
    if (x->parent == x->parent->parent->left) {
      y=x->parent->parent->right;
      if (y->red) {
	x->parent->red=0;
	y->red=0;
	x->parent->parent->red=1;
	x=x->parent->parent;
      } else {
	if (x == x->parent->right) {
	  x=x->parent;
	  LeftRotate(tree,x);
	}
	x->parent->red=0;
	x->parent->parent->red=1;
	RightRotate(tree,x->parent->parent);
      } 
    } else { /* case for x->parent == x->parent->parent->right */
      y=x->parent->parent->left;
      if (y->red) {
	x->parent->red=0;
	y->red=0;
	x->parent->parent->red=1;
	x=x->parent->parent;
      } else {
	if (x == x->parent->left) {
	  x=x->parent;
	  RightRotate(tree,x);
	}
	x->parent->red=0;
	x->parent->parent->red=1;
	LeftRotate(tree,x->parent->parent);
      } 
    }
  }
  tree->root->left->red=0;
  return(newNode);

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not red in RBTreeInsert");
  Assert(!tree->root->red,"root not red in RBTreeInsert");
#endif
}

/*  Before calling Insert RBTree the node x should have its key set */

/***********************************************************************/
/*  FUNCTION:  RBTreeInsert */
/**/
/*  INPUTS:  tree is the red-black tree to insert a node which has a key */
/*           pointed to by key and info pointed to by info.  */
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

rb_red_blk_node * RBTreeInsert(rb_red_blk_tree* tree, const std::vector<box>& key, int level, const std::vector<int>& field_order, int priority) {

	if (level == key.size()) return nullptr;
 
	rb_red_blk_node * y;
	rb_red_blk_node * x;
	rb_red_blk_node * newNode;

	x = (rb_red_blk_node*)SafeMalloc(sizeof(rb_red_blk_node));
	x->key = key[field_order[level]];
	rb_red_blk_node * out_ptr;
	if (TreeInsertHelp(tree, x, key, level, field_order, priority, out_ptr)){
		//insertion finds identical box.
		//do nothing for now
		return out_ptr;
	}

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
					LeftRotate(tree, x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				RightRotate(tree, x->parent->parent);
			}
		} else { /* case for x->parent == x->parent->parent->right */
			y = x->parent->parent->left;
			if (y->red) {
				x->parent->red = 0;
				y->red = 0;
				x->parent->parent->red = 1;
				x = x->parent->parent;
			} else {
				if (x == x->parent->left) {
					x = x->parent;
					RightRotate(tree, x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				LeftRotate(tree, x->parent->parent);
			}
		}
	}
	tree->root->left->red = 0;
	//printf("Done: [%u %u]\n", newNode->key);
	return(newNode);

#ifdef DEBUG_ASSERT
	Assert(!tree->nil->red, "nil not red in RBTreeInsert");
	Assert(!tree->root->red, "root not red in RBTreeInsert");
#endif
}


bool TreeInsertHelp(rb_red_blk_tree* tree, rb_red_blk_node* z, const std::vector<box>& b, int level, const std::vector<int>& field_order, int priority, rb_red_blk_node*& out_ptr) {
	/*  This function should only be called by InsertRBTree  */
	rb_red_blk_node* x;
	rb_red_blk_node* y;
	rb_red_blk_node* nil = tree->nil;

	z->left = z->right = nil;
	y = tree->root;
	x = tree->root->left;
	while (x != nil) {
		y = x;
		int compare_result = CompareBox(x->key, z->key);
		if (compare_result == 1) { /* x.key > z.key */
			x = x->left;
		} else if (compare_result == -1) { /* x.key < z.key */
			x = x->right;
		} else if (compare_result == 0) {  /* x.key = z.key */
			printf("Warning compare_result == 0??\n");
			if (level != b.size() - 1) {
				RBTreeInsert(x->rb_tree_next_level, b, level + 1, field_order, priority);
			} else {
			//	x->node_max_priority = std::max(x->node_max_priority, priority);
				//x->nodes_priority[x->num_node_priority++] = priority;
				out_ptr = x;
			}
			free(z);
			return true;
		} else {  /* x.key || z.key */
			printf("x:[%u %u], z:[%u %u]\n", x->key[LowDim], x->key[HighDim], z->key[LowDim], z->key[HighDim]);
			printf("Warning TreeInsertHelp : x.key || z.key\n");
		}
	}
	z->parent = y;
	if ((y == tree->root) ||
		(1 == CompareBox(y->key, z->key))) { /* y.key > z.key */
		y->left = z;
	} else {
		y->right = z;
	}
	//found new one to insert to but will not propagate
	out_ptr = z;

	return false;

#ifdef DEBUG_ASSERT
	Assert(!tree->nil->red, "nil not red in TreeInsertHelp");
#endif
}


/***********************************************************************/
/*  FUNCTION:  TreeSuccessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the successor of. */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
  
rb_red_blk_node* TreeSuccessor(rb_red_blk_tree* tree,rb_red_blk_node* x) { 
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;

  if (nil != (y = x->right)) { /* assignment to y is intentional */
    while(y->left != nil) { /* returns the minium of the right subtree of x */
      y=y->left;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->right) { /* sentinel used instead of checking for nil */
      x=y;
      y=y->parent;
    }
    if (y == root) return(nil);
    return(y);
  }
}

/***********************************************************************/
/*  FUNCTION:  Treepredecessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the predecessor of. */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/

rb_red_blk_node* TreePredecessor(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* y;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;

  if (nil != (y = x->left)) { /* assignment to y is intentional */
    while(y->right != nil) { /* returns the maximum of the left subtree of x */
      y=y->right;
    }
    return(y);
  } else {
    y=x->parent;
    while(x == y->left) { 
      if (y == root) return(nil); 
      x=y;
      y=y->parent;
    }
    return(y);
  }
}

/***********************************************************************/
/*  FUNCTION:  InorderTreePrint */
/**/
/*    INPUTS:  tree is the tree to print and x is the current inorder node */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively prints the nodes of the tree */
/*              inorder using the PrintKey and PrintInfo functions. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:    This function should only be called from RBTreePrint */
/***********************************************************************/

void InorderTreePrint(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;
  if (x != tree->nil) {
    InorderTreePrint(tree,x->left);
    //printf("  key="); 
	printf("tree->count = %d\n", tree->count);
  tree->PrintKey(x->key);
  //  printf("  l->key=");
   // if( x->left != nil) tree->PrintKey(x->left->key);
  //  printf("  r->key=");
  //  if( x->right != nil)  tree->PrintKey(x->right->key);
   // printf("  p->key=");
   // if( x->parent != root) /*printf("NULL"); else*/ tree->PrintKey(x->parent->key);
   // printf("  red=%i\n",x->red);
    InorderTreePrint(tree,x->right);
  }
}


/***********************************************************************/
/*  FUNCTION:  TreeDestHelper */
/**/
/*    INPUTS:  tree is the tree to destroy and x is the current node */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively destroys the nodes of the tree */
/*              postorder using the DestroyKey and DestroyInfo functions. */
/**/
/*    Modifies Input: tree, x */
/**/
/*    Note:    This function should only be called by RBTreeDestroy */
/***********************************************************************/

void TreeDestHelper(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* nil=tree->nil;
  if (x != nil) {
	  if (x->rb_tree_next_level != nullptr)
		RBTreeDestroy(x->rb_tree_next_level);
    TreeDestHelper(tree,x->left);
    TreeDestHelper(tree,x->right);
    delete x;
  }
}


/***********************************************************************/
/*  FUNCTION:  RBTreeDestroy */
/**/
/*    INPUTS:  tree is the tree to destroy */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Destroys the key and frees memory */
/**/
/*    Modifies Input: tree */
/**/
/***********************************************************************/

void RBTreeDestroy(rb_red_blk_tree* tree) {
   TreeDestHelper(tree,tree->root->left);
  delete (tree->root);
  delete tree->nil;
  delete tree; 
}


/***********************************************************************/
/*  FUNCTION:  RBTreePrint */
/**/
/*    INPUTS:  tree is the tree to print */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  This function recursively prints the nodes of the tree */
/*             inorder using the PrintKey and PrintInfo functions. */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/

void RBTreePrint(rb_red_blk_tree* tree) {
  InorderTreePrint(tree,tree->root->left);
}


/***********************************************************************/
/*  FUNCTION:  RBExactQuery */
/**/
/*    INPUTS:  tree is the tree to print and q is a pointer to the key */
/*             we are searching for */
/**/
/*    OUTPUT:  returns the a node with key equal to q.  If there are */
/*             multiple nodes with key equal to q this function returns */
/*             the one highest in the tree */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/
  
int RBExactQuery( rb_red_blk_tree*  tree, const Packet& q,int level, const std::vector<int>& fieldOrder) {

	//printf("entering level %d - tree->GetMaxPriority =%d\n", level,tree->GetMaxPriority());

  //check if singleton
	if (level == fieldOrder.size()) {
		return tree->GetMaxPriority();
	}
  else if (tree->count == 1) {  
	//  auto chain_boxes = tree->chain_boxes; 
	  for (size_t i = level; i < fieldOrder.size(); i++) { 
		  if (q[fieldOrder[i]] < tree->chain_boxes[i - level][0]) return -1;
		  if (q[fieldOrder[i]] > tree->chain_boxes[i - level][1]) return -1;
	  }
	  return tree->GetMaxPriority();
  }

  rb_red_blk_node* x = tree->root->left;
  rb_red_blk_node* nil = tree->nil;
  int compVal;
  if (x == nil) return -1;
  compVal= CompareQuery(x->key,  q, level, fieldOrder);
 // printf("Compval = %d\n", compVal);
  while(0 != compVal) {/*assignemnt*/
    if (1 == compVal) { /* x->key > q */
      x=x->left;
    } 
	else if (-1 == compVal) /*x->key < q */{
      x=x->right;
    }
	if (x == nil) return -1;
	compVal = CompareQuery(x->key, q, level, fieldOrder);
  }

 // printf("level = %d, priority = %d\n", level, x->mid_ptr.node_max_priority);
  return RBExactQuery(x->rb_tree_next_level,q,level+1,fieldOrder);
}


int RBExactQueryIterative(rb_red_blk_tree*  tree, const Packet& q, const std::vector<int>& fieldOrder) {
	 
	int level = 0;
	int compVal;
	while (true) { 
		//check if singleton 
		if (level == fieldOrder.size()) {
			return tree->GetMaxPriority();
		} else if (tree->count == 1) {
			//  auto chain_boxes = tree->chain_boxes; 
			for (size_t i = level; i < fieldOrder.size(); i++) {
				if (q[fieldOrder[i]] < tree->chain_boxes[i - level][0]) return -1;
				if (q[fieldOrder[i]] > tree->chain_boxes[i - level][1]) return -1;
			}
		 
			return tree->GetMaxPriority();
		}

 		rb_red_blk_node* x = tree->root->left;
		rb_red_blk_node* nil = tree->nil;
		
		if (x == nil) return -1;
		compVal = CompareQuery(x->key, q, level, fieldOrder);
		// printf("Compval = %d\n", compVal);
		while (0 != compVal) {/*assignemnt*/
			if (1 == compVal) { /* x->key > q */
				x = x->left;
			} else if (-1 == compVal) /*x->key < q */{
				x = x->right;
			}
			if (x == nil) return -1;
			compVal = CompareQuery(x->key, q, level, fieldOrder);
		}
		tree = x->rb_tree_next_level;
		level++;
	}

	// printf("level = %d, priority = %d\n", level, x->mid_ptr.node_max_priority);
	//return RBExactQuery(x->rb_tree_next_level, q, level + 1, fieldOrder);
}

int RBExactQueryPriority(rb_red_blk_tree*  tree, const Packet& q, int level, const std::vector<int>& fieldOrder, int priority_so_far) {

	//printf("entering level %d - tree->GetMaxPriority =%d\n", level,tree->GetMaxPriority());
	if (priority_so_far > tree->GetMaxPriority()) return -1;
	//check if singleton
	if (level == fieldOrder.size()) {
		return tree->GetMaxPriority();
	} else if (tree->count == 1) {
		//  auto chain_boxes = tree->chain_boxes; 
		for (size_t i = level; i < fieldOrder.size(); i++) {
			if (q[fieldOrder[i]] < tree->chain_boxes[i - level][0]) return -1;
			if (q[fieldOrder[i]] > tree->chain_boxes[i - level][1]) return -1;
		}
		return tree->GetMaxPriority();
	}

	rb_red_blk_node* x = tree->root->left;
	rb_red_blk_node* nil = tree->nil;
	int compVal;
	if (x == nil) return -1;
	compVal = CompareQuery(x->key, q, level, fieldOrder);
	// printf("Compval = %d\n", compVal);
	while (0 != compVal) {/*assignemnt*/
		if (1 == compVal) { /* x->key > q */
			x = x->left;
		} else if (-1 == compVal) /*x->key < q */{
			x = x->right;
		}
		if (x == nil) return -1;
		compVal = CompareQuery(x->key, q, level, fieldOrder);
	}

	// printf("level = %d, priority = %d\n", level, x->mid_ptr.node_max_priority);
	return RBExactQueryPriority(x->rb_tree_next_level, q, level + 1, fieldOrder,priority_so_far);
}

/***********************************************************************/
/*  FUNCTION:  RBDeleteFixUp */
/**/
/*    INPUTS:  tree is the tree to fix and x is the child of the spliced */
/*             out node in RBTreeDelete. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: tree, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

void RBDeleteFixUp(rb_red_blk_tree* tree, rb_red_blk_node* x) {
  rb_red_blk_node* root=tree->root->left;
  rb_red_blk_node* w;

  while( (!x->red) && (root != x)) {
    if (x == x->parent->left) {
      w=x->parent->right;
      if (w->red) {
	w->red=0;
	x->parent->red=1;
	LeftRotate(tree,x->parent);
	w=x->parent->right;
      }
      if ( (!w->right->red) && (!w->left->red) ) { 
	w->red=1;
	x=x->parent;
      } else {
	if (!w->right->red) {
	  w->left->red=0;
	  w->red=1;
	  RightRotate(tree,w);
	  w=x->parent->right;
	}
	w->red=x->parent->red;
	x->parent->red=0;
	w->right->red=0;
	LeftRotate(tree,x->parent);
	x=root; /* this is to exit while loop */
      }
    } else { /* the code below is has left and right switched from above */
      w=x->parent->left;
      if (w->red) {
	w->red=0;
	x->parent->red=1;
	RightRotate(tree,x->parent);
	w=x->parent->left;
      }
      if ( (!w->right->red) && (!w->left->red) ) { 
	w->red=1;
	x=x->parent;
      } else {
	if (!w->left->red) {
	  w->right->red=0;
	  w->red=1;
	  LeftRotate(tree,w);
	  w=x->parent->left;
	}
	w->red=x->parent->red;
	x->parent->red=0;
	w->left->red=0;
	RightRotate(tree,x->parent);
	x=root; /* this is to exit while loop */
      }
    }
  }
  x->red=0;

#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not black in RBDeleteFixUp");
#endif
}


void ClearStack(std::stack<std::pair<rb_red_blk_tree *, rb_red_blk_node *>>& st, rb_red_blk_tree* tree) {
	while (!st.empty()) {
		auto e = st.top();
		st.pop();
		if (e.first == tree) {
			RBDelete(tree, e.second);
			return;
		}
		RBDelete(e.first, e.second);
		RBTreeDestroy(e.first);
	}

}
void RBTreeDeleteWithPathCompression(rb_red_blk_tree*& tree, const std::vector<box>& key, int level, const std::vector<int>& fieldOrder, int priority, bool& JustDeletedTree) {

 
	if (level == fieldOrder.size()) {
		tree->count--;
		tree->PopPriority(priority); 
		if (tree->count == 0) {
			RBTreeDestroy(tree);
			JustDeletedTree = true;
		}
		return ;
	}
	if (tree->count == 1) {
	

		if (level == 0) {
			tree->count = 0;
			tree->chain_boxes.clear();
			tree->ClearPriority(); 
		}
		else {
			RBTreeDestroy(tree); 
			JustDeletedTree = true;
		}
		return ;
	}
	if (tree->count == 2) {
		int run = 0;
		rb_red_blk_tree * temp_tree = tree;
	
		//first time tree->count ==2; this mean you need to create chain box here;
		//keep going until you find the node that contians tree->count = 1
	//	std::stack<std::pair<rb_red_blk_tree *, rb_red_blk_node *>> stack_so_far;

		while (true) {
			if (temp_tree->count == 1 || level + run == fieldOrder.size()) {
				tree->chain_boxes.insert(end(tree->chain_boxes), begin(temp_tree->chain_boxes), end(temp_tree->chain_boxes));
		
				auto newtree = RBTreeCreate();
				newtree->chain_boxes = tree->chain_boxes;

				if (temp_tree->GetSizeList() == 2) {
					temp_tree->PopPriority(priority);
				}
				int new_priority = temp_tree->GetMaxPriority();
				RBTreeDestroy(tree);

				tree = newtree;
				tree->count = 1;
				tree->PushPriority(new_priority);
				return ;
			}
			/*if (level + run == fieldOrder.size()) {
				auto newtree = RBTreeCreate();
				newtree->chain_boxes = tree->chain_boxes;
				RBTreeDestroy(tree);
				tree = newtree;
				tree->count = 1;
				if (temp_tree->GetSizeList() == 2) {
					temp_tree->PopPriority(priority);
				}
				tree->PushPriority(temp_tree->GetMaxPriority());

			//	RBTreeDestroy(temp_tree);
				//ClearStack(stack_so_far, tree);
				//tree->PopPriority(priority);
				return ;
			}*/
			temp_tree->count--;
			rb_red_blk_node * x = temp_tree->root->left;
			if (x->left == temp_tree->nil && x->right == temp_tree->nil) {
				tree->chain_boxes.push_back(x->key);
				//stack_so_far.push(std::make_pair(temp_tree, x));
				temp_tree = x->rb_tree_next_level;
			}
			else 
			{
				int compare_result = CompareBox(x->key, key[fieldOrder[level+run]]);
				//stack_so_far.push(std::make_pair(temp_tree, x));
				if (compare_result == 0) { //hit top = delete top then go leaf node to collect correct chain box
					if (x->left == temp_tree->nil) {
						temp_tree = x->right->rb_tree_next_level;
						tree->chain_boxes.push_back(x->right->key);
					}
					else {
						temp_tree = x->left->rb_tree_next_level;
						tree->chain_boxes.push_back(x->left->key);
					}
				//	tree->chain_boxes.push_back(x->left == temp_tree->nil?x->right->key:x->left->key);
				//	temp_tree = x->rb_tree_next_level;
				}
				else {
					temp_tree = x->rb_tree_next_level;
					tree->chain_boxes.push_back(x->key);
				}
				
				/*if (compare_result == -1){  root < z.key 
						//delete right child
						//stack_so_far.push(std::make_pair(temp_tree, x->right)); 
						tree->chain_boxes.push_back( x->key);
						temp_tree = x->right->rb_tree_next_level;
				} else if(compare_result == 1) {
						//delete left child
						//stack_so_far.push(std::make_pair(temp_tree, x->left));
						temp_tree = x->left->rb_tree_next_level;
						tree->chain_boxes.push_back(x->key);
				} else {
					printf("Warning::RBTreeDeleteWithPathCompression Overlap at level %d\n",level+run);
				}*/
				
			}
			run++;
		
		}
		return ;
	}

	
	rb_red_blk_node* x;
	rb_red_blk_node* y;
	rb_red_blk_node* nil = tree->nil;
	y = tree->root;
	x = tree->root->left;


	while (x != nil) {
		y = x;
		int compare_result = CompareBox(x->key, key[fieldOrder[level]]);
		if (compare_result == 1) { /* x.key > z.key */
			x = x->left;
		} else if (compare_result == -1) { /* x.key < z.key */
			x = x->right;
		} else if (compare_result == 0) {  /* x.key = z.key */ 	
			bool justDelete = false;
			tree->count--;
			RBTreeDeleteWithPathCompression(x->rb_tree_next_level, key, level + 1, fieldOrder, priority, justDelete);
			if (justDelete) RBDelete(tree, x);

			return; 
		
		} else {  /* x.key || z.key */
			printf("x:[%u %u], key:[%u %u]\n", x->key[LowDim], x->key[HighDim], key[fieldOrder[level]][LowDim], key[fieldOrder[level]][HighDim]);
			printf("Warning RBFindNodeSequence : x.key || key[fieldOrder[level]]\n");
		}
	}

	printf("Error RBTreeDeleteWithPathCompression: can't find a node at level %d\n", level);
	exit(0);


}
/***********************************************************************/
/*  FUNCTION:  RBDelete */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Deletes z from tree and frees the key and info of z */
/*             using DestoryKey and DestoryInfo.  Then calls */
/*             RBDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input: tree, z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/

void RBDelete(rb_red_blk_tree* tree, rb_red_blk_node* z){
  rb_red_blk_node* y;
  rb_red_blk_node* x;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* root=tree->root;

  y= ((z->left == nil) || (z->right == nil)) ? z : TreeSuccessor(tree,z);
  x= (y->left == nil) ? y->right : y->left;
  if (root == (x->parent = y->parent)) { /* assignment of y->p to x->p is intentional */
    root->left=x;
  } else {
    if (y == y->parent->left) {
      y->parent->left=x;
    } else {
      y->parent->right=x;
    }
  }
  if (y != z) { /* y should not be nil in this case */

#ifdef DEBUG_ASSERT
    Assert( (y!=tree->nil),"y is nil in RBDelete\n");
#endif
    /* y is the node to splice out and x is its child */

    if (!(y->red)) RBDeleteFixUp(tree,x);
  
  //  tree->DestroyKey(z->key);
  //  tree->DestroyInfo(z->info);
    y->left=z->left;
    y->right=z->right;
    y->parent=z->parent;
    y->red=z->red;
    z->left->parent=z->right->parent=y;
    if (z == z->parent->left) {
      z->parent->left=y; 
    } else {
      z->parent->right=y;
    }
    free(z); 
  } else {
//    tree->DestroyKey(y->key);
 //   tree->DestroyInfo(y->info);
    if (!(y->red)) RBDeleteFixUp(tree,x);
    free(y);
  }
  
#ifdef DEBUG_ASSERT
  Assert(!tree->nil->red,"nil not black in RBDelete");
#endif
}


/***********************************************************************/
/*  FUNCTION:  RBDEnumerate */
/**/
/*    INPUTS:  tree is the tree to look for keys >= low */
/*             and <= high with respect to the Compare function */
/**/
/*    OUTPUT:  stack containing pointers to the nodes between [low,high] */
/**/
/*    Modifies Input: none */
/***********************************************************************/

stk_stack* RBEnumerate(rb_red_blk_tree* tree,const box& low, const box&  high) {
  stk_stack* enumResultStack;
  rb_red_blk_node* nil=tree->nil;
  rb_red_blk_node* x=tree->root->left;
  rb_red_blk_node* lastBest=nil;

  enumResultStack=StackCreate();
  while(nil != x) {
    if ( 1 == (CompareBox(x->key,high)) ) { /* x->key > high */
      x=x->left;
    } else {
      lastBest=x;
      x=x->right;
    }
  }
  while ( (lastBest != nil) && (1 != CompareBox(low,lastBest->key))) {
    StackPush(enumResultStack,lastBest);
    lastBest=TreePredecessor(tree,lastBest);
  }
  return(enumResultStack);
}

/***********************************************************************/
/*  FUNCTION:  RBSerializeIntoRulesRecursion */
/**/
/*    INPUTS:  tree: 'tree' at 'level' for current 'fieldOrder'
/*              
/**/
/*    OUTPUT:  a vector of Rule */
/**/
/*    Modifies Input: none */
/***********************************************************************/

void  RBSerializeIntoRulesRecursion(rb_red_blk_tree * treenode, rb_red_blk_node * node, int level, const std::vector<int>& fieldOrder, std::vector<box>& box_so_far, std::vector<Rule>& rules_so_far) {
	if (level == fieldOrder.size() ) {
		for (int n : treenode->priority_list) {
			Rule r(fieldOrder.size());
			for (int i = 0; i < r.dim; i++){
				r.range[fieldOrder[i]] = box_so_far[i];
			}
			r.priority = n;
			rules_so_far.push_back(r);
		}
		return;
	}
	if (treenode->count == 1) {
		box_so_far.insert(std::end(box_so_far), begin(treenode->chain_boxes), end(treenode->chain_boxes));
		for (int n : treenode->priority_list) {
			Rule r(fieldOrder.size());

			for (int i = 0; i < r.dim; i++){
				r.range[fieldOrder[i]] = box_so_far[i];
			}
			r.priority = n;
			rules_so_far.push_back(r);
		}
		
		for (size_t i = 0; i < treenode->chain_boxes.size(); i++)
			box_so_far.pop_back();

		return;
	}

	if (treenode->nil == node) return;

	box_so_far.push_back(node->key);
	auto tree = node->rb_tree_next_level;
	RBSerializeIntoRulesRecursion(tree,tree->root->left, level + 1, fieldOrder, box_so_far, rules_so_far);
	box_so_far.pop_back();

	RBSerializeIntoRulesRecursion(treenode,node->left, level, fieldOrder, box_so_far, rules_so_far);
	RBSerializeIntoRulesRecursion(treenode,node->right, level, fieldOrder, box_so_far, rules_so_far);

}

std::vector<Rule> RBSerializeIntoRules(rb_red_blk_tree* tree, const std::vector<int>& fieldOrder) {
	std::vector<box> boxes_so_far;
	std::vector<Rule> rules_so_far;
	RBSerializeIntoRulesRecursion(tree,tree->root->left, 0, fieldOrder, boxes_so_far, rules_so_far);
	return rules_so_far;
}


/******************/



//Practice of self-documenting codes.

int  CalculateMemoryConsumptionRecursion(rb_red_blk_tree * treenode, rb_red_blk_node * node, int level, const std::vector<int>& fieldOrder) {
	if (level == fieldOrder.size()) {

		int num_rules_in_this_leaf = treenode->priority_list.size();
		int sizeofint = 4;
		int also_max_priority = 1;
		return (num_rules_in_this_leaf + also_max_priority)*sizeofint;

	}
	if (treenode->count == 1) {
		int size_of_remaining_intervals = 0;
		for (size_t i = level; i < fieldOrder.size(); i++) {
			int field = fieldOrder[i];
			int intervalsize;
			if (field == 0 || field == 1) {
				intervalsize = 9;
			}
			if (field == 2 || field == 3) {
				intervalsize = 4;
			}
			if (field == 4) {
				intervalsize = 1;
			}
			size_of_remaining_intervals += intervalsize;
		} 
		int num_rules_in_this_leaf = treenode->priority_list.size();
		int sizeofint = 4;
		int also_max_priority = 1;

		return   size_of_remaining_intervals + (num_rules_in_this_leaf + also_max_priority)*sizeofint;
	}

	if (treenode->nil == node) return 0;
	
	int memory_usage = 0; 
	auto tree = node->rb_tree_next_level;

	memory_usage += CalculateMemoryConsumptionRecursion(tree, tree->root->left, level + 1, fieldOrder);
	memory_usage += CalculateMemoryConsumptionRecursion(treenode, node->left, level, fieldOrder);
	memory_usage += CalculateMemoryConsumptionRecursion(treenode, node->right, level, fieldOrder);

	int number_pointers_in_internal_node = 4;
	int aux_data_internal_node_byte = 1;
	int sizeofint = 4;
	int also_max_priority_interval_node = 1;

	int field = fieldOrder[level]%5;
	int intervalsize;
	if (field == 0 || field == 1) {
		intervalsize = 9;
	}
	if (field == 2 || field == 3) {
		intervalsize = 4;
	}
	if (field == 4) {
		intervalsize = 1;
	}
	return memory_usage + intervalsize+ (number_pointers_in_internal_node+ also_max_priority_interval_node) *sizeofint + aux_data_internal_node_byte;
}

int CalculateMemoryConsumption(rb_red_blk_tree* tree, const std::vector<int>& fieldOrder) {

	return CalculateMemoryConsumptionRecursion(tree, tree->root->left, 0, fieldOrder);
}
