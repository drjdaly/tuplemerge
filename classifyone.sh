#!/bin/sh

Classifiers="PartitionSort,PriorityTuple,TMOffline,TMOnline,PartitionSortOffline"


Program=./main

Limit=20

d="32k"
f="fw5_seed_1"

SourceDir=../Classifiers/pretty
PacketDir=../Packets/pretty
OutputDir=../Output/Raw/pretty
TerminalDir=../Output/Raw/Terminal/pretty

File=${SourceDir}/${d}/${f}.rules
Output=${OutputDir}/${d}/${f}.rules.csv
Packets=${PacketDir}/${d}/${f}.rules.trace
Terminal=${Terminal}/${d}/${f}.txt

echo $f #
if [ ! -d ${OutputDir}/${d} ]; then
	mkdir -p ${OutputDir}/${d}/
fi
#if [ ! -d ${Terminal}/${d} ]; then
#	mkdir ${Terminal}/${d}/
#fi

${Program} f=${File} c=${Classifiers} o=${Output} TM.Limit.Insert=${Limit} TM.Limit.Build=${Limit} TM.Limit.Collide=${Limit} PS.SplitFactor=2 p=${Packets}

