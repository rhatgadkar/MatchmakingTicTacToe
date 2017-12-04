#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include <string>
#include <pthread.h>

class NamedPipe
{
public:
	NamedPipe();
	~NamedPipe();
	std::string readPipe(unsigned len) const;
	void writePipe(const std::string& text, unsigned len) const;

private:
	int m_fifofd;
};

#endif  // NAMED_PIPE_H
