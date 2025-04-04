#pragma once
#include <memory>
#include "iostream"

class PktDef
{

public:

	enum CmdType
	{
		DRIVE,
		SLEEP,
		RESPONSE
	};

	enum Direction : unsigned char
	{
		FORWARD = 1,
		BACKWARD = 2,
		RIGHT = 3,
		LEFT = 4
	};

#pragma pack(push, 1)
	struct Header
	{
		unsigned short PktCount;
		unsigned char Drive : 1;
		unsigned char Status : 1;
		unsigned char Sleep : 1;
		unsigned char Ack : 1;
		unsigned char Padding : 4;
		unsigned short Length;
	} Head;
#pragma pack(pop)

	struct DriveBody
	{
		Direction direction;
		unsigned char duration;
		unsigned char speed;
	} DriveBody;

	struct TelemBody
	{
		unsigned short int LastPktCounter;
		unsigned short int CurrentGrade;
		unsigned short int HitCount;
		unsigned char LastCmd;
		unsigned char LastCmdValue;
		unsigned char LastCmdSpeed;
	}TelemBody;

	PktDef() : RawBuffer(nullptr)
	{
		CmdPacket.Header.PktCount = 0;
		CmdPacket.Header.Drive = 0;
		CmdPacket.Header.Status = 0;
		CmdPacket.Header.Sleep = 0;
		CmdPacket.Header.Ack = 0;
		CmdPacket.Header.Padding = 0;
		CmdPacket.Header.Length = 0;
		CmdPacket.Data = nullptr;
		CmdPacket.CRC = 0;
	}

	PktDef(char* src)
	{
		memcpy(&CmdPacket.Header, src, sizeof(Head));

		if (CmdPacket.Header.Length > BASEPKTSIZE && CmdPacket.Header.Ack == 1 && CmdPacket.Header.Status == 1)
		{
			memcpy(&TelemBody, src + sizeof(Head), CmdPacket.Header.Length - BASEPKTSIZE);
		}

		memcpy(&CRC, src + CmdPacket.Header.Length - 1, sizeof(CRC));

		CheckCRC(src, CmdPacket.Header.Length - 1);
	}

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

	void SetBodyData(char* buffer, int size)
	{
		CmdPacket.Data = new char[size];
		memcpy(CmdPacket.Data, buffer, size);
		CmdPacket.Header.Length = BASEPKTSIZE + size;
	}

	void SetPktCount(int count)
	{
		CmdPacket.Header.PktCount = count;
	}

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

	bool GetAck()
	{
		return (bool)CmdPacket.Header.Ack;
	}

	int GetLength()
	{
		return CmdPacket.Header.Length;
	}

	char* GetBodyData()
	{
		return CmdPacket.Data;
	}

	int GetPktCount()
	{
		return CmdPacket.Header.PktCount;
	}

	bool CheckCRC(char* buffer, int size)
	{
		int totalParity = 0;

		for (int i = 0; i < size; ++i)
		{
			totalParity += CountOnes((unsigned char)buffer[i]);
		}

		return totalParity == CRC;
	}

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

	int GetCRC()
	{
		return CRC;
	}

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
			memcpy(RawBuffer + sizeof(Head), &DriveBody, CmdPacket.Header.Length - BASEPKTSIZE);
		}

		memcpy(RawBuffer + CmdPacket.Header.Length - 1, &CRC, sizeof(CRC));

		return RawBuffer;
	}

private:
	struct CmdPacket
	{
		struct Header Header;
		char* Data;
		char CRC;
	} CmdPacket;

	char CRC;
	char* RawBuffer;

	const int BASEPKTSIZE = 6;
};