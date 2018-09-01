class NamedPipe(object):

    def close(self):
        self.fifofd.close()
