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
#include "RuleSplitter.h"

#include <algorithm>
#include <set>
#include <unordered_set>

#include "../TupleMerge/SlottedTable.h"

using namespace std;
using namespace RulesetSplitter;

namespace RulesetSplitter {
	size_t SplitCostWithinLimit(const vector<Rule>& rules, const vector<size_t>& indices, const vector<int>& fieldOrder, size_t limit, size_t fieldIndex) {
		if (indices.empty()) {
			return 0;
		} else if (fieldIndex < fieldOrder.size()) {
			int field = fieldOrder[fieldIndex];
			size_t children = 0;
			vector<Range> segments = Segmentize(rules, indices, field);
			for (const Range& s : segments) {
				vector<size_t> sublist;
				for (size_t index : indices) {
					const Rule& ri = rules[index];
					if (ri.range[field][LowDim] <= s[LowDim] && ri.range[field][HighDim] >= s[HighDim]) {
						sublist.push_back(index);
					}
				}
				size_t cost = SplitCostWithinLimit(rules, sublist, fieldOrder, limit - children, fieldIndex + 1);
				
				children += cost;
				if (children > limit) break;
			}
			return children;
		} else {
			return 1;
		}
	}
	
	bool CanSplitWithinLimit(const vector<Rule>& rules, const vector<size_t>& indices, const vector<int>& fieldOrder, size_t limit) {
		size_t cost = SplitCostWithinLimit(rules, indices, fieldOrder, limit, 0);
		return cost <= limit;
	}
	
	vector<Rule> SplitRules(const vector<Rule>& rules, const vector<int>& fieldOrder, size_t index) {
		if (rules.empty()) {
			return rules;
		} else if (index < fieldOrder.size()) {
			int field = fieldOrder[index];
			vector<Rule> results;
			vector<Range> segments = Segmentize(rules, field);
			for (Range s : segments) {
				vector<Rule> sublist;
				for (const Rule r : rules) {
					if (r.range[field][LowDim] <= s[LowDim] && r.range[field][HighDim] >= s[HighDim]) {
						Rule copy = r;
						copy.range[field] = s;
						sublist.push_back(copy);
					}
				}
				vector<Rule> splits = SplitRules(sublist, fieldOrder, index + 1);
				results.insert(results.end(), splits.begin(), splits.end());
			}
			return results;
		} else {
			Rule r = *std::max_element(rules.begin(), rules.end(), [](const Rule& r1, const Rule& r2) { return r1.priority < r2.priority;});
			
			return vector<Rule>{r};
		}
	}
	
	vector<Range> Segmentize(const vector<Rule>& rules, const vector<size_t>& indices, int field) {
		set<Point> highs;
		
		for (size_t i : indices) {
			highs.insert(rules[i].range[field][LowDim] - 1);
			highs.insert(rules[i].range[field][HighDim]);
		}
		vector<Point> bounds(highs.begin(), highs.end());
		sort(bounds.begin(), bounds.end());
		
		Point left = 0;
		vector<Range> segments;
		for (Point h : highs) {
			segments.push_back({left, h});
			left = h + 1;
		}
		return segments;
	}
	
	vector<Range> Segmentize(const vector<Rule>& rules, int field) {
		set<Point> highs;
		
		for (const Rule& r : rules) {
			highs.insert(r.range[field][LowDim] - 1);
			highs.insert(r.range[field][HighDim]);
		}
		vector<Point> bounds(highs.begin(), highs.end());
		sort(bounds.begin(), bounds.end());
		
		Point left = 0;
		vector<Range> segments;
		for (Point h : highs) {
			segments.push_back({left, h});
			left = h + 1;
		}
		return segments;
	}
	
	bool SegmentsContainRange(const vector<Range>& segments, const Range& s, size_t low, size_t high) {
		if (low >= high) return false;
		size_t mid = (low + high) / 2;
		if (segments[mid][LowDim] == s[LowDim] && segments[mid][HighDim] == s[HighDim]) return true;
		else if (s[HighDim] < segments[mid][LowDim]) {
			return SegmentsContainRange(segments, s, low, mid);
		} else if (s[LowDim] > segments[mid][HighDim]) {
			return SegmentsContainRange(segments, s, mid + 1, high);
		} else {
			return false;
		}
	}
	
	bool SegmentsContainRange(const vector<Range>& segments, const Range& s) {
		return SegmentsContainRange(segments, s, 0, segments.size());
	}
	
	vector<Rule> ChooseAndSplit(const vector<Rule>& rules, const vector<size_t>& indices, const vector<int>& fieldOrder, vector<size_t>& remain, size_t limit) {
		vector<size_t> is = indices;
		
		for (size_t fieldIndex = 0; fieldIndex < fieldOrder.size(); fieldIndex++) {
			int field = fieldOrder[fieldIndex];
			if (CanSplitWithinLimit(rules, is, fieldOrder, limit)) {
				vector<Rule> rl;
				for (size_t i : is) {
					rl.push_back(rules[i]);
				}
				return SplitRules(rl, fieldOrder, 0);
			} else {
				vector<Range> segments = Segmentize(rules, is, field);
				vector<size_t> js;
				for (size_t i : is) {
					if (SegmentsContainRange(segments, rules[i].range[field])) {
						js.push_back(i);
					} else {
						remain.push_back(i);
					}
				}
				if (js.empty()) {
					return vector<Rule>();
				}
				is = js;
			}
		}
		vector<Rule> rl;
		for (size_t i : is) {
			rl.push_back(rules[i]);
		}
		return SplitRules(rl, fieldOrder, 0);
	}
	
	size_t Seek(const vector<Range>& segments, Point p, size_t l, size_t h) {
		size_t m = (l + h) / 2;
		if (p < segments[m][LowDim]) {
			return Seek(segments, p, l, m);
		} else if (p > segments[m][HighDim]) {
			return Seek(segments, p, m + 1, h);
		} else {
			return m;
		}
	}
	
	size_t Seek(const vector<Range>& segments, Point p) {
		return Seek(segments, p, 0, segments.size());
	}
	
	size_t CountSegmentsInDim(const Rule& r, const vector<Range>& segments, int field) {
		return Seek(segments, r.range[field][HighDim]) - Seek(segments, r.range[field][LowDim]) + 1;
	}
	
	void SelectionHelper(const vector<Rule>& rules, const vector<size_t>& indices, unordered_set<size_t>& keepIndices, const vector<int>& fieldOrder, const vector<size_t>& limits, size_t fieldIndex) {
		if (fieldIndex < fieldOrder.size() && indices.size() > 1) {
			int field = fieldOrder[fieldIndex];
			size_t limit = limits[fieldIndex];
		
			vector<Range> segments = Segmentize(rules, indices, field);
			for (const size_t i : indices) {
				if (CountSegmentsInDim(rules[i], segments, field) > limit) {
					keepIndices.erase(i);
				}
			}
			for (const Range& s : segments) {
				vector<size_t> is;
				for (size_t i : indices) {
					if (keepIndices.count(i) && (rules[i].range[field][LowDim] <= s[LowDim]) && rules[i].range[field][HighDim] >= s[HighDim]) {
						is.push_back(i);
					}
				}
				SelectionHelper(rules, is, keepIndices, fieldOrder, limits, fieldIndex + 1);
			}
		}
	}
	
	vector<Rule> ChooseAndSplit(const vector<Rule>& rules, const vector<size_t>& indices, const vector<int>& fieldOrder, vector<size_t>& remain, vector<size_t>& limits) {
		unordered_set<size_t> keepIndices(indices.begin(), indices.end());
		SelectionHelper(rules, indices, keepIndices, fieldOrder, limits, 0);
		vector<Rule> rl;
		for (size_t i : indices) {
			if (keepIndices.count(i)) {
				rl.push_back(rules[i]);
			} else {
				remain.push_back(i);
			}
		}
		return SplitRules(rl, fieldOrder, 0);
	}
	
	bool SplitsExistingRules(const Rule& r, const vector<Rule>& rules, const vector<int>& fieldOrder) {
		vector<Rule> keep = rules;
		for (int f : fieldOrder) {
			vector<Rule> next;
			auto s = r.range[f];
			for (Rule ri : keep) {
				auto si = ri.range[f];
				if (s[LowDim] > si[HighDim] || s[HighDim] < si[LowDim]) {
					// No overlap - Ignore
				} else if (s[LowDim] <= si[LowDim] && s[HighDim] >= si[HighDim]) {
					next.push_back(ri);
				} else {
					return true;
				}
			}
			keep = next;
		}
		return false;
	}
	
	void RemoveIrrelevant(vector<Rule>& rl, const Rule& r, const vector<int>& fieldOrder) {
		int field = fieldOrder[0];
		rl.erase(remove_if(rl.begin(), rl.end(), [=,&r](const Rule& rx) { return rx.range[field][HighDim] < r.range[field][LowDim] || rx.range[field][LowDim] > r.range[field][HighDim]; }), rl.end());
	}
	
	void SplitAgainstExistingRules(const Rule& r, const vector<Rule>& rules, const vector<int>& fieldOrder, vector<Rule>& result, size_t fieldIndex = 0) {
		if (rules.empty()) {
			result.push_back(r);
		} else if (fieldIndex < fieldOrder.size()) {
			int field = fieldOrder[fieldIndex];
			vector<Rule> rl = rules;
			sort(rl.begin(), rl.end(), [=](const Rule& rx, const Rule& ry) { return rx.range[field][LowDim] < ry.range[field][LowDim]; });
			vector<Rule> copy = rl;
			copy.push_back(r);
			auto iter = rl.begin();
			
			auto segments = Segmentize(copy, field);
			vector<Rule> overlaps;
			Rule rprime = r;
			for (Range s : segments) {			
				if (SegmentUtil::Intersects(r.range[field], s)) {
					while (iter != rl.end() && iter->range[field][LowDim] <= s[HighDim]) {
						overlaps.push_back(*iter);
						iter++;
					}
					overlaps.erase(remove_if(overlaps.begin(), overlaps.end(), [=](const Rule& rx) { return rx.range[field][HighDim] < s[LowDim];}), overlaps.end());
					
					rprime.range[field] = s;
					SplitAgainstExistingRules(rprime, overlaps, fieldOrder, result, fieldIndex + 1);
				}
			}
		} else {
			result.push_back(r);
		}
	}
	
	size_t PiecesAgainstExistingRules_alt(const Rule& r, const vector<Rule>& rules, const vector<int>& fieldOrder, size_t limit, size_t fieldIndex = 0) {
		if (rules.empty()) {
			// No other rules to split against
			return 1;
		} else if (fieldIndex < fieldOrder.size()) {
			size_t numPieces = 0;
			int field = fieldOrder[fieldIndex];
			vector<Rule> rl = rules;
			// Sort rules by the entry point
			// Then we scan forward with the segments
			// So each rule only has to be visited a fixed number of times
			/*
			if (fieldIndex && rl.size() > 10) {
				rl.erase(remove_if(rl.begin(), rl.end(), [=,&r](const Rule& rx) { return rx.range[field][HighDim] < r.range[field][LowDim] || rx.range[field][LowDim] > r.range[field][HighDim]; }), rl.end());
				if (rl.empty()) return 1;
				printf("Rules: %lu, Rl: %lu\n", rules.size(), rl.size());
			}*/
			//exit(0);
			//return rl.size();
			
			sort(rl.begin(), rl.end(), [=](const Rule& rx, const Rule& ry) { return rx.range[field][LowDim] < ry.range[field][LowDim]; });
			auto iter = rl.begin();
			
			auto segments = Segmentize(rl, field);
			vector<Rule> overlaps;
			for (Range s : segments) {
				if (SegmentUtil::Intersects(r.range[field], s)) {
				//r.range[field][LowDim] <= s[LowDim] && r.range[field][HighDim] >= s[HighDim]) {
					while (iter != rl.end() && iter->range[field][LowDim] <= s[HighDim]) {
						overlaps.push_back(*iter);
						iter++;
					}
					// I think it takes more effort to keep it sorted than to visit each element once
					overlaps.erase(remove_if(overlaps.begin(), overlaps.end(), [=](const Rule& rx) { return rx.range[field][HighDim] < s[LowDim];}), overlaps.end());
					/*for (const Rule& ri : rules) {
						if (ri.range[field][LowDim] <= s[LowDim] && ri.range[field][HighDim] >= s[HighDim]) {
							overlaps.push_back(ri);
						}
					}*/
					numPieces += PiecesAgainstExistingRules_alt(r, overlaps, fieldOrder, limit - numPieces, fieldIndex + 1);
				}
				if (numPieces > limit) break;
			}
			
			if (r.range[field][HighDim] > segments.back()[HighDim]) {
				// Extends past all existing rules
				numPieces++;
			}
			if (numPieces == 0) {
				printf("Dim: %d\n", field);
				for (Range s : segments) {
					SegmentUtil::Print(s);
				}
				r.Print();
			}
			return numPieces;
		} else {
			// End of the line
			return 1;
		}
	}
	
	size_t PiecesAgainstExistingRules(const Rule& r, const vector<Rule>& rules, const vector<int>& fieldOrder, size_t limit, size_t fieldIndex = 0) {

		if (rules.empty()) {
			// No other rules to split against
			return 1;
		} else if (fieldIndex < fieldOrder.size()) {
			size_t numPieces = 0;
			int field = fieldOrder[fieldIndex];
			auto segments = Segmentize(rules, field);

			for (Range s : segments) {
				if (SegmentUtil::Intersects(r.range[field], s)) {
				//r.range[field][LowDim] <= s[LowDim] && r.range[field][HighDim] >= s[HighDim]) {
					vector<Rule> overlaps;
					for (const Rule& ri : rules) {
						if (ri.range[field][LowDim] <= s[LowDim] && ri.range[field][HighDim] >= s[HighDim]) {
							overlaps.push_back(ri);
						}
					}
					numPieces += PiecesAgainstExistingRules(r, overlaps, fieldOrder, limit - numPieces, fieldIndex + 1);
				}

				if (numPieces > limit) break;
			}

			if (r.range[field][HighDim] > segments.back()[HighDim]) {
				// Extends past all existing rules
				numPieces++;
			}

			if (numPieces == 0) {
				printf("Dim: %d\n", field);
				for (Range s : segments) {
					SegmentUtil::Print(s);
				}
				r.Print();
			}
			return numPieces;
		} else {
			// End of the line
			return 1;
		}
	}
	
	vector<Rule> CoreAndSplit(const vector<Rule>& core, const vector<Rule>& rules, const vector<int>& fieldOrder, const vector<size_t>& indices, vector<size_t>& remain, size_t limit) {
		printf("C&S: %lu / %lu\n", core.size(), indices.size());
		vector<pair<size_t, size_t>> costsAndIndices;
		
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		std::chrono::duration<double> elapsed_seconds;
		
		// Compute cost of each rule against a set of core rules
		start = std::chrono::steady_clock::now();
		for (size_t index : indices) {
			Rule r = rules[index];
			vector<Rule> cored = core;
			RemoveIrrelevant(cored, r, fieldOrder);
			if (!SplitsExistingRules(r, cored, fieldOrder)) {
				//if (cored.size() < core.size() / 2) 
				{
					//printf("Removed\n");
					//printf("Core Size: %lu\n", cored.size());
					size_t cost = PiecesAgainstExistingRules_alt(r, cored, fieldOrder, limit);
					//printf("Costed\n");
					costsAndIndices.push_back(pair<size_t, size_t>(cost, index));
				}
			}
		}
		end = std::chrono::steady_clock::now();
		elapsed_seconds = end - start;
		printf("Estimation time: %f \n", elapsed_seconds.count());
		//printf("\tCosted: %lu\n", costsAndIndices.size());
		//printf("\tMin Cost: %lu\n", costsAndIndices[0].first);
		
		// Sort rules by cost and then by index
		sort(costsAndIndices.begin(), costsAndIndices.end());
		unordered_set<size_t> keepIndices;
		vector<Rule> keep;
		size_t expected = 0;
		for (auto pair : costsAndIndices) {
			/*if (pair.first < limit) {
				Rule r = rules[pair.second];
				size_t newCost = PiecesAgainstExistingRules(r, keep, fieldOrder);
				if (newCost < 1) {
					printf("%lu\n", newCost);
					r.Print();
					exit(0);
				}
				if (newCost <= pair.first) {
					keep.push_back(r);
					keepIndices.insert(pair.second);
					expected += newCost;
					limit -= newCost;
				}
			}*/
			
			// Grab all of the rules until we exceed the allowable threshold
			if (pair.first < limit /*&& pair.first < 20*/) {
				//printf("%lu / %lu\n", pair.first, limit);
				limit -= pair.first;
				expected += pair.first;
				keep.push_back(rules[pair.second]);
				keepIndices.insert(pair.second);
			} else {
				//printf("x%lu / %lu\n", pair.first, limit);
				break;
			}
		}
		//printf("\tMax: %lu / %lu\n", costsAndIndices.back().first, limit);
		//printf("\tKeeping: %lu\n", keepIndices.size());
		//printf("\tExpected: %lu\n", expected);
		for (size_t index : indices) {
			if (!keepIndices.count(index)) {
				remain.push_back(index);
			}
		}
		start = std::chrono::steady_clock::now();
		vector<Rule> result = SplitRules(keep, fieldOrder, 0);
		end = std::chrono::steady_clock::now();
		elapsed_seconds = end - start;
		printf("Splitting time: %f \n", elapsed_seconds.count());
		
		//printf("\tResult: %lu\n", result.size());
		//exit(0);
		
		//printf("C&S\n");
		//printf("\tCore: %2f\n", 1.0 * core.size() / indices.size());
		//printf("\tKicked: %2f\n", 1.0 - 1.0 * costsAndIndices.size() / indices.size());
		//printf("\tAdmitted: %2f\n", 1.0 * keep.size() / indices.size());
		return result;
	}
}

SplitSort::SplitSort(const unordered_map<string, string>& args) 
		: PacketClassifier(), 
		  //rules(), mitrees(0), ruleFragments(), 
		  splitFactor(GetUIntOrElse(args, "PS.SplitFactor", 2)) {
	printf("SplitFactor: %lu\n", splitFactor);
	ruleFragments.reserve(100);
}

SplitSort::~SplitSort() {
	for (auto tree : mitrees) {
		delete tree;
	}
}

void SplitSort::ConstructClassifier(const vector<Rule>& rules) {
	ruleFragments.reserve(rules.size());
	/*for (const Rule& r : rules) {
		InsertRule(r);
	}*/
	
	vector<size_t> indices;
	for (size_t i = 0; i < rules.size(); i++) {
		indices.push_back(i);
	}
	while (!indices.empty()) {
	  //printf("%lu\n", indices.size());
		vector<Rule> rl;
		for (size_t i : indices) {
			rl.push_back(rules[i]);
		}
		auto rulesAndFieldOrder = SortableRulesetPartitioner::GreedyFieldSelection(rl);
		vector<int> fieldOrder = rulesAndFieldOrder.second;
		vector<size_t> nextIndices;
		size_t limit = indices.size() * splitFactor;
		vector<size_t> limits(fieldOrder.size(), splitFactor);
		
		//vector<Rule> bucket = RulesetSplitter::ChooseAndSplit(rules, indices, fieldOrder, nextIndices, limit);
		//vector<Rule> bucket = RulesetSplitter::ChooseAndSplit(rules, indices, fieldOrder, nextIndices, limits);
		//vector<Rule> core = rulesAndFieldOrder.first;
		
		//vector<Rule> bucket = RulesetSplitter::CoreAndSplit(core, rules, fieldOrder, indices, nextIndices, limit);
		vector<Rule> bucket;
		if (mitrees.empty()) {
			bucket = RulesetSplitter::CoreAndSplit(rulesAndFieldOrder.first, rules, fieldOrder, indices, nextIndices, limit);
		} else {
			bucket = RulesetSplitter::ChooseAndSplit(rules, indices, fieldOrder, nextIndices, limits);
		}
		
		if (bucket.empty()) {
			bucket = rulesAndFieldOrder.first;
			unordered_map<int, size_t> priorToIndices;
			for (size_t i : indices) {
				priorToIndices[rules[i].priority] = i;
			}
			for (Rule& r : bucket) {
				priorToIndices.erase(r.priority);
			}
			nextIndices.clear();
			for (auto pair : priorToIndices) {
				nextIndices.push_back(pair.second);
			}
		}
		
		SortableRuleset srl(bucket, fieldOrder);
		auto tree = new OptimizedMITree(srl);
		mitrees.push_back(tree);
		indices = nextIndices;
		sort(indices.begin(), indices.end());
	}
	sort(mitrees.begin(), mitrees.end(), [](auto t1, auto t2) { return t1->MaxPriority() > t2->MaxPriority();}); 
	
}

