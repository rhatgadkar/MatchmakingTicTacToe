#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

class NamedPipe
{
public:
	NamedPipe();
	virtual ~NamedPipe();

protected:
	int m_fifofd;
};

#endif  // NAMED_PIPE_H
