#include "server_parent_connection.h"
#include "parent_server.h"
#include "../constants.h"

int main()
{
	ServerParentConnection spc(PARENT_PORT);
	ParentServer ps(spc);
	ps.run();

	return 0;
}
