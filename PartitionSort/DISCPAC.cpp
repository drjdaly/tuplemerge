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
#include "DISCPAC.h"


void DISCPAC::ConstructMITrees(const std::vector<SortableRuleset>& sb) {
	for (const auto& b : sb) {
		auto mitree = new MITree(b.GetFieldOrdering());
		auto rules = b.GetRule();
		
		//sort rule in the first field
		std::sort(begin(rules), end(rules), [&b](const Rule& lhs, const Rule& rhs){
			for (int i = 0; i < rhs.dim; i++) {
				if (lhs.range[b.GetFieldOrdering()[i]][0] < lhs.range[b.GetFieldOrdering()[i]][0]) return 1;
			}
			return 0;
		});
		std::queue<std::pair<int, int>> q;
		q.push(std::make_pair(0, ((int)rules.size() - 1)));
		//insert in level-order traversal to obtain perfectly balanced tree
		while (!q.empty()) {
			std::pair<int,int> range = q.front();
			q.pop();
			if (range.first > range.second) continue;
			int mid = (range.first + range.second) / 2;
			bool priority_change = 0;
			auto MRule = mitree->InsertRule(rules[mid]);
			MRule->ptr_to_bucket = mitree;
			rule_index[MRule->one_rule.id] = all_ruleset.size();
			all_ruleset.push_back(MRule);

			q.push(std::make_pair(range.first, mid - 1));
			q.push(std::make_pair(mid+1, range.second));

		}
		MITrees.push_back(mitree);
	}
}

void SmartAdaptiveDISCPAC::ConstructMITrees(const std::vector<SortableRuleset>& sb) {
	for (const auto& b : sb) {
		auto mitree = new SmartPriorityMITree(b.GetFieldOrdering(),10);
		auto rules = b.GetRule();
		//sort rule in the first field
		std::sort(begin(rules), end(rules), [&b](const Rule& lhs, const Rule& rhs){
			for (int i = 0; i < rhs.dim; i++) {
				if (lhs.range[b.GetFieldOrdering()[i]][0] < lhs.range[b.GetFieldOrdering()[i]][0]) return 1;
			}
			return 0;
		});
		std::queue<std::pair<int, int>> q;
		q.push(std::make_pair(0, ((int)rules.size() - 1)));
		//insert in level-order traversal to obtain perfectly balanced tree
		while (!q.empty()) {
			std::pair<int, int> range = q.front();
			q.pop();
			if (range.first > range.second) continue;
			int mid = (range.first + range.second) / 2;
			bool priority_change = 0;
		
			auto MRule = mitree->InsertRule(rules[mid], priority_change);

		//	MRule->ptr_to_smart_priority_bucket = mitree;

			//rule_index[MRule->one_rule.id] = all_ruleset.size();

			all_ruleset.push_back(MRule);

			q.push(std::make_pair(range.first, mid - 1));
			q.push(std::make_pair(mid + 1, range.second));

		}
		mitree->AdjustGuard();
		SmartPriorityMITrees.push_back(mitree);
	}

	InsertionSortSmartPriorityMITree();
}

void PriorityDISCPAC::ConstructMITrees(const std::vector<SortableRuleset>& sb) {
	
	for (const auto& b : sb) {
		auto mitree = new PriorityMITree(b.GetFieldOrdering());
		auto rules = b.GetRule();

		//sort rule in the first field
		std::sort(begin(rules), end(rules), [&b](const Rule& lhs, const Rule& rhs){
			for (int i = 0; i < rhs.dim; i++) {
				if (lhs.range[b.GetFieldOrdering()[i]][0] < lhs.range[b.GetFieldOrdering()[i]][0]) return 1;
			}
			return 0;
		});
		std::queue<std::pair<int, int>> q;
		q.push(std::make_pair(0, ((int)rules.size() - 1)));
		//insert in level-order traversal to obtain perfectly balanced tree
		while (!q.empty()) {
			std::pair<int, int> range = q.front();
			q.pop();
			if (range.first > range.second) continue;
			int mid = (range.first + range.second) / 2;
			bool priority_change = 0;
			auto MRule = mitree->InsertRule(rules[mid],priority_change);
			MRule->ptr_to_priority_bucket = mitree;
			rule_index[MRule->one_rule.id] = all_ruleset.size();
			all_ruleset.push_back(MRule);

			q.push(std::make_pair(range.first, mid - 1));
			q.push(std::make_pair(mid + 1, range.second));

		}
		PriorityMITrees.push_back(mitree);
	}

	InsertionSortMITrees();
}

