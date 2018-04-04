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
#include "MITree.h"
//extern int global_counter;
MITreeRule *  MITree::InsertRule(const Rule& one_rule) {

	if (CanInsertRule(one_rule)) {
		num_rules++;
		auto it = new MITreeRule(one_rule.priority, std::vector<IntervalTreeNode *>(field_order.size()), std::vector<std::list<MITreeRule *>::iterator>(field_order.size()), one_rule);
		InsertRuleRecursion(one_rule, it, root, 0);

		MRules.push_back(it);
		return it;
	}
	else return nullptr;

}
MITreeRule * PriorityMITree::InsertRule(const Rule& one_rule) {

	max_priority = std::max(one_rule.priority, max_priority);
	priority_container.insert(one_rule.priority);
	num_rules++;
	auto it = new MITreeRule(one_rule.priority, std::vector<IntervalTreeNode *>(field_order.size()), std::vector<std::list<MITreeRule *>::iterator>(field_order.size()), one_rule);
	InsertRuleRecursion(one_rule, it, root, 0);

	MRules.push_back(it);
	return it;

}
MITreeRule * PriorityMITree::InsertRule(const Rule& one_rule, bool& priority_track) {
	
	if (CanInsertRule(one_rule)) {
		
		if (max_priority < one_rule.priority) priority_track = true;
		else priority_track = false;
		return PriorityMITree::InsertRule(one_rule);
	} else {
	
		return nullptr;
	}
}
bool MITree::CanInsertRule(const Rule& one_rule) const {
	return CanInsertRuleRecursion(one_rule, root, 0);
}

void MITree::InsertRuleRecursion(const Rule& one_rule,MITreeRule * rule_holder, MInterval* current_interval, int level) {

	current_interval->IncrementCount();
	//TODO:: Semantic Check
	IntervalTreeNode * foundGINode = current_interval->GetNodeFromIntervalTree(one_rule.range[field_order[level]][LOW], one_rule.range[field_order[level]][HIGH]);// it->FirstIntervalSearch(one_rule.range[field_order[level]][LOW], one_rule.range[field_order[level]][HIGH]);
	//if not found, (sentinel node) nil is returned, which is a node that contains nullptr 
	auto foundGI = (MInterval*)(foundGINode->GetStoredInterval());
	if (!foundGI) {
		rule_holder->MITree_node_ptrs[level] = current_interval->InsertMInterval(BuildMIntervalFromLevel(one_rule, rule_holder, level + 1));
		rule_holder->MITree_rule_ptrs_in_node[level] = ((MInterval *)(rule_holder->MITree_node_ptrs[level]->GetStoredInterval()))->InsertRuleHolder(rule_holder);
		return;
	} else if (foundGI->GetHighPoint() == (one_rule.range[field_order[level]][HIGH]) && foundGI->GetLowPoint() == (one_rule.range[field_order[level]][LOW])) {
		if (level == depth - 1) {
			foundGI->IncrementCount();
			rule_holder->MITree_node_ptrs[level] = foundGINode;
			rule_holder->MITree_rule_ptrs_in_node[level] = foundGI->InsertRuleHolder(rule_holder);
			// done
			return;
		} else {
			rule_holder->MITree_node_ptrs[level] = foundGINode;
			rule_holder->MITree_rule_ptrs_in_node[level] = foundGI->InsertRuleHolder(rule_holder);
			InsertRuleRecursion(one_rule,rule_holder, foundGI, level + 1);
			return;
		}
	} else {
		//Found partial overlap
		printf("Error::MITree InsertRule:: Found partial overlap \n foundGI = [%u, %u] \n Rule = [%u %u]\n", foundGI->GetLowPoint(), foundGI->GetHighPoint(), one_rule.range[field_order[level]][LOW], one_rule.range[field_order[level]][HIGH]);
		exit(1);
		return;
	}
}

bool MITree::CanInsertRuleRecursion(const Rule& one_rule, MInterval * current_interval, int level) const {

	MInterval * foundMI = current_interval->GetNextMInterval(one_rule.range[field_order[level]][LOW], one_rule.range[field_order[level]][HIGH]);

	if (!foundMI) return true;
	else if (foundMI->GetHighPoint() == (one_rule.range[field_order[level]][HIGH]) && foundMI->GetLowPoint() == (one_rule.range[field_order[level]][LOW])) {
		
		return 	 (level == depth - 1) ? true : CanInsertRuleRecursion(one_rule, foundMI, level + 1);
	} else 
	return false;
}

