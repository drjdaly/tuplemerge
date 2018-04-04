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
 
#ifndef  SIMULATION_H
#define  SIMULATION_H

#include "ElementaryClasses.h"
#include "Utilities/MapExtensions.h"

#include <map>
#include <unordered_map>

typedef uint32_t Memory;

class PartitionPacketClassifier {
public:
	virtual int ComputeNumberOfBuckets(const std::vector<Rule>& rules) = 0;
};

class ClassifierTable {
public:
	virtual int ClassifyAPacket(const Packet& packet) const = 0;
	virtual void Insertion(const Rule& rule, bool& priorityChange) = 0;
	virtual void Deletion(const Rule& rule, bool& priorityChange) = 0;
	virtual bool CanInsertRule(const Rule& rule) const = 0;

	virtual size_t NumRules() const = 0;
	virtual int MaxPriority() const = 0;

	virtual Memory MemSizeBytes(Memory ruleSize) const = 0;
	virtual std::vector<Rule> GetRules() const = 0;
};

class PacketClassifier {
public:
	virtual void ConstructClassifier(const std::vector<Rule>& rules) = 0;
	virtual int ClassifyAPacket(const Packet& packet) = 0;
	virtual void DeleteRule(size_t index) = 0;
	virtual void InsertRule(const Rule& rule) = 0;
	virtual Memory MemSizeBytes() const = 0;
	virtual int MemoryAccess() const = 0;
	virtual size_t NumTables() const = 0;
	virtual size_t RulesInTable(size_t tableIndex) const = 0;
	virtual size_t PriorityOfTable(size_t tableIndex) const = 0;

	int TablesQueried() const {	return queryCount; }
	int NumPacketsQueriedNTables(int n) const { return GetOrElse<int, int>(packetHistogram, n, 0); };

protected:
	void QueryUpdate(int query) { 
		packetHistogram[query]++;
		queryCount += query;
	}

private:
	int queryCount = 0;
	std::unordered_map<int, int> packetHistogram;
};

class ListClassifier : public PacketClassifier {
public:
	virtual void ConstructClassifier(const std::vector<Rule>& rules) {
		this->rules = rules;
	}
	virtual int ClassifyAPacket(const Packet& packet) {
		for (const Rule& r : rules) {
			if (r.MatchesPacket(packet)) {
				return r.priority;
			}
		}
		return -1;
	}
	virtual void DeleteRule(size_t index) {};
	virtual void InsertRule(const Rule& rule) {};
	virtual Memory MemSizeBytes() const {
		return 0;
	};
	virtual int MemoryAccess() const {
		return rules.size();
	}
	virtual size_t NumTables() const {
		return 1;
	};
	virtual size_t RulesInTable(size_t tableIndex) const {
		return rules.size();
	};

private:
	std::vector<Rule> rules;
};


enum RequestType{
	ClassifyPacket,
	Insertion,
	Deletion
};
struct Request {
	Request(RequestType request_type) : request_type(request_type) {}
	RequestType request_type;
	int random_index_trace;
};

struct Bookkeeper {
	Bookkeeper(){}
	Bookkeeper(const std::vector<Rule> rules) : rules(rules) {}
	Rule GetOneRuleAndPop(int i) {
		if (rules.size() == 0) {
			printf("warning: Bookeeper has no rule");
			return Rule();
		}
		Rule to_return = rules[i];
		if (i != rules.size() - 1) rules[i] = std::move(rules[rules.size() - 1]);
		rules.pop_back();
		return to_return;
	}
	std::vector<Rule> GetRules() const {
		return rules;
	}
	void InsertRule(const Rule& r) {
		rules.push_back(r);
	}
	int size() const { return rules.size(); }
private:
	std::vector<Rule> rules;
};
class Simulator{
public:
	Simulator(){}
	~Simulator(){}
	Simulator(const std::vector<Rule>& ruleset) :ruleset(ruleset) {}
	Simulator(const std::vector<Rule>& ruleset, const std::vector<Packet>& packets) :ruleset(ruleset), packets(packets) {}

	static int PerformPartitioning(PartitionPacketClassifier& ppc, const std::vector<Rule>& ruleset, std::map<std::string, std::string>& summary);

	std::vector<Request> SetupComputation(int num_packet, int num_insert, int num_delete);
	std::vector<int>  PerformOnlyPacketClassification(PacketClassifier& classifier, std::map<std::string, std::string>& summary) const;
	std::vector<int>  PerformPartialBuild(PacketClassifier& classifier, std::map<std::string, std::string>& summary, double frac) const;
	std::vector<int>  PerformPacketClassification( PacketClassifier& classifier, const std::vector<Request>& sequence, std::map<std::string, double>& trial) const;

private:

	std::vector<Request> GenerateRequests(int num_packet, int num_insert, int num_delete) const;

	std::vector<Rule> ruleset;
	std::vector<Packet> packets;
	Bookkeeper rules_in_use;
	Bookkeeper available_pool;
};

#endif
