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
#ifndef  MITree_H
#define  MITree_H
#include "../ElementaryClasses.h"
#include "SortableRulesetPartitioner.h"
#include "IntervalTree.h"

#include <algorithm>

class MITree;
class PriorityMITree;
class SmartPriorityMITree;
struct MITreeRule
{
	MITreeRule(int priority, const std::vector<IntervalTreeNode *>& v, const std::vector<std::list<MITreeRule *>::iterator>& MITree_rule_ptrs_in_node, const Rule& one_rule) : one_rule(one_rule), priority(priority), MITree_node_ptrs(v), MITree_rule_ptrs_in_node(MITree_rule_ptrs_in_node){}
	void ClearContent() {
		MITree_rule_ptrs_in_node = std::vector<std::list<MITreeRule *>::iterator>(one_rule.dim);
		MITree_node_ptrs = std::vector<IntervalTreeNode *>(one_rule.dim);
	}
	std::vector<IntervalTreeNode *> MITree_node_ptrs;
	std::vector<std::list<MITreeRule *>::iterator> MITree_rule_ptrs_in_node;
	int priority;

	Rule one_rule;
	//union ptr_to_tree {
	PriorityMITree * ptr_to_priority_bucket;
	SmartPriorityMITree * ptr_to_smart_priority_bucket;
	MITree * ptr_to_bucket;
//	} ptr_to_the_tree;
};

class MInterval : public Interval {
public:

	int depth;
	~MInterval() { delete mid_pointer; }
	MInterval() : count(0) { mid_pointer = new IntervalTree(); }
	MInterval(unsigned int a, unsigned int b) :a(a), b(b), count(0) { mid_pointer = new IntervalTree();  }
	MInterval(unsigned int a, unsigned int b, IntervalTree * t) :a(a), b(b), mid_pointer(t), count(1) {  }
	
	unsigned int GetLowPoint() const { return a; }
	unsigned int GetHighPoint() const { return b; }

	void Print() const {
		std::cout << GetLowPoint() << "," << GetHighPoint() << " ";
	}
	/*IntervalTree * GetIntervalTree() {
		return mid_pointer;
	}*/

	MInterval * GetNextMInterval(unsigned int x) const {
		return (MInterval *)(mid_pointer->FirstIntervalSearchRetInterval(x, x));
	}
	MInterval * GetNextMInterval(unsigned int x, unsigned int y) const {
		return (MInterval *)(mid_pointer->FirstIntervalSearchRetInterval(x, y));
	}
	IntervalTreeNode  * GetNodeFromIntervalTree(unsigned int x, unsigned int y) const {
		return mid_pointer->FirstIntervalSearch(x, y);
	}
	IntervalTreeNode  * InsertMInterval(MInterval * m){
		return mid_pointer->Insert(m);
	}
	 
	std::list<MITreeRule *>::iterator InsertRuleHolder(MITreeRule * rule_holder_in) {
		if (rule_holder_in->priority >= max_priority) max_priority = rule_holder_in->priority;
		rule_holder.push_front(rule_holder_in);
		return begin(rule_holder);
	}	
	void DeleteRuleHolder(std::list<MITreeRule *>::iterator&  it) {

		if (rule_holder.size() == 1) {
			rule_holder.erase(it);
			max_priority = -1;
		} else if ((*it)->priority == max_priority) {
			rule_holder.erase(it);
			int max_pri = -1;
			for (auto &holder : rule_holder) {
				max_pri = std::max(max_pri, holder->priority);
			}
			max_priority = max_pri;
		}
		else {
			rule_holder.erase(it);
		}
	}
	void ClearRuleHolder() {
		rule_holder.clear();
	}
	int GetListRuleHolderSize() const {
		return rule_holder.size();
	}
	int GetRulePriority() const {
		return max_priority; 
	}
	Rule GetRule() const {
		auto holder = begin(rule_holder); 
		return (*holder)->one_rule;
	}
	int GetCount() const {
		return count;
	}
	void IncrementCount()   {
		count++;
	}
	void DecrementCount()   {
		count--;
	}
	IntervalTreeNode * GetRootTreeNode() {
		return mid_pointer->GetRootTreeNode();
	}
	bool GetFieldGuard()  const {
		return field_guard;
	}
	void SetFieldGuard(bool t)    {
		field_guard = t;
	}
	
	int GetNumberOfRules() const {
		return rule_holder.size();
	}
	std::list<MITreeRule *> rule_holder;

private:
	IntervalTree * mid_pointer;
	unsigned int a, b;
	
	int max_priority = -1;
	int count=0;

	bool field_guard=0;

};

class MITree {

public:
	static const int LOW = 0;
	static const int HIGH = 1;
	~MITree() {
		delete root;
	}
	MITree(const SortableRuleset& sb) : field_order(sb.GetFieldOrdering()), depth(sb.GetRule()[0].dim) {
		root = new MInterval();
		auto rules = sb.GetRule();
		for (const auto& r :rules) {
			if (!CanInsertRule(r)) { printf("Warning: cannot insert rule in the constructor\n"); exit(1); }
			MRules.push_back(InsertRule(r));
		}
	}
	MITree(const std::vector<int>& field_order) :field_order(field_order), depth(field_order.size()){ root = new MInterval(); num_rules = 0; }
	MITree(const Rule &r) {
		root = new MInterval(); 
		num_rules = 0;
		depth = r.dim ;
		field_order = SortableRulesetPartitioner::GetFieldOrderByRule(r);
	}
	MITree(int dim) {
		root = new MInterval();
		num_rules = 0;
		std::vector<int> field_order0(dim);
		std::iota(field_order0.begin(), field_order0.end(), 0);
		field_order = Random::shuffle_vector(field_order0);
		depth = dim;
	}
	MITree(){ root = new MInterval(); field_order = { 0, 1,2}; depth = field_order.size(); }

	
	MITreeRule * InsertRule(const Rule& one_rule);
	bool CanInsertRule(const Rule& one_rule) const;
	
