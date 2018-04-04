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
#include "OptimizedMITree.h"

/*
std::vector<box> OptimizedMITree::RangeToBoxes(const std::vector<std::array<unsigned, 2>>& range, int key_size) const  {
	
	std::vector<box> vb; 
	
	for (int i = range.size() - 1; i >= 0; i -= key_size) {
		box b1;
		for (int j = 0; j < key_size; j++) {
			if (i - key_size + 1 + j >= 0)
				b1[j] = range[field_order[i - key_size + 1 + j]];
			else
				b1[j] = { { 0, 0 } };
		}
		vb.insert(begin(vb), b1);
	}

	//PRINT BOXes
	/*for (auto& b : vb) {
	printf("[[%u %u]] ", b);
	}
	printf("\n");
	return vb;

}*/
/*
std::vector<query> OptimizedMITree::PacketToQueries(const Packet& p, int key_size) const {
	std::vector<query> vq;
	for (int i=0;i<p.size();i++) {
		query x;
		x[0] = p[field_order[i]];
		vq.push_back(x);
	}
	for (int i = p.size() - 1; i >= 0; i -= key_size) {
		query b1;
		for (int j = 0; j < key_size; j++) {
			if (i - key_size + 1 + j >= 0)
				b1[j] = p[field_order[i - key_size + 1 + j]];
			else
				b1[j] = 0;
		}
		vq.insert(begin(vq), b1);
	}
	return vq;

}*/
