/*
 * MIT License
 *
 * Copyright (c) 2017 by J. Daly at Michigan State University
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

#include <vector>

typedef std::array<Point, 2> GRange;

class EffectiveGrid {
public:
	EffectiveGrid(const std::vector<Rule>& rules, int dim);

	size_t SizeOfRule(const Rule& r) const;
	size_t SizeOfRuleList(const std::vector<Rule>& rules) const;

	void SplitRule(const Rule& r, std::vector<Rule>& receivingList) const;
	void SplitRuleList(const std::vector<Rule>& rules, std::vector<Rule>& receivingList) const;
	
	static size_t GridSize(const std::vector<Rule>& rules, int numDims);
	static size_t GridVolume(const std::vector<Rule>& rules, int numDims);
	static void GridSplit(const std::vector<Rule>& rules, int numDims, std::vector<Rule>& receivingList);

	static void SortableSplit(const std::vector<Rule>& rules, const std::vector<int>& dimOrder, std::vector<Rule>& receivingList, size_t dimIndex = 0);
private:
	int dim;
	std::vector<GRange> ranges;

	size_t BinarySearch(size_t lowIndex, size_t highIndex, unsigned int pt) const;

	static void RemoveConcealedRules(std::vector<Rule>& rules);
};

