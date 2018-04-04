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

#ifndef  ELEM_H
#define  ELEM_H
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>
#include <chrono> 
#include <array>
#define FieldSA 0
#define FieldDA 1
#define FieldSP 2
#define FieldDP 3
#define FieldProto 4

#define LowDim 0
#define HighDim 1
 
#define POINT_SIZE_BITS 32

typedef uint32_t Point;
typedef std::vector<Point> Packet;

struct Rule
{
	//Rule(){};
	Rule(int dim = 5) : dim(dim), range(dim, { { 0, 0 } }), prefix_length(dim, 0){ markedDelete = 0; }
 
	int dim;
	int	priority;

	int id;
	int tag;
	bool markedDelete = 0;

	std::vector<unsigned> prefix_length;

	std::vector<std::array<Point,2>> range;

	bool inline MatchesPacket(const Packet& p) const {
		for (int i = 0; i < dim; i++) {
			if (p[i] < range[i][LowDim] || p[i] > range[i][HighDim]) return false;
		}
		return true;
	}
	
	bool inline IntersectsRule(const Rule& r) const {
		for (int i = 0; i < dim; i++) {
			if (range[i][HighDim] < r.range[i][LowDim] || range[i][LowDim] > r.range[i][HighDim]) return false;
		}
		return true;
	}

	void Print() const {
		for (int i = 0; i < dim; i++) {
			printf("%u:%u ", range[i][LowDim], range[i][HighDim]);
		}
		printf("\n");
	}
};

class Interval {
public:
	Interval() {}
	virtual ~Interval();
	virtual Point GetLowPoint() const = 0;
	virtual Point GetHighPoint() const = 0;
	virtual void Print() const;
};

struct interval : public Interval {
	 
	interval(unsigned int a, unsigned int b, int id) : a(a), b(b), id(id) {}
	~interval(){}
	Point GetLowPoint() const { return a; }
	Point GetHighPoint() const { return b; }
	void Print()const {};

	Point a, b;
	bool operator < (const interval& rhs) const {
		if (a != rhs.a) {
			return a < rhs.a;
		} else return b < rhs.b;
	}
	bool operator == (const interval& rhs) const {
		return a == rhs.a && b == rhs.b;
	}
	int id;
	int weight;

};
struct EndPoint {
	EndPoint(double val, bool isRightEnd, int id) : val(val), isRightEnd(isRightEnd), id(id){}
	bool operator < (const EndPoint & rhs) const {
		return val < rhs.val;
	}
	double val;
	bool isRightEnd;
	int id;
};
class Random {
public:
	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	static int random_int(int low, int high)
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_int_distribution < int >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ low, high });
	}

	// random number generator from Stroustrup: 
	// http://www.stroustrup.com/C++11FAQ.html#std-random
	// static: there is only one initialization (and therefore seed).
	static int random_unsigned_int()
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_int_distribution < unsigned int >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ 0, 4294967295 });
	}
	static double random_real_btw_0_1()
	{
		//static std::mt19937  generator;
		using Dist = std::uniform_real_distribution < double >;
		static Dist uid{};
		return uid(generator, Dist::param_type{ 0,1 });
	}

	template <class T>
	static std::vector<T> shuffle_vector(std::vector<T> vi) {
		//static std::mt19937  generator;
		std::shuffle(std::begin(vi), std::end(vi), generator);
		return vi;
	}
private:
	static std::mt19937 generator;
};

struct SQLiteData  {
	SQLiteData(){}
	SQLiteData(const std::string& classifier,
			   const std::string& mode,
			   const std::string& rule_seed,
			   int dim,
			   int rule_size,
			   double result) : classifier(classifier),
			   mode(mode),
			   rule_seed(rule_seed),
			   rule_size(rule_size),
			   dim(dim),
			   result(result),
			   threshold(threshold)
	{}
	std::string classifier;
	std::string mode;

	int dim;
	std::string rule_seed;
	int rule_size;
	double result;
	int threshold;
};


enum TestMode {
	ModeClassification, //included mode construction already
	ModeUpdate,
	ModeSizeAndMemoryAccess,
	ModePartial,
	ModePartitioning,
	ModeValidation
};

enum PartitioningMode {
	PartitionLowerBound = 0x0001,
	PartitionMaximumIndpendentSet = 0x0002,
	PartitionGreedyFieldSelection = 0x0004,
	PartitionSemiOnline = 0x0008,
	PartitionOnline = 0x0010,
	PartitionTupleSpace = 0x0100,
	PartitionAll = 0xFFFF
};

enum ClassifierTests {
	TestNone = 0x0000,
	TestDiscPac = 0x0001,
	TestPriorityDiscPac = 0x0002,
	TestPartitionSort = 0x0004,
	TestOnlineConstruction = 0x0008,
	TestTupleSpaceSearch = 0x0010,
	TestPriorityTuple = 0x0020,
	TestForge = 0x0040,
	TestForgeHybrid = 0x0080,
	TestPriorityDiscPacHalfConstruction = 0x1000,
	TestSaxPac = 0x2000,
	TestPartitionSortOffline = 0x8000,
	TestForgeOnline = 0x10000,
	TestForgeHybridOnline = 0x20000,
	TestForgePredict = 0x40000,
	TestSplitSort = 0x80000,
	TestBitCuts = 0x100000,
	TestAll = 0xFFFFFFFF
};

enum PSMode {
	NoCompression,
	PathCompression,
	PriorityNode,
	NoIntermediateTree
};


inline ClassifierTests operator|(ClassifierTests a, ClassifierTests b) {
	return static_cast<ClassifierTests>(static_cast<int>(a) | static_cast<int>(b));
}

inline void SortRules(std::vector<Rule>& rules) {
	sort(rules.begin(), rules.end(), [](const Rule& rx, const Rule& ry) { return rx.priority >= ry.priority; });
}

inline void SortRules(std::vector<Rule*>& rules) {
	sort(rules.begin(), rules.end(), [](const Rule* rx, const Rule* ry) { return rx->priority >= ry->priority; });
}

inline void PrintRules(const std::vector<Rule>& rules) {
	for (const Rule& r : rules) {
		r.Print();
	}
}

#endif
