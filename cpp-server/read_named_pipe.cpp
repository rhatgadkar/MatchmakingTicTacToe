#include "read_named_pipe.h"
#include "named_pipe.h"
#include "constants.h"
#include "exceptions.h"
#include <sys/stat.h>  // for mkfifo
#include <fcntl.h>  // for open
#include <string>
#include <cstring>
using namespace std;

ReadNamedPipe::ReadNamedPipe(const char* fifo_name, bool create)
{
	int status;

	if (create)
	{
		status = mkfifo(fifo_name, S_IFIFO | 0666);
		if (status == -1)
			throw runtime_error("ReadNamedPipe::ReadNamedPipe::mkfifo");
	}

	m_fifofd = open(fifo_name, O_RDONLY | O_NDELAY);
	if (m_fifofd == -1)
		throw runtime_error("ReadNamedPipe::ReadNamedPipe::open");
}

string ReadNamedPipe::readPipe(unsigned len) const
{
	int status;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	status = read(m_fifofd, buf, len);
	if (status == -1)
		throw runtime_error("ReadNamedPipe::read : read returned -1");
	if (strlen(buf) != len)
		throw runtime_error("ReadNamedPipe::read : could not read len");
	string newStr(buf);
	return newStr;
}
