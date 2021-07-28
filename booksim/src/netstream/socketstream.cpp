#include "socketstream.h"

#include <cstdio>
#include <cerrno>

int SocketStream::listen(const char *host, int port)
{
	char *socket_path = "./socket";

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create a socket
    if ( (so = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Error creating socket." << endl;
        return -1;
    }

	// Bind it to the listening port
	unlink(socket_path);
	if (::bind(so, (struct sockaddr*)&addr, sizeof(addr)) != false) {
		 cout << "Error binding socket." << endl;
		 return -1;
	}
	// Listen for connections
	if (::listen(so, NS_MAX_PENDING) != 0) {
		 cout << "Error listening on socket." << endl;
		 return -1;
	}
	bIsAlive = true;

#ifdef NS_DEBUG
	cout << "Listening on socket" << endl;
#endif

	return 0;
}

// accept a new connection
SocketStream* SocketStream::accept()
{
	struct sockaddr_in clientaddr;
	socklen_t clientaddrlen = sizeof clientaddr;
	int clientsock = ::accept(so, (struct sockaddr*)&clientaddr, &clientaddrlen);
	if ( clientsock < 0 ){
    	cout << "Error accepting a connection";
    	return NULL;
	}
	// prevent small packets from getting stuck in OS queues
	//int on = 1;
	//setsockopt (so, SOL_TCP, TCP_NODELAY, &on, sizeof (on));

	return new SocketStream(clientsock, (struct sockaddr*)&clientaddr, clientaddrlen);
}


// read from the socket
int SocketStream::get(void *data, int number)
{

    int remaining = number;
    int received = 0;
    char *dataRemaining = (char*) data;

    errno = 0;
    while (remaining > 0 && (errno == 0 || errno == EINTR))
    {
        received = recv(so, dataRemaining, remaining, 0); // MSG_WAITALL
        if (received > 0)
        {
            dataRemaining += received;
            remaining -= received;
        }
    }

    return number - remaining;
}

// write to socket
int SocketStream::put(const void *data, int number)
{
    // MSG_NOSIGNAL prevents SIGPIPE signal from being generated on failed send
    return send(so, data, number, MSG_NOSIGNAL);
}
