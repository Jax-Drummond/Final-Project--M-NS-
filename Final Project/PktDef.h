#pragma once
#include <memory>
#include "iostream"

class PktDef
{

public:
	/// <summary>
	/// The type of command
	/// </summary>
	enum CmdType
	{
		DRIVE,
		SLEEP,
		RESPONSE
	};

	/// <summary>
	/// The direction you want to move in
	/// </summary>
	enum Direction : unsigned char
	{
		FORWARD = 1,
		BACKWARD = 2,
		RIGHT = 3,
		LEFT = 4
	};
	
#pragma pack(push, 1)
	/// <summary>
	/// Header of the packet
	/// </summary>
	struct Header
	{
		unsigned short PktCount;
		unsigned char Drive : 1;
		unsigned char Status : 1;
		unsigned char Sleep : 1;
		unsigned char Ack : 1;
		unsigned char Padding : 4;
		unsigned char Length;
	} Head;
#pragma pack(pop)

	/// <summary>
	/// For use in sending a drive command
	/// </summary>
#pragma pack(push, 1)
	struct DriveBody
	{
		Direction direction;
		unsigned char duration;
		unsigned char speed;
		
	} DriveBody;
#pragma pack(pop)

	/// <summary>
	/// For use in getting the status of the bot
	/// </summary>
	struct TelemBody
	{
		unsigned short int LastPktCounter;
		unsigned short int CurrentGrade;
		unsigned short int HitCount;
		unsigned char LastCmd;
		unsigned char LastCmdValue;
		unsigned char LastCmdSpeed;
	}TelemBody;

	/// <summary>
	/// Default constructor
	/// </summary>
	PktDef() : RawBuffer(nullptr)
	{
		CmdPacket.Header.PktCount = 0;
		CmdPacket.Header.Drive = 0;
		CmdPacket.Header.Status = 0;
		CmdPacket.Header.Sleep = 0;
		CmdPacket.Header.Ack = 0;
		CmdPacket.Header.Padding = 0;
		CmdPacket.Header.Length = BASEPKTSIZE;
		CmdPacket.Data = nullptr;
		CmdPacket.CRC = 0;
	}

	/// <summary>
	/// Creates a packet from recieved data
	/// </summary>
	/// <param name="src">The src data</param>
	PktDef(char* src)
	{
		memcpy(&CmdPacket.Header, src, sizeof(Head));

		int bodySize = CmdPacket.Header.Length - BASEPKTSIZE;

		if (bodySize != 0) {

			if (CmdPacket.Header.Ack == 1 && CmdPacket.Header.Status == 1)
			{
				memcpy(&TelemBody, src + sizeof(Head), bodySize);
			}
			else if (CmdPacket.Header.Ack == 1 && CmdPacket.Header.Drive == 1)
			{
				CmdPacket.Data = new char[bodySize];
				memcpy(CmdPacket.Data, src + sizeof(Head), bodySize);
			}
		}

		memcpy(&CRC, src + CmdPacket.Header.Length - 1, sizeof(CRC));

		CheckCRC(src, CmdPacket.Header.Length - 1);
	}

	/// <summary>
	/// Sets the cmd of the packet
	/// </summary>
	/// <param name="type">The type of command</param>
	void SetCmd(CmdType type)
	{
		switch (type)
		{
		case PktDef::DRIVE:
			CmdPacket.Header.Drive = 1;
			break;
		case PktDef::SLEEP:
			CmdPacket.Header.Sleep = 1;
			break;
		case PktDef::RESPONSE:
			CmdPacket.Header.Status = 1;
			break;
		default:
			break;
		}
		CmdPacket.Header.Ack = 1;
	}

	/// <summary>
	/// Sets the body data of the packet
	/// </summary>
	/// <param name="buffer">The data buffer</param>
	/// <param name="size">The size of the buffer</param>
	void SetBodyData(char* buffer, int size)
	{
		if (CmdPacket.Data != nullptr) 
		{
			delete[] CmdPacket.Data;
		}

		if (buffer == nullptr || size <= 0) 
		{
			CmdPacket.Data = nullptr;
			CmdPacket.Header.Length = BASEPKTSIZE;
			return;
		}

		CmdPacket.Data = new char[size];
		memcpy(CmdPacket.Data, buffer, size);
		CmdPacket.Header.Length = BASEPKTSIZE + size;
	}

