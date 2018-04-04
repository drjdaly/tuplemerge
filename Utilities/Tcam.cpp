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
#include "Tcam.h"

#include <algorithm>

using namespace std;

namespace tcam {
	unsigned int PrefixSizeHelper(unsigned int low, unsigned int high, unsigned int boundLow, unsigned int boundHigh) {
		if (boundHigh < low || boundLow > high) return 0;
		if (low <= boundLow && high >= boundHigh) return 1;

		unsigned int boundMid = (boundLow / 2) + (boundHigh / 2); // Guard against overflow
		return PrefixSizeHelper(low, high, boundLow, boundMid) + PrefixSizeHelper(low, high, boundMid + 1, boundHigh);
	}

	unsigned int SizeAsPrefixes(const array<unsigned int, 2>& range) {
		unsigned int low = range[LowDim];
		unsigned int high = range[HighDim];
		return PrefixSizeHelper(low, high, 0, 0xFFFFFFFF);
	}

	unsigned int NumOfPrefixRules(const Rule& r) {
		unsigned int area = 1;
		for (const auto& range : r.range) {
			area *= SizeAsPrefixes(range);
		}
		return area;
	}
	unsigned int SizeAsPrefixRules(const vector<Rule>& rules) {
		unsigned int sum = 0;
		for (const Rule& r : rules) {
			sum += NumOfPrefixRules(r);
		}
		return sum;
	}
}
