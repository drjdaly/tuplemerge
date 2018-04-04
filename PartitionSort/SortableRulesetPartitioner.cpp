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
#include "SortableRulesetPartitioner.h"
#include "../Utilities/IntervalUtilities.h"
#include <assert.h>

std::pair<std::vector<SortableRulesetPartitioner::part>, bool> SortableRulesetPartitioner::IsThisPartitionSortable(const part& apartition, int current_field){
	std::vector<WeightedInterval> wi = Utilities::CreateUniqueInterval(apartition, current_field);
	std::multiset<unsigned int> low, hi;
	for (auto & i : wi) {
		low.insert(i.GetLow());
		hi.insert(i.GetHigh());
	}
	std::vector<part> new_partitions;
	for (auto& w : wi) {
		part temp_partition_rule;
		for (auto& r : w.GetRules()) {
			temp_partition_rule.push_back(r);
		}
		new_partitions.push_back(temp_partition_rule);
	}
	return std::make_pair(new_partitions, Utilities::GetMaxOverlap(low, hi) == 1);
}
std::pair<std::vector<SortableRulesetPartitioner::part>, bool> SortableRulesetPartitioner::IsEntirePartitionSortable(const std::vector<part>& all_partition, int current_field){
	std::vector<part> new_entire_partition;
	for (const auto& p : all_partition) {
		auto pvi = IsThisPartitionSortable(p, current_field);
		bool isSortable = pvi.second;
		if (!isSortable) return make_pair(new_entire_partition, false);
		new_entire_partition.insert(end(new_entire_partition), begin(pvi.first), end(pvi.first));
	}
	return make_pair(new_entire_partition, true);
}
bool SortableRulesetPartitioner::IsBucketReallySortable(const SortableRuleset & b) {
	std::vector<part> all_partitions;
	int num_fs = 0;
	all_partitions.push_back(b.GetRule());
	int num_itr = 0;
	for (int f : b.GetFieldOrdering()) {
		auto p = IsEntirePartitionSortable(all_partitions, f);
		bool isSortable = p.second;
		if (!isSortable) return 0;
		all_partitions = p.first;
	}
	return 1;
}




std::pair<std::vector<SortableRulesetPartitioner::part>, int> SortableRulesetPartitioner::MWISonPartition(const part& apartition, int current_field){


	std::vector<WeightedInterval> wi = Utilities::CreateUniqueInterval(apartition, current_field);
	//printf("Field %d\n", current_field);
	auto mwis = Utilities::MWISIntervals(wi);


	std::vector<part> vp;
	std::vector<WeightedInterval> temp_wi;
	std::vector<part> new_partitions;
	for (int i : mwis.first) {
		part temp_partition_rule;
		for (auto& r : wi[i].GetRules()) {
			temp_partition_rule.push_back(r);
		}
		new_partitions.push_back(temp_partition_rule);
	}

	return make_pair(new_partitions, mwis.second);
}

std::pair<std::vector<SortableRulesetPartitioner::part>, int> SortableRulesetPartitioner::MWISonEntirePartition(const std::vector<part>& all_partition, int current_field){
	std::vector<part> new_entire_partition;
	int sum_weight = 0;
	for (const auto& p : all_partition) {
	
		auto pvi = MWISonPartition(p, current_field);
		new_entire_partition.insert(end(new_entire_partition), begin(pvi.first), end(pvi.first));
		sum_weight += pvi.second;
	}
	return std::make_pair(new_entire_partition, sum_weight);
}

void SortableRulesetPartitioner::BestFieldAndConfiguration(std::vector<part>& all_partition, std::vector<int>& current_field, int num_fields)
{
	 
	std::vector<SortableRulesetPartitioner::part> best_new_partition;
	int best_so_far_mwis = -1;
	int current_best_field = -1;
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {
	
			auto mwis = MWISonEntirePartition(all_partition, j);
			if (mwis.second >= best_so_far_mwis) {
				best_so_far_mwis = mwis.second;
				current_best_field = j;
				best_new_partition = mwis.first;
			}
		}
	}
	all_partition = best_new_partition;
	current_field.push_back(current_best_field);
}

std::pair<bool, std::vector<int>> SortableRulesetPartitioner::GreedyFieldSelectionTwoIterations(const std::vector<Rule>& rules) {
	if (rules.size() == 0) {
		printf("Warning: GreedyFieldSelection rule size = 0\n ");
	}
	int num_fields = rules[0].dim;

	std::vector<int> current_field;
	std::vector<part> all_partitions;

	all_partitions.push_back(rules);
	for (int i = 0; i < 2; i++) {
		BestFieldAndConfiguration(all_partitions, current_field, num_fields);
	}

	std::vector<Rule> current_rules;
	for (auto& r : all_partitions) {
		current_rules.insert(end(current_rules), begin(r), end(r));
	}
	//fill in the rest of the field in order
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {
			current_field.push_back(j);
		}
	}
	return std::make_pair(current_rules.size()==rules.size(), current_field);
}
std::pair<std::vector<Rule>, std::vector<int>> SortableRulesetPartitioner::GreedyFieldSelection(const std::vector<Rule>& rules)
{
	if (rules.size() == 0) {
		printf("Warning: GreedyFieldSelection rule size = 0\n ");
	}
	int num_fields = rules[0].dim;
	
	std::vector<int> current_field;
	std::vector<part> all_partitions;
 
	all_partitions.push_back(rules);
	for (int i = 0; i < num_fields; i++) {
		BestFieldAndConfiguration(all_partitions, current_field,num_fields);
	}

	std::vector<Rule> current_rules;
	for (auto& r : all_partitions) {
		current_rules.insert(end(current_rules), begin(r), end(r));
	}
	//fill in the rest of the field in order
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {;
			current_field.push_back(j);
		}
	}
	return std::make_pair(current_rules, current_field);
}

std::vector<SortableRuleset> SortableRulesetPartitioner::SortableRulesetPartitioningGFS(const std::vector<Rule>& rules ) {
	std::vector<Rule> current_rules = rules;
	std::vector<SortableRuleset> all_buckets;
	for (size_t i = 0; i < current_rules.size(); i++) current_rules[i].id = i;
	while (!current_rules.empty()) {
		
		for (int i = 0; i < (int)current_rules.size(); i++) current_rules[i].tag = i;
		auto rule_and_field_order = GreedyFieldSelection(current_rules);
		for (auto& r : rule_and_field_order.first)  {
			current_rules[r.tag].markedDelete = 1;
		}

		if (rule_and_field_order.first.size() == 0) {
			printf("Warning rule_and_field_order.first = 0 in SortableRulesetPartitioning\n");
			exit(0);
		}

		SortableRuleset sb = SortableRuleset(rule_and_field_order.first, rule_and_field_order.second);
 
		all_buckets.push_back(sb);
		current_rules.erase(
			remove_if(begin(current_rules), end(current_rules),
			[](const Rule& r) {
			return r.markedDelete; })
				, end(current_rules));
		//printf("%d\n", sb.size());
	}
	return all_buckets;
}

std::vector<SortableRuleset> SortableRulesetPartitioner::AdaptiveIncrementalInsertion(const std::vector<Rule>& rules, int threshold) {
	std::vector<SortableRuleset> all_buckets;
	
	auto InsertRuleIntoAllBucket = [threshold](const Rule& r, std::vector<SortableRuleset>& b) {
		for (auto& bucket : b) {
			if (bucket.InsertRule(r)) {
				if (bucket.size() < threshold) {
					//reconstruct 
					auto temp_rules = bucket.GetRule();
					auto result = SortableRulesetPartitioner::GreedyFieldSelection(temp_rules);
					bucket = SortableRuleset(result.first, result.second);
				}
				return;
			}
		}
		std::vector<int> field_order0(r.dim);
		if (threshold == 0) {
			std::iota(field_order0.begin(), field_order0.end(), 0);
			field_order0 = Random::shuffle_vector(field_order0);
		}
		else {
			field_order0 = SortableRulesetPartitioner::GetFieldOrderByRule(r);
		}

		std::vector<Rule> vr = { r };
		b.emplace_back(vr, field_order0);
	};

	int i = 0;
	for (const auto& r : rules) {

		InsertRuleIntoAllBucket(r, all_buckets);
	}
	return all_buckets;
}

