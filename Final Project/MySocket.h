#pragma once
#include <string>
#include <netinet/in.h>
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

const int DEFAULT_SIZE = 9; // Don't know yet


class MySocket
{
private:
	char* Buffer;
	// WelcomeSocket and ConnectionSocket go somewhere here
	struct sockaddr_in SvrAddr; // Store Connection information
	SocketType mySocket; // Type of the socket
	string IPAddr; // IPv4 address
	int Port; // Port number to be used
	ConnectionType connectionType; // Defines the Transport layer protocol being used
	bool bTCPConnect; // Flag to determine if a connection has been established
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

		// Some more stuff I think

	}

	~MySocket()
	{
		delete[] Buffer;
	}

	/// <summary>
	/// Establish a TCP/IP socket connection
	/// </summary>
	void ConnectTCP()
	{
		if (connectionType == UDP) return;
		// TCP code here
	}

	/// <summary>
	/// Disconnect an established TCP/IP socket connection
	/// </summary>
	void DisconnectTCP()
	{
		if (connectionType == UDP) return;
		// TCP code here
	}

	/// <summary>
	/// Used to transmit data over the socket
	/// </summary>
	/// <param name="data">The data to send</param>
	/// <param name="size">The size of the data</param>
	void SendData(const char* data, int size)
	{
		// Send data here
	}

	/// <summary>
	/// Used to get last recieved data stored in buffer
	/// </summary>
	/// <param name="buffer">Where you want it saved</param>
	/// <returns>Bytes written</returns>
	int GetData(char* buffer)
	{
		// Things go on here
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
		mySocket = type;
	}



};