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
#include "TupleMergeOnline.h"

using namespace std;
using namespace ForgeUtils;
using namespace TupleMergeUtils;

bool AreEqual(Tuple t1, Tuple t2) {
	for (size_t d = 0; d < t1.size(); d++) {
		if (t1[d] != t2[d]) return false;
	}
	return true;
}

int Delta(Tuple t1, Tuple t2) {
	return Sum(t1) - Sum(t2);
}

void ForgeUtils::Crazify(Tuple& tuple) {
	int delta = abs(tuple[FieldSA] - tuple[FieldDA]);
	
	if (tuple[FieldSA] > tuple[FieldDA] + 4) {
		tuple[FieldDA] = 0;
		tuple[FieldDP] = 16;
	}
	else if (tuple[FieldDA] > tuple[FieldSA] + 4) {
		tuple[FieldSA] = 0;
		tuple[FieldSP] = 16;
	}
	
	
	if (tuple[FieldSA] == 32) tuple[FieldSA] -= 4;
	else if (tuple[FieldSA] > 24) tuple[FieldSA] -= 3;
	else if (tuple[FieldSA] > 16) tuple[FieldSA] -= 2;
	else if (tuple[FieldSA] > 8) tuple[FieldSA] -= 1;
	
	if (tuple[FieldDA] == 32) tuple[FieldDA] -= 4;
	else if (tuple[FieldDA] > 24) tuple[FieldDA] -= 3;
	else if (tuple[FieldDA] > 16) tuple[FieldDA] -= 2;
	else if (tuple[FieldDA] > 8) tuple[FieldDA] -= 1;

}

size_t CollisionsForTuple(const std::vector<Rule>& rules, const Tuple& tuple) {
	size_t maxCollide = 0;
	unordered_map<uint32_t, size_t> hashCollisions;
	for (const Rule& r : rules) {
		uint32_t hash = Hash(r, tuple);
		hashCollisions[hash]++;
		maxCollide = max(hashCollisions[hash], maxCollide);
	}
	return maxCollide;
}

// ************
// TupleMergeOnline
// ************

TupleMergeOnline::TupleMergeOnline(const std::unordered_map<std::string, std::string>& args) 
	: collideLimit(GetIntOrElse(args, "TM.Limit.Collide", 10)) {
}

TupleMergeOnline::~TupleMergeOnline() {
	for (auto t : tables) {
		delete t;
	}
}

void TupleMergeOnline::ConstructClassifier(const std::vector<Rule>& rules) {
	for (const Rule& r : rules) {
		InsertRule(r);
	}
}

int TupleMergeOnline::ClassifyAPacket(const Packet& p) {
	int prior = -1;
	int q = 0;
	for (auto & t : tables) {
		if (t->MaxPriority() > prior) {
			prior = max(prior, t->ClassifyAPacket(p));
			q++;
		}
	}
	QueryUpdate(q);
	return prior;
}

void TupleMergeOnline::DeleteRule(size_t index){
	Rule r = rules[index];
	rules[index] = rules[rules.size() - 1];
	rules.pop_back();

	SlottedTable* tbl = assignments[r.priority];
	assignments.erase(r.priority);

	bool hasChanged;
	tbl->Deletion(r, hasChanged);

	if (tbl->IsEmpty()) {
		tables.erase(find(tables.begin(), tables.end(), tbl), tables.end());
		delete tbl;
	}

	if (hasChanged) {
		Resort();
	}
}

void Relax(Tuple& tuple) {
	const int deltaThreshold = 4;
	int delta = tuple[FieldSA] - tuple[FieldDA];
	if (-delta > deltaThreshold) {
		tuple[FieldSA] = 0;
		tuple[FieldSP] = 16;
	} else if (delta > deltaThreshold) {
		tuple[FieldDA] = 0;
		tuple[FieldDP] = 16;
	}
	
	if (tuple[FieldSA] == 32) tuple[FieldSA] -= 4;
	else if (tuple[FieldSA] > 24) tuple[FieldSA] -= 3;
	else if (tuple[FieldSA] > 16) tuple[FieldSA] -= 2;
	else if (tuple[FieldSA] > 8) tuple[FieldSA] -= 1;
	
	if (tuple[FieldDA] == 32) tuple[FieldDA] -= 4;
	else if (tuple[FieldDA] > 24) tuple[FieldDA] -= 3;
	else if (tuple[FieldDA] > 16) tuple[FieldDA] -= 2;
	else if (tuple[FieldDA] > 8) tuple[FieldDA] -= 1;
}

void TupleMergeOnline::InsertRule(const Rule& rule) {
	rules.push_back(rule);
	Tuple tuple;
	PreferedTuple(rule, tuple);
	
	for (auto table : tables) {
		if (table->CanInsert(tuple)) {
			bool hasChanged = false;
			table->Insertion(rule, hasChanged);
			assignments[rule.priority] = table;
			
			if (table->NumCollisions(rule) > collideLimit) {
				// Split Table
				vector<Rule> collisions = table->Collisions(rule);
				Tuple compatTuple;
				BestTuple(collisions, compatTuple);
				Tuple superTuple = compatTuple;
				for (const Rule& r : collisions) {
					Tuple t;
					PreferedTuple(r, t);
					for (size_t d = 0; d < tuple.size(); d++) {
						superTuple[d] = max(superTuple[d], t[d]);
					}
				}
				size_t bestD = 0;
				int bestDelta = 0;
				for (size_t d = 0; d < tuple.size(); d++) {
					int delta = superTuple[d] - compatTuple[d];
					if (delta > bestDelta) {
						bestD = d;
						bestDelta = delta;
					}
				}
				compatTuple[bestD] = (superTuple[bestD] + compatTuple[bestD]) / 2;
				SlottedTable* target = FindOrMake(compatTuple);
				
				vector<Rule> rl = table->GetRules();
				for (Rule& r : rl) {
					Tuple t;
					PreferedTuple(r, t);
					if (target->CanInsert(t)) {
						table->Deletion(r, hasChanged);
						target->Insertion(r, hasChanged);
						assignments[r.priority] = target;
					}
				}
			}
			
			if (hasChanged) {
				Resort();
			}
			return;
		}
	}
	// Could not insert
	// So create a new table
	{
		bool ignore;
		Relax(tuple);
		SlottedTable * table = new SlottedTable(tuple);
		table->Insertion(rule, ignore);
		tables.push_back(table);
		assignments[rule.priority] = table;
		Resort();
	}
}

SlottedTable* TupleMergeOnline::FindOrMake(const Tuple& t) {
	for (auto table : tables) {
		if (table->IsThatTuple(t)) {
			return table;
		}
	}
	SlottedTable* table = new SlottedTable(t);
	tables.push_back(table);
	return table;
}
