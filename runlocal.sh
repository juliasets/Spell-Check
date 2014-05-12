#!/bin/bash


local=/tmp/test/
global=$HOME/tmp/test/

jobs=$global/jobs
working=$local/working
results=$global/results
execs=$local/execs


mkdir -p $jobs
mkdir -p $working
mkdir -p $results
mkdir -p $execs


cp spellcheck-splitjob.py $execs
cp splitjob.py $execs
cp joinjob.py $execs
cp jobd.py $execs
cp job.py $execs
cp ../SpellCorrector/threadedSpellCheck.exe $execs
cp -nr ../Dictionary $local

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
echo $procs



