#!/bin/sh
Classifiers="PartitionSort,PriorityTuple,TMOffline,TMOnline,PartitionSortOffline"

RuleList="acl1_seed_1"
SourceDir=./
PacketDir=./
OutputDir=../Output/

Limit=10

echo $f #
if [ ! -d ${OutputDir} ]; then
	mkdir -p ${OutputDir}/
fi

File=${SourceDir}/${RuleList}.rules
Output=${OutputDir}/${RuleList}.rules.csv
Packets=${PacketDir}/${RuleList}.rules.trace
./main f=${File} c=${Classifiers} o=${Output} TM.Limit.Collide=${Limit} p=${Packets}