std::vector<int> SortableRulesetPartitioner::GetFieldOrderByRule(const Rule& r)
 {
	 const int HIGH = 1, LOW = 0;
	 //assume ClassBench Format
	 std::vector<int> rank(r.dim, 0);
	 // 0 -> point, 1 -> shorter than half range, 2 -> longer than half range
	 //assign rank to each field
	 for (int i = 0; i < r.dim; i++) {
		 int imod5 = i % 5;
		 if (imod5 == 0 || imod5 == 1) {
			 //IP
			 unsigned int length = r.range[imod5][HIGH] - r.range[imod5][LOW] + 1;
			 if (length == 1) rank[i] = 0;
			 else if (length > 0 && length < (1 << 31)) rank[i] = 1;
			 else if (length == 0) rank[i] = 3;
			 else rank[i] = 2;
		 } else if (imod5 == 2 || imod5 == 3) {
			 //Port
			 unsigned int length = r.range[imod5][HIGH] - r.range[imod5][LOW] + 1;
			 if (length == 1) rank[i] = 0;
			 else if (length < (1 << 15)) rank[i] = 1;
			 else if (length < (1 << 16)) rank[i] = 2;
			 else rank[i] = 3;
		 } else {
			 //Protocol
			 unsigned int length = r.range[imod5][HIGH] - r.range[imod5][LOW] + 1;
			 if (length == 1) rank[i] = 0;
			 else if (length < (1 << 7)) rank[i] = 1;
			 else if (length < 256) rank[i] = 2;
			 else rank[i] = 3;

		 }
	 }
	 return sort_indexes(rank);
 }

int SortableRulesetPartitioner::ComputeMaxIntersection(const std::vector<Rule>& ruleset){
	if (ruleset.size() == 0) printf("Warning SortableRulesetPartitioner::ComputeMaxIntersection: Empty ruleset\n");
	//auto  result = SortableRulesetPartitioner::GreedyFieldSelection(ruleset);
	auto field_order = { 0,1,2,3,4 };//result.second;
	//std::reverse(begin(field_order), end(field_order));
	return ComputeMaxIntersectionRecursive(ruleset, ruleset[0].dim - 1, field_order);
}


int SortableRulesetPartitioner::ComputeMaxIntersectionRecursive(const std::vector<Rule>& rules, int field_depth, const std::vector<int>& field_order){

	if (rules.size() == 0) return 0;
	if (field_depth <= 0) {
		std::multiset<unsigned int> Astart;
		std::multiset<unsigned int> Aend;
		std::vector<Rule> rules_rr = Utilities::RedundancyRemoval(rules);
		for (auto v : rules_rr){
			Astart.insert(v.range[field_order[field_depth]][LOW]);
			Aend.insert(v.range[field_order[field_depth]][HIGH]);
		}
		int max_olap = Utilities::GetMaxOverlap(Astart, Aend);
		return max_olap;
	}


	std::vector<std::pair<unsigned int, unsigned int>> start_pointx;
	std::vector<std::pair<unsigned int, unsigned int>>  end_pointx;

	std::vector<unsigned int> queries;
	for (int i = 0; i< (int)rules.size(); i++) {
		start_pointx.push_back(std::make_pair(rules[i].range[field_order[field_depth]][LOW], i));
		end_pointx.push_back(std::make_pair(rules[i].range[field_order[field_depth]][HIGH], i));
		queries.push_back(rules[i].range[field_order[field_depth]][LOW]);
		queries.push_back(rules[i].range[field_order[field_depth]][HIGH]);
	}

	sort(begin(start_pointx), end(start_pointx));
	sort(begin(end_pointx), end(end_pointx));
	sort(begin(queries), end(queries));

	int i_s = 0;
	int i_e = 0;
	int max_POM = 0;
	std::vector<Rule> rule_this_loop;
	std::vector<int> index_rule_this_loop(rules.size(), -1); //index by priority
	std::vector<int> inverse_index_rule_this_loop(rules.size(), -1);
	for (unsigned int qx : queries) {
		bool justInserted = 0;
		bool justDeleted = 0;
		//insert as long as start point is equal to qx
		if (start_pointx.size() != i_s) {
			while (qx == start_pointx[i_s].first) {
				index_rule_this_loop[start_pointx[i_s].second] = (int)rule_this_loop.size();
				inverse_index_rule_this_loop[(int)rule_this_loop.size()] = start_pointx[i_s].second;
				rule_this_loop.push_back(rules[start_pointx[i_s].second]);
				i_s++;
				justInserted = 1;
				if (start_pointx.size() == i_s) break;
			}
		}
		if (justInserted) {
			int this_POM = ComputeMaxIntersectionRecursive(rule_this_loop, field_depth - 1, field_order);
			max_POM = std::max(this_POM, max_POM);
		}

		// delete as long as qx  > ie
		if (end_pointx.size() != i_e){
			while (qx == end_pointx[i_e].first) {
				int priority = end_pointx[i_e].second;
				if (index_rule_this_loop[priority] < 0) {
					printf("ERROR in OPTDecompositionRecursive: index_rule_this_loop[priority] < 0 \n");
					exit(1);
				}

				int index_to_delete = index_rule_this_loop[priority];
				int index_swap_in_table = inverse_index_rule_this_loop[(int)rule_this_loop.size() - 1];
				inverse_index_rule_this_loop[index_to_delete] = index_swap_in_table;
				index_rule_this_loop[index_swap_in_table] = index_to_delete;
				iter_swap(begin(rule_this_loop) + index_rule_this_loop[priority], end(rule_this_loop) - 1);
				rule_this_loop.erase(end(rule_this_loop) - 1);


				i_e++;
				justDeleted = 1;
				if (end_pointx.size() == i_e) break;
			}
		}
		if (justDeleted) {
			int this_POM = ComputeMaxIntersectionRecursive(rule_this_loop, field_depth - 1, field_order);
			max_POM = std::max(this_POM, max_POM);
		}
	}

	return max_POM;

}
std::vector<SortableRuleset>  SortableRulesetPartitioner::MaximumIndepdenentSetPartitioning(const std::vector<Rule>& rules) {
	std::vector<Rule> current_rules = rules;
	std::vector<SortableRuleset> all_buckets;
	int sum_rank = 0;
	while (!current_rules.empty()) {
		for (int i = 0; i < (int)current_rules.size(); i++) current_rules[i].id = i;
		auto out = MaximumIndependentSetAllFields(current_rules);
		std::vector<Rule> temp_rules = out.first;
		std::vector<int> field_order = out.second;
		for (auto& r : temp_rules)  {
			current_rules[r.id].markedDelete = 1;
		}

		all_buckets.push_back(SortableRuleset(temp_rules, field_order));
		current_rules.erase(
			remove_if(begin(current_rules), end(current_rules),
			[](const Rule& r) {
			return r.markedDelete; })
				, end(current_rules));
	}
	return all_buckets;
}

