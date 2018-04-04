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
#ifndef  LIS_H
#define  LIS_H

#include <vector>

using namespace std; 
/* Finds longest strictly increasing subsequence. O(n log k) algorithm. */
//CREDIT: http://www.algorithmist.com/index.php/Longest_Increasing_Subsequence.cpp
//Date obtained: 6/8/2016

pair<vector<int>, vector<int>> lis(const vector<int> &a);

/*
#include <cstdio>
int main()
{
	int a[] = { 1, 9, 3, 8, 11, 4, 5, 6, 4, 19, 7, 1, 7 };
	vector<int> seq(a, a + sizeof(a) / sizeof(a[0])); // seq : Input Vector
	vector<int> lis;                              // lis : Vector containing indexes of longest subsequence 
	find_lis(seq, lis);

	//Printing actual output 
	for (size_t i = 0; i < lis.size(); i++)
		printf("%d ", seq[lis[i]]);
	printf("\n");

	return 0;
}
*/

#endif
