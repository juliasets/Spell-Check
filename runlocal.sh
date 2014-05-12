#!/bin/bash


test=/tmp/test/
jobs=$test/jobs
working=$test/working
results=$test/results
execs=$test/execs


mkdir -p $jobs
mkdir -p $working
mkdir -p $results
mkdir -p $execs


cp spellcheck-splitjob.py $execs
cp splitjob.py $execs
cp joinjob.py $execs
cp jobd.py $execs
cp job.py $execs
cp ../execs/PipeSpellCheck $execs


cd $execs


./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
./jobd.py $jobs $working $results &
procs=$(jobs -p)
disown -a
echo procs