MInterval * MITree::BuildMIntervalFromLevel(const Rule& one_rule, MITreeRule * rule_holder, int level) {

	IntervalTree * it = new IntervalTree();
	
	if (level < depth) {
		rule_holder->MITree_node_ptrs[level] = it->Insert(BuildMIntervalFromLevel(one_rule, rule_holder, level + 1));
		rule_holder->MITree_rule_ptrs_in_node[level] = ((MInterval *)(rule_holder->MITree_node_ptrs[level]->GetStoredInterval()))->InsertRuleHolder(rule_holder);
	}

	return new MInterval(one_rule.range[field_order[level - 1]][LOW], one_rule.range[field_order[level - 1]][HIGH], it);
}

void MITree::Print() const {
	std::queue<Interval *> q;
	q.push(root);
	while (!q.empty()) {
		MInterval * current = (MInterval *)(q.front());
		q.pop();

		if (current != root) current->Print();

	/*	std::vector<std::shared_ptr<IntervalTreeNode>> vit = current->TraverseInOrderIntoVector();
		for (auto it : vit) {
			q.push(it->GetStoredInterval());
		}*/
	}
}
/*
void SmartPriorityMITree::DeleteRule(MITreeRule * mrule, bool& priority_track) {
	 
	if (mrule == nullptr) printf("warning nullptr\n");
 //maintain multiset of length of interval;
	auto range = mrule->one_rule.range[field_order[0]];
	unsigned long long length = range[HIGH] - range[LOW] + 1; 
	auto itr = length_container.equal_range(length); 
	length_container.erase(itr.first);
	if (num_rules == 1) {
		max_length = 0;
		field_guard = 0;
	} else if (length == max_length) {
		max_length = *length_container.rbegin();
		field_guard = IsThisLengthLarge(max_length);
	}
	
	PriorityMITree::DeleteRule(mrule, priority_track);

}*/
void PriorityMITree::DeleteRule(MITreeRule * mrule, bool& priority_track) {

	if (mrule == nullptr) printf("warning nullptr\n");
	auto pit = priority_container.equal_range(mrule->priority);
	
	priority_container.erase(pit.first);
	if (num_rules == 1) {
		max_priority = -1;
		priority_track = true;
	} else if (mrule->priority == max_priority) {
		priority_track = true;
		max_priority = *priority_container.rbegin();
	}

	MITree::DeleteRule(mrule);



}
void MITree::DeleteRule(MITreeRule * mrule) {
	num_rules--;
	MInterval * MInterval_ptr;
	root->DecrementCount();
	for (int i = mrule->MITree_node_ptrs.size() - 1; i >= 0; i--) {
		MInterval_ptr = (MInterval*)(mrule->MITree_node_ptrs[i]->GetStoredInterval());
		if (MInterval_ptr == nullptr) printf("Warning nullptr");
		MInterval_ptr->DecrementCount();
		MInterval_ptr->DeleteRuleHolder(mrule->MITree_rule_ptrs_in_node[i]);
		/*if (MInterval_ptr->GetCount() == 0) {
		if (i == 0) {
		MInterval_ptr = root.g;
		}
		else MInterval_ptr = (MInterval*)(mrule->MITree_node_ptrs[i-1]->GetStoredInterval());
		MInterval_ptr->GetIntervalTree()->DeleteNode(mrule->MITree_node_ptrs[i]);

		}*/
	}
}

int MITree::ClassifyAPacket(const Packet& one_packet)const {
	return ClassifyAPacketRecursion(one_packet, root, 0);
}


int MITree::ClassifyAPacketRecursion(const Packet& one_packet, MInterval*  current_interval, int level) const {

	if (!current_interval) return -1;
	if (level == depth - 1) {

		auto MI = current_interval->GetNextMInterval(one_packet[field_order[level]]);
		return MI?MI->GetRulePriority():-1;
	}
	else {

		return ClassifyAPacketRecursion(one_packet, current_interval->GetNextMInterval(one_packet[field_order[level]]), level + 1);
		
	}
}

