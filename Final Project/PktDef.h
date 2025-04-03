#pragma once
#include <memory>


class PktDef
{

public:

	enum CmdType
	{
		DRIVE,
		SLEEP,
		RESPONSE
	};

	enum Direction
	{
		FORWARD = 1,
		BACKWARD = 2,
		RIGHT = 3,
		LEFT = 4
	};

	struct Header
	{
		unsigned short int PktCount;
		int Drive : 1;
		int Status : 1;
		int Sleep : 1;
		int Ack : 1;
		int Padding : 4;
		unsigned short int Length;
	} Head;

	struct DriveBody
	{
		Direction direction : 8;
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
		Head.PktCount = 0;
		Head.Drive = 0;
		Head.Status = 0;
		Head.Sleep = 0;
		Head.Ack = 0;
		Head.Padding = 0;
		Head.Length = 0;
		CmdPacket.Data = nullptr;
		CmdPacket.CRC = 0;
	}

	PktDef(char* src)
	{
		memcpy(&Head, src, sizeof(Head));

		if (Head.Length > BASEPKTSIZE && Head.Ack == 1 && Head.Status = 1)
		{
			memcpy(&TelemBody, src + sizeof(Head), Head.Length - BASEPKTSIZE);
		}

		memcpy(&CRC, src + Head.Length - 1, sizeof(CRC));
	}

	void SetCmd(CmdType type)
	{
		switch (type)
		{
		case PktDef::DRIVE:
			Head.Drive = 1;
			break;
		case PktDef::SLEEP:
			Head.Sleep = 1;
			break;
		case PktDef::RESPONSE:
			Head.Status = 1;
			break;
		default:
			break;
		}
	}

	void SetBodyData(char* buffer, int size)
	{
		CmdPacket.Data = new char[size];
		memcpy(CmdPacket.Data, buffer, size);
		Head.Length = BASEPKTSIZE + size;
	}

	void SetPktCount(int count)
	{
		Head.PktCount = count;
	}

	CmdType GetCmd()
	{
		if (Head.Drive)
		{
			return DRIVE;
		}
		else if (Head.Sleep)
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
		return (bool)Head.Ack;
	}

	int GetLength()
	{
		return Head.Length;
	}

	char* GetBodyData()
	{
		return CmdPacket.Data;
	}

	int GetPktCount()
	{
		return Head.PktCount;
	}

	bool CheckCRC(char* buffer, int size)
	{
		int currentParity = 0;

		// TODO: Binary stuff

		if (currentParity == CRC)
		{
			return true;
		}
		return false;
	}

	void CalcCRC()
	{
		int currentParity = 0;

		// TODO: Binary Stuff

		CRC = currentParity;
	}

	char* GenPacket()
	{
		if (RawBuffer)
		{
			delete[] RawBuffer;
		}
		RawBuffer = new char[Head.Length];

		memcpy(RawBuffer, &Head, sizeof(Head));

		if (Head.Length > BASEPKTSIZE)
		{
			memcpy(RawBuffer + sizeof(Head), &DriveBody, Head.Length - BASEPKTSIZE);
		}

		memcpy(RawBuffer + Head.Length - 1, &CRC, sizeof(CRC));

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