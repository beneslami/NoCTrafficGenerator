#include "socketstream.h"

#include <cstdio>
#include <cerrno>

int SocketStream::connect(const char *host, int port)
{
    char *socket_path = "./socket";
    // Create a socket.
    if ( (so = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        std::cout << "Error creating socket." << std::endl;
        return -1;
    }
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(NS_HOST);
    addr.sin_port = htons(NS_PORT);

    // Connect to the server.
    if (::connect(so, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("connect");
        std::cout << "Connection failed." << std::endl;
        return -1;
    }

    // prevent small packets from getting stuck in OS queues
    //int on = 1;
    //setsockopt (so, SOL_TCP, TCP_NODELAY, &on, sizeof (on));

    bIsAlive = true;

#ifdef NS_DEBUG
    std::cout << "Connected to host" << std::endl;
#endif

    return 0;
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