void PriorityMITree::Reconstruct() {
	std::vector<Rule> rules;
	for (auto mr : MRules) {
		rules.push_back(mr->one_rule);
	}
//	global_counter++;

	auto result = SortableRulesetPartitioner::GreedyFieldSelection(rules);
	if (result.first.size() != rules.size()) return;
	auto IsIdenticalVector = [](const std::vector<int>& lhs, const std::vector<int>& rhs) {
		for (size_t i = 0; i < lhs.size(); i++) {
			if (lhs[i] != rhs[i]) return false;
		}
		return true;
	};
	if (IsIdenticalVector(field_order, result.second)) return;

	field_order = result.second;
	delete root;
	root = new MInterval();
	auto AdHocInsertRule = [&](const Rule& one_rule, MITreeRule * it) {
		it->ClearContent();
		InsertRuleRecursion(one_rule, it, root, 0);
	};
	for (auto mr : MRules) {
		 AdHocInsertRule(mr->one_rule,mr);
	}
}



MITreeRule * SmartPriorityMITree::InsertRule(const Rule& one_rule, bool& priority_track) {

	 
	bool is_this_rule_large = false;
	unsigned long long length = 0;
	if (CanInsertRule(one_rule, is_this_rule_large, length)) { 

		/*max_length = std::max(max_length, length);
		if (!field_guard) {
			field_guard = is_this_rule_large;
		}*/
		if (max_priority < one_rule.priority) priority_track = true;
		else priority_track = false;

		return PriorityMITree::InsertRule(one_rule);
	} else {

		return nullptr;

	}
}

/*
MITreeRule * SmartPriorityMITree::InsertRule(const Rule& one_rule) {

	max_priority = std::max(one_rule.priority, max_priority);
	priority_container.insert(one_rule.priority);
	num_rules++;
	auto it = new MITreeRule(one_rule.priority, std::vector<IntervalTreeNode *>(field_order.size()), std::vector<std::list<MITreeRule *>::iterator>(field_order.size()), one_rule);
	SmartPriorityMITree::InsertRuleRecursion(one_rule, it, root, 0);

	MRules.push_back(it);
	return it;
}



void SmartPriorityMITree::InsertRuleRecursion(const Rule& one_rule, MITreeRule * rule_holder, MInterval* current_interval, int level) {


	
	current_interval->IncrementCount();
	if (!current_interval->GetFieldGuard() && level ==0) {
		auto range = one_rule.range[field_order[level]];
		if (IsThisIntervalLarge(range[0], range[1], level)) {
			current_interval->SetFieldGuard(true);
		}
	}

	MITree::InsertRuleRecursion(one_rule, rule_holder, current_interval, level);

}
*/
void SmartPriorityMITree::Reconstruct() {
	PriorityMITree::Reconstruct(); 
	//field_guard = SerializeHelper(root->GetRootTreeNode(), 0);
	//root->SetFieldGuard(SerializeHelper(root->GetRootTreeNode(), 0));

	//if (FindFirstFieldGuard() != root->GetFieldGuard()) printf("There is error\n");
	//assuming that  the tree size in this reconstruction is small.
	//basically, traverse all nodes in the tree and update the field guards

	/*for (auto rule : MRules) {
		auto range = rule->one_rule.range[field_order[i]];
		field_guard[i] = IsThisIntervalLarge(range[0], range[1], i);
	}*/
	/*if (field_guard[0]) 
	{
		SetFieldGuardAsOneParition(1);
	}
	else 
	{
		int run_second_field = 0;
		std::vector<Partition> partition_so_far;
		SerializeHelper(root->GetRootTreeNode(), 0, true, run_second_field, partition_so_far);
		int sum_yes = 0, sum_no = 0;
		for (Partition& p : partition_so_far) {
			if (p.isThisPartitionLarge) sum_yes++;
			else sum_no++;
		}
		field_guard[1] = sum_yes > sum_no ? 1 : 0;
	}
	*/
}
 
bool inline SmartPriorityMITree::IsThisIntervalLarge(unsigned int a, unsigned int b, int field) const  {
	
	int imod5 = field_order[field] % 5;
	unsigned length = b - a + 1;
	if (imod5 == 0 || imod5 == 1) {
		if (length > 2147483648 || length ==0) {
			return true;
		}
	} else if (imod5 == 2 || imod5 == 3) {

		if (length > 32768) {
			return true;
		}
	} else {
		if (length > 128) {
			return true;
		}
	}
	return false;
}


