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
#pragma once

#include "../Simulation.h"

#include "SlottedTable.h"

namespace ForgeUtils {
	void Crazify(TupleMergeUtils::Tuple& tuple);
}

class TupleMergeOnline : public PacketClassifier {
public:
	TupleMergeOnline(const std::unordered_map<std::string, std::string>& args);
	~TupleMergeOnline();
	
	virtual void ConstructClassifier(const std::vector<Rule>& rules);
	virtual int ClassifyAPacket(const Packet& p);
	virtual void DeleteRule(size_t index);
	virtual void InsertRule(const Rule& r);
	virtual Memory MemSizeBytes() const {
		int ruleSizeBytes = 19; // TODO variables sizes
		int sizeBytes = 0;
		for (const auto table : tables) {
			sizeBytes += table->MemSizeBytes(ruleSizeBytes);
		}
		int assignmentsSizeBytes = rules.size() * POINTER_SIZE_BYTES;
		int arraySize = tables.size() * POINTER_SIZE_BYTES;
		return sizeBytes + assignmentsSizeBytes + arraySize;
	}
	virtual int MemoryAccess() const {
		int cost = 0;
		/*
		for (const auto t : tables) {
			cost += t->MemoryAccess() + 1;
		}*/
		return cost;
	}
	virtual size_t NumTables() const { return tables.size(); }
	virtual size_t RulesInTable(size_t index) const { return tables[index]->NumRules(); }
	virtual size_t PriorityOfTable(size_t index) const {
		return tables[index]->MaxPriority();
	}

protected:
	void Resort() {
		sort(tables.begin(), tables.end(), [](auto& tx, auto& ty) { return tx->MaxPriority() > ty->MaxPriority(); });
	}
	SlottedTable* FindOrMake(const TupleMergeUtils::Tuple& t);
	
	std::vector<SlottedTable*> tables;
	std::unordered_map<int, SlottedTable*> assignments; // Priority -> Table

	std::vector<Rule> rules;

	int collideLimit;
};



