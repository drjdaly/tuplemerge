# Variables to control Makefile operation

OVSPATH = OVS/
IOPATH = IO/
SQLPATH = SQLite/
MITPATH = PartitionSort/
FORGEPATH = TupleMerge/
TRACEPATH = ClassBenchTraceGenerator/
TREEPATH = Trees/
UTILPATH = Utilities/
BVPATH = BitVector/
VPATH = $(OVSPATH) $(MITPATH) $(TRACEPATH) $(IOPATH) $(UTILPATH) $(FORGEPATH) $(TREEPATH) $(SPPATH) $(BVPATH) $(SQLPATH)

CXX = g++
CXXFLAGS = -g -std=c++14 -pedantic -fpermissive -fopenmp -O3

# Targets needed to bring the executable up to date

main: main.o Simulation.o InputReader.o OutputWriter.o trace_tools.o TupleMergeOnline.o TupleMergeOffline.o SlottedTable.o DISCPAC.o IntervalTree.o LongestIncreasingSubsequence.o SortableRulesetPartitioner.o misc.o MITree.o OptimizedMITree.o PartitionSort.o red_black_tree.o RuleSplitter.o stack.o cmap.o TupleSpaceSearch.o IntervalUtilities.o EffectiveGrid.o MapExtensions.o Tcam.o
	$(CXX) $(CXXFLAGS) -o main *.o $(LIBS)

# -------------------------------------------------------------------

main.o: main.cpp ElementaryClasses.h SortableRulesetPartitioner.h InputReader.h Simulation.h BruteForce.h cmap.h TupleSpaceSearch.h trace_tools.h PartitionSort.h IntervalUtilities.h hash.h OptimizedMITree.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Simulation.o: Simulation.cpp Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c Simulation.cpp

# ** IO **

InputReader.o: InputReader.cpp InputReader.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(IOPATH)InputReader.cpp

OutputWriter.o: OutputWriter.cpp OutputWriter.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(IOPATH)OutputWriter.cpp

# ** Trace **

trace_tools.o: trace_tools.cc trace_tools.h
	$(CXX) $(CXXFLAGS) -c $(TRACEPATH)trace_tools.cc

# ** TupleMerge **
	
TupleMergeOffline.o: TupleMergeOffline.cpp TupleMergeOffline.h SlottedTable.h TupleMergeOnline.h
	$(CXX) $(CXXFLAGS) -c $(FORGEPATH)TupleMergeOffline.cpp

TupleMergeOnline.o: TupleMergeOnline.cpp TupleMergeOnline.h SlottedTable.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(FORGEPATH)TupleMergeOnline.cpp

SlottedTable.o: SlottedTable.cpp SlottedTable.h Simulation.h TupleSpaceSearch.h
	$(CXX) $(CXXFLAGS) -c $(FORGEPATH)SlottedTable.cpp

# ** PartitionSort **

DISCPAC.o: DISCPAC.cpp DISCPAC.h misc.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)DISCPAC.cpp

IntervalTree.o: IntervalTree.cpp IntervalTree.h misc.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)IntervalTree.cpp

LongestIncreasingSubsequence.o: LongestIncreasingSubsequence.cpp LongestIncreasingSubsequence.h misc.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)LongestIncreasingSubsequence.cpp

SortableRulesetPartitioner.o: SortableRulesetPartitioner.cpp SortableRulesetPartitioner.h ElementaryClasses.h IntervalUtilities.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)SortableRulesetPartitioner.cpp

misc.o: misc.cpp misc.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)misc.cpp

MITree.o: MITree.cpp MITree.h misc.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)MITree.cpp

OptimizedMITree.o: OptimizedMITree.cpp OptimizedMITree.h red_black_tree.h misc.h stack.h ElementaryClasses.h SortableRulesetPartitioner.h IntervalUtilities.h Simulation.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)OptimizedMITree.cpp

PartitionSort.o: PartitionSort.cpp PartitionSort.h OptimizedMITree.h red_black_tree.h misc.h stack.h ElementaryClasses.h SortableRulesetPartitioner.h IntervalUtilities.h Simulation.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)PartitionSort.cpp

red_black_tree.o: red_black_tree.cpp red_black_tree.h misc.h stack.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)red_black_tree.cpp

stack.o: stack.cpp stack.h misc.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)stack.cpp
	
RuleSplitter.o: RuleSplitter.cpp RuleSplitter.h SortableRulesetPartitioner.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)RuleSplitter.cpp

# ** TupleSpace **

cmap.o: cmap.cpp cmap.h hash.h ElementaryClasses.h random.h
	$(CXX) $(CXXFLAGS) -c  $(OVSPATH)cmap.cpp

TupleSpaceSearch.o: TupleSpaceSearch.cpp TupleSpaceSearch.h Simulation.h ElementaryClasses.h cmap.h hash.h
	$(CXX) $(CXXFLAGS) -c $(OVSPATH)TupleSpaceSearch.cpp

# ** Utils **

IntervalUtilities.o: IntervalUtilities.cpp IntervalUtilities.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)IntervalUtilities.cpp

EffectiveGrid.o : EffectiveGrid.cpp EffectiveGrid.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)EffectiveGrid.cpp

MapExtensions.o : MapExtensions.cpp MapExtensions.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)MapExtensions.cpp

Tcam.o : Tcam.cpp Tcam.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)Tcam.cpp

.PHONY: clean
.PHONY: uninstall

clean:
	rm -f *.o

uninstall: clean
	rm main
