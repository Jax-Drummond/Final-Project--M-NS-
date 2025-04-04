#include "PktDef.h"
#include "iostream"

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

	std::cout << pkt.GetCRC() << std::endl;

}