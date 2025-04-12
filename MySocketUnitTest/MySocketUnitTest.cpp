#include "pch.h"
#include "CppUnitTest.h"
#include "MySocket.h"
#include "MockWinSock.h"
#include <string>
#include <cstring>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {

			template<>
			std::wstring ToString<SocketType>(const SocketType& type)
			{
				switch (type)
				{
				case SocketType::CLIENT:
					return L"CLIENT";
				case SocketType::SERVER:
					return L"SERVER";
				default:
					return L"UNKNOWN";
				}
			}
		}
	}
}

namespace MySocketUnitTest
{

	TEST_CLASS(MySocketUnitTest)
	{
	public:
		
		TEST_METHOD(Constructor_TCP_Client)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8080, TCP, 1024);
			Assert::AreEqual(string("127.0.0.1"), sock.GetIPAddr());
			Assert::AreEqual(8080, sock.GetPort());
			Assert::AreEqual((int)CLIENT, (int)sock.GetType());
		}

		TEST_METHOD(Constructor_UDP_Server)
		{
			MySocket sock(SERVER, "127.0.0.1", 9090, UDP, 512);
			Assert::AreEqual(string("127.0.0.1"), sock.GetIPAddr());
			Assert::AreEqual(9090, sock.GetPort());
		}

		TEST_METHOD(SetIPBeforeConnect)
		{
			MySocket sock(CLIENT, "127.0.0.1", 1234, TCP, 256);
			sock.DisconnectTCP();
			sock.SetIPAdr("192.168.1.100");
			Assert::AreEqual(string("192.168.1.100"), sock.GetIPAddr());
		}

		TEST_METHOD(SetPortBeforeConnect)
		{
			MySocket sock(CLIENT, "127.0.0.1", 1234, TCP, 256);
			sock.DisconnectTCP();
			sock.SetPort(4321);
			Assert::AreEqual(4321, sock.GetPort());
		}

		TEST_METHOD(SetTypeBeforeConnect)
		{
			MySocket sock(CLIENT, "127.0.0.1", 1234, TCP, 256);
			sock.DisconnectTCP();
			sock.SetType(SERVER);
			Assert::AreEqual((int)SERVER, (int)sock.GetType());
		}

		TEST_METHOD(SendData_TCP)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8888, TCP, 128);
			const char* msg = "TestData";
			sock.SendData(msg, strlen(msg));
			// Send is mocked to return 5
		}

		TEST_METHOD(SendData_UDP)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8888, UDP, 128);
			const char* msg = "UDPData";
			sock.SendData(msg, strlen(msg));
		}

		TEST_METHOD(GetData_TCP)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8888, TCP, 128);
			char buffer[128] = { 0 };
			int bytes = sock.GetData(buffer);
			Assert::AreEqual(5, bytes);
			Assert::AreEqual(0, strncmp(buffer, "hello", 5));
		}

		TEST_METHOD(GetData_UDP)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8888, UDP, 128);
			char buffer[128] = { 0 };
			int bytes = sock.GetData(buffer);
			Assert::AreEqual(5, bytes);
			Assert::AreEqual(0, strncmp(buffer, "world", 5));
		}

		TEST_METHOD(DisconnectTCP_Test)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8888, TCP, 128);
			sock.DisconnectTCP();
		}
		
		TEST_METHOD(TCP_Server_Connect)
		{
			MySocket sock(SERVER, "127.0.0.1", 8081, TCP, 1024);
			sock.ConnectTCP(); // triggers bind, listen, accept
			Assert::AreEqual((int)SERVER, (int)sock.GetType());
		}

		TEST_METHOD(SetIPAfterConnect_ShouldFail)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8084, TCP, 1024);
			sock.SetIPAdr("192.168.1.200"); // should not override since connection is active

			// Should still be original IP
			Assert::AreEqual(string("127.0.0.1"), sock.GetIPAddr());
		}

		TEST_METHOD(SetPortAfterConnect_ShouldFail)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8085, TCP, 1024);
			sock.SetPort(9999); // should be ignored due to active connection

			Assert::AreEqual(8085, sock.GetPort());
		}

		TEST_METHOD(SetTypeAfterConnect_ShouldFail)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8086, TCP, 1024);
			sock.SetType(SERVER); // should be blocked

			Assert::AreEqual((int)CLIENT, (int)sock.GetType());
		}

		TEST_METHOD(Buffer_DefaultSize_WhenZero)
		{
			MySocket sock(CLIENT, "127.0.0.1", 8087, TCP, 0); // should use DEFAULT_SIZE = 14

			char buffer[32] = { 0 };
			int bytes = sock.GetData(buffer);
			Assert::AreEqual(5, bytes); // recv() always mocked to 5
		}

		TEST_METHOD(TCP_Client_Connect_Failure)
		{
			extern int mock_connect_result;
			mock_connect_result = -1;

			MySocket sock(CLIENT, "127.0.0.1", 8082, TCP, 1024);

			// Should not crash, but connection won't be established
			mock_connect_result = 0; // reset for other tests
		}

		TEST_METHOD(TCP_Server_Accept_Failure)
		{
			extern int mock_accept_result;
			mock_accept_result = -1;

			MySocket sock(SERVER, "127.0.0.1", 8083, TCP, 1024);
			sock.ConnectTCP(); // triggers failed accept

			mock_accept_result = 2; // reset
		}


	};
}
