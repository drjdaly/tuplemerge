/*
 * MIT License
 *
 * Copyright (c) 2016 by S. Yingchareonthawornchai at Michigan State University
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
#include "LongestIncreasingSubsequence.h"

pair<vector<int>,vector<int>> lis(const vector<int> &a)
{
	vector<int> b;
	vector<int> p(a.size());
	int u, v;

	if (a.empty()) return make_pair(b,b);

	b.push_back(0);

	for (size_t i = 1; i < a.size(); i++)
	{
		// If next element a[i] is greater than last element of
		// current longest subsequence a[b.back()], just push it at back of "b" and continue
		if (a[b.back()] < a[i])
		{
			p[i] = b.back();
			b.push_back(i);
			continue;
		}

		// Binary search to find the smallest element referenced by b which is just bigger than a[i]
		// Note : Binary search is performed on b (and not a).
		// Size of b is always <=k and hence contributes O(log k) to complexity.    
		for (u = 0, v = b.size() - 1; u < v;)
		{
			int c = (u + v) / 2;
			if (a[b[c]] < a[i]) u = c + 1; else v = c;
		}

		// Update b if new value is smaller then previously referenced value 
		if (a[i] < a[b[u]])
		{
			if (u > 0) p[i] = b[u - 1];
			b[u] = i;
		}
	}

	for (u = b.size(), v = b.back(); u--; v = p[v]) b[u] = v;
	vector<int> out;
	for (int i = 0; i < b.size(); i++) {
		out.push_back(a[b[i]]);
	}

	return make_pair(out, b);
}