	void DeleteRule(MITreeRule * mrule);
	void  Print() const;
	int ClassifyAPacket(const Packet& one_packet)const;

	std::vector<MITreeRule *> MRules;

	int size() const { return num_rules; }
	std::vector<int> GetFieldOrder() const { return field_order; }
	int CountRules() const { return MRules.size(); }

	SortableRuleset ExportSortablePartition() const {
		std::vector<Rule> temp_rules;
		for (const auto t : MRules) {
			temp_rules.push_back(t->one_rule);
		}
		return SortableRuleset(temp_rules, field_order);
	}

	int CountNumberOfNodes() const {
		return CountNumberOfNodes(root->GetRootTreeNode(), 0);
	}
	int  CountNumberOfNodes(IntervalTreeNode * current_node, int level) const;
	int CountMaxHeight(IntervalTreeNode * current_node, int level) const;
	int NumberOfMemoryAccess()  const {
		return CountMaxHeight(root->GetRootTreeNode(),0);
	}
	int Space()  const {
		return CountNumberOfNodes()*24;
	}

protected:
	
	bool MatchRemainingField(const Rule& r, const Packet& p) const {
		for (int i = 0; i < depth; i++) {
			if (r.range[field_order[i]][LOW] > p[field_order[i]]) return false;
			if (r.range[field_order[i]][HIGH] < p[field_order[i]]) return false;
		}
		return true;
	}
	MInterval * root;
	std::vector<int> field_order;
	int depth;
	int num_rules = 0;

	void InsertRuleRecursion(const Rule& one_rule, MITreeRule * rule_holder, MInterval * current_interval, int level);
	bool CanInsertRuleRecursion(const Rule& one_rule, MInterval * current_interval, int level) const;
	//void RemoveMIntervalFromLevel(MInterval * minterval, MITreeRule * mrule, int level);

	MInterval * BuildMIntervalFromLevel(const Rule& one_rule, MITreeRule * rule_holder, int level);

	int ClassifyAPacketRecursion(const Packet& one_packet, MInterval* current_interval, int level) const;
	//int ClassifyAPacketRecursion(const Packet& one_packet, const std::shared_ptr<MInterval>& current_interval, int level) const;
	
};

class PriorityMITree : public MITree {
public:
	PriorityMITree(const Rule& r) : MITree(r)
	{
		max_priority = -1;
	}
	PriorityMITree(int dim) : MITree(dim) 
	{
		max_priority = -1;
	}
	PriorityMITree(const std::vector<int>& field_order) : MITree(field_order) {
		max_priority = -1;
	}
	void Reconstruct();
	int GetMaxPriority() const { return max_priority; }
	MITreeRule * InsertRule(const Rule& one_rule);
	MITreeRule * InsertRule(const Rule& one_rule, bool& priority_track);
	void DeleteRule(MITreeRule * mrule, bool& priority_track);

protected:
	int max_priority = -1;
	std::multiset<int> priority_container;

};


struct Partition {
	bool isThisPartitionLarge = false;
};

class SmartPriorityMITree : public PriorityMITree{
public:
	SmartPriorityMITree(const Rule& r, int threshold) : threshold(threshold), PriorityMITree(r)
	{
	}
	SmartPriorityMITree(int dim, int threshold) : threshold(threshold), PriorityMITree(dim)
	{
	}
	SmartPriorityMITree(const std::vector<int>& field_order, int threshold) : threshold(threshold), PriorityMITree(field_order) {

	}
	bool SameGuardSmallTree(const Rule& r) const {
		if (size() > threshold) return true;
		auto range_field0 = r.range[field_order[0]];
		if (FindFirstFieldGuard() != IsThisIntervalLarge(range_field0[0], range_field0[1], 0)) return false;
		return true;
	}
	MITreeRule * InsertRule(const Rule& one_rule, bool& priority_track);
	
	void Reconstruct();
	void AdjustGuard() {
		SerializeHelper(root->GetRootTreeNode(), 0);
	}
	
	int size() const {
		return MRules.size();
	}

private:
	//MITreeRule * InsertRule(const Rule& one_rule);
	bool CanInsertRule(const Rule& one_rule, bool& is_this_rule_large, unsigned long long& length) const;

	bool  SerializeHelper(IntervalTreeNode * current_node, int level);
	//void InsertRuleRecursion(const Rule& one_rule, MITreeRule * rule_holder, MInterval * current_interval, int level);
	bool FindFirstFieldGuard() const {
		for (auto rule : MRules) {
			auto range = rule->one_rule.range[field_order[0]];
			if( IsThisIntervalLarge(range[0], range[1], 0)) return true;
		}
		return false;
	}
//	void SerializeHelper(IntervalTreeNode * current_node, int level, bool is_new_root, int run_second_field, std::vector<Partition>& partition_so_far) const;
	bool inline IsThisIntervalLarge(unsigned   a, unsigned   b, int i) const;
	bool inline IsThisLengthLarge(unsigned long long length) const {
		int imod5 = field_order[0] % 5;
		if (imod5 == 0 || imod5 == 1) {
			if (length > 2147483648) {
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
 
	int threshold = 10;
	//std::multiset<unsigned long long> length_container;
	unsigned long long max_length = 0;
	bool field_guard = 0; //0 - small, 1 - large

};




#endif