bool SmartPriorityMITree::SerializeHelper(IntervalTreeNode * current_node, int level)  
{   
	//dummy -- use to enforce one partition only
	if (level > 0) return 0;

	if (level > depth - 1) return 0;
	if (current_node->GetStoredInterval() == nullptr) {
		return 0;
	}
	IntervalTreeNode * it = current_node;
	bool isthisnodelarge = IsThisIntervalLarge(current_node->GetStoredInterval()->GetLowPoint(), current_node->GetStoredInterval()->GetHighPoint(), level);
	auto MIntervalAbove = (MInterval *)(it->GetStoredInterval());
	MIntervalAbove->SetFieldGuard(SerializeHelper(MIntervalAbove->GetRootTreeNode(), level + 1));
	return SerializeHelper(it->left, level) || SerializeHelper(it->right, level) || isthisnodelarge;
}


bool SmartPriorityMITree::CanInsertRule(const Rule& one_rule, bool& is_this_rule_large, unsigned long long& length) const {
	
	//auto range = one_rule.range[field_order[0]];
	//length = range[HIGH] - range[LOW] + 1;
	//is_this_rule_large = IsThisLengthLarge(length);
	if (size() == 0) return true;
	//if (is_this_rule_large != field_guard) return false;
	return PriorityMITree::CanInsertRuleRecursion(one_rule, root, 0);
}

/*
bool SmartPriorityMITree::CanInsertRuleRecursion(const Rule& one_rule, MInterval * current_interval, int level) const {

	if (level < 1) {
		auto range = one_rule.range[field_order[level]];
		if (IsThisIntervalLarge(range[0], range[1], level) != current_interval->GetFieldGuard()) {
			return false;
		}
	}

	MInterval * foundMI = current_interval->GetNextMInterval(one_rule.range[field_order[level]][LOW], one_rule.range[field_order[level]][HIGH]);

	if (!foundMI) return true;
	else if (foundMI->GetHighPoint() == (one_rule.range[field_order[level]][HIGH]) && foundMI->GetLowPoint() == (one_rule.range[field_order[level]][LOW])) {

		return 	 (level == depth - 1) ? true : CanInsertRuleRecursion(one_rule, foundMI, level + 1);
	} else
		return false;
}*/


/* backup
int MITree::ClassifyAPacketRecursion(const Packet& one_packet, const std::shared_ptr<MInterval>& current_interval, int level) const {

    if (!current_interval) return INFTY;
	if (level == depth - 1) {

		auto MI = std::static_pointer_cast<MInterval>(current_interval->GetIntervalTree()->FirstIntervalSearch(one_packet[field_order[level]], one_packet[field_order[level]])->GetStoredInterval());
		return MI ? MI->GetRulePriority() : INFTY;
	}
	else {
		/*
		if (current_interval->GetListRuleHolderSize() == 1)  {
			//check false positive;
			auto one_rule = current_interval->GetRule();
			for (int i = level; i < depth; i++) {
				if (one_rule.range[field_order[i]][LOW] > one_packet[field_order[i]] || one_rule.range[field_order[i]][HIGH] < one_packet[field_order[i]]) {
					return INFTY;
				}
			}
			return  one_rule.priority;
		}

		return ClassifyAPacketRecursion(one_packet, std::static_pointer_cast<MInterval>(current_interval->GetIntervalTree()->FirstIntervalSearch(one_packet[field_order[level]], one_packet[field_order[level]])->GetStoredInterval()),level+1);
		
	}
}*/


int MITree::CountNumberOfNodes(IntervalTreeNode * current_node, int level) const 
{
	
	if (level > depth - 1) return 0;
	if (current_node->GetStoredInterval() == nullptr) {
		return 0;
	}
	MInterval * mint = ((MInterval *)(current_node->GetStoredInterval()));
	if (mint->GetNumberOfRules() <= 1) return 1;
	IntervalTreeNode * it = current_node;  

	return CountNumberOfNodes(it->left, level) + CountNumberOfNodes(it->right, level) + CountNumberOfNodes(((MInterval *)(it->GetStoredInterval()))->GetRootTreeNode(), level + 1) + 1;
}
int MITree::CountMaxHeight(IntervalTreeNode * current_node, int level) const
{
	if (level > 1) return ((MInterval *)(current_node->GetStoredInterval()))->GetNumberOfRules();
	if (current_node->GetStoredInterval() == nullptr) {
		return 0;
	}
	IntervalTreeNode * it = current_node;

	return std::max(std::max(CountMaxHeight(it->left, level), CountMaxHeight(it->right, level)), CountMaxHeight(((MInterval *)(it->GetStoredInterval()))->GetRootTreeNode(), level + 1)) + 1;
}
