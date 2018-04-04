#pragma once

#include "../Simulation.h"
#include "../Trees/TreeUtils.h"

#include "BitSet.h"

class FieldMatcher {
public:
	virtual size_t Match(Point x) const = 0;
};

class BinaryRangeSearch : public FieldMatcher {
public:
	BinaryRangeSearch(std::vector<Range>& ranges);

	size_t Match(Point x) const;

private:
	size_t Seek(Point x, size_t l, size_t r) const;

	std::vector<Point> dividers;
	std::vector<size_t> indices;
};

class BitVector : public PacketClassifier {
public:
	BitVector();
	~BitVector();

	virtual void ConstructClassifier(const std::vector<Rule>& rules);
	virtual int ClassifyAPacket(const Packet& packet);

	virtual void DeleteRule(size_t index) {
		printf("Deletion not supported\n");
	}
	virtual void InsertRule(const Rule& rule) {
		printf("Insertion not supported\n");
	}
	virtual Memory MemSizeBytes() const {
		return 0; // TODO
	}
	virtual int MemoryAccess() const {
		return 0; // TODO
	}
	virtual size_t NumTables() const {
		return 1;
	}
	virtual size_t RulesInTable(size_t tableIndex) const {
		return rules.size();
	}
	virtual size_t PriorityOfTable(size_t tableIndex) const {
		return rules.size();
	}

private:
	std::vector<Rule> rules;
	std::vector<FieldMatcher*> matchers;
	std::vector<std::vector<BitSet>> fields;
};

class BitVector64 : public PacketClassifier {
public:
	BitVector64();
	~BitVector64();

	virtual void ConstructClassifier(const std::vector<Rule>& rules);
	virtual int ClassifyAPacket(const Packet& packet);

	virtual void DeleteRule(size_t index) {
		printf("Deletion not supported\n");
	}
	virtual void InsertRule(const Rule& rule) {
		printf("Insertion not supported\n");
	}
	virtual Memory MemSizeBytes() const {
		return 0; // TODO
	}
	virtual int MemoryAccess() const {
		return 0; // TODO
	}
	virtual size_t NumTables() const {
		return 1;
	}
	virtual size_t RulesInTable(size_t tableIndex) const {
		return rules.size();
	}
	virtual size_t PriorityOfTable(size_t tableIndex) const {
		return rules.size();
	}

private:
	std::vector<Rule> rules;
	std::vector<FieldMatcher*> matchers;
	std::vector<std::vector<BitSet64>> fields;
};

Point Mask(int len);
int PrefixLength(Range s);
