#!/usr/bin/python2

import sys
import os
import subprocess
import shutil
import time

from job import Job



def remainingjobs (jobname, jobs):
    joblist = sorted(
        (job for job in os.listdir(jobs) if \
            not job.endswith('.ignore') and \
                Job(job).jobname == jobname))
    return joblist



def main (argv = None):
    argv = [] if argv == None else argv
    if len(argv) != 3:
        raise ValueError(
            'Must provide the FILE that was split, ' \
            'a JOBS directory, and a RESULTS directory.')
    fname = argv[0]
    jobs = argv[1]
    results = argv[2]
    jobname = subprocess.Popen(['sha256sum'],
        stdin=open(fname), stdout=subprocess.PIPE). \
            communicate()[0].split()[0]

    while len(remainingjobs(jobname, jobs)) != 0:
        time.sleep(0.1)
    # Job is complete now.

    resultlist = sorted(result for result in os.listdir(results) if \
        result.endswith('.result') and \
        result.split(',')[1].strip('.result') == jobname)

    for result in resultlist:
        fname = os.path.join(results, result)
        with open(fname) as f:
            for line in f:
                sys.stdout.write(line)
        os.unlink(fname)



if __name__ == '__main__':
    main(sys.argv[1:])