int DISCPAC::ClassifyAPacket(const Packet& p)  {
	int result = -1;
	for (const auto& t : MITrees) 
		result = std::max(t->ClassifyAPacket(p), result);
	return result;
}
int PriorityDISCPAC::ClassifyAPacket(const Packet& p)  {

	int result = -1;
	int i = 0;
	for (const auto& t : PriorityMITrees) {
		if (result > t->GetMaxPriority()){
			break;
		}
		result = std::max(t->ClassifyAPacket(p), result);
	}
	
	return result;
}
int SmartAdaptiveDISCPAC::ClassifyAPacket(const Packet& p)  {
	int result = -1;
	int i = 0;
	for (const auto& t : SmartPriorityMITrees) {
		if (result > t->GetMaxPriority()){
			break;
		}
		result = std::max(t->ClassifyAPacket(p), result);
	}

	return result;
}
void DISCPAC::DeleteRule(size_t index) {
	if (index < 0 || index > rule_index.size()) {
		printf("Warning:: Out of Bound DeleteRule operation\n this operation is ignored\n");
		return;
	}
	MITreeRule * mrule_to_delete = all_ruleset[rule_index[index]];
	MITree*  tree_weak = mrule_to_delete->ptr_to_bucket;
	tree_weak->DeleteRule(mrule_to_delete);

	if (index != rule_index.size() - 1)
	{
		// Beware of move assignment to self
		// see http://stackoverflow.com/questions/13127455/
		rule_index[index] = std::move(rule_index.back());
	}
	rule_index.pop_back();
	//lazy delete -- to perform at insertion
}

void SmartAdaptiveDISCPAC::DeleteRule(size_t index) {
	if (index < 0 || index > rule_index.size()) {
		printf("Warning:: Out of Bound DeleteRule operation\n this operation is ignored\n");
		return;
	}
	MITreeRule * mrule_to_delete = all_ruleset[rule_index[index]];

	SmartPriorityMITree*  tree_weak = mrule_to_delete->ptr_to_smart_priority_bucket;
	bool prioritychange = 0;

	tree_weak->DeleteRule(mrule_to_delete, prioritychange);
	if (index != rule_index.size() - 1)
	{
		// Beware of move assignment to self
		// see http://stackoverflow.com/questions/13127455/
		rule_index[index] = std::move(rule_index.back());
	}
	rule_index.pop_back();

	if (prioritychange) {
		InsertionSortSmartPriorityMITree();
	}
	//delete immediately
	if (tree_weak->size() == 0) {
		SmartPriorityMITrees.pop_back();
		delete tree_weak;
	}
}

void PriorityDISCPAC::DeleteRule(size_t index) {
	if (index < 0 || index > rule_index.size()) {
		printf("Warning:: Out of Bound DeleteRule operation\n this operation is ignored\n");
		return;
	}
	MITreeRule * mrule_to_delete = all_ruleset[rule_index[index]];

	PriorityMITree*  tree_weak = mrule_to_delete->ptr_to_priority_bucket;
	bool prioritychange = 0;
	tree_weak->DeleteRule(mrule_to_delete, prioritychange);

	if (index != rule_index.size() - 1)
	{
		// Beware of move assignment to self
		// see http://stackoverflow.com/questions/13127455/
		rule_index[index] = std::move(rule_index.back());
	}
	rule_index.pop_back();

	if (prioritychange) {
		InsertionSortMITrees();
	}
	//delete immediately
	if (tree_weak->size() == 0) {
		PriorityMITrees.pop_back();
		delete tree_weak;
	}
}
void DISCPAC::InsertRule(const Rule& one_rule) {
	//printf("%d inserted\n", one_rule.priority);
	//Lazy deletion implementation: when bucket is empty --> delete from vector in insertion rule function
	int i = 0;
	for (auto itr = begin(MITrees); itr != end(MITrees) /* !!! */;)
	{
		if ((*itr)->size() == 0)
		{
			MITree* todie = *itr;
			itr = MITrees.erase(itr);
			delete todie;

		} else
		{
			auto MRule = (*itr)->InsertRule(one_rule);
			if (MRule) {

				MRule->ptr_to_bucket = *itr;
				rule_index.push_back(all_ruleset.size());

				all_ruleset.push_back(MRule);
				return;
			}
			++itr;

		}
	}

	auto tree_ptr = new MITree(one_rule.dim);
	auto MRule = tree_ptr->InsertRule(one_rule);

	MRule->ptr_to_bucket = tree_ptr;
	MITrees.push_back(tree_ptr);
	rule_index.push_back(all_ruleset.size());
	all_ruleset.push_back(MRule);


}
void PriorityDISCPAC::InsertRule(const Rule& one_rule) {

	int i = 0;
	for (auto mitree : PriorityMITrees)
	{
			bool prioritychange = false;
			auto MRule = mitree->InsertRule(one_rule, prioritychange);
			if (MRule) {
				if (prioritychange) {
					InsertionSortMITrees();
				}

				MRule->ptr_to_priority_bucket = mitree;
				rule_index.push_back(all_ruleset.size());
				
				all_ruleset.push_back(MRule);
				return;
			}
	}

	//printf("new buckets num bucket=%d\n", MITrees.size());
	//random field generated

	/**
	auto MRule = mitree->InsertRule(rules[mid], priority_change);
	if (priority_change) InsertionSortMITrees();
	MRule->ptr_to_bucket = mitree;
	rule_index[MRule->one_rule.id] = all_ruleset.size();
	all_ruleset.push_back(MRule);
	*/

	bool priority_change = false;

	//auto tree_ptr = new PriorityMITree(one_rule.dim);
	auto tree_ptr = new PriorityMITree(one_rule);
	auto MRule = tree_ptr->InsertRule(one_rule, priority_change);

	MRule->ptr_to_priority_bucket = tree_ptr;
	PriorityMITrees.push_back(tree_ptr);
	rule_index.push_back(all_ruleset.size());
	all_ruleset.push_back(MRule);
	
	InsertionSortMITrees();

}

void AdaptiveDISCPAC::InsertRuleAdaptive(const Rule& one_rule, int threshold) {

	for (auto mitree : PriorityMITrees)
	{
		bool prioritychange = false;
		auto MRule = mitree->InsertRule(one_rule, prioritychange);
		if (MRule) {

		
			if (mitree->size() < threshold) {
				mitree->Reconstruct();
			}
			if (prioritychange) {
				InsertionSortMITrees();
			}

			MRule->ptr_to_priority_bucket = mitree;
			rule_index.push_back(all_ruleset.size());

			all_ruleset.push_back(MRule);
			
			return;
		}
	}

	bool priority_change = false;
	auto tree_ptr = new PriorityMITree(one_rule);
	auto MRule = tree_ptr->InsertRule(one_rule, priority_change);

	MRule->ptr_to_priority_bucket = tree_ptr;
	PriorityMITrees.push_back(tree_ptr);
	rule_index.push_back(all_ruleset.size());
	all_ruleset.push_back(MRule);


	InsertionSortMITrees();
}



Memory DISCPAC::MemSizeBytes() const {
	return 0;
}

void PriorityDISCPAC::InsertionSortMITrees() {  
	
	int i, j,  numLength = PriorityMITrees.size();
	PriorityMITree * key;
	for (j = 1; j < numLength; j++)
	{
		key = PriorityMITrees[j];
		for (i = j - 1; (i >= 0) && (PriorityMITrees[i]->GetMaxPriority() < key->GetMaxPriority()); i--)
		{
			PriorityMITrees[i + 1] = PriorityMITrees[i];
		}
		PriorityMITrees[i + 1] = key;
	}
/*	for (auto t : MITrees) {
		printf("[%d,%d] ", t->GetMaxPriority(),t->size()); 
	}
	printf("\n");*/
}

void SmartAdaptiveDISCPAC::InsertionSortSmartPriorityMITree() {

	int i, j, numLength = SmartPriorityMITrees.size();
	SmartPriorityMITree * key;
	for (j = 1; j < numLength; j++)
	{
		key = SmartPriorityMITrees[j];
		for (i = j - 1; (i >= 0) && (SmartPriorityMITrees[i]->GetMaxPriority() < key->GetMaxPriority()); i--)
		{
			SmartPriorityMITrees[i + 1] = SmartPriorityMITrees[i];
		}
		SmartPriorityMITrees[i + 1] = key;
	}
 
}


void SmartAdaptiveDISCPAC::InsertRuleAdaptive(const Rule& one_rule, int threshold) {

	for (auto mitree : SmartPriorityMITrees)
	{
		//if (!mitree->SameGuardSmallTree(one_rule)) continue;
		bool prioritychange = false;
		auto MRule = mitree->InsertRule(one_rule, prioritychange);
		if (MRule) {

			if (mitree->size() < threshold) { 
				mitree->Reconstruct();
			}
			if (prioritychange) {
				InsertionSortSmartPriorityMITree();
			}

			MRule->ptr_to_smart_priority_bucket = mitree;
			rule_index.push_back(all_ruleset.size());

			all_ruleset.push_back(MRule);

			return;
		}
	}

	bool priority_change = false;
	auto tree_ptr = new SmartPriorityMITree(one_rule,threshold);
	auto MRule = tree_ptr->InsertRule(one_rule, priority_change);

	MRule->ptr_to_smart_priority_bucket = tree_ptr;
	SmartPriorityMITrees.push_back(tree_ptr);
	rule_index.push_back(all_ruleset.size());
	all_ruleset.push_back(MRule);


	InsertionSortSmartPriorityMITree();
}
