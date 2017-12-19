#include "write_named_pipe.h"
#include "named_pipe.h"
#include "constants.h"
#include "exceptions.h"
#include <sys/stat.h>  // for mkfifo
#include <fcntl.h>  // for open
#include <string>
using namespace std;

WriteNamedPipe::WriteNamedPipe(bool create)
{
	int status;

	if (create)
	{
		status = mkfifo(FIFO_NAME, S_IFIFO | 0666);
		if (status == -1)
			throw runtime_error("NamedPipe::NamedPipe::mkfifo");
	}

	m_fifofd = open(FIFO_NAME, O_WRONLY);
	if (m_fifofd == -1)
		throw runtime_error("NamedPipe::NamedPipe::open");
}

void WriteNamedPipe::writePipe(const string& text, unsigned len) const
{
	int status;

	status = write(m_fifofd, text.c_str(), len);
	if (status == -1)
		throw runtime_error("NamedPipe::write::write");
}
