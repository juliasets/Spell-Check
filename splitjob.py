#!/usr/bin/python2

import sys
import os
import subprocess
import shutil

from job import Job



def get_priority (jobs):
    joblist = sorted(
        (job for job in os.listdir(jobs) if \
            not job.endswith('.ignore')))
    if len(joblist) > 0:
        newest = Job(joblist[-1])
        priority = newest.priority + 1
    else:
        priority = 0
    return priority



def main (argv = None):
    argv = [] if argv == None else argv
    if len(argv) != 2:
        raise ValueError(
            'Must provide a FILE to split and a JOBS directory.')
    fname = argv[0]
    jobs = argv[1]
    priority = get_priority(jobs)
    jobname = subprocess.Popen(['sha256sum'],
        stdin=open(fname), stdout=subprocess.PIPE). \
            communicate()[0].split()[0]
    subjobnum = 0

    f = open(fname)
    i = 0
    while True:
        i += 1
        if i % 10 == 0: priority = get_priority(jobs)
        outname = '%015i,%s,%015i,%i,%f' % (priority,
            jobname, subjobnum, 0, 0.)
        data = "".join(f.readline() for i in xrange(100))
        if data == '':
            break
        outpath = os.path.join(jobs, outname)
        with open(outpath + '.ignore', 'wb') as outf:
            outf.write(
"""
import subprocess
proc = subprocess.Popen(
    ['sh', '-c', 'cat'],
    stdin=subprocess.PIPE)
proc.communicate(%s)
exit(proc.returncode)
""" % repr(data)
            )
        try:
            shutil.move(outpath + '.ignore', outpath)
        except:
            os.unlink(outpath + '.ignore')
            raise
        subjobnum += 1



if __name__ == '__main__':
    main(sys.argv[1:])



