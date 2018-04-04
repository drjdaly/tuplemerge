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
//#include <vld.h>

#include "ElementaryClasses.h"
#include "PartitionSort/SortableRulesetPartitioner.h"
#include "IO/InputReader.h"
#include "IO/OutputWriter.h"
#include "Simulation.h"

#include "PartitionSort/MITree.h"
#include "PartitionSort/DISCPAC.h"
#include "PartitionSort/RuleSplitter.h"
#include "BruteForce.h"
#include "TupleMerge/TupleMergeOnline.h"
#include "TupleMerge/TupleMergeOffline.h"
#include "OVS/cmap.h"
#include "OVS/TupleSpaceSearch.h"
#include "ClassBenchTraceGenerator/trace_tools.h"

#include "PartitionSort/PartitionSort.h"
#include <stdio.h>


#include <assert.h>
#include <memory>
#include <chrono>
#include <string>
#include <sstream>

using namespace std;
unsigned int HashPacket(const Packet& p, int sip_length, int dip_length) {
	uint32_t hash = 0;
	uint32_t max_uint = 0xFFFFFFFF;
	hash = hash_add(hash, p[0] & ~(max_uint >> sip_length));
	hash = hash_add(hash, p[1] & ~(max_uint >> dip_length));
	return hash_finish(hash, 16);
}

unsigned int inline hash_rule(const Rule& r)  {
	uint32_t hash = 0;
	hash = hash_add(hash, r.range[0][0]);
	hash = hash_add(hash, r.range[1][0]);
	return hash_finish(hash, 16);

}

void PrepareSimulators(const unordered_map<string, string>& args, ClassifierTests tests, unordered_map<string, PacketClassifier*>& classifiers) {
	if (tests & ClassifierTests::TestDiscPac) {
		classifiers["DISCPAC"] = new DISCPAC();
	}
	if (tests & ClassifierTests::TestPriorityDiscPac) {
		classifiers["PriorityDISCPAC"] = new PriorityDISCPAC;
	}
	if (tests & ClassifierTests::TestPartitionSort) {
		classifiers["PartitionSort"] = new PartitionSort;
	}
	if (tests & ClassifierTests::TestPartitionSortOffline) {
		classifiers["PartitionSortOffline"] = new PartitionSortOffline;
	}
	if (tests & ClassifierTests::TestSplitSort) {
		classifiers["SplitSort"] = new SplitSort(args);
	}
	if (tests & ClassifierTests::TestPriorityDiscPacHalfConstruction) {
		classifiers["PriorityDISCPACHalfConstruction"] = new PriorityDISCPACHalfConstruction;
	}
	if (tests & ClassifierTests::TestOnlineConstruction) {
		classifiers["PartitionSortOnline"] = new PartitionSort;
	}
	if (tests & ClassifierTests::TestTupleSpaceSearch) {
		classifiers["Tuple"] = new TupleSpaceSearch;
	}
	if (tests & ClassifierTests::TestPriorityTuple) {
		classifiers["PriorityTuple"] = new PriorityTupleSpaceSearch();
	}
	if (tests & ClassifierTests::TestForge) {
		classifiers["TupleMerge-Offline"] = new TupleMergeOffline(args);
	}
	
	if (tests & ClassifierTests::TestForgeOnline) {
		classifiers["TupleMerge-Online"] = new TupleMergeOnline(args);
	}
}


vector<int> RunSimulatorClassificationTrial(Simulator& s, const string& name, PacketClassifier& classifier, vector<map<string, string>>& data) {
	map<string, string> d = { { "Classifier", name } };
	printf("%s\n", name.c_str());
	auto r = s.PerformOnlyPacketClassification(classifier, d);
	data.push_back(d);
	return r;
}

pair< vector<string>, vector<map<string, string>>>  RunSimulatorOnlyClassification(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile = "") {
	printf("Classification Simulation\n");
	Simulator s(rules, packets);

	vector<string> header = { "Classifier", "ConstructionTime(ms)", "ClassificationTime(s)", "Size(bytes)", "MemoryAccess", "Tables", "TableSizes", "TablePriorities", "TableQueries", "AvgQueries" };
	vector<map<string, string>> data;

	unordered_map<string, PacketClassifier*> classifiers;
	PrepareSimulators(args, tests, classifiers);
	
	for (auto& pair : classifiers) {
		RunSimulatorClassificationTrial(s, pair.first, *pair.second, data);
		delete pair.second;
	}

	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}


