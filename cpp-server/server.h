#ifndef SERVER_H
#define SERVER_H

class Server
{
public:
	Server() {}
	virtual ~Server() {}
	virtual void run() = 0;
};

#endif  // SERVER_H
