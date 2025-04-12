#include "PktDef.h"
#include "MySocket.h"
#include "iostream"

using namespace std;

void main()
{
	PktDef pkt;
	pkt.SetPktCount(1);
	pkt.SetCmd(PktDef::DRIVE);

	struct PktDef::DriveBody body;
	body.direction = PktDef::FORWARD;
	body.duration = 10;
	body.speed = 80;

	pkt.SetBodyData((char*)&body, sizeof(PktDef::DriveBody));

	pkt.CalcCRC();

	std::cout <<  "CRC: " << pkt.GetCRC() << std::endl;

	MySocket mySocket(SERVER, "127.0.0.1", 98009, TCP, 25);


	cout << "IP:" << mySocket.GetIPAddr() << endl;
	cout << "PORT:" << mySocket.GetPort() << endl;



}