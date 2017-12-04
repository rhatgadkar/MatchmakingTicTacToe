#include "named_pipe.h"
#include <sys/stat.h>  // for mkfifo
#include <fcntl.h>  // for open
#include <string>
#include <cstring>
#include "constants.h"
#include <pthread.h>
#include <unistd.h>
#include "exceptions.h"
using namespace std;

NamedPipe::NamedPipe()
{
	int status;

	status = mkfifo(FIFO_NAME, S_IFIFO | 0666);
	if (status == -1)
		throw runtime_error("NamedPipe::NamedPipe::mkfifo");

	m_fifofd = open(FIFO_NAME, O_RDWR | O_NDELAY);
	if (m_fifofd == -1)
		throw runtime_error("NamedPipe::NamedPipe::open");
}

NamedPipe::~NamedPipe()
{
	close(m_fifofd);
	unlink(FIFO_NAME);
}

string NamedPipe::readPipe(unsigned len) const
{
	int status;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	status = read(m_fifofd, buf, len);
	if (status == -1)
		throw runtime_error("NamedPipe::read::read");
	string newStr(buf);
	return newStr;
}

void NamedPipe::writePipe(const string& text, unsigned len) const
{
	int status;

	status = write(m_fifofd, text.c_str(), len);
	if (status == -1)
		throw runtime_error("NamedPipe::write::write");
}
