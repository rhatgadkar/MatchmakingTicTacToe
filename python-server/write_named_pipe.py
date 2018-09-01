from named_pipe import NamedPipe
import os
import fcntl

class WriteNamedPipe(NamedPipe):

    def __init__(self, fifo_name, create = True):
        if create:
            try:
                os.mkfifo(fifo_name)
            except:
                raise RuntimeError("WriteNamedPipe::WriteNamedPipe::mkfifo")
        try:
            self.fifofd = open(fifo_name, "w+", 0)
            fd = self.fifofd.fileno()
            flag = fcntl.fcntl(fd, fcntl.F_GETFL)
            fcntl.fcntl(fd, fcntl.F_SETFL, flag | os.O_NONBLOCK)
        except:
            raise RuntimeError("WriteNamedPipe::WriteNamedPipe::open")

    def write_pipe(self, text):
        try:
            self.fifofd.write(text)
        except:
            raise RuntimeError("WriteNamedPipe::write::write")
