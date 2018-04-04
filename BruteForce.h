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
#ifndef  BF_H
#define  BF_H

#include "Simulation.h"

class BruteForce : public PacketClassifier {
public:
	BruteForce(){}
	void ConstructClassifier(const std::vector<Rule>& r) {
		rules = r;
		rules.reserve(100000);
	}


 
	int MemoryAccess() const {
		return 0;
	}
	size_t NumTables() const {
		return 0;
	}
	size_t RulesInTable(size_t tableIndex) const{
		return 0;
	}


	int ClassifyAPacket(const Packet& one_packet) {
		
		int result = -1;
		auto IsPacketMatchToRule = [](const Packet& p, const Rule& r) {
			for (int i = 0; i < r.dim; i++) {
				if (p[i] < r.range[i][0]) return 0;
				if (p[i] > r.range[i][1]) return 0;
			}
			return 1;
		};
		
		for (size_t j = 0; j < rules.size(); j++) {
			if (IsPacketMatchToRule(one_packet, rules[j])) {
				result = std::max(rules[j].priority, result);
			}
		}
		return result;
	}

	void DeleteRule(size_t i) {
		if (i < 0 || i >= rules.size()) {
			printf("Warning index delete rule out of bound: do nothing here\n");
			printf("%lu vs. size: %lu\n", i, rules.size());
			return;
		}
		if (i != rules.size() -1)
		rules[i]=std::move(rules[rules.size() - 1]);
		rules.pop_back();
	}
	void InsertRule(const Rule& one_rule) {
	//	printf("%d inserted\n", one_rule.priority);
		rules.push_back(one_rule);
	}
	int Size() const {
		return 0;
	}
	std::vector<Rule> GetRules() const {
		return rules;
	}
private:
	std::vector<Rule> rules;

};

#endif