	/// <summary>
	/// Sets the packet count of the buffer
	/// </summary>
	/// <param name="count"></param>
	void SetPktCount(int count)
	{
		CmdPacket.Header.PktCount = count;
	}

	/// <summary>
	/// Gets the current cmd type of the packet
	/// </summary>
	/// <returns>CmdType</returns>
	CmdType GetCmd()
	{
		if (CmdPacket.Header.Drive)
		{
			return DRIVE;
		}
		else if (CmdPacket.Header.Sleep)
		{
			return SLEEP;
		}
		else
		{
			return RESPONSE;
		}
	}

	/// <summary>
	/// Gets whether or not the packet is acked
	/// </summary>
	/// <returns>bool</returns>
	bool GetAck()
	{
		return (bool)CmdPacket.Header.Ack;
	}

	/// <summary>
	/// Gets the total size of the packet
	/// </summary>
	/// <returns>int</returns>
	int GetLength()
	{
		return CmdPacket.Header.Length;
	}

	/// <summary>
	/// Gets the data of the body
	/// </summary>
	/// <returns>char*</returns>
	char* GetBodyData()
	{
		return CmdPacket.Data;
	}

	/// <summary>
	/// Gets the current pkt count
	/// </summary>
	/// <returns>pkt count</returns>
	int GetPktCount()
	{
		return CmdPacket.Header.PktCount;
	}

	/// <summary>
	/// Checks a raw buffer with the current crc to see if they match
	/// </summary>
	/// <param name="buffer">The raw buffer</param>
	/// <param name="size">The size of the buffer</param>
	/// <returns>bool</returns>
	bool CheckCRC(char* buffer, int size)
	{
		int totalParity = 0;

		for (int i = 0; i < size; ++i)
		{
			totalParity += CountOnes((unsigned char)buffer[i]);
		}

		return totalParity == CRC;
	}
	
	/// <summary>
	/// Sets the crc based on the bits
	/// </summary>
	void CalcCRC()
	{
		int totalParity = 0;

		unsigned char* headerBytes = (unsigned char*)&CmdPacket.Header;
		for (int i = 0; i < sizeof(Header); ++i)
		{
			totalParity += CountOnes(headerBytes[i]);
		}

		int bodySize = CmdPacket.Header.Length - sizeof(Header) - 1; 
		for (int i = 0; i < bodySize; ++i)
		{
			totalParity += CountOnes((unsigned char)CmdPacket.Data[i]);
		}

		CRC = totalParity;
	}

	/// <summary>
	///  Gets the current CRC
	/// </summary>
	/// <returns>int</returns>
	int GetCRC()
	{
		return CRC;
	}

	/// <summary>
	/// Counts the number of 1 bits
	/// </summary>
	/// <param name="byte">the byte to count</param>
	/// <returns>int</returns>
	int CountOnes(unsigned char byte)
	{
		int count = 0;
		while (byte)
		{
			count += byte & 1;
			byte >>= 1;
		}
		return count;
	}

	/// <summary>
	/// Serializes the Packet
	/// </summary>
	/// <returns>char*</returns>
	char* GenPacket()
	{
		if (RawBuffer)
		{
			delete[] RawBuffer;
		}
		RawBuffer = new char[CmdPacket.Header.Length];

		memcpy(RawBuffer, &CmdPacket.Header, sizeof(Head));

		if (CmdPacket.Header.Length > BASEPKTSIZE)
		{
			memcpy(RawBuffer + sizeof(Head), CmdPacket.Data, CmdPacket.Header.Length - BASEPKTSIZE);
		}

		memcpy(RawBuffer + CmdPacket.Header.Length - 1, &CRC, sizeof(CRC));

		return RawBuffer;
	}

private:
	/// <summary>
	/// The packet to be sent
	/// </summary>
	struct CmdPacket
	{
		struct Header Header;
		char* Data;
		char CRC;
	} CmdPacket;


	/// <summary>
	/// The tail of the packet
	/// </summary>
	char CRC;
	/// <summary>
	/// The raw buffer
	/// </summary>
	char* RawBuffer;
	/// <summary>
	/// The base size of the packet
	/// </summary>
	const int BASEPKTSIZE = 5;
};