#include "named_pipe.h"
#include <sys/stat.h>  // for mkfifo
#include <fcntl.h>  // for open
#include <string>
#include <cstring>
#include "constants.h"
#include <pthread.h>
using namespace std;

NamedPipe::NamedPipe()
{
	mkfifo(FIFO_NAME, S_IFIFO | 0666);
	m_fifofd = open(NamedPipe::NAME, O_WRONLY);
	if (m_fifofd == -1)
	{
		cerr << "NamedPipe::NamedPipe::open" << endl;
		throw RuntimeError();
	}
}

NamedPipe::~NamedPipe()
{
	close(m_fifofd);
}

string NamedPipe::read(unsigned len)
{
	int status;
	char buf[MAXBUFLEN];

	memset(buf, 0, MAXBUFLEN);
	status = read(m_fifofd, buf, len);
	if (status == -1)
	{
		cerr << "NamedPipe::read::read" << endl;
		throw RuntimeError();
	}
	string newStr(buf);
	return newStr;
}

void NamedPipe::write(const string& text, unsigned len)
{
	int status;

	status = write(m_fifofd, text.c_str(), len);
	if (status == -1)
	{
		cerr << "NamedPipe::write::write" << endl;
		throw RuntimeError();
	}
}
