#include "named_pipe.h"
#include "constants.h"
#include <unistd.h>
using namespace std;

NamedPipe::~NamedPipe()
{
	close(m_fifofd);
	unlink(FIFO_NAME);
}
