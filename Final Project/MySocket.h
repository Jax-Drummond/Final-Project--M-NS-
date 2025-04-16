#pragma once
#include <string>
#include <cstring>
#include "PktDef.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#endif

using namespace std;

/// <summary>
/// The Type of Socket
/// </summary>
enum SocketType { CLIENT, SERVER };

/// <summary>
/// The Type of Connection
/// </summary>
enum ConnectionType { TCP, UDP };

const int DEFAULT_SIZE = 14;

class MySocket
{
private:
    char* Buffer;
    int WelcomeSocket = -1;
    int ConnectionSocket = -1;
    struct sockaddr_in SvrAddr;
    SocketType mySocket;
    string IPAddr;
    int Port;
    ConnectionType connectionType;
    bool bTCPConnect = false;
    int MaxSize;
    int status = 0;

#ifdef _WIN32
    static bool initialized;
    static void EnsureWSAStartup()
    {
        if (!initialized)
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            {
                cerr << "WSAStartup failed!" << endl;
            }
            else
            {
                initialized = true;
            }
        }
    }
#endif

public:
    MySocket(SocketType socketType, string Ip, unsigned int port, ConnectionType connectionType, unsigned int bufferSize)
    {
#ifdef _WIN32
        EnsureWSAStartup();
#endif
        mySocket = socketType;
        IPAddr = Ip;
        Port = port;
        this->connectionType = connectionType;
        MaxSize = bufferSize <= 0 ? DEFAULT_SIZE : bufferSize;
        Buffer = new char[MaxSize];

        memset(&SvrAddr, 0, sizeof(SvrAddr));
        SvrAddr.sin_family = AF_INET;
        SvrAddr.sin_port = htons(Port);
        inet_pton(AF_INET, IPAddr.c_str(), &SvrAddr.sin_addr);

        switch (connectionType)
        {
        case TCP:
            ConnectTCP();
            break;
        case UDP:
            ConnectionSocket = socket(AF_INET, SOCK_DGRAM, 0);
            #ifdef _WIN32
            DWORD timeout = 5 * 1000 + 0 / 1000;
            setsockopt(ConnectionSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        #else
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            setsockopt(ConnectionSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        #endif
            if (mySocket == SERVER)
            {
                bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
            }
            break;
        }
    }

    ~MySocket()
    {
        delete[] Buffer;
#ifdef _WIN32
        if (WelcomeSocket != -1) closesocket(WelcomeSocket);
        if (ConnectionSocket != -1) closesocket(ConnectionSocket);
#else
        if (WelcomeSocket != -1) close(WelcomeSocket);
        if (ConnectionSocket != -1) close(ConnectionSocket);
#endif
    }

    void ConnectTCP()
    {
        if (connectionType == UDP) return;

        if (mySocket == CLIENT)
        {
            ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == 0)
            {
                status = 1;
                bTCPConnect = true;
            }
            else
            {
                status = -1;
                perror("TCP Connect failed");
            }
        }
        else if (mySocket == SERVER)
        {
            WelcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
            bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
            listen(WelcomeSocket, 1);
            socklen_t addr_size = sizeof(SvrAddr);
            ConnectionSocket = accept(WelcomeSocket, (struct sockaddr*)&SvrAddr, &addr_size);
            if (ConnectionSocket >= 0)
            {
                status = 1;
                bTCPConnect = true;
            }
            else
            {
                status = -1;
                perror("TCP Accept failed");
            }
        }
    }

    void DisconnectTCP() {
        if (connectionType == UDP) return;

#ifdef _WIN32
        if (ConnectionSocket != -1) closesocket(ConnectionSocket);
#else
        if (ConnectionSocket != -1) close(ConnectionSocket);
#endif
        ConnectionSocket = -1;
        bTCPConnect = false;
    }

    void SendData(const char* data, int size) {

        CROW_LOG_DEBUG << "ConnectionSocket: " << to_string(ConnectionSocket);
        
        if (connectionType == TCP)
        {
            send(ConnectionSocket, data, size, 0);
        }
        else
        {
            sendto(ConnectionSocket, data, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
        }
    }

    int GetData(char* buffer)
    {
        int bytesReceived = 0;
        memset(Buffer, 0, MaxSize);

        if (connectionType == TCP)
        {
            bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
        }
        else
        {
            socklen_t addrLen = sizeof(SvrAddr);
            bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrLen);
        }

        if (bytesReceived > 0)
        {
            memcpy(buffer, Buffer, bytesReceived);
        }

        return bytesReceived;
    }

    int GetStatus()
    {
        return status;
    }

    string GetIPAddr() { return IPAddr; }

    void SetIPAdr(string ip)
    {
        if (!bTCPConnect) IPAddr = ip;
        else cerr << "ERROR: Can't set IP. Connection already established." << endl;
    }

    void SetPort(int port)
    {
        if (!bTCPConnect) Port = port;
        else cerr << "ERROR: Can't set port. Connection already established." << endl;
    }

    int GetPort() { return Port; }

    SocketType GetType() { return mySocket; }

    void SetType(SocketType type)
    {
        if (!bTCPConnect) mySocket = type;
        else cerr << "ERROR: Can't set socket type. Connection already established." << endl;
    }
};

#ifdef _WIN32
bool MySocket::initialized = false;
#endif