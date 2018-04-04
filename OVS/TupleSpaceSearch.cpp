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
#include "TupleSpaceSearch.h"
#include "../TupleMerge/SlottedTable.h"

// Values for Bernstein Hash
#define HashBasis 5381
#define HashMult 33

void TupleTable::Insertion(const Rule& r) {

	cmap_node * new_node = new cmap_node(r); /*key & rule*/
	cmap_insert(&map_in_tuple, new_node, HashRule(r));

	/*uint32_t key = HashRule(r);
	std::vector<Rule>& rl = table[key];
	rl.push_back(r);
	sort(rl.begin(), rl.end(), [](const Rule& rx, const Rule& ry) { return rx.priority >= ry.priority; });*/
}

void TupleTable::Deletion(const Rule& r) {
	//find node containing the rule
	unsigned int hash_r = HashRule(r);
	cmap_node * found_node = cmap_find(&map_in_tuple, hash_r);
	while (found_node != nullptr) {
		if (found_node->priority == r.priority) {
			cmap_remove(&map_in_tuple, found_node, hash_r);
			break;
		}
		found_node = found_node->next;
	}

	/*uint32_t key = HashRule(r);
	std::vector<Rule>& rl = table[key];
	rl.erase(std::remove_if(rl.begin(), rl.end(), [=](const Rule& ri) { return ri.priority == r.priority; }), rl.end());*/
}

int TupleTable::WorstAccesses() const {
	// TODO
	return 1;//cmap_largest_chain(&map_in_tuple);
}

int TupleTable::ClassifyAPacket(const Packet& p)  {
	
	cmap_node * found_node = cmap_find(&map_in_tuple, HashPacket(p));
	int priority = -1;
	while (found_node != nullptr) {
		if (found_node->rule_ptr->MatchesPacket(p)) {
			priority = std::max(priority, found_node->priority);
		}
		found_node = found_node->next;
	}
	return priority;

/*	auto ptr = table.find(HashPacket(p));
	if (ptr != table.end()) {
		for (const Rule& r : ptr->second) {
			if (r.MatchesPacket(p)) {
				return r.priority;
			}
		}
	}
	return -1;*/
}

bool inline TupleTable::IsPacketMatchToRule(const Packet& p, const Rule& r) {
	for (int i = 0; i < r.dim; i++) {
		if (p[i] < r.range[i][LowDim]) return false;
		if (p[i] > r.range[i][HighDim]) return false;
	}
	return true;
}

uint32_t inline TupleTable::HashRule(const Rule& r) const {
	uint32_t hash = 0;
	for (size_t i = 0; i < dims.size(); i++) {
		hash = hash_add(hash, r.range[dims[i]][LowDim] & TupleMergeUtils::Mask(tuple[dims[i]]));
	}
	return hash_finish(hash, 16);

/*	uint32_t hash = HashBasis;
	for (size_t d = 0; d < tuple.size(); d++) {
		hash *= HashMult;
		hash += r.range[d][LowDim] & ForgeUtils::Mask(tuple[d]);
	}
	return hash;*/

}

uint32_t inline TupleTable::HashPacket(const Packet& p) const {
	uint32_t hash = 0;
	uint32_t max_uint = 0xFFFFFFFF;

	for (size_t i = 0; i < dims.size(); i++) {
		uint32_t mask = lengths[i] != 32 ? ~(max_uint >> lengths[i]) : max_uint;
		hash = hash_add(hash, p[dims[i]] & mask);
	}
	return hash_finish(hash, 16);

	/*uint32_t hash = HashBasis;
	for (size_t d = 0; d < tuple.size(); d++) {
		hash *= HashMult;
		hash += p[d] & ForgeUtils::Mask(tuple[d]);
	}
	return hash;*/
}

void TupleSpaceSearch::ConstructClassifier(const std::vector<Rule>& r){
	/*int multiples = 5;
	for (int i = 0; i < r[0].dim / multiples; i++) {
		dims.push_back(i * multiples);
		dims.push_back(i * multiples + 1);
	}*/
	dims.push_back(FieldSA);
	dims.push_back(FieldDA);

	for (const auto& Rule : r) {
		InsertRule(Rule);
	}

	rules = r;
	rules.reserve(100000);
}
int TupleSpaceSearch::ClassifyAPacket(const Packet& packet) {
	int priority = -1;
	int query = 0;
	for (auto& tuple : all_tuples) {
		auto result = tuple.second.ClassifyAPacket(packet);
		priority = std::max(priority, result);
		query++;
	}
	QueryUpdate(query);
	return priority;
}
void TupleSpaceSearch::DeleteRule(size_t i) {

	if (i < 0 || i >= rules.size()) {
		printf("Warning index delete rule out of bound: do nothing here\n");
		printf("%lu vs. size: %lu", i, rules.size());
		return;
	}

	auto hit = all_tuples.find(KeyRulePrefix(rules[i]));
	if (hit != end(all_tuples)) {
		//there is a tuple
		hit->second.Deletion(rules[i]);
		if (hit->second.IsEmpty()) {
			//destroy tuple and erase from the map
			hit->second.Destroy();
			all_tuples.erase(hit);
		}
	} else {
		//nothing to do?
		printf("Warning DeleteRule: no matching tuple in the rule; it should be here when inserted\n");
		exit(0);
	}
	if (i != rules.size() - 1)
		rules[i] = std::move(rules[rules.size() - 1]);
	rules.pop_back();
}
void TupleSpaceSearch::InsertRule(const Rule& rule) {
	auto hit = all_tuples.find(KeyRulePrefix(rule));
	if (hit != end(all_tuples)) {
		//there is a tuple
		hit->second.Insertion(rule);
	} else {
		//create_tuple
		std::vector<unsigned int> lengths;
		for (int d : dims) {
			lengths.push_back(rule.prefix_length[d]);
		}
		all_tuples.insert(std::make_pair(KeyRulePrefix(rule), TupleTable(dims, lengths, rule)));
	}
	rules.push_back(rule);
}

int TupleSpaceSearch::WorstAccesses() const {
	int cost = 0;
	for (auto pair : all_tuples) {
		cost += pair.second.WorstAccesses() + 1;
	}
	return cost;
}

void PriorityTuple::Insertion(const Rule& r, bool& priority_change) {

	if (r.priority > maxPriority) {
		maxPriority = r.priority;
		priority_change = true;
	}
	priority_container.insert(r.priority);
	TupleTable::Insertion(r);
}
void  PriorityTuple::Deletion(const Rule& r, bool& priority_change) {

	auto pit = priority_container.equal_range(r.priority);
	priority_container.erase(pit.first);
	if (priority_container.size() == 0)  {
		maxPriority = -1;
		priority_change = true;
	} else if (r.priority == maxPriority) {
		maxPriority = *priority_container.rbegin();
		priority_change = true;
	} else priority_change = false;
	TupleTable::Deletion(r);
}


int PriorityTupleSpaceSearch::ClassifyAPacket(const Packet& packet) {
	int priority = -1;
	int q = 0;
	for (auto& tuple : priority_tuples_vector) {
		//if (tuple->maxPriority < 0) printf("priority %d\n", tuple->maxPriority);
		if (priority > tuple->maxPriority) break;
		auto result = tuple->ClassifyAPacket(packet);
		q++;
		priority = priority > result ? priority : result;
	}
	QueryUpdate(q);
	return priority;
}
void PriorityTupleSpaceSearch::DeleteRule(size_t i) {
	if (i < 0 || i >= rules.size()) {
		printf("Warning index delete rule out of bound: do nothing here\n");
		printf("%lu vs. size: %lu", i, rules.size());
		return;
	}
	bool priority_change = false;

	auto hit = all_priority_tuples.find(KeyRulePrefix(rules[i]));

	if (hit != end(all_priority_tuples)) {
		//there is a tuple
		hit->second->Deletion(rules[i], priority_change);
		if (hit->second->IsEmpty()) {
			//destroy tuple and erase from the map
			
			all_priority_tuples.erase(hit);
			hit->second->Destroy();
			RetainInvaraintOfPriorityVector();
			priority_tuples_vector.pop_back();

		} else if (priority_change) {
			//sort tuple again
			RetainInvaraintOfPriorityVector();
		}

	} else {
		//nothing to do?
		printf("Warning DeleteRule: no matching tuple in the rule; it should be here when inserted\n");
		exit(0);
	}
	if (i != rules.size() - 1)
		rules[i] = std::move(rules[rules.size() - 1]);
	rules.pop_back();

}
void PriorityTupleSpaceSearch::InsertRule(const Rule& rule) {
	bool priority_change = false;
	auto hit = all_priority_tuples.find(KeyRulePrefix(rule));
	if (hit != end(all_priority_tuples)) {
		//there is a tuple
		hit->second->Insertion(rule, priority_change);
		if (priority_change) {
			RetainInvaraintOfPriorityVector();
		}
	} else {
		//create_tuple
		std::vector<unsigned int> lengths;
		for (int d : dims) {
			lengths.push_back(rule.prefix_length[d]);
		}
		auto ptuple = new PriorityTuple(dims, lengths, rule);
		all_priority_tuples.insert(std::make_pair(KeyRulePrefix(rule), ptuple));
		// add to priority vector
		priority_tuples_vector.push_back(ptuple);
		RetainInvaraintOfPriorityVector();
	}
	rules.push_back(rule);
}


int PriorityTupleSpaceSearch::WorstAccesses() const {
	int cost = 0;
	for (const PriorityTuple* t : priority_tuples_vector) {
		cost += t->WorstAccesses() + 1;
	}
	return cost;
}
