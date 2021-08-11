#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "socketstream.h"
#include <stdint.h>

#define INVALID_MESSAGE -1
#define INITIALIZE_REQ  0
#define INITIALIZE_RES  1
#define STEP_REQ  		2
#define STEP_RES  		3
#define INJECT_REQ  	4
#define INJECT_RES  	5
#define EJECT_REQ  		6
#define EJECT_RES  		7
#define QUIT_REQ  		8
#define QUIT_RES  		9
#define ACKNOWLEDGE     10

struct StreamMessage
{
    int size;
    int type;
    StreamMessage() :
        size(-1), type(INVALID_MESSAGE)
    {
    }

    /*
     * Message sending function.
     */
    friend SocketStream& operator<<(SocketStream& os, StreamMessage& msg);

    /*
     * Message receiving function. Use this function if you don't know the message type in advance
     * NOTE: must destroy msg using Message::destroy(Message*)
     */
    friend SocketStream& operator>>(SocketStream& is, StreamMessage*& msg);

    /*
     * Message receiving function. Use this function if you know the message type in advance
     */
    friend SocketStream& operator>>(SocketStream& is, StreamMessage& msg);

    static void destroy(StreamMessage* msg);
};

struct InitializeReqMsg: StreamMessage
{
    InitializeReqMsg()
    {
        size = sizeof(InitializeReqMsg);
        type = INITIALIZE_REQ;
    }
};

struct InitializeResMsg: StreamMessage
{
    InitializeResMsg()
    {
        size = sizeof(InitializeResMsg);
        type = INITIALIZE_RES;
    }
};

struct StepReqMsg: StreamMessage
{
	StepReqMsg()
    {
        size = sizeof(StepReqMsg);
        type = STEP_REQ;
    }
};

struct StepResMsg: StreamMessage
{
	StepResMsg()
    {
        size = sizeof(StepResMsg);
        type = STEP_RES;
    }
};

struct InjectReqMsg: StreamMessage
{
	InjectReqMsg()
    {
        size = sizeof(InjectReqMsg);
        type = INJECT_REQ;
    }
    int source;
    int dest;
    int id;
    int packetSize;
    int network;
    int cl;
    int msgType;
    unsigned long long address;
};

struct InjectResMsg: StreamMessage
{
	InjectResMsg()
    {
        size = sizeof(InjectResMsg);
        type = INJECT_RES;
    }
    int source;
    int dest;
    int id;
    int packetSize;
    int network;
    int cl;
    int msgType;
    unsigned long long address;
};

struct EjectReqMsg: StreamMessage
{
	EjectReqMsg()
    {
        size = sizeof(EjectReqMsg);
        type = EJECT_REQ;
    }
    int coreNum;
};

struct EjectResMsg: StreamMessage
{
	EjectResMsg()
    {
        size = sizeof(EjectResMsg);
        type = EJECT_RES;
    }
    int source;
    int dest;
    int id;
    int packetSize;
    int network;
    int cl;
    int msgType;
    unsigned long long address;
    int remainingRequests;

};

struct QuitReqMsg: StreamMessage
{
	QuitReqMsg()
    {
        size = sizeof(QuitReqMsg);
        type = QUIT_REQ;
    }
    int flag;
};

struct QuitResMsg: StreamMessage
{
	QuitResMsg()
    {
        size = sizeof(QuitResMsg);
        type = QUIT_RES;
    }
    int flag;
};

#endif
