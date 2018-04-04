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
#include "TupleMergeOffline.h"

using namespace std;
using namespace TupleMergeUtils;

// ************
// TupleMergeOffline
// ************
TupleMergeOffline::TupleMergeOffline(const unordered_map<string, string>& args) : TupleMergeOnline(args) {
	collideLimit = GetIntOrElse(args, "TM.Limit.Offline.Collide", collideLimit);
}

TupleMergeOffline::~TupleMergeOffline() {
	for (auto table : tables) {
		delete table;
	}
}

void TupleMergeOffline::ConstructClassifier(const vector<Rule>& rules) {
	this->rules = rules;
	
	vector<Rule> remain = rules;
	sort(remain.begin(), remain.end(), [](const Rule& x, const Rule& y) { return x.priority > y.priority; } );
	
	while (!remain.empty()) {
		remain = SelectTable(remain);
	}
	Resort();
}

vector<Rule> TupleMergeOffline::SelectTable(const vector<Rule>& rules) {
	Tuple bestTuple;
	size_t bestIndex = 0;
	size_t bestSize = 0;
	
	Tuple current;
	for (const Rule& r : rules) {
		Tuple t;
		PreferedTuple(r, t);
		bool hasChanged = false;
		if (current.empty()) {
			current = t;
			hasChanged = true;
		}
		for (size_t d = 0; d < t.size(); d++) {
			if (current[d] > t[d]) {
				current[d] = t[d];
				hasChanged = true;
			}
		}
		if (hasChanged) {
			size_t firstOut = rules.size();
			size_t size = 0;
			unordered_map<uint32_t, size_t> hashCounts;
			for (size_t i = 0; i < rules.size(); i++) {
				Tuple ti;
				PreferedTuple(rules[i], ti);
				if (CompatibilityCheck(ti, current)) {
					uint32_t hash = Hash(rules[i], current);
					hashCounts[hash]++;
					if (hashCounts[hash] > collideLimit) {
						firstOut = min(firstOut, i);
					} else {
						size++;
					}
				} else {
					firstOut = min(firstOut, i);
				}
				if (firstOut < bestIndex) {
					goto breakout;
				}
			}
			
			if (firstOut > bestIndex || size > bestSize) {
				bestTuple = current;
				bestIndex = firstOut;
				bestSize = size;
			}
		}
	}
breakout:
	
	SlottedTable* table = new SlottedTable(bestTuple);
	vector<Rule> remain;
	for (const Rule& r : rules) {
		Tuple tr;
		PreferedTuple(r, tr);
		if (table->CanInsert(tr)) {
			if (table->NumCollisions(r) < collideLimit) {
				bool ignore;
				table->Insertion(r, ignore);
				assignments[r.priority] = table;
			} else {
				remain.push_back(r);
			}
		} else {
			remain.push_back(r);
		}
	}
	tables.push_back(table);
	return remain;
}

void TupleMergeOffline::CombineTables() {
	for (auto i1 = tables.begin(); i1 != tables.end(); i1++) {
		for (auto i2 = i1 + 1; i2 != tables.end();) {
			if ((*i1)->CanTakeRulesFrom(*i2)) {
				// If all rules from i2 can go into i1, transfer them
				// Don't worry about collision limits
				// Then delete i2
				vector<Rule> rl = (*i2)->GetRules();
				for (Rule& r : rl) {
					bool ignore;
					(*i1)->Insertion(r, ignore);
					assignments[r.priority] = *i1;
				}
				delete (*i2);
				i2 = tables.erase(i2);
			} else {
				i2++;
			}
		}
	}
}
