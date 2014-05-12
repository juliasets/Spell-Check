#!/usr/bin/python2

import sys
import os
import shutil
import time
import subprocess
import traceback

from job import Job



class Daemon (object):


    def __init__ (self, jobs, working, results):
        self.jobs = os.path.abspath(jobs)
        self.working = os.path.abspath(working)
        self.results = os.path.abspath(results)


    def dojob (self):
        joblist = sorted(os.listdir(self.jobs))
        for job in joblist:
            if job.endswith('.ignore'):
                continue
            job = Job(os.path.join(self.jobs, job))
            if job.acceptable():
                break
        else:
            return False
        # We have received a job. Let's try to accept it.
        print job.priority, job.subjobnum, job.jobname
        if not job.accept(): return None # Recover from race condition.
        # We have accepted a job! It's ours now.
        with job:
            workingpath = job.copy_to_dir(self.working)
            resultpath = workingpath + '.result'
            job.rmpaths.extend([workingpath, resultpath])
            result = subprocess.call([sys.executable, workingpath],
                stdout = open(resultpath, 'wb'))
            if result == 0:
                try:
                    shutil.move(resultpath, self.results)
                except shutil.Error:
                    pass
                job.finish(True)
                return True
        return None


    def run (self):
        result = False
        while True:
            try:
                result = self.dojob()
            except KeyboardInterrupt:
                raise
            except:
                traceback.print_exc()
                result = None
            if result == False:
                # There were no available jobs to do.
                time.sleep(1) # Wait for more jobs.
            elif result == True:
                continue # A job was completed.
            elif result == None:
                continue # An error or race condition occurred.



def main (argv = None):
    argv = [] if argv == None else argv
    if len(argv) != 3:
        raise ValueError(
            'Must provide JOBS, WORKING, and RESULTS directories.')
    jobs, working, results = argv
    daemon = Daemon(jobs, working, results)
    daemon.run()



if __name__ == '__main__':
    main(sys.argv[1:])



