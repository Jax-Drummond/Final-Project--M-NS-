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
	body.duration = 15;
	body.speed = 80;

	cout << "body size " << sizeof(body) << endl;
	pkt.SetBodyData((char*)&body, sizeof(body)); // add 1

	pkt.CalcCRC();

	MySocket mySocket(CLIENT, "127.0.0.1", 59822, UDP, 25);

	mySocket.SendData(pkt.GenPacket(), pkt.GetLength());

	char RX[5];

	mySocket.GetData(RX);

	PktDef pkt2(RX);

	cout << "RX Ack: " << pkt2.GetAck() << endl;
	cout << "RX Drive: " << to_string(pkt2.Head.Drive) << endl;
	cout << "RX PktCount: " << pkt2.GetPktCount() << endl;



}