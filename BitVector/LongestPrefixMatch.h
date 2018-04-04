#pragma once

#include "BitVector.h"

class LongestPrefixMatch : public FieldMatcher {
public:
	LongestPrefixMatch(std::vector<Range>& ranges);
	~LongestPrefixMatch();

	size_t Match(Point x) const;
private:
	std::map<int, std::unordered_map<Point, size_t>> table; // Length -> Low Bound -> Index
};

