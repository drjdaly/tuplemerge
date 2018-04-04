/*
 * MIT License
 *
 * Copyright (c) 2016, 2017 by J. Daly at Michigan State University
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
#include "EffectiveGrid.h"

#include<functional>
#include<unordered_set>

using namespace std;

template<class S, class T>
std::vector<T> ConvertDistinct(const std::vector<S>& v, std::function<T(S)> f) {
	std::unordered_set<T> w;
	for (const S& s : v) {
		w.insert(f(s));
	}
	return std::vector<T>(w.begin(), w.end());
	//r.assign(w.begin(), w.end());
}

EffectiveGrid::EffectiveGrid(const vector<Rule>& rules, int dim) : dim(dim) {
	vector<unsigned int> lows = ConvertDistinct<Rule, unsigned int>(rules, [=](const Rule& r) { return r.range[dim][LowDim]; });
	vector<unsigned int> highs = ConvertDistinct<Rule, unsigned int>(rules, [=](const Rule& r) { return r.range[dim][HighDim]; });
	sort(lows.begin(), lows.end());
	sort(highs.begin(), highs.end());

	if (!rules.empty()) {
		unsigned int left = lows[0];
		size_t l = 1, h = 0;
		while (l < lows.size() || h < highs.size()) {
			while (l < lows.size() && lows[l] <= left) l++;
			while (h < highs.size() && highs[h] < left) h++;
			if (h >= highs.size() || (l < lows.size() && lows[l] <= highs[h])) {
				ranges.push_back({ left, lows[l] - 1 });
				left = lows[l];
				l++;
			} else if (l >= lows.size() || (h < highs.size() && highs[h] < lows[l])) {
				ranges.push_back({ left, highs[h] });
				left = highs[h] + 1;
				h++;
			}
		}
	}

	//for (const GRange& r : ranges) {
	//	printf("(%u, %u)\n", r[LowDim], r[HighDim]);
	//}
	//printf("\n");
}

/*
 * Computes the size of the rule in effective grid space
 */
size_t EffectiveGrid::SizeOfRule(const Rule& r) const {
	unsigned int low = r.range[dim][LowDim];
	unsigned int high = r.range[dim][HighDim];
	size_t lowIndex = BinarySearch(0, ranges.size(), low);
	size_t highIndex = BinarySearch(0, ranges.size(), high);
	return highIndex - lowIndex + 1;
}

/*
 * Finds the sum size of all of the rules in effective grid space
 */
size_t EffectiveGrid::SizeOfRuleList(const std::vector<Rule>& rules) const {
	size_t sum = 0;
	for (const Rule& r : rules) {
		sum += SizeOfRule(r);
	}
	return sum;
}

/*
 * Splits the rule into many rules so each rule only takes up a single cell in the effective grid of this dimension
 * These rules are placed into the receiving list
 */
void EffectiveGrid::SplitRule(const Rule& r, std::vector<Rule>& receivingList) const {
	unsigned int low = r.range[dim][LowDim];
	unsigned int high = r.range[dim][HighDim];
	size_t lowIndex = BinarySearch(0, ranges.size(), low);
	size_t highIndex = BinarySearch(0, ranges.size(), high);
	for (size_t i = lowIndex; i <= highIndex; i++) {
		Rule copy = r;
		copy.range[dim] = ranges[i];
		receivingList.push_back(copy);
	}
}

/*
 * Splits the whole rule list and places all of the results into the receiving list
 */
void EffectiveGrid::SplitRuleList(const std::vector<Rule>& rules, std::vector<Rule>& receivingList) const {
	for (const Rule& r : rules) {
		SplitRule(r, receivingList);
	}
}

/*
 * Helper function to find the range where a given point is enclosed
 */
size_t EffectiveGrid::BinarySearch(size_t l, size_t h, unsigned int pt) const {
	if (h <= l) return -1;
	size_t m = (l + h) / 2;
	if (ranges[m][LowDim] > pt) {
		return BinarySearch(l, m, pt);
	} else if (ranges[m][HighDim] < pt) {
		return BinarySearch(m + 1, h, pt);
	} else {
		return m;
	}
}

/*
 * Finds the size of the rules in a d-dimensional effective grid
 * This is likely to be faster than splitting the rule list and counting the results
 */
size_t EffectiveGrid::GridSize(const vector<Rule>& rules, int numDims) {
	vector<EffectiveGrid> grids;
	for (int i = 0; i < numDims; i++) {
		grids.push_back(EffectiveGrid(rules, i));
	}

	size_t size = 0;
	for (const Rule& r : rules) {
		size_t vol = 1;
		for (const EffectiveGrid& g : grids) {
			vol *= g.SizeOfRule(r);
		}
		//printf("\t%u\n", vol);
		size += vol;
	}
	return size;
}

/*
 * Finds the size of the d-dimensional effective grid itself
 */
size_t EffectiveGrid::GridVolume(const vector<Rule>& rules, int numDims) {
	size_t volume = 1;
	for (int i = 0; i < numDims; i++) {
		EffectiveGrid g(rules, i);
		volume *= g.ranges.size();
	}
	return volume;
}

/* 
 * Splits all of the rules so they occupy a single cell in a d-dimensional effective grid
 * The results are placed into the receiving list
 */
void EffectiveGrid::GridSplit(const vector<Rule>& rules, int numDims, vector<Rule>& receivingList) {
	vector<Rule> rl = rules;
	for (int d = 0; d < numDims; d++) {
		EffectiveGrid grid(rules, d);
		vector<Rule> rec;
		grid.SplitRuleList(rl, rec);
		rl = rec;
	}
	receivingList = rl;
}

void EffectiveGrid::SortableSplit(const vector<Rule>& rules, const vector<int>& dimOrder, vector<Rule>& receivingList, size_t dimIndex) {
	if (dimIndex >= dimOrder.size()) {
		for (const Rule& r : rules) {
			receivingList.push_back(r);
		}
	} else {
		// TODO : Needs RR
		int dim = dimOrder[dimIndex];
		EffectiveGrid grid(rules, dim);
		vector<Rule> rec;
		grid.SplitRuleList(rules, rec);

		// Group rules on dim
		vector<unsigned int> lows = ConvertDistinct<Rule, unsigned int>(rec, [=](const Rule& r) {return r.range[dim][LowDim]; });
		// Operate on each group
		for (unsigned int l : lows) {
			vector<Rule> rl;
			for (const Rule& r : rec) {
				if (r.range[dim][LowDim] == l) {
					rl.push_back(r);
				}
			}
			RemoveConcealedRules(rl);
			SortableSplit(rl, dimOrder, receivingList, dimIndex + 1);
		}
	}
}

void EffectiveGrid::RemoveConcealedRules(vector<Rule>& rules) {
	for (auto ri = rules.begin(); ri != rules.end(); ri++) {
		rules.erase(remove_if(ri + 1, rules.end(), [ri](Rule rj) {
			for (int d = 0; d < ri->dim; d++) {
				if (rj.range[d][LowDim] < ri->range[d][LowDim] || rj.range[d][HighDim] > ri->range[d][HighDim]) {
					return true;
				}
			}
			return false;
		}), rules.end());
	}
}