std::pair<std::vector<Rule>, std::vector<int>> SortableRulesetPartitioner::MaximumIndependentSetAllFields(const std::vector<Rule>& rules){
	std::vector<int> all_fields(rules[0].dim);
	std::iota(begin(all_fields), end(all_fields), 0);
	std::vector<Rule> best_so_far_rules(0);
	std::vector<int> field_order;
	do {
		auto vrules = MaximumIndependentSetGivenField(rules, all_fields);
		if (vrules.size() > best_so_far_rules.size()) {
			best_so_far_rules = vrules;
			field_order = all_fields;
		}
	} while (std::next_permutation(all_fields.begin(), all_fields.end()));
	return make_pair(best_so_far_rules, field_order);
}

std::vector<Rule> SortableRulesetPartitioner::MaximumIndependentSetGivenField(const std::vector<Rule>& rules, const std::vector<int>& fields){
		return MaximumIndependentSetGivenFieldRecursion(rules, fields, 0, 0, 0).GetRules();
}

WeightedInterval SortableRulesetPartitioner::MaximumIndependentSetGivenFieldRecursion(const std::vector<Rule>& rules, const std::vector<int>& fields, int depth, unsigned a, unsigned b){
	if (a > b) {
		printf("Error: interval [a,b] where a > b???\n");
		exit(1);
	}
	std::vector<WeightedInterval> vwi = Utilities::CreateUniqueInterval(rules, fields[depth]);
	std::vector<WeightedInterval> vwi_opt;
	if (depth < rules[0].dim - 1) {
		for (auto& wi : vwi) {
			vwi_opt.push_back(MaximumIndependentSetGivenFieldRecursion(wi.GetRules(), fields, depth + 1, wi.GetLow(), wi.GetHigh()));
		}
	} else {
		vwi_opt = vwi;
	}
	auto mwis = Utilities::MWISIntervals(vwi_opt);
	std::vector<Rule> rules_mwis;
	for (int i : mwis.first) {
		for (auto& r : vwi_opt[i].GetRules()) {
			rules_mwis.push_back(r);
		}
	}
	return WeightedInterval(rules_mwis, a, b);
}