vector<int> RunSimulatorPartialBuildTrial(Simulator& s, const string& name, PacketClassifier& classifier, vector<map<string, string>>& data, const unordered_map<string, string>& args) {
	map<string, string> d = { { "Classifier", name } };
	printf("%s\n", name.c_str());
	double frac = GetDoubleOrElse(args, "Build.Frac", 0.5);
	auto r = s.PerformPartialBuild(classifier, d, frac);
	data.push_back(d);
	return r;
}

pair< vector<string>, vector<map<string, string>> > RunSimulatorPartialBuildClassification(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile = "") {
	printf("Classification PartialBuild\n");
	Simulator s(rules, packets);

	vector<string> header = { "Classifier", "ConstructionTime(ms)", "ClassificationTime(s)", "Size(bytes)", "MemoryAccess", "Tables", "TableSizes", "TablePriorities", "TableQueries", "AvgQueries" };
	vector<map<string, string>> data;

	unordered_map<string, PacketClassifier*> classifiers;
	PrepareSimulators(args, tests, classifiers);
	
	for (auto& pair : classifiers) {
		RunSimulatorPartialBuildTrial(s, pair.first, *pair.second, data, args);
		delete pair.second;
	}

	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}

void RunSimulatorUpdateTrial(const Simulator& s, const string& name, PacketClassifier& classifier, const vector<Request>& req,vector<map<string, string>>& data, int reps) {


	map<string, string> d = { { "Classifier", name } };
	map<string, double> trial;

	printf("%s\n", name.c_str());

	for (int r = 0; r < reps; r++) { 
		s.PerformPacketClassification(classifier, req, trial);
	}
	for (auto pair : trial) {
		d[pair.first] = to_string(pair.second / reps);
	}
	data.push_back(d);
}

pair< vector<string>, vector<map<string, string>>>  RunSimulatorUpdates(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile, int repetitions = 1) {
	printf("Update Simulation\n");

	vector<string> header = { "Classifier", "UpdateTime(s)" };
	vector<map<string, string>> data;

	Simulator s(rules, packets);
	const auto req = s.SetupComputation(0, 500000, 500000);
	
	unordered_map<string, PacketClassifier*> classifiers;
	PrepareSimulators(args, tests, classifiers);
	
	for (auto pair : classifiers) {
		RunSimulatorUpdateTrial(s, pair.first, *pair.second, req, data, repetitions);
		delete pair.second;
	}

	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}

bool Validation(const unordered_map<string, PacketClassifier*> classifiers, const vector<Rule>& rules, const vector<Packet>& packets, int threshold = 10) {
	int numWrong = 0;
	vector<Rule> sorted = rules;
	sort(sorted.begin(), sorted.end(), [](const Rule& rx, const Rule& ry) { return rx.priority >= ry.priority; });
	for (const Packet& p : packets) {
		unordered_map<string, int> results;
		int result = -1;
		for (const auto& pair : classifiers) {
			result = pair.second->ClassifyAPacket(p);
			results[pair.first] = result;
		}
		if (!all_of(results.begin(), results.end(), [=](const auto& pair) { return pair.second == result; })) {
			numWrong++;
			for (auto x : p) {
				printf("%u ", x);
			}
			printf("\n");
			for (const auto& pair : results) {
				printf("\t%s: %d\n", pair.first.c_str(), pair.second);
			}
			for (const Rule& r : sorted) {
				if (r.MatchesPacket(p)) {
					printf("\tTruth: %d\n", r.priority);
					break;
				}
			}
		}
		if (numWrong >= threshold) {
			break;
		}
	}
	return numWrong == 0;
}

void RunValidation(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests) {
	printf("Validation Simulation\n");
	unordered_map<string, PacketClassifier*> classifiers;
	PrepareSimulators(args, tests, classifiers);

	printf("Building\n");
	for (auto& pair : classifiers) {
		printf("\t%s\n", pair.first.c_str());
		pair.second->ConstructClassifier(rules);
	}

	printf("Testing\n");
	int threshold = GetIntOrElse(args, "Validate.Threshold", 5);
	if (Validation(classifiers, rules, packets, threshold)) {
		printf("All classifiers are in accord\n");
	}

	for (auto& pair : classifiers) {
		delete pair.second;
	}
}

void RunSimulatorUpdates (const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile, SQLiteData& sql, const string& db_name, int repeat = 1) {
	auto pair_head_data = RunSimulatorUpdates(args, packets, rules, tests, outfile, repeat);
	OutputWriter::WriteToSQLite(db_name, sql, pair_head_data.first, pair_head_data.second);

}
void RunSimulatorOnlyClassification(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile,  SQLiteData& sql, const string& db_name) {
	auto pair_head_data = RunSimulatorOnlyClassification(args, packets,rules,tests,outfile);
	OutputWriter::WriteToSQLite(db_name, sql, pair_head_data.first, pair_head_data.second);
}

