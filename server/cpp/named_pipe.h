#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include <string>
#include <pthread.h>

class NamedPipe
{
public:
	NamedPipe();
	~NamedPipe();
	std::string read(std::string& text, unsigned len);
	void write(const std::string& text, unsigned len);

private:
	int m_fifofd;
	static const char* NAME;
};

#endif  // NAMED_PIPE_H
