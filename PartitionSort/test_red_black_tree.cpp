#include"OptimizedMITree.h"

#include<stdio.h>
#include<ctype.h> 

/*
int main() {
	Rule r1(4);
	r1.priority = 1;
	r1.range[0] = { { 1, 3 } };
	r1.range[1] = { { 1, 3 } }; 
	r1.range[2] = { { 0, 1 } };
	r1.range[3] = { { 0, 1 } };
	Rule r2(4);
	r2.priority = 2;
	r2.range[0] = { { 1, 3 } };
	r2.range[1] = { { 1, 3 } };
	r2.range[2] = { { 0, 1 } };
	r2.range[3] = { { 0, 3 } };
	Rule r3(4);
	r3.priority = 3;
	r3.range[0] = { { 2, 2 } };
	r3.range[1] = { { 2, 2 } };
	r3.range[2] = { { 0, 1 } };
	r3.range[3] = { { 0, 2 } };
	Rule r4(4);
	r4.priority = 4;
	r4.range[0] = { { 1, 3 } };
	r4.range[1] = { { 1, 3 } };
	r4.range[2] = { { 0, 1 } };
	r4.range[3] = { { 2, 5 } };
	Rule r5(4);
	r5.priority = 5;
	r5.range[0] = { { 1, 3 } };
	r5.range[1] = { { 4, 5 } };
	r5.range[2] = { { 0, 1 } };
	r5.range[3] = { { 0, 1 } };

	OptimizedMITree mitree;
	bool prioritychange = 0;
	mitree.Insertion(r1,prioritychange); 
	//mitree.Insertion(r4,prioritychange);
	//mitree.Insertion(r3,prioritychange);
	mitree.Insertion(r4,prioritychange);
	 
	//printf("Inserting r5\n");
	mitree.Insertion(r5,prioritychange);
	 
	printf("Done\n");
	Packet p5 = { 1, 4, 1, 1 };
	Packet p1 = { 1, 1, 1, 1 };
	Packet p2 = { 1, 1, 1, 2 };  
	printf("p1 result: %d\n", mitree.ClassifyAPacket(p1));
	printf("p2 result: %d\n", mitree.ClassifyAPacket(p2));
	printf("p5 result: %d\n", mitree.ClassifyAPacket(p5));
	//r1.priority = 120;
	//mitree.Deletion(r5, prioritychange); 
	mitree.Deletion(r1, prioritychange); 
 
	printf("last query\n");
	printf("p1 result: %d\n", mitree.ClassifyAPacket(p1));

	//NOW TEST SERIALIZE
	//only r4 alone
	auto vr4 = mitree.SerializeIntoRules();
	auto PrintRule = [](const Rule& r) {
		printf("Priority %d\n", r.priority);
		for (int i = 0; i < r.dim; i++){
			printf("[%u %u] ", r.range[i]);
		}
		printf("\n");
	};
	auto PrintVectorRules = [&PrintRule](const std::vector<Rule>& vr) {
		for (const auto &r : vr) {
			PrintRule(r);
		}
	};
	 
	PrintVectorRules(vr4);

	mitree.Insertion(r1);
	mitree.Insertion(r5);

	PrintVectorRules(mitree.SerializeIntoRules());

	//mitree.Deletion(r1, prioritychange);
	//printf("p1 result: %d\n", mitree.ClassifyAPacket(p1));
}  */
/*
int main() {
  stk_stack* enumResult;
  int option=0;
  int newKey,newKey2;
  int* newInt;
  rb_red_blk_node* newNode;
  rb_red_blk_tree* tree;

  tree=RBTreeCreate(IntComp,IntDest,InfoDest,IntPrint,InfoPrint);
  while(option!=8) {
    printf("choose one of the following:\n");
    printf("(1) add to tree\n(2) delete from tree\n(3) query\n");
    printf("(4) find predecessor\n(5) find sucessor\n(6) enumerate\n");
    printf("(7) print tree\n(8) quit\n");
    do option=fgetc(stdin); while(-1 != option && isspace(option));
    option-='0';
    switch(option)
      {
      case 1:
	{
	  printf("type key for new node\n");
	  scanf("%i",&newKey);
	  newInt=(int*) malloc(sizeof(int));
	  *newInt=newKey;
	  RBTreeInsert(tree,newInt,0);
	}
	break;
	
      case 2:
	{
	  printf("type key of node to remove\n");
	  scanf("%i",&newKey);
	  if ( ( newNode=RBExactQuery(tree,&newKey ) ) ) RBDelete(tree,newNode);/*assignment
	  else printf("key not found in tree, no action taken\n");
	}
	break;

      case 3:
	{
	  printf("type key of node to query for\n");
	  scanf("%i",&newKey);
	  if ( ( newNode = RBExactQuery(tree,&newKey) ) ) {/*assignment
	    printf("data found in tree at location %i\n",(int)newNode);
	  } else {
	    printf("data not in tree\n");
	  }
	}
	break;
      case 4:
	{
	  printf("type key of node to find predecessor of\n");
	  scanf("%i",&newKey);
	  if ( ( newNode = RBExactQuery(tree,&newKey) ) ) {/*assignment
	    newNode=TreePredecessor(tree,newNode);
	    if(tree->nil == newNode) {
	      printf("there is no predecessor for that node (it is a minimum)\n");
	    } else {
	      printf("predecessor has key %i\n",*(int*)newNode->key);
	    }
	  } else {
	    printf("data not in tree\n");
	  }
	}
	break;
      case 5:
	{
	  printf("type key of node to find successor of\n");
	  scanf("%i",&newKey);
	  if ( (newNode = RBExactQuery(tree,&newKey) ) ) {
	    newNode=TreeSuccessor(tree,newNode);
	    if(tree->nil == newNode) {
	      printf("there is no successor for that node (it is a maximum)\n");
	    } else {
	      printf("successor has key %i\n",*(int*)newNode->key);
	    }
	  } else {
	    printf("data not in tree\n");
	  }
	}
	break;
      case 6:
	{
	  printf("type low and high keys to see all keys between them\n");
	  scanf("%i %i",&newKey,&newKey2);
	  enumResult=RBEnumerate(tree,&newKey,&newKey2);	  
	  while ( (newNode = StackPop(enumResult)) ) {
	    tree->PrintKey(newNode->key);
	    printf("\n");
	  }
	  free(enumResult);
	}
	break;
      case 7:
	{
	  RBTreePrint(tree);
	}
	break;
      case 8:
	{
	  RBTreeDestroy(tree);
	  return 0;
	}
	break;
      default:
	printf("Invalid input; Please try again.\n");
      }
  }
  return 0;
}
*/



