#pragma once
#include <string>
#include "sys/socket.h"
#include "netinet/in.h"
#include <arpa/inet.h>
#include "PktDef.h"

using namespace std;
/// <summary>
/// The Type of Socket
/// </summary>
enum SocketType
{
	CLIENT,
	SERVER
};

/// <summary>
/// The Type of Connection
/// </summary>
enum ConnectionType
{
	TCP,
	UDP
};

const int DEFAULT_SIZE = 14; // Don't know yet


class MySocket
{
private:
	char* Buffer;
	// WelcomeSocket and ConnectionSocket go somewhere here
	int WelcomeSocket = -1;
	int ConnectionSocket= - 1;
	struct sockaddr_in SvrAddr; // Store Connection information
	SocketType mySocket; // Type of the socket
	string IPAddr; // IPv4 address
	int Port; // Port number to be used
	ConnectionType connectionType; // Defines the Transport layer protocol being used
	bool bTCPConnect = false; // Flag to determine if a connection has been established
	int MaxSize; // Stores the maximum number of bytes the buffer is allocated to. Helps prevent overflow and sync issues.
public:
	MySocket(SocketType socketType, string Ip, unsigned int port, ConnectionType connectionType, unsigned int bufferSize)
	{
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
			if (mySocket == SERVER) {
				bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
			}
			break;
		default:
			break;
		}
		
		// Some more stuff I think

	}

	~MySocket()
	{
		delete[] Buffer;
		if (WelcomeSocket != -1) close(WelcomeSocket);
		if (ConnectionSocket != -1) close(ConnectionSocket);
	}

	/// <summary>
	/// Establish a TCP/IP socket connection
	/// </summary>
	void ConnectTCP()
	{
		if (connectionType == UDP) {
            cerr << "ConnectTCP() called on a UDP socket." << endl;
            return;
        }

        if (mySocket == CLIENT) {
			ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == 0) {
                bTCPConnect = true;
            } else {
                perror("TCP Connect failed");
            }
        } else if (mySocket == SERVER) {
			bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
			listen(WelcomeSocket, 1);
			socklen_t addr_size = sizeof(SvrAddr);
            ConnectionSocket = accept(WelcomeSocket, (struct sockaddr*)&SvrAddr, &addr_size);
            if (ConnectionSocket >= 0) {
                bTCPConnect = true;
            } else {
                perror("TCP Accept failed");
            }
        }
	}

	/// <summary>
	/// Disconnect an established TCP/IP socket connection
	/// </summary>
	void DisconnectTCP()
	{
		if (connectionType == UDP) return;
		
		if (ConnectionSocket != -1) {
			close(ConnectionSocket);
			ConnectionSocket = -1;
		}
		bTCPConnect = false;
	}

	/// <summary>
	/// Used to transmit data over the socket
	/// </summary>
	/// <param name="data">The data to send</param>
	/// <param name="size">The size of the data</param>
	void SendData(const char* data, int size)
	{
		if (connectionType == TCP) {
			send(ConnectionSocket, data, size, 0);
		}
		else {
			sendto(ConnectionSocket, data, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
		}
	}

	/// <summary>
	/// Used to get last recieved data stored in buffer
	/// </summary>
	/// <param name="buffer">Where you want it saved</param>
	/// <returns>Bytes written</returns>
	int GetData(char* buffer)
	{
		int bytesReceived = 0;
		memset(Buffer, 0, MaxSize);

		if (connectionType == TCP) {
			bytesReceived = recv(ConnectionSocket, Buffer, MaxSize, 0);
		}
		else {
			socklen_t addrLen = sizeof(SvrAddr);
			bytesReceived = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addrLen);
		}

		if (bytesReceived > 0) {
			memcpy(buffer, Buffer, bytesReceived);
		}

		return bytesReceived;
	}

	/// <summary>
	/// Gets the configured ip address
	/// </summary>
	/// <returns></returns>
	string GetIPAddr()
	{
		return IPAddr;
	}

	/// <summary>
	/// Sets the IP address
	/// </summary>
	/// <param name="ip">The ip to be set</param>
	void SetIPAdr(string ip)
	{
		if (!bTCPConnect)
		{
			IPAddr = ip;
		}
		else
		{
			cerr << "ERROR: Can't set ip. Connection has already been established." << endl;
		}
	}

	/// <summary>
	/// Sets the port
	/// </summary>
	/// <param name="port">The port to be set</param>
	void SetPort(int port)
	{
		if (!bTCPConnect)
		{
			Port = port;
		}
		else
		{
			cerr << "ERROR: Can't set port. Connection has already been established." << endl;
		}
	}

	/// <summary>
	/// Gets the currently set port
	/// </summary>
	/// <returns>The port</returns>
	int GetPort()
	{
		return Port;
	}

	/// <summary>
	/// Gets the current socket type
	/// </summary>
	/// <returns>SocketType</returns>
	SocketType GetType()
	{
		return mySocket;
	}

	/// <summary>
	/// Sets the current socket type
	/// </summary>
	/// <param name="type">The type to be set</param>
	void SetType(SocketType type)
	{
		if (!bTCPConnect) {
			mySocket = type;
		}
		else {
			cerr << "ERROR: Can't set socket type. Connection already established." << endl;
		}
	}

};