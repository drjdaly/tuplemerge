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
#ifndef  DISCPAC_H
#define  DISCPAC_H

#include "MITree.h"
#include "SortableRulesetPartitioner.h"

#include "../Simulation.h"
#include "../OVS/TupleSpaceSearch.h"

#include <memory>
#include <chrono>
#include <fstream>
#include <vector>

class DISCPAC : public PacketClassifier {
public:
	DISCPAC() {}
	~DISCPAC() { 
		for (auto r : all_ruleset) delete r; 
		for (auto t : MITrees) delete t; 
	}
	void ConstructClassifier(const std::vector<Rule>& rules) {
		rule_index.resize(rules.size());
		ConstructMITrees(SortableRulesetPartitioner::SortableRulesetPartitioningGFS(rules));
	}

	DISCPAC(const std::vector<SortableRuleset>& sb) {
		ConstructMITrees(sb);
	}

	int ClassifyAPacket(const Packet& one_packet);
	void DeleteRule(size_t index);
	void InsertRule(const Rule& one_rule);
	Memory MemSizeBytes() const;
	/*std::vector<rule> GetRules() const {
		return rules;
	}*/

	void PlotBucketDistribution() {
		std::sort(begin(MITrees), end(MITrees), [](MITree * lhs, MITree * rhs){ return lhs->size() > rhs->size(); });
		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		for (auto x : MITrees) {
			log << x->size() << " ";
		}
		log << std::endl;
	}
	void PrintNumberOfMITrees() {
		printf("%lu \n", MITrees.size());
	}
	int MemoryAccess() const {
		printf("warning unimplemented MemoryAccess()\n");
		return 0;
	}
	size_t NumTables() const { return MITrees.size(); }
	size_t RulesInTable(size_t index) const { return MITrees[index]->size(); }
	size_t PriorityOfTable(size_t index) const {
		return 0; // Must search all tables
	}
protected:
	void ConstructMITrees(const std::vector<SortableRuleset>& sb);
	std::vector<MITree *> MITrees;
	std::vector<MITreeRule *> all_ruleset;
	std::vector<int> rule_index;

};

class PriorityDISCPAC : public PacketClassifier {
public:
	~PriorityDISCPAC() {
		for (auto r : all_ruleset) delete r;
		for (auto t : PriorityMITrees) delete t;
	}
	int MemoryAccess() const {
		printf("warning unimplemented MemoryAccess()\n");
		return 0;
	}
	void ConstructClassifier(const std::vector<Rule>& rules) {
		rule_index.resize(rules.size());
		ConstructMITrees(SortableRulesetPartitioner::SortableRulesetPartitioningGFS(rules));
		//printf("Number of Buckets: %d \n priority: ", PriorityMITrees.size());
		//for (auto tree : PriorityMITrees) {
		//	printf(" %d ", tree->GetMaxPriority());
		//}
	//	printf("\n");
	}
	int ClassifyAPacket(const Packet& one_packet);
	void DeleteRule(size_t index);
	void InsertRule(const Rule& one_rule);
	Memory MemSizeBytes() const {
		return 0;
	}
	void PloztPriorityBucketDistribution() {
		InsertionSortMITrees();
		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		for (auto x : PriorityMITrees) {
			log << x->size() << " ";
		}
		log << std::endl;
	}
	void PrintNumberOfMITrees() {
		printf("%lu \n", PriorityMITrees.size());
	}
	int GetNumberMITrees() const {
		return PriorityMITrees.size();
	}
	size_t NumTables() const { return GetNumberMITrees(); }
	size_t RulesInTable(size_t tableIndex) const { 
		return PriorityMITrees[tableIndex]->CountRules(); 
	}
	size_t PriorityOfTable(size_t index) const {
		return PriorityMITrees[index]->GetMaxPriority();
	}
protected:
	void ConstructMITrees(const std::vector<SortableRuleset>& sb);
	std::vector<PriorityMITree *> PriorityMITrees;
	void InsertionSortMITrees();
	std::vector<MITreeRule *> all_ruleset;
	std::vector<int> rule_index;
};

class PriorityDISCPACHalfConstruction: public PriorityDISCPAC {
public:
	void ConstructClassifier(const std::vector<Rule>& ruleset) {
		std::vector<Rule> rules = ruleset;
		rules = Random::shuffle_vector(rules);
		rule_index.resize(rules.size()/2);
		std::vector<Rule> first_half(begin(rules), begin(rules) + rules.size() / 2);
		std::vector<Rule> second_half(begin(rules) + rules.size() / 2, end(rules)); 
		ConstructMITrees(SortableRulesetPartitioner::SortableRulesetPartitioningGFS(first_half));
		for (const Rule& r : second_half) {
			InsertRule(r);
		} 
	}
};

class PriorityDISCPACNoConstruction : public PriorityDISCPAC {
public:
	void ConstructClassifierAdaptive(const std::vector<Rule>& ruleset, int adaptive_threshold) {
		std::vector<Rule> rules = ruleset;
		rules = Random::shuffle_vector(rules);
		ConstructMITrees(SortableRulesetPartitioner::AdaptiveIncrementalInsertion(rules,adaptive_threshold));
	}
	void ConstructClassifier(const std::vector<Rule>& ruleset) {
		//intentionally insert rule in priority order 
		std::vector<Rule> rules = ruleset;
		rules = Random::shuffle_vector(rules);
		for (const Rule& r : rules) {
			InsertRule(r);
		}
	}
private:
	void InsertRuleAdaptive(const Rule& r, int);
};

class AdaptiveDISCPAC: public PriorityDISCPAC {
public:
	AdaptiveDISCPAC() {}
	AdaptiveDISCPAC(int threshold) : threshold(threshold) {}

	void ConstructClassifierAdaptive(const std::vector<Rule>& ruleset, int adaptive_threshold) {
		for (const Rule& r : ruleset) {
			InsertRuleAdaptive(r, adaptive_threshold);
		}
	}
	void ConstructClassifier(const std::vector<Rule>& ruleset) {
		for (const Rule& r : ruleset) {
			InsertRule(r);
		}
	}
	void InsertRule(const Rule& r) {
		InsertRuleAdaptive(r, threshold);
	}
	void InsertRuleAdaptive(const Rule& r, int);
private:
	int threshold;
};

class SmartAdaptiveDISCPAC : public PacketClassifier {
public:
	SmartAdaptiveDISCPAC() {}
	SmartAdaptiveDISCPAC(int threshold) : threshold(threshold) {}
	~SmartAdaptiveDISCPAC() {
		for (auto r : all_ruleset) delete r;
		for (auto t : SmartPriorityMITrees) delete t;
	}

	int ClassifyAPacket(const Packet& one_packet);
	void DeleteRule(size_t index);
	Memory MemSizeBytes() const {
		int memory_nodes_byte = 0;
		for (auto t : SmartPriorityMITrees) {
			memory_nodes_byte += t->Space();
		}
		return memory_nodes_byte + 4 * SmartPriorityMITrees.size() + 20 * all_ruleset.size();
	}
	int MemoryAccess() const {
		int memory_access = 0;
		for (auto t : SmartPriorityMITrees) {
			memory_access += t->NumberOfMemoryAccess();
		}
		return memory_access;
	}
	std::vector<SortableRuleset> ExportSortableRulesets() {
		InsertionSortSmartPriorityMITree();
		std::vector<SortableRuleset> temp_sortableruleset;
		for (auto n : SmartPriorityMITrees) {
			temp_sortableruleset.push_back(n->ExportSortablePartition());
		}
		return temp_sortableruleset;
	}
	void ConstructClassifierAdaptive(const std::vector<Rule>& ruleset, int adaptive_threshold) {
		for (const Rule& r : ruleset) {
			InsertRuleAdaptive(r, adaptive_threshold);
		}
	}
	void HalfConstructClassifierAdaptive(const std::vector<Rule>& ruleset, int adaptive_threshold) {
	
		std::vector<Rule> rules = ruleset;
		
		std::vector<Rule> first_half(begin(rules), begin(rules) + rules.size() / 2);
		std::vector<Rule> second_half(begin(rules) + rules.size() / 2, end(rules));

		rule_index.resize(ruleset.size()/2);
		ConstructMITrees(SortableRulesetPartitioner::SortableRulesetPartitioningGFS(first_half));
		for (const Rule& r : second_half) {
			InsertRule(r);
		}
	}
	void ConstructClassifier(const std::vector<Rule>& ruleset) {
		for (const Rule& r : ruleset) {
			InsertRule(r);
		}
	}
	void InsertRule(const Rule& r) {
		InsertRuleAdaptive(r, 10);
	}
	void InsertRuleAdaptive(const Rule& r, int);



