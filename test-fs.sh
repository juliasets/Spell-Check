#!/bin/bash


test=/tmp/test/
jobs=$test/jobs
working=$test/working
results=$test/results


mkdir -p $jobs
mkdir -p $working
mkdir -p $results

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


od /dev/urandom | head -n 50000 >$test/test1in &
od /dev/urandom | head -n 50000 >$test/test2in &
od /dev/urandom | head -n 50000 >$test/test3in &
od /dev/urandom | head -n 50000 >$test/test4in &
wait


./splitjob.py $test/test1in $jobs &
./splitjob.py $test/test2in $jobs &
./splitjob.py $test/test3in $jobs &
./splitjob.py $test/test4in $jobs &
wait


./joinjob.py $test/test1in $jobs $results >$test/test1out &
./joinjob.py $test/test2in $jobs $results >$test/test2out &
./joinjob.py $test/test3in $jobs $results >$test/test3out &
./joinjob.py $test/test4in $jobs $results >$test/test4out &
time wait


kill $procs


diff $test/test1in $test/test1out | wc
diff $test/test2in $test/test2out | wc
diff $test/test3in $test/test3out | wc
diff $test/test4in $test/test4out | wc


rm $test/test1in $test/test1out
rm $test/test2in $test/test2out
rm $test/test3in $test/test3out
rm $test/test4in $test/test4out



