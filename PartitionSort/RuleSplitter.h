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
#pragma once
#include "../ElementaryClasses.h"
#include "../Utilities/IntervalUtilities.h"
#include "../Utilities/MapExtensions.h"
#include "../Simulation.h"
#include "SortableRulesetPartitioner.h"
#include "OptimizedMITree.h"

namespace RulesetSplitter {
	typedef std::array<Point,2> Range;
	
	bool CanSplitWithinLimit(const std::vector<Rule>& rules, const std::vector<size_t>& indices, const std::vector<int>& fieldOrder, size_t limit);
	
	std::vector<Rule> SplitRules(const std::vector<Rule>& rules, const std::vector<int>& fieldOrder);
	
	std::vector<Range> Segmentize(const std::vector<Rule>& rules, const std::vector<size_t>& indices, int field);
	std::vector<Range> Segmentize(const std::vector<Rule>& rules, int field);
	
	std::vector<Rule> ChooseAndSplit(const std::vector<Rule>& rules, const std::vector<size_t>& indices, const std::vector<int>& fieldOrder, std::vector<size_t>& remain, size_t limit);
	std::vector<Rule> ChooseAndSplit(const std::vector<Rule>& rules, const std::vector<size_t>& indices, const std::vector<int>& fieldOrder, std::vector<size_t>& remain, const std::vector<size_t>& limits);
}

class SplitSort : public PacketClassifier {
public:
	SplitSort(const std::unordered_map<std::string, std::string>& args);
	~SplitSort();
	
	void ConstructClassifier(const std::vector<Rule>& rules);
	int ClassifyAPacket(const Packet& packet);
	
	void DeleteRule(size_t index);
	void InsertRule(const Rule& rule);

	Memory MemSizeBytes() const {
		int size_total_bytes = 0;
		for (const auto& t : mitrees) {
			size_total_bytes += t->MemoryConsumption();
		}
		int size_array_pointers = mitrees.size();
		int size_of_pointer = 4;
		return size_total_bytes + size_array_pointers*size_of_pointer;
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
	
protected:
	//std::vector<Rule> rules;
	std::vector<std::pair<Rule,OptimizedMITree *>> rules;
	std::vector<OptimizedMITree *> mitrees;
	std::unordered_map<int, std::vector<Rule>> ruleFragments;
	const size_t splitFactor;
};
