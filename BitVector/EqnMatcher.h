#pragma once

#include "BitVector.h"

struct EqnTerm {
	Point mask;
	Point xored;
	size_t boost;
};

class EqnMatcher : public FieldMatcher {
public:
	EqnMatcher(std::vector<Range>& ranges);
	~EqnMatcher();

	virtual size_t Match(Point x) const;

private:
	std::vector<EqnTerm> terms;
};

