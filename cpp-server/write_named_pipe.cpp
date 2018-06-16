#include "write_named_pipe.h"
#include "named_pipe.h"
#include "constants.h"
#include "exceptions.h"
#include <sys/stat.h>  // for mkfifo
#include <fcntl.h>  // for open
#include <string>
using namespace std;

WriteNamedPipe::WriteNamedPipe(const char* fifo_name, bool create)
{
	int status;

	if (create)
	{
		status = mkfifo(fifo_name, S_IFIFO | 0666);
		if (status == -1)
			throw runtime_error("WriteNamedPipe::WriteNamedPipe::mkfifo");
	}

	m_fifofd = open(fifo_name, O_WRONLY);
	if (m_fifofd == -1)
		throw runtime_error("WriteNamedPipe::WriteNamedPipe::open");
}

void WriteNamedPipe::writePipe(const string& text, unsigned len) const
{
	int status;

	status = write(m_fifofd, text.c_str(), len);
	if (status == -1)
		throw runtime_error("WriteNamedPipe::write::write");
}