//////////
std::pair<std::vector<SortableRulesetPartitioner::part>, int> SortableRulesetPartitioner::FastMWISonPartition(const part& apartition, int field) {

	if (apartition.size() == 0) {
		return make_pair(std::vector<part>(), 0);
	}
	std::vector<interval> rules_given_field;
	rules_given_field.reserve(apartition.size());
	int i = 0;
	for (const auto & r : apartition) {
		rules_given_field.emplace_back(r.range[field][0], r.range[field][1], i++);
	}
	std::sort(std::begin(rules_given_field), std::end(rules_given_field), [](const interval& lhs, const interval& rhs) {
		if (lhs.b < rhs.b) {
			return lhs.b < rhs.b;
		}
		return lhs.a < lhs.a;
	});

	std::vector<LightWeightedInterval> wi = Utilities::FastCreateUniqueInterval(rules_given_field);

	auto mwis = Utilities::FastMWISIntervals(wi);

	std::vector<part> vp;
	std::vector<part> new_partitions;
	for (int i : mwis.first) {
		part temp_partition_rule;
		for (auto& ind : wi[i].GetRuleIndices()) {
			temp_partition_rule.push_back(apartition[rules_given_field[ind].id]);
		}
		new_partitions.push_back(temp_partition_rule);
	}

	return make_pair(new_partitions, mwis.second);
}



std::pair<std::vector<SortableRulesetPartitioner::part>, int> SortableRulesetPartitioner::FastMWISonEntirePartition(const std::vector<part>& all_partition, int current_field) {
	std::vector<part> new_entire_partition;
	int sum_weight = 0;
	for (const auto& p : all_partition) {

		auto pvi = FastMWISonPartition(p, current_field);
		new_entire_partition.insert(end(new_entire_partition), begin(pvi.first), end(pvi.first));
		sum_weight += pvi.second;
	}
	return std::make_pair(new_entire_partition, sum_weight);
}

void SortableRulesetPartitioner::FastBestFieldAndConfiguration(std::vector<part>& all_partition, std::vector<int>& current_field, int num_fields)
{

	std::vector<SortableRulesetPartitioner::part> best_new_partition;
	int best_so_far_mwis = -1;
	int current_best_field = -1;
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {

			auto mwis = FastMWISonEntirePartition(all_partition, j);
			if (mwis.second >= best_so_far_mwis) {
				best_so_far_mwis = mwis.second;
				current_best_field = j;
				best_new_partition = mwis.first;
			}
		}
	}
	all_partition = best_new_partition;
	current_field.push_back(current_best_field);
}

std::pair<bool, std::vector<int>> SortableRulesetPartitioner::FastGreedyFieldSelectionTwoIterations(const std::vector<Rule>& rules) {
	if (rules.size() == 0) {
		printf("Warning: GreedyFieldSelection rule size = 0\n ");
	}
	int num_fields = rules[0].dim;

	std::vector<int> current_field;
	std::vector<part> all_partitions;

	all_partitions.push_back(rules);
	for (int i = 0; i < 2; i++) {
		FastBestFieldAndConfiguration(all_partitions, current_field, num_fields);
	}

	std::vector<Rule> current_rules;
	for (auto& r : all_partitions) {
		current_rules.insert(end(current_rules), begin(r), end(r));
	}
	//fill in the rest of the field in order
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {
			current_field.push_back(j);
		}
	}
	return std::make_pair(current_rules.size() == rules.size(), current_field);
}
std::pair<std::vector<Rule>, std::vector<int>> SortableRulesetPartitioner::FastGreedyFieldSelection(const std::vector<Rule>& rules)
{
	if (rules.size() == 0) {
		printf("Warning: GreedyFieldSelection rule size = 0\n ");
	}
	int num_fields = rules[0].dim;

	std::vector<int> current_field;
	std::vector<part> all_partitions;

	all_partitions.push_back(rules);
	for (int i = 0; i < num_fields; i++) {
		FastBestFieldAndConfiguration(all_partitions, current_field, num_fields);
	}

	std::vector<Rule> current_rules;
	for (auto& r : all_partitions) {
		current_rules.insert(end(current_rules), begin(r), end(r));
	}
	//fill in the rest of the field in order
	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {
			;
			current_field.push_back(j);
		}
	}
	return std::make_pair(current_rules, current_field);
}

std::pair<bool, std::vector<int>> SortableRulesetPartitioner::FastGreedyFieldSelectionForAdaptive(const std::vector<Rule>& rules) {
	if (rules.size() == 0) {
		printf("Warning: GreedyFieldSelection rule size = 0\n ");
	}
	int num_fields = rules[0].dim;

	std::vector<int> current_field;
	std::vector<part> all_partitions;

	all_partitions.push_back(rules);
	for (int i = 0; i < num_fields; i++) {
		FastBestFieldAndConfiguration(all_partitions, current_field, num_fields);
	}

	for (int j = 0; j < num_fields; j++) {
		if (std::find(begin(current_field), end(current_field), j) == end(current_field)) {
			current_field.push_back(j);
		}
	}

	return std::make_pair(all_partitions[0].size() == rules.size(), current_field);
}
