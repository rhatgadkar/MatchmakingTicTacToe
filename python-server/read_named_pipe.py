from named_pipe import NamedPipe
import os
import fcntl

class ReadNamedPipe(NamedPipe):

    def __init__(self, fifo_name, create = True):
        if create:
            try:
                os.mkfifo(fifo_name)
            except:
                raise RuntimeError("ReadNamedPipe::ReadNamedPipe::mkfifo")
        try:
            self.fifofd = open(fifo_name, "r+", 0)
            fd = self.fifofd.fileno()
            flag = fcntl.fcntl(fd, fcntl.F_GETFL)
            fcntl.fcntl(fd, fcntl.F_SETFL, flag | os.O_NONBLOCK)
        except:
            raise RuntimeError("ReadNamedPipe::ReadNamedPipe::open")

    def read_pipe(self, size):
        try:
            return str(self.fifofd.read(size))
        except:
            raise RuntimeError("ReadNamedPipe::read : read failed")