void RunPartitioningTrial(const std::vector<Rule>& ruleset, PartitionPacketClassifier& classifier, const string& name, vector<map<string, string>>& data) {
	map<string, string> d = { { "Classifier", name } };
	printf("%s\n", name.c_str());
	Simulator::PerformPartitioning(classifier, ruleset, d);
	data.push_back(d);
}

pair< vector<string>, vector<map<string, string>>> RunPartitioning(const vector<Rule>& rules, const string& outfile, PartitioningMode partition_mode) {
	printf("Run Partitioning\n");

	vector<string> header = { "Classifier", "ConstructionTime(s)", "NumberOfPartitions" };
	vector<map<string, string>> data;
	
	/*if (partition_mode & PartitioningMode::PartitionLowerBound) {
		RunPartitioningTrial(rules, LowerBound(), "LB", data);
	}*/
	if (partition_mode & PartitioningMode::PartitionMaximumIndpendentSet) {
		MaximumIndependentSetConstruction c;
		RunPartitioningTrial(rules, c, "MISF", data);
	}
	if (partition_mode & PartitioningMode::PartitionGreedyFieldSelection) {
		GreedyFieldSelectionConstruction c;
		RunPartitioningTrial(rules, c, "GFS", data);
	}
	/*if (partition_mode & PartitioningMode::PartitionSemiOnline) {
		RunPartitioningTrial(rules, SemiOnlineConstruction(), "SO", data);
	}*/
	if (partition_mode & PartitioningMode::PartitionOnline) {
		OnlineConstruction c;
		RunPartitioningTrial(rules, c, "FO", data);
	}
	if (partition_mode & PartitioningMode::PartitionTupleSpace) {
		TupleSpace c;
		RunPartitioningTrial(rules, c, "TS", data);
	}
	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}

void RunPartitioning(const vector<Rule>& rules, PartitioningMode partition_mode, const string& outfile, SQLiteData& sql, const string& db_name) {
	auto pair_head_data = RunPartitioning(rules, outfile,partition_mode);
	OutputWriter::WriteToSQLite(db_name, sql, pair_head_data.first, pair_head_data.second);
}




static const struct cmap_node *
find(const struct cmap *cmap, int key)
{
	
	auto hash_val = hash_int(key, 0);
	const cmap_node * found_node = cmap_find(cmap, hash_val);
	 

	while (found_node != nullptr) {
		if (found_node->key == key) return found_node;
		found_node = found_node->next;
	}  

	return nullptr;
}


ClassifierTests ParseClassifier(const string& line) {
	vector<string> tokens;
	Split(line, ',', tokens);
	ClassifierTests tests = ClassifierTests::TestNone;

	for (const string& classifier : tokens) {
		//cout << classifier << endl;
		if (classifier == "DiscPac") {
			tests = tests | TestDiscPac;
		}
		else if (classifier == "PriorityDiscPac") {
			tests = tests | TestPriorityDiscPac;
		} 
		else if (classifier == "PartitionSort") {
			tests = tests | TestPartitionSort;
		}
		else if (classifier == "SplitSort") {
			tests = tests | TestSplitSort;
		}
		else if (classifier == "TupleSpaceSearch") {
			tests = tests | TestTupleSpaceSearch;
		}
		else if (classifier == "PriorityTuple") {
			tests = tests | TestPriorityTuple;
		}
		else if (classifier == "PriorityDiscPacHalfConstruction") {
			tests = tests | TestPriorityDiscPacHalfConstruction;
		}
		else if (classifier == "OnlineConstruction") {
			tests = tests | TestOnlineConstruction;
		}
		else if (classifier == "TMOffline") {
			tests = tests | TestForge;
		}
		else if (classifier == "TMOnline") {
			tests = tests | TestForgeOnline;
		}
		else if (classifier == "All") {
			tests = tests | TestAll;
		}
		else if (classifier == "BitCuts") {
			tests = tests | TestBitCuts;
		}
		else if (classifier == "PartitionSortOffline") {
			tests = tests | TestPartitionSortOffline;
		}
		else {
			printf("Unknown ClassifierTests: %s\n", classifier.c_str());
			exit(EINVAL);
		}
	}
	return tests;
}

