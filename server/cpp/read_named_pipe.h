#ifndef READ_NAMED_PIPE_H
#define READ_NAMED_PIPE_H

#include "named_pipe.h"
#include <string>

class ReadNamedPipe : public NamedPipe
{
public:
	ReadNamedPipe();
	virtual ~ReadNamedPipe() {}
	std::string readPipe(unsigned len) const;
};

#endif  // READ_NAMED_PIPE_H
