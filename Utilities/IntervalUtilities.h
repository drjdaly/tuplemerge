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
#ifndef  UTIL_H
#define  UTIL_H
#include "../ElementaryClasses.h"


class WeightedInterval;
struct LightWeightedInterval;
class Utilities {
	static const int LOW = 0;
	static const int HIGH = 1;
public:
	static bool  IsIdentical(const Rule& r1, const Rule& r2);
	static int GetMaxOverlap(const std::multiset<unsigned int>& lo, const std::multiset< unsigned int>& hi);

	static std::pair<std::vector<int>, int> MWISIntervals(const std::vector<WeightedInterval>&I);
	static std::pair<std::vector<int>, int> FastMWISIntervals(const std::vector<LightWeightedInterval>&I);
	static std::pair<std::vector<int>, int> MWISIntervals(const std::vector<Rule>& I, int x);

	static std::vector<WeightedInterval> CreateUniqueInterval(const std::vector<Rule>& rules, int field);
	static std::vector<LightWeightedInterval> FastCreateUniqueInterval(const std::vector<interval>& rules);
	static std::vector<std::vector<WeightedInterval>> CreateUniqueIntervalsForEachField(const std::vector<Rule>& rules);
 
	static std::vector<Rule> RedundancyRemoval(const std::vector<Rule>& rules);

};
 
struct LightWeightedInterval {
	static const int LOW = 0;
	static const int HIGH = 1;
	LightWeightedInterval(unsigned int a, unsigned int b, int w) : a(a), b(b), w(w) {}
	unsigned int a;
	unsigned int b;
	int w;
	unsigned int GetLow() const { return a; }
	unsigned int GetHigh() const { return b; }
	int GetWeight() const { return w; }
	void Push(int x) {
		rule_indices.push_back(x);
	}
	std::vector<int> GetRuleIndices() const {
		return rule_indices;
	}
	std::vector<int> rule_indices;
};


class WeightedInterval{
	static const int LOW = 0;
	static const int HIGH = 1;
public:

	WeightedInterval(const std::vector<Rule>& rules, unsigned int a, unsigned int b) : rules(rules) {
		if (rules.size() == 0) {
			std::cout << "ERROR: EMPTY RULE AND CONSTRUCT INTERVAL?" << std::endl;
			exit(0);
		}
		ival = std::make_pair(a, b);
		SetWeightBySizePlusOne();
		field = 0;
	}
	WeightedInterval(const std::vector<Rule>& rules, int field) : rules(rules), field(field) {
		if (rules.size() == 0) {
			std::cout << "ERROR: EMPTY RULE AND CONSTRUCT INTERVAL?" << std::endl;
			exit(0);
		}
		ival = std::make_pair(rules[0].range[field][LOW], rules[0].range[field][HIGH]);
		SetWeightBySizePlusOne();
	}
	int CountPOM(int second_field) {
		std::multiset<unsigned int> Astart;
		std::multiset<unsigned int> Aend;
		for (int i = 0; i < (int)rules.size();
			 i++) {
			Astart.insert(rules[i].range[second_field][LOW]);
			Aend.insert(rules[i].range[second_field][HIGH]);
		}
		return Utilities::GetMaxOverlap(Astart, Aend);
	}
	void SetWeightByPenaltyPOM(int second_field) {
		weight = std::max((int)rules.size() - 100 * CountPOM(second_field), 1);
	}
	void SetWeightBySizePlusOne() {
		weight = rules.size() +1;
	}
	void SetWeightBySize() {
		weight = rules.size();
	}
	unsigned int GetLow() const{ return ival.first; }
	unsigned int GetHigh() const { return ival.second; }
	std::vector<Rule> GetRules() const{ return rules; }
	int GetField() const { return field; }
	int GetWeight() const { return weight; }
protected:
	std::pair<unsigned int, unsigned int> ival;
	int weight = 100000;
	std::vector<Rule> rules;
	int  field;
};

namespace SegmentUtil {
	void Print(const std::array<Point,2>& s);
	
	bool Intersects(const std::array<Point,2>& s, const std::array<Point,2>& t);
}


#endif
