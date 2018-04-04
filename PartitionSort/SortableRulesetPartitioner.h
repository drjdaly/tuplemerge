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
#ifndef  SORTABLE_H
#define  SORTABLE_H
#include "../ElementaryClasses.h"
#include "../Utilities/IntervalUtilities.h"

class SortableRuleset;
class SortableRulesetPartitioner {

	typedef std::vector<Rule> part;
public:
	static const int LOW = 0 , HIGH = 1;
	static bool IsBucketReallySortable(const SortableRuleset & b);

	/*For GrInd */
	//This function takes O(d^2 nlog n) time.
	static std::vector<SortableRuleset> SortableRulesetPartitioningGFS(const std::vector<Rule>& rules);
	static std::pair<std::vector<Rule>, std::vector<int>> GreedyFieldSelection(const std::vector<Rule>& rules);
	static std::pair<bool, std::vector<int>> GreedyFieldSelectionTwoIterations(const std::vector<Rule>& rules);


	static std::pair<bool, std::vector<int>> FastGreedyFieldSelectionTwoIterations(const std::vector<Rule>& rules);

	static std::pair<std::vector<Rule>, std::vector<int>> FastGreedyFieldSelection(const std::vector<Rule>& rules);
	static std::pair<bool, std::vector<int>> FastGreedyFieldSelectionForAdaptive(const std::vector<Rule>& rules);

	static std::vector<int> GetFieldOrderByRule(const Rule& r);
	static std::vector<SortableRuleset>  AdaptiveIncrementalInsertion(const std::vector<Rule>& rules, int);

	/*For MISF*/
	//Warning: this function takes O(d!dn log n) time.
	static std::vector<SortableRuleset>  MaximumIndepdenentSetPartitioning(const std::vector<Rule>& rules);

	/*For NON*/
	//Warning: this function takes O(n^d) time.
	static int  ComputeMaxIntersection(const std::vector<Rule>& rules);


	
private:
	//credits: http://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
	template <typename T> static
	std::vector<int> sort_indexes(const std::vector<T> &v) {

		// initialize original index locations
		std::vector<int> idx(v.size());
		for (int i = 0; i != idx.size(); ++i) idx[i] = i;

		// sort indexes based on comparing values in v
		sort(idx.begin(), idx.end(),
			 [&v](int i1, int i2) {return v[i1] < v[i2]; });

		return idx;
	}


	/*Helper For GrInd */
	static std::pair<std::vector<SortableRulesetPartitioner::part>, bool> IsThisPartitionSortable(const part& apartition, int current_field);
	static std::pair<std::vector<SortableRulesetPartitioner::part>, bool> IsEntirePartitionSortable(const std::vector<part>& all_partition, int current_field);
	static std::pair<std::vector<part>, int> MWISonPartition(const part& apartition, int current_field);
	static std::pair<std::vector<part>, int> MWISonEntirePartition(const std::vector<part>& all_partition, int current_field);
	static void BestFieldAndConfiguration(std::vector<part>& all_partition, std::vector<int>& current_field, int num_field);

	static std::pair<std::vector<part>, int> FastMWISonPartition(const part& apartition, int current_field);
	static std::pair<std::vector<part>, int> FastMWISonEntirePartition(const std::vector<part>& all_partition, int current_field);
	static void FastBestFieldAndConfiguration(std::vector<part>& all_partition, std::vector<int>& current_field, int num_field);


	/*Helper For MISF*/
	static std::pair<std::vector<Rule>, std::vector<int>> MaximumIndependentSetAllFields(const std::vector<Rule>& rules);
	static std::vector<Rule>  MaximumIndependentSetGivenField(const std::vector<Rule>& rules, const std::vector<int>& fields);
	static WeightedInterval MaximumIndependentSetGivenFieldRecursion(const std::vector<Rule>& rules, const std::vector<int>& fields, int depth, unsigned int a, unsigned int b);


	/*Helper For NON*/
	//Warning: this function takes O(n^d) time.
	//field order is the suggested order to explore
	static int ComputeMaxIntersectionRecursive(const std::vector<Rule>& rules, int field_depth, const std::vector<int>& field_order);

};


class SortableRuleset  {
public:
	SortableRuleset(const std::vector<Rule>& rule_list, const std::vector<int>& field_order) : rule_list(rule_list), field_order(field_order) {}
	const std::vector<Rule>& GetRule() const{ return rule_list; }
	const std::vector<int>& GetFieldOrdering() const { return field_order; }
	bool InsertRule(const Rule& r) {
		//This function is for debugging purpose.
		std::vector<Rule> temp_rule_list = rule_list;
		temp_rule_list.push_back(r);
		if (SortableRulesetPartitioner::IsBucketReallySortable(SortableRuleset(temp_rule_list, field_order))) {
			rule_list = std::move(temp_rule_list);
			return true;
		}
		return false;
	}
	int size() const {
		return rule_list.size();
	}

	void Print() const {
		for (const Rule& r : rule_list) {
			for (int f : field_order) {
				printf("[%u, %u]", r.range[f][LowDim], r.range[f][HighDim]);
			}
			printf("->%u\n", r.priority);
		}
	}

	SortableRuleset & operator = (const SortableRuleset &t)
	{
		// Check for self assignment
		if (this != &t)
		{
			this->rule_list = t.GetRule();
			this->field_order = t.GetFieldOrdering();
		}

		return *this;
	}

private:
	std::vector<Rule> rule_list;
	std::vector<int> field_order;


};

#endif