	void PlotBucketDistribution() {

		std::sort(begin(SmartPriorityMITrees), end(SmartPriorityMITrees), [](const SmartPriorityMITree* lhs, const SmartPriorityMITree* rhs) {
			return rhs->size() < lhs->size();
		});
		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		for (auto x : SmartPriorityMITrees) {
			log << x->size() << " ";
		}
		log << std::endl;
	}
	void PlotPriorityBucketDistribution() {

		InsertionSortSmartPriorityMITree();
		std::ofstream log("logfile.txt", std::ios_base::app | std::ios_base::out);
		for (auto x : SmartPriorityMITrees) {
			log << x->size() << " ";
		}
		log << std::endl;
	}
	void PrintNumberOfMITrees() {
		printf("%lu \n", SmartPriorityMITrees.size());
	}
	int GetNumberOfMITrees() const {
		return SmartPriorityMITrees.size();
	}
	size_t NumTables() const { return GetNumberOfMITrees(); }
	size_t RulesInTable(size_t index) const { return SmartPriorityMITrees[index]->size(); }
	size_t PriorityOfTable(size_t index) const {
		return SmartPriorityMITrees[index]->GetMaxPriority();
	}
protected:
	void ConstructMITrees(const std::vector<SortableRuleset>& sb);
	void InsertionSortSmartPriorityMITree();

	std::vector<SmartPriorityMITree *> SmartPriorityMITrees;

	std::vector<int> rule_index;
	std::vector<MITreeRule *> all_ruleset;


	int threshold = 10;
};
 

/*
*
* These are classes designed for measuring ruleset partitioning performance;  
*
*/

class LowerBound : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {
		return SortableRulesetPartitioner::ComputeMaxIntersection(rules);
	}

};
class MaximumIndependentSetConstruction : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {
		return SortableRulesetPartitioner::MaximumIndepdenentSetPartitioning(rules).size();
	}
};


class GreedyFieldSelectionConstruction : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {
		return SortableRulesetPartitioner::SortableRulesetPartitioningGFS(rules).size();
	}
};

//construct half of ruleset by GFS and then insert online
class SemiOnlineConstruction : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {

		PriorityDISCPACHalfConstruction phc;
		phc.ConstructClassifier(rules);
		SmartAdaptiveDISCPAC adp;
		adp.HalfConstructClassifierAdaptive(rules, 10);
	//	return phc.GetNumberMITrees();
		return adp.GetNumberOfMITrees();
	}
};

class OnlineConstruction : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {
		SmartAdaptiveDISCPAC adp;
		adp.ConstructClassifierAdaptive(rules,10);
		return adp.GetNumberOfMITrees();
	}

};


class TupleSpace : public PartitionPacketClassifier {
public:
	int ComputeNumberOfBuckets(const std::vector<Rule>& rules) {
		PriorityTupleSpaceSearch tp;
		tp.ConstructClassifier(rules);
		return tp.GetNumberOfTuples();
	}

};



#endif
