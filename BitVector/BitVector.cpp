#include "BitVector.h"
#include "LongestPrefixMatch.h"
#include "EqnMatcher.h"

#include <set>

using namespace std;
using namespace TreeUtils;

int PrefixLength(Range s) {
	Point delta = s.high - s.low;
	int len = 0;
	while (delta) {
		delta >>= 1;
		len++;
	}
	return len;
}

BinaryRangeSearch::BinaryRangeSearch(vector<Range>& ranges) {
	set<Point> points;
	for (const Range & s : ranges) {
		points.insert(s.low);
		points.insert(s.high + 1);
	}
	points.erase(0);
	bool isFound = false;
	dividers.insert(dividers.end(), points.begin(), points.end());
	for (size_t j = 0; j < ranges.size(); j++) {
		if (ranges[j].ContainsPoint(0)) {
			indices.push_back(j);
			isFound = true;
			break;
		}
	}
	if (!isFound) {
		indices.push_back(ranges.size());
	}
	for (size_t i = 0; i < dividers.size(); i++) {
		isFound = false;
		for (size_t j = 0; j < ranges.size(); j++) {
			if (ranges[j].ContainsPoint(dividers[i])) {
				indices.push_back(j);
				isFound = true;
				break;
			}
		}
		if (!isFound) {
			indices.push_back(ranges.size());
		}
	}
}

size_t BinaryRangeSearch::Match(Point x) const {
	size_t index = Seek(x, 0, dividers.size());
	return indices[index];
}

size_t BinaryRangeSearch::Seek(Point x, size_t l, size_t r) const {
	//printf("%u %u\n", l, r);
	if (l == r) return l;
	size_t m = (l + r) / 2;
	if (x < dividers[m]) {
		return Seek(x, l, m);
	} else {
		return Seek(x, m + 1, r);
	}
}

// *********
// BitVector
// *********

BitVector::BitVector() {}


BitVector::~BitVector() {
	for (auto m : matchers) {
		delete m;
	}
}

void BitVector::ConstructClassifier(const std::vector<Rule>& rules) {
	this->rules.insert(this->rules.end(), rules.begin(), rules.end());
	SortRules(this->rules);

	size_t dMax = 2;// this->rules[0].range.size();
	for (size_t d = 0; d < dMax; d++) {
		fields.push_back(vector<BitSet>());

		set<Range, RangeComp> ranges;
		for (const Rule& rule : this->rules) {
			ranges.insert(V2R(rule.range[d]));
		}

		vector<Range> rangeVector(ranges.begin(), ranges.end());
		//matchers.push_back(new BinaryRangeSearch(rangeVector));
		matchers.push_back(new LongestPrefixMatch(rangeVector));
		//matchers.push_back(new EqnMatcher(rangeVector));

		for (const Range& s : rangeVector) {
			BitSet matches(this->rules.size());
			for (size_t i = 0; i < this->rules.size(); i++) {
				if (ContainsRange(V2R(this->rules[i].range[d]), s)) {
					matches.Set(i);
				}
			}
			fields[d].push_back(matches);
		}
	}
}

int BitVector::ClassifyAPacket(const Packet& packet) {
	BitSet sol(rules.size(), true);

	for (size_t i = 0; i < matchers.size(); i++) {
		size_t j = matchers[i]->Match(packet[i]);
		if (j < fields[i].size()) {
			sol &= fields[i][j];
			//fields[i][j].Print();
		} else {
			//printf("Early out\n");
			return -1;
		}
	}
	//sol.Print();

	for (size_t index = sol.FindFirst(); index < rules.size(); index = sol.FindNext(index)) {
		//printf("%u\n", index);
		if (rules[index].MatchesPacket(packet)) {
			//printf("Match!\n");
			return rules[index].priority;
		}
	}
	//printf("Failed\n");
	return -1;
}

// ***********
// BitVector64
// ***********

BitVector64::BitVector64() {}


BitVector64::~BitVector64() {
	for (auto m : matchers) {
		delete m;
	}
}

void BitVector64::ConstructClassifier(const std::vector<Rule>& rules) {
	this->rules.insert(this->rules.end(), rules.begin(), rules.end());
	SortRules(this->rules);

	size_t dMax = 2;// this->rules[0].range.size();
	for (size_t d = 0; d < dMax; d++) {
		fields.push_back(vector<BitSet64>());

		set<Range, RangeComp> ranges;
		for (const Rule& rule : this->rules) {
			ranges.insert(V2R(rule.range[d]));
		}

		vector<Range> rangeVector(ranges.begin(), ranges.end());
		//matchers.push_back(new BinaryRangeSearch(rangeVector));
		//matchers.push_back(new LongestPrefixMatch(rangeVector));
		matchers.push_back(new EqnMatcher(rangeVector));

		for (const Range& s : rangeVector) {
			BitSet64 matches;
			for (size_t i = 0; i < this->rules.size(); i++) {
				if (ContainsRange(V2R(this->rules[i].range[d]), s)) {
					matches.Set(i);
				}
			}
			fields[d].push_back(matches);
		}
	}
}

int BitVector64::ClassifyAPacket(const Packet& packet) {
	BitSet64 sol(true);

	for (size_t i = 0; i < matchers.size(); i++) {
		size_t j = matchers[i]->Match(packet[i]);
		if (j < fields[i].size()) {
			sol &= fields[i][j];
			//fields[i][j].Print();
		} else {
			//printf("Early out\n");
			return -1;
		}
	}
	//sol.Print();

	for (size_t index = sol.FindFirst(); index < rules.size(); index = sol.FindNext(index)) {
		//printf("%u\n", index);
		if (rules[index].MatchesPacket(packet)) {
			//printf("Match!\n");
			return rules[index].priority;
		}
	}
	//printf("Failed\n");
	return -1;
}