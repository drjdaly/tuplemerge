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
#include "SlottedTable.h"

using namespace TupleMergeUtils;
using namespace std;

// Values for Bernstein Hash
#define HashBasis 5381
#define HashMult 33

namespace TupleMergeUtils {
	int LeastSignificantBit(int x) {
		int c = 32;
		x &= -x;
		if (x) c--;
		if (x & 0x0000FFFF) c -= 16;
		if (x & 0x00FF00FF) c -= 8;
		if (x & 0x0F0F0F0F) c -= 4;
		if (x & 0x33333333) c -= 2;
		if (x & 0x55555555) c -= 1;
		return c;
	}

	int CountSetBits(int x) {
		unsigned int c = 0;
		while (x) {
			x &= (x - 1);
			c++;
		}
		return c;
	}

	bool CompatibilityCheck(const Tuple& ruleTuple, const Tuple& tableTuple) {
		for (size_t i = 0; i < ruleTuple.size(); i++) {
			if (ruleTuple[i] < tableTuple[i]) return false;
		}
		return true;
	}
	
	bool AreSame(const Tuple& t1, const Tuple& t2) {
		for (size_t i = 0; i < t1.size(); i++) {
			if (t1[i] != t2[i]) return false;
		}
		return true;
	}

	int Sum(const vector<int>& v) {
		return std::accumulate(v.begin(), v.end(), 0);
	}

	int Log2(unsigned int x) {
		int lg = 0;
		for (; x; x >>= 1) lg++;
		return lg;
	}

	void PreferedTuple(const Rule& r, Tuple& tuple) {
		for (int i = 0; i < r.dim; i++) {
			tuple.push_back(r.prefix_length[i]);
		}
	}

	void BestTuple(const vector<Rule>& rules, Tuple& tuple) {
		for (const Rule& r : rules) {
			Tuple t;
			PreferedTuple(r, t);
			if (t.size() > tuple.size()) {
				tuple = t;
			} else {
				for (size_t i = 0; i < tuple.size(); i++) {
					if (tuple[i] > t[i]) {
						tuple[i] = t[i];
					}
				}
			}
		}
	}

	uint32_t Mask(int bits) {
		// Shifting is undefined if bits > 32
		// We want to make sure that for bits = 32 we get the correct result
		if (bits == 32) return 0xFFFFFFFFu;
		else return ~(0xFFFFFFFFu >> bits);
	}
	
	inline uint32_t Shift(uint32_t word, int t) {
		if (t == 0) return 0u;
		else return word >> (32 - t);
	}
	
	inline vector<uint32_t> RuleToBytes(const Rule& r, const Tuple& tuple) {
		vector<uint32_t> result;
		for (size_t index = 0; index < tuple.size(); index++) {
			int t = tuple[index];
			int p = r.range[index][LowDim] & Mask(tuple[index]);
			result.push_back(p >> 24);
			result.push_back((p >> 16) & 0xFF);
			result.push_back((p >> 8) & 0xFF);
			result.push_back(p & 0xFF);
		}
		return result;
	}
	
	inline vector<uint32_t> PacketToBytes(const Packet& packet, const Tuple& tuple) {
		vector<uint32_t> result;
		for (size_t index = 0; index < tuple.size(); index++) {
			int t = tuple[index];
			int p = packet[index] & Mask(tuple[index]);
			result.push_back(p >> 24);
			result.push_back((p >> 16) & 0xFF);
			result.push_back((p >> 8) & 0xFF);
			result.push_back(p & 0xFF);
		}
		return result;
	}
	
	inline uint32_t HashMultiplyAndAdd(const vector<uint32_t>& v) {
		uint32_t hash = HashBasis;
		for (size_t d = 0; d < v.size(); d++) {
			hash *= HashMult;
			hash += v[d];
		}
		return hash;
	}

	inline uint32_t HashKeyMultiplyAndAdd(const vector<unsigned int>& packet, const Tuple& tuple) {
		uint32_t hash = HashBasis;
		for (size_t d = 0; d < tuple.size(); d++) {
			hash *= HashMult;
			hash += packet[d] & Mask(tuple[d]);
		}
		return hash;
	}

	inline uint32_t HashRuleMultiplyAndAdd(const Rule& r, const Tuple& tuple) {
		uint32_t hash = HashBasis;

		for (size_t d = 0; d < tuple.size(); d++) {
			hash *= HashMult;
			hash += r.range[d][LowDim] & Mask(tuple[d]);
		}
		return hash;
	}

	inline uint32_t HashPacketElf(const vector<unsigned int>& packet, const Tuple& tuple) {
		uint32_t hash = 0;
		for (size_t d = 0; d < tuple.size(); d++) {
			hash <<= 4;
			hash += packet[d] & Mask(tuple[d]);
			uint32_t high = hash & 0xf0000000u;
			hash ^= (high >> 24);
			hash &= ~high;
		}
		return hash;
	}

	inline uint32_t HashRuleElf(const Rule& r, const Tuple& tuple) {
		uint32_t hash = 0;
		for (size_t d = 0; d < tuple.size(); d++) {
			hash <<= 4;
			hash += r.range[d][LowDim] & Mask(tuple[d]);
			uint32_t high = hash & 0xf0000000u;
			hash ^= (high >> 24);
			hash &= ~high;
		}
		return hash;
	}

	inline uint32_t HashKnuthPacket(const vector<unsigned int>& packet, const Tuple& tuple) {
		uint32_t hash = 0;
		for (size_t d = 0; d < tuple.size(); d++) {
			hash = (hash << 5) ^ (hash >> 27);
			hash ^= packet[d] & Mask(tuple[d]);
		}
		return hash;
	}

	inline uint32_t HashKnuthRule(const Rule& r, const Tuple& tuple) {
		uint32_t hash = 0;
		for (size_t d = 0; d < tuple.size(); d++) {
			hash = (hash << 5) ^ (hash >> 27);
			hash ^= r.range[d][LowDim] & Mask(tuple[d]);
		}
		return hash;
	}

	uint32_t Hash(const Rule& r, const Tuple& tuple) {
		return HashRuleMultiplyAndAdd(r, tuple);
	}
	
	uint32_t Hash(const Packet& p, const Tuple& tuple) {
		return HashKeyMultiplyAndAdd(p, tuple);
	}
	
	bool IsHashable(const vector<Rule>& rl, size_t collisionLimit) {
		vector<int> tuple;
		BestTuple(rl, tuple);
		unordered_map<uint32_t, int> counts;
		for (const Rule& r : rl) {
			uint32_t h = Hash(r, tuple);
			if (counts[h] < collisionLimit) {
				counts[h]++;
			} else {
				return false;
			}
		}
		return true;
	}

	void PrintTuple(const Tuple& tuple) {
		for (size_t d = 0; d < tuple.size(); d++) {
			printf("%d ", tuple[d]);
		}
		printf("\n");
	}

	void PrintHashKey(const Rule& r, const Tuple& tuple) {
		for (size_t d = 0; d < tuple.size(); d++) {
			printf("%u ", r.range[d][LowDim] & Mask(tuple[d]));
			printf("%lu\n", d);
		}
		printf("-> %u \n", Hash(r, tuple));
		for (size_t d = 0; d < tuple.size(); d++) {
			printf("[%d %u] ", tuple[d], Mask(tuple[d]));
		}
		printf("\n");
	}

}

inline Tuple AddressOfRule(const Rule& r) {
	return {r.prefix_length[0], r.prefix_length[1]};
}

inline uint32_t HashAddress(const Tuple& t) {
	return (t[0] << 6) + t[1];
}

inline uint32_t HashTuple(const Tuple& t) {
	uint32_t hash = 0;
	for (auto i : t) {
		hash <<= 6;
		hash += i;
	}
	return hash;
}

inline uint32_t HashRuleAddress(const Rule& r) {
	return (r.prefix_length[0] << 6) + r.prefix_length[1];
}

inline vector<int> Dimify(const Tuple& t) {
	vector<int> sol;
	if (t[FieldSA] > 0) sol.push_back(FieldSA);
	if (t[FieldDA] > 0) sol.push_back(FieldDA);
	if (t[FieldSP] > 16) sol.push_back(FieldSP);
	if (t[FieldDP] > 16) sol.push_back(FieldDP);
	if (t[FieldProto] > 24) sol.push_back(FieldProto);
	return sol;
}

inline vector<unsigned int> Lengthify(const Tuple& t, const vector<int> dims) {
	vector<unsigned int> sol;
	for (int i : dims) { 
		sol.push_back(t[i]);
	}
	return sol;
}

// ************
// SlottedTable
// ************

SlottedTable::SlottedTable(const Tuple& tuple) 
	: dims(Dimify(tuple)), lengths(Lengthify(tuple, dims))
{
	cmap_init(&map_in_tuple);
}

int SlottedTable::WorstAccesses() const {
	// TODO
	return 1;//cmap_largest_chain(&map_in_tuple);
}

int SlottedTable::ClassifyAPacket(const Packet& p) const {
	
	cmap_node * found_node = cmap_find(&map_in_tuple, HashPacket(p));
	int priority = -1;
	while (found_node != nullptr) {
		if (found_node->rule_ptr->MatchesPacket(p)) {
			priority = std::max(priority, found_node->priority);
		}
		found_node = found_node->next;
	}
	return priority;
}

bool SlottedTable::IsThatTuple(const Tuple& tuple) const {
	auto td = Dimify(tuple);
	if (td == dims) {
		auto ld = Lengthify(tuple, td);
		return ld == lengths;
	} else {
		return false;
	}
}

bool SlottedTable::CanTakeRulesFrom(const SlottedTable* table) const {
	for (size_t i = 0; i < dims.size(); i++) {
		int dim = dims[i];
		size_t j;
		for (j = 0; j < table->dims.size(); j++) {
			if (table->dims[j] == dim) break;
		}
		if (j == table->dims.size()) return false;
		if (lengths[i] > table->lengths[j]) return false;
	}
	return true;
}

bool SlottedTable::HaveSameTuple(const SlottedTable* table) const {
	return dims == table->dims && lengths == table->lengths;
}

size_t SlottedTable::NumCollisions(const Rule& r) const {
	size_t collide = 0;
	cmap_node * node = cmap_find(&map_in_tuple, HashRule(r));
	while (node != nullptr) {
		collide++;
		node = node->next;
	}
	return collide;
}

vector<Rule> SlottedTable::Collisions(const Rule& r) const {
	vector<Rule> rules;
	cmap_node * node = cmap_find(&map_in_tuple, HashRule(r));
	while (node != nullptr) {
		rules.push_back(*node->rule_ptr);
		node = node->next;
	}
	return rules;
}

vector<Rule> SlottedTable::GetRules() const {
	vector<Rule> rules;
	cmap_cursor cursor = cmap_cursor_start(&map_in_tuple);
	
	while (cursor.node != nullptr) {
		rules.push_back(*cursor.node->rule_ptr);
		cmap_cursor_advance(&cursor);
	}
	
	return rules;
}

uint32_t inline SlottedTable::HashRule(const Rule& r) const {
	uint32_t hash = HashBasis;
	for (size_t i = 0; i < dims.size(); i++) {
		hash *= HashMult;
		hash += r.range[dims[i]][LowDim] & TupleMergeUtils::Mask(lengths[i]);
	}
	return hash;

}

uint32_t inline SlottedTable::HashPacket(const Packet& p) const {
	uint32_t hash = HashBasis;
	for (size_t i = 0; i < dims.size(); i++) {
		hash *= HashMult;
		hash += p[dims[i]] & TupleMergeUtils::Mask(lengths[i]);
	}
	return hash;
}

void SlottedTable::Insertion(const Rule& r, bool& priority_change) {
	cmap_node * new_node = new cmap_node(r);
	cmap_insert(&map_in_tuple, new_node, HashRule(r));

	priority_container.insert(r.priority);
	if (r.priority > maxPriority) {
		maxPriority = r.priority;
		priority_change = true;
	}
	
}
bool SlottedTable::Deletion(const Rule& r, bool& priority_change) {
	auto pit = priority_container.equal_range(r.priority);
	if (pit.first != pit.second) {
		unsigned int hash_r = HashRule(r);
		cmap_node * found_node = cmap_find(&map_in_tuple, hash_r);
		while (found_node != nullptr) {
			if (found_node->priority == r.priority) {
				cmap_remove(&map_in_tuple, found_node, hash_r);
				break;
			}
			found_node = found_node->next;
		}
		priority_container.erase(pit.first);
		if (priority_container.size() == 0)  {
			maxPriority = -1;
			priority_change = true;
		} else if (r.priority == maxPriority) {
			maxPriority = *priority_container.rbegin();
			priority_change = true;
		} //else priority_change = false;
		return true;
	} else {
		return false;
	}
}

