import sys
import os
import shutil
import time
import thread
from threading import RLock



class Job (object):


    def __init__ (self, path):
        self.mutex = RLock()
        self.__init(path)
        self.mine = False
        self.rmpaths = []


    def __init (self, path):
        self.path = path
        self.priority, self.jobname, self.subjobnum, \
            self.accepted, self.timeaccepted = \
                os.path.basename(self.path).split(',')
        self.priority = int(self.priority)
        self.subjobnum = int(self.subjobnum)
        self.accepted = int(self.accepted)
        self.timeaccepted = float(self.timeaccepted)


    def __enter__ (self):
        return self


    def __exit__ (self, t, v, trace):
        self.finish()


    def __claim (self):
        with self.mutex:
            newpath = os.path.join(
                os.path.dirname(self.path),
                '%015i,%s,%015i,%i,%f' % (self.priority, self.jobname,
                    self.subjobnum, 1, time.time())
            )
            try:
                shutil.move(self.path, newpath)
            except IOError:
                return False
            self.__init(newpath)
            self.mine = True
            return True


    def __maintain (self):
        while True:
            time.sleep(1)
            if self.mine:
                self.__claim()
            else:
                return


    def copy_to_dir (self, path):
        path = os.path.join(path, self.getidentifier())
        with self.mutex:
            shutil.copy(self.path, path)
        return path


    def getidentifier (self):
        return '%015i,%s' % (self.subjobnum, self.jobname)


    def acceptable (self):
        if not os.path.exists(self.path): return False
        if self.accepted == 0: return True
        if time.time() - self.timeaccepted > 10: return True
        return False


    def accept (self):
        if not self.acceptable(): return False
        if self.__claim():
            thread.start_new_thread(self.__maintain, ())
            return True
        return False


    def finish (self, success = False):
        with self.mutex:
            self.mine = False
            try:
                if success:
                    os.unlink(self.path)
            except OSError:
                pass
            for path in self.rmpaths:
                try:
                    os.unlink(path)
                except OSError:
                    pass



