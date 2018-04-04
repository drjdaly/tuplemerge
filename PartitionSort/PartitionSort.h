/*
 * MIT License
 *
 * Copyright (c) 2016, 2017 by S. Yingchareonthawornchai and J. Daly at Michigan State University
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
#ifndef  PSORT_H
#define  PSORT_H

#include "OptimizedMITree.h"
#include "../Simulation.h"
#include "LongestIncreasingSubsequence.h"
#include "SortableRulesetPartitioner.h"
#include "DISCPAC.h"

class PartitionSort : public PacketClassifier {

public:
	~PartitionSort() {
		for (auto x : mitrees) {
			free(x);
		}
	}
	void ConstructClassifier(const std::vector<Rule>& rules) override {
		/*SmartAdaptiveDISCPAC sad;
		sad.ConstructClassifier(rules);
		auto buckets = sad.ExportSortableRulesets();
		*/
		this->rules.reserve(rules.size());
		for (const auto& r : rules) {
			InsertRule(r);
		}
		//ReportLIS();
		/*
		for (auto& b : buckets)  {
			mitrees.push_back(new OptimizedMITree(b)); 
		}
		*/
		/*
		auto buckets = SortableRulesetPartitioner::SortableRulesetPartitioningGFS(rules);
		for (auto& b : buckets)  {
			mitrees.push_back(new OptimizedMITree(b));
		}*/
		//for (auto& b : mitrees) {
		//	if (b->NumRules() < 20) {
		//		printf("Tree [%u rules]: ", b->NumRules());
		//		b->PrintFieldOrder();
		//		for (auto& r : b->SerializeIntoRules()) {
		//			r.Print();
		//		}
		//	}
		//}
	}
	int ClassifyAPacket(const Packet& packet) {
		int result = -1;
		int query = 0;
		for (const auto& t : mitrees) {
		
			if (result > t->MaxPriority()){
				break;
			}
			query++;
			result = std::max(t->ClassifyAPacket(packet), result);
		}
		QueryUpdate(query);
		return result;
		
	}  
	void DeleteRule(size_t index);
	void InsertRule(const Rule& one_rule);
	
	Memory MemSizeBytes() const {
		int size_total_bytes = 0;
		for (const auto& t : mitrees) {
			size_total_bytes += t->MemoryConsumption();
		}
		int size_array_pointers = mitrees.size() * POINTER_SIZE_BYTES;
		int assignmentsSize = rules.size() * POINTER_SIZE_BYTES;
		return size_total_bytes + size_array_pointers + assignmentsSize;
	}
	int MemoryAccess() const {
		return 0;
	}
	size_t NumTables() const {
		return mitrees.size();
	}
	size_t RulesInTable(size_t index) const { return mitrees[index]->NumRules(); }
	size_t PriorityOfTable(size_t index) const {
		return mitrees[index]->MaxPriority();
	}


	void ReportLIS() const {
		//construct min priority order 
		int max_priority = 0;
		for (auto p : rules) {
			max_priority = std::max(p.first.priority,max_priority);
		}
		std::vector<int> bucket_list;
		std::vector<int> priority_list;
		int end_ind = 0;
		int num_bucket = 0;
		for (auto tree : mitrees) {
			auto vr = tree->GetRules();
			printf("%lu ", vr.size());
			for (auto& r: vr) {
				priority_list.push_back(max_priority - r.priority);
				bucket_list.push_back(num_bucket);
			}
			 std::sort(begin(priority_list)+ end_ind,end(priority_list));
		//	random_shuffle(begin(priority_list) + end_ind, end(priority_list));
			end_ind = priority_list.size(); 
			num_bucket++;
		}

 
		printf("n=%lu\n", priority_list.size());
		int i = 1;
		while (!priority_list.empty()) {
			auto pair_result = lis(priority_list);
			printf("%d - lis = %lu ", i++, pair_result.first.size());
			vector<int> thislevelb;
			for (int x : pair_result.second) {

				thislevelb.push_back(bucket_list[x]);
				priority_list[x] = bucket_list [x]= -1;
			}
			priority_list.erase(std::remove(priority_list.begin(), priority_list.end(), -1), priority_list.end());
			bucket_list.erase(std::remove(bucket_list.begin(), bucket_list.end(), -1), bucket_list.end());
			std::set<int> unique_set(begin(bucket_list), end(bucket_list));
		

			std::set<int> unique_set2(begin(thislevelb), end(thislevelb));
			printf(", thislevelb=%lu, remainingb=%lu\n", unique_set2.size(), unique_set.size());
		}

	}
protected:
	std::vector<OptimizedMITree *> mitrees;
	std::vector<std::pair<Rule,OptimizedMITree *>> rules;

	 
	void InsertionSortMITrees() {
		int i, j, numLength = mitrees.size();
		OptimizedMITree * key;
		for (j = 1; j < numLength; j++)
		{
			key = mitrees[j];
			for (i = j - 1; (i >= 0) && (mitrees[i]-> MaxPriority() < key-> MaxPriority()); i--)
			{
				mitrees[i + 1] = mitrees[i];
			}
			mitrees[i + 1] = key;
		}
	}

};


class PartitionSortOffline : public PartitionSort {

public:
	void ConstructClassifier(const std::vector<Rule>& rules) override { 
		auto buckets = SortableRulesetPartitioner::SortableRulesetPartitioningGFS(rules);
		for (auto& b : buckets)  {
			mitrees.push_back(new OptimizedMITree(b));
		}
		InsertionSortMITrees();
		/*for (auto& tree : mitrees) {
			printf("%lu [%lu]\n", tree->NumRules(), tree->MaxPriority());
		}*/
	}
};

#endif
