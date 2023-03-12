// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <scene_rdl2/render/util/StrUtil.h>

#include <functional>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

class TlSvr
//
// This class provides the functionality of server side telnet connection. 
// Only support p2p (point to point) connection so far and not support multiple
// incoming input from multiple clients. Only support IPv4 so far.
// Using this class, it is very easy to implement interactive command line console
// functionality to the non interactive application.
//
// The following is a pseudo code of typical implementation for command line console by TlSvr.
// This implementation is one of the threads of the server process.
//
//    TlSvr svr;
//    if (!svr.open(20000)) return; // port is 20000
//
//    while (1) {
//        std::string cmdLine;
//        int recvByte = svr.recv(cmdLine);
//        if (recvByte == 0) {
//            usleep(10000); // 10ms sleep for yielding CPU resource
//        } else if (recvByte < 0) {
//            // error
//            break;
//        } else {
//            // parse cmdLine here and do something ...
//            svr.send("..test..test..test\n"); // you can send back string to the telnet client.
//            if (cmdLine == "exit") break;
//        }
//    }
//    svr.close();
//    
{
public:
    // These 2 callback definitions are used to define message display functionality.
    // We have 2 categories. info and error.
    // Info callback is called for information dump purposes like a new connection established.
    // Error callback is called when an error has happened internally.
    // You don't need to set a call back function if you don't want to.
    using INFOMSG_CALLBACK = std::function<void(const std::string &)>;
    using ERRMSG_CALLBACK = std::function<void(const std::string &)>;

    TlSvr();
    ~TlSvr();

    //
    // You can use serverPortNum = 0 for auto search of available port by the kernel.
    // In this case, you can figure out the result port number by the return value of this API.
    // Port is not opened until when it is needed (delayed open). If you specify a port number
    // for the open API, this port is not opened until the very first recv operation.
    // This open API only supports internet domain socket and not unix domain.
    //
    // If you don't call open(), all other public API internally skip all socket related execution.
    // This means skip open() is an easiest way to disable all TlSvr functionality.
    //
    int open(const int serverPortNum,
             INFOMSG_CALLBACK infoMsgCallBack = nullptr,
             ERRMSG_CALLBACK errMsgCallBack = nullptr); // return opened port number. 0 is error

    // non blocking receive : return recv byte or 0:empty -1:EOF -2:otherError
    int recv(std::string & recvStr,
             INFOMSG_CALLBACK infoMsgCallBack = nullptr,
             ERRMSG_CALLBACK errMsgCallBack = nullptr);

    // blocking send
    // Send only works after established socket connection by recv API.
    bool send(const std::string & sendStr,
              INFOMSG_CALLBACK infoMsgCallBack = nullptr,
              ERRMSG_CALLBACK errMsgCallBack = nullptr);

    void close();

    bool isConnectionEstablised() const; // true:establised false:not_yet

protected:
    bool setupServerPort(INFOMSG_CALLBACK infoMsgCallBack, ERRMSG_CALLBACK errMsgCallBack);
    bool socketBindAndListen(INFOMSG_CALLBACK infoMsgCallBack, ERRMSG_CALLBACK errMsgCallBack);
    bool acceptSocket(INFOMSG_CALLBACK infoMsgCallBack, ERRMSG_CALLBACK errMsgCallBack);
    void connectionClosed(INFOMSG_CALLBACK infoMsgCallBack);
    int  socketCheck(ERRMSG_CALLBACK errMsgCallBack);

    int mPort;                  // server port
    int mBaseSock;              // server socket fd
    int mSock;                  // incoming socket fd
    bool mConnectionReady;      // connection-established(true) or not(false)
    int mRecvSize;              // received data size
    bool mEndLine;              // end line condition
};

} // namespace grid_util
} // namespace scene_rdl2

