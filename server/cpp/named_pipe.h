#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include <unistd.h>

class NamedPipe
{
public:
	virtual ~NamedPipe() { close(m_fifofd); };

protected:
	int m_fifofd;
};

#endif  // NAMED_PIPE_H