PartitioningMode ParseModePartitioning(const string& mode) {

	if (mode == "PartitionAll") {
		return PartitionAll;
	} else if (mode == "PartitionLowerBound") {
		return PartitionLowerBound;
	} else if (mode == "PartitionMaximumIndpendentSet") {
		return PartitionMaximumIndpendentSet;
	} else if (mode == "PartitionGreedyFieldSelection") {
		return PartitionGreedyFieldSelection;
	}
	else if (mode == "PartitionSemiOnline") {
		return PartitionSemiOnline;
	}
	else if (mode == "PartitionOnline") {
		return PartitionOnline;
	}
	else if (mode == "PartitionAll") {
		return PartitionAll;
	}
	else {
		printf("Unknown Partition mode: %s\n", mode.c_str());
		exit(EINVAL);
	}
}
TestMode ParseMode(const string& mode) {
	printf("%s\n", mode.c_str());
	if (mode == "Classification") {
		return ModeClassification;
	}
	else if (mode == "Update") {
		return ModeUpdate;
	}
	else if (mode == "Partitioning") {
		return ModePartitioning;
	} else if (mode == "Partial") {
		return ModePartial;
	}
	else if (mode == "Validate") {
		return ModeValidation;
	}
	else {
		printf("Unknown mode: %s\n", mode.c_str());
		exit(EINVAL);
	}
}


//int main(int argc, char* argv[]) {
//	RealLifeDecompositiion();
//}

/*
 * TODO: Parameters
 * -f Filter File
 * -p Packet File
 * -o Output file
 * -c Classifier
 * -m Mode
 * -r Repeat and average
 * -d Database file
 */
int main(int argc, char* argv[]) {
	unordered_map<string, string> args = ParseArgs(argc, argv);

	string filterFile = GetOrElse(args, "f", "Buckets\\64k_1\\fw1_seed_1.rules");
	string packetFile = GetOrElse(args, "p", "Auto");
	string packetOutFile = GetOrElse(args, "pout", "");
	string outputFile = GetOrElse(args, "o", "");

	string database = GetOrElse(args, "d", "");
	bool doShuffle = GetBoolOrElse(args, "Shuffle", true);

	//set by default
	ClassifierTests classifier = ParseClassifier(GetOrElse(args, "c", "PartitionSort,PartitionSortOffline"));
	TestMode mode = ParseMode(GetOrElse(args, "m", "Classification"));
	PartitioningMode partition_mode = ParseModePartitioning(GetOrElse(args, "b", "PartitionGreedyFieldSelection"));
	int repeat = GetIntOrElse(args, "r", 1);

	if (GetBoolOrElse(args, "?", false)) {
		printf("Arguments:\n");
		printf("\t-f <file> Filter File\n");
		printf("\t-p <file> Packet File\n");
		printf("\t-o <file> Output File\n");
		printf("\t-c <classifier> Classifier\n");
		printf("\t-m <mode> Classification Mode\n");
		printf("\t-r <x> Repeat and average\n");
		printf("\t-d [<database> Database File]\n");
		printf("\t-b [<partitioning mode> Partitioning Mode]\n");
		exit(0);
	}
	
	//assign mode and classifer
	vector<Rule> rules = InputReader::ReadFilterFile(filterFile);

	vector<Packet> packets;

	if (packetFile == "Auto") packets = GeneratePacketsFromRuleset(rules, 1000000);
	else if(packetFile != "") packets = InputReader::ReadPackets(packetFile);

	if (packetOutFile != "") {
		OutputWriter::WritePackets(packetOutFile, packets);
	}

	if (doShuffle) {
		rules = Random::shuffle_vector(rules);
	}

	if (database.size() > 0) {
		auto sql = InputReader::ExtractDatabaseInfo(filterFile, mode, classifier);
		switch (mode)
		{
			case ModeClassification:
				RunSimulatorOnlyClassification(args, packets, rules, classifier, outputFile, sql, database);
				break;
			case ModeUpdate:
				RunSimulatorUpdates(args, packets, rules, classifier, outputFile, sql, database);
				break;
			case ModePartitioning:
				//shuffle the rules
				RunPartitioning(rules, partition_mode,outputFile,  sql, database);
				break;
			case ModeValidation:
				RunValidation(args, packets, rules, classifier);
				break;
		}
	}
	else {
		switch (mode)
		{
			case ModeClassification:
				RunSimulatorOnlyClassification(args, packets, rules, classifier, outputFile);
				break;
			case ModeUpdate:
				RunSimulatorUpdates(args, packets, rules, classifier, outputFile, repeat);
				break;
			case ModePartial:
				RunSimulatorPartialBuildClassification(args, packets, rules, classifier, outputFile);
				break;
			case ModeValidation:
				RunValidation(args, packets, rules, classifier);
				break;
		}
	}
	printf("Done\n");
	return 0;
}


