#ifndef SERVER_H
#define SERVER_H

class Server
{
public:
	virtual void run() = 0;
	virtual ~Server() {}
};

#endif  // SERVER_H
