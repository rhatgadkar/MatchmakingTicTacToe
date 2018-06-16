#ifndef WRITE_NAMED_PIPE_H
#define WRITE_NAMED_PIPE_H

#include "named_pipe.h"
#include <string>

class WriteNamedPipe : public NamedPipe
{
public:
	WriteNamedPipe(const char* fifo_name, bool create = true);
	virtual ~WriteNamedPipe() {}
	void writePipe(const std::string& text, unsigned len) const;
};

#endif  // WRITE_NAMED_PIPE_H