void SplitSort::DeleteRule(size_t index) {
	auto pair = rules[index];
	Rule rule = pair.first;
	auto mitree = pair.second;

	vector<Rule> pieces = ruleFragments[rule.priority];
	ruleFragments.erase(index);
	bool priorityChange;
	for (Rule r: pieces) {
		mitree->Deletion(r, priorityChange);
	}
	if (priorityChange) {
		sort(mitrees.begin(), mitrees.end(), [](auto tree1, auto tree2) { return tree1->MaxPriority() > tree2->MaxPriority();});
	}
	
	if (index != rules.size() - 1) {
		rules[index] = std::move(rules[rules.size() - 1]);
	}
	rules.pop_back();
}

void SplitSort::InsertRule(const Rule& rule) {
	bool isInserted = false;
	bool priorityChange = false;
	// First see if we can insert without splitting
	if (false && !isInserted) {
		for (auto mitree : mitrees)
		{
			bool success = mitree->TryInsertion(rule, priorityChange);
			if (success) {
				
				mitree->ReconstructIfNumRulesLessThanOrEqualTo(10);
				rules.push_back(std::make_pair(rule, mitree));
				isInserted = true;
				ruleFragments[rule.priority] = {rule};
				break;
			}
		}
	}
	
	// Then try insert with splitting
	if (!isInserted) {
		for (auto mitree : mitrees) {
			if (mitree->TryInsertion(rule, priorityChange)) {
				mitree->ReconstructIfNumRulesLessThanOrEqualTo(10);
				rules.push_back(std::make_pair(rule, mitree));
				isInserted = true;
				ruleFragments[rule.priority] = {rule};
				break;
			}
			if (mitree->NumRules() > 20) { // Min threshold
				vector<Rule> core = mitree->GetRules();
				vector<int> fieldOrder = mitree->GetFieldOrder();
				size_t limit = 10; //core.size(); // TODO : limit
				RemoveIrrelevant(core, rule, fieldOrder);
				bool collides = false;
				for (Rule rc : core) {
					if (rc.IntersectsRule(rule)) {
						collides = true;
						break;
					}
				}
				
				if (!collides && !SplitsExistingRules(rule, core, fieldOrder)) {
					size_t cost = PiecesAgainstExistingRules_alt(rule, core, fieldOrder, limit);
					if (cost < limit) { // TODO : limit
						//printf("\nSplitting: %d at cost %lu against %lu rules\n", rule.priority, cost, core.size());
						//for (Rule r : core) {
						//	r.Print();
						//}
						//ForgeUtils::PrintTuple(fieldOrder);
						//rule.Print();
						vector<Rule> pieces;
						SplitAgainstExistingRules(rule, core, fieldOrder, pieces);
						//core.push_back(rule);
						//pieces = SplitRules(core, fieldOrder, 0);
						//pieces.erase(remove_if(pieces.begin(), pieces.end(), [=](Rule rx) { return rx.priority != rule.priority; } ), pieces.end());
						for (Rule& r : pieces) {
							bool ok = mitree->TryInsertion(r, priorityChange);
							if (!ok) {
								// Collided with another rule
								printf("Failed\n");
							}
							//printf("%d : ", ok);
							//r.Print();
						}
						ruleFragments[rule.priority] = pieces;
						rules.push_back(std::make_pair(rule, mitree));
						isInserted = true;
						break;
					}
				}
			}
		}
	}
	
	// Last create a new tree
	if (!isInserted) {
		auto tree_ptr = new OptimizedMITree(rule);
		tree_ptr->TryInsertion(rule, priorityChange);
		rules.push_back(std::make_pair(rule, tree_ptr));
		mitrees.push_back(tree_ptr);  
		ruleFragments[rule.priority] = {rule};
		isInserted = true;
		
	}
	
	if (priorityChange) {
		sort(mitrees.begin(), mitrees.end(), [](auto tree1, auto tree2) { return tree1->MaxPriority() > tree2->MaxPriority();});
	}
}

int SplitSort::ClassifyAPacket(const Packet& packet) {
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
