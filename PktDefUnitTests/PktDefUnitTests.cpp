#include "pch.h"
#include "CppUnitTest.h"
#include "PktDef.h"   // include the header file containing the PktDef class
#include <cstring>
#include <vector>

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {

            template<>
            std::wstring ToString<PktDef::CmdType>(const PktDef::CmdType& t)
            {
                switch (t)
                {
                case PktDef::DRIVE:    return L"DRIVE";
                case PktDef::SLEEP:    return L"SLEEP";
                case PktDef::RESPONSE: return L"RESPONSE";
                default:               return L"Unknown CmdType";
                }
            }

        }
    }
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PktDefUnitTests
{
    TEST_CLASS(PktDefUnitTests)
    {
    public:


        // Test that the default constructor initializes all members.
        TEST_METHOD(DefaultConstructorTest)
        {
            PktDef pkt;
            Assert::AreEqual(0, pkt.GetPktCount());
            Assert::AreEqual(6, pkt.GetLength());
            Assert::IsFalse(pkt.GetAck());
            Assert::IsNull(pkt.GetBodyData());
        }

        // Generate a valid raw packet using an original instance and then use that buffer to construct a new PktDef.
        TEST_METHOD(ConstructorFromRawBufferTest)
        {
            // Create an original packet.
            PktDef pktOriginal;
            pktOriginal.SetPktCount(5);
            pktOriginal.SetCmd(PktDef::RESPONSE);
            char sampleData[3] = { static_cast<char>(0x99), static_cast<char>(0x88), static_cast<char>(0x77) };
            pktOriginal.SetBodyData(sampleData, 3);
            pktOriginal.CalcCRC();
            char* rawPacket = pktOriginal.GenPacket();

            // Construct a new instance using the raw packet.
            PktDef pktFromBuffer(rawPacket);

            // Validate that key fields match.
            Assert::AreEqual(pktOriginal.GetPktCount(), pktFromBuffer.GetPktCount());
            Assert::AreEqual(pktOriginal.GetLength(), pktFromBuffer.GetLength());
            Assert::AreEqual((int)pktOriginal.GetCmd(), (int)pktFromBuffer.GetCmd());
            Assert::AreEqual(pktOriginal.GetCRC(), pktFromBuffer.GetCRC());

            delete[] rawPacket;
        }

        // Test that the raw constructor does copy telemetry data when the required flags are set.
        TEST_METHOD(ConstructorTelemetryTest)
        {
            const int telemetrySize = 9;
            const int packetSize = 6 + telemetrySize;

            // Allocate a raw buffer large enough to hold header, telemetry data, and CRC.
            char rawPacket[16] = { 0 };

            // Fill in header
            PktDef::Header header = {};
            header.PktCount = 99;
            header.Drive = 0;
            header.Status = 1;
            header.Sleep = 0;
            header.Ack = 1;
            header.Padding = 0;
            header.Length = packetSize;

            memcpy(rawPacket, &header, sizeof(PktDef::Header));

            // Define telemetry values.
            unsigned short expectedLastPktCounter = 123;
            unsigned short expectedCurrentGrade = 456;
            unsigned short expectedHitCount = 789;
            unsigned char expectedLastCmd = 10;
            unsigned char expectedLastCmdValue = 11;
            unsigned char expectedLastCmdSpeed = 12;

            // Copy telemetry data after the header.
            int offset = sizeof(PktDef::Header);
            memcpy(rawPacket + offset, &expectedLastPktCounter, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            memcpy(rawPacket + offset, &expectedCurrentGrade, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            memcpy(rawPacket + offset, &expectedHitCount, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            rawPacket[offset++] = expectedLastCmd;
            rawPacket[offset++] = expectedLastCmdValue;
            rawPacket[offset++] = expectedLastCmdSpeed;

            // Compute CRC.
            int totalParity = 0;
            for (int i = 0; i < packetSize - 1; ++i) {
                unsigned char byte = static_cast<unsigned char>(rawPacket[i]);
                while (byte) {
                    totalParity += (byte & 1);
                    byte >>= 1;
                }
            }
            rawPacket[packetSize - 1] = static_cast<char>(totalParity);

            // Create a PktDef from the raw packet.
            PktDef pkt(rawPacket);

            // Verify that telemetry fields have been copied correctly.
            Assert::AreEqual(expectedLastPktCounter, pkt.TelemBody.LastPktCounter);
            Assert::AreEqual(expectedCurrentGrade, pkt.TelemBody.CurrentGrade);
            Assert::AreEqual(expectedHitCount, pkt.TelemBody.HitCount);
            Assert::AreEqual(expectedLastCmd, pkt.TelemBody.LastCmd);
            Assert::AreEqual(expectedLastCmdValue, pkt.TelemBody.LastCmdValue);
            Assert::AreEqual(expectedLastCmdSpeed, pkt.TelemBody.LastCmdSpeed);
        }

        // Test that the raw constructor does NOT copy telemetry data when required flags are not set.
        TEST_METHOD(ConstructorWithoutTelemetryTest)
        {
            // Create a raw packet with extra telemetry data, but with the Status flag not set.
            const int extraDataSize = 9;
            const int packetSize = 6 + extraDataSize;
            char rawPacket[16] = { 0 };

            // Fill in the header.
            PktDef::Header header = {};
            header.PktCount = 55;
            header.Drive = 1;
            header.Status = 0;
            header.Sleep = 0;
            header.Ack = 1;
            header.Padding = 0;
            header.Length = packetSize;
            memcpy(rawPacket, &header, sizeof(PktDef::Header));

            // Fill telemetry fields with known values.
            unsigned short expectedLastPktCounter = 321;
            unsigned short expectedCurrentGrade = 654;
            unsigned short expectedHitCount = 987;
            unsigned char expectedLastCmd = 20;
            unsigned char expectedLastCmdValue = 21;
            unsigned char expectedLastCmdSpeed = 22;

            int offset = sizeof(PktDef::Header);
            memcpy(rawPacket + offset, &expectedLastPktCounter, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            memcpy(rawPacket + offset, &expectedCurrentGrade, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            memcpy(rawPacket + offset, &expectedHitCount, sizeof(unsigned short));
            offset += sizeof(unsigned short);
            rawPacket[offset++] = expectedLastCmd;
            rawPacket[offset++] = expectedLastCmdValue;
            rawPacket[offset++] = expectedLastCmdSpeed;

            // Compute and write CRC.
            int totalParity = 0;
            for (int i = 0; i < packetSize - 1; ++i) {
                unsigned char byte = static_cast<unsigned char>(rawPacket[i]);
                while (byte) {
                    totalParity += (byte & 1);
                    byte >>= 1;
                }
            }
            rawPacket[packetSize - 1] = static_cast<char>(totalParity);

            // Construct the object using the raw packet.
            PktDef pkt(rawPacket);

            // Since telemetry was not copied, we expect TelemBody to remain as default.
            Assert::AreEqual(0, (int)pkt.TelemBody.LastPktCounter);
            Assert::AreEqual(0, (int)pkt.TelemBody.CurrentGrade);
            Assert::AreEqual(0, (int)pkt.TelemBody.HitCount);
            Assert::AreEqual(0, (int)pkt.TelemBody.LastCmd);
            Assert::AreEqual(0, (int)pkt.TelemBody.LastCmdValue);
            Assert::AreEqual(0, (int)pkt.TelemBody.LastCmdSpeed);
        }

        // Test SetCmd for DRIVE.
        TEST_METHOD(SetCmdTestDrive)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            Assert::AreEqual(PktDef::DRIVE, pkt.GetCmd());
            Assert::IsTrue(pkt.GetAck());
        }

        // Test SetCmd for SLEEP.
        TEST_METHOD(SetCmdTestSleep)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::SLEEP);
            Assert::AreEqual(PktDef::SLEEP, pkt.GetCmd());
            Assert::IsTrue(pkt.GetAck());
        }

        // Test SetCmd for RESPONSE.
        TEST_METHOD(SetCmdTestResponse)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::RESPONSE);
            Assert::AreEqual(PktDef::RESPONSE, pkt.GetCmd());
            Assert::IsTrue(pkt.GetAck());
        }

        // Verifies that the body data is copied and header length is set to BASEPKTSIZE + body size.
        TEST_METHOD(SetBodyDataTest)
        {
            PktDef pkt;
            char sampleData[5] = { 1, 2, 3, 4, 5 };
            pkt.SetBodyData(sampleData, 5);
            Assert::AreEqual(6 + 5, pkt.GetLength());
            char* body = pkt.GetBodyData();
            for (int i = 0; i < 5; i++)
            {
                Assert::AreEqual(sampleData[i], body[i]);
            }
        }

        // Test SetPktCount and GetPktCount.
        TEST_METHOD(SetPktCountTest)
        {
            PktDef pkt;
            pkt.SetPktCount(42);
            Assert::AreEqual(42, pkt.GetPktCount());
        }

        // Test the CountOnes function for several known values.
        TEST_METHOD(CountOnesTest)
        {
            PktDef pkt;
            Assert::AreEqual(0, pkt.CountOnes(0x00));
            Assert::AreEqual(1, pkt.CountOnes(0x01));
            Assert::AreEqual(2, pkt.CountOnes(0x03));
            Assert::AreEqual(8, pkt.CountOnes(0xFF));
        }

        // Test CalcCRC and GetCRC.
        TEST_METHOD(CalcAndGetCRCTest)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            char sampleData[3] = { static_cast<char>(0xAA), static_cast<char>(0x55), static_cast<char>(0xFF) };
            pkt.SetBodyData(sampleData, 3);
            pkt.CalcCRC();
            int crc = pkt.GetCRC();

            // Generate the raw packet and check that its CRC is valid.
            char* packetBuffer = pkt.GenPacket();
            // CheckCRC computes the parity on the packet (excluding the last byte) and compares it to the stored CRC.
            bool crcValid = pkt.CheckCRC(packetBuffer, pkt.GetLength() - 1);
            Assert::IsTrue(crcValid);

            // Clean up the allocated buffer.
            delete[] packetBuffer;
        }

        // Test that modifying the generated packet causes CheckCRC to fail.
        TEST_METHOD(CheckCRCFailTest)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            char sampleData[3] = { static_cast<char>(0xAA), static_cast<char>(0x55), static_cast<char>(0xFF) };
            pkt.SetBodyData(sampleData, 3);
            pkt.CalcCRC();
            char* packetBuffer = pkt.GenPacket();

            // Corrupt the first body byte in a way that changes the parity.
            packetBuffer[sizeof(PktDef::Header)] = static_cast<char>(packetBuffer[sizeof(PktDef::Header)] + 1);

            // Check that CheckCRC now returns false.
            bool crcValid = pkt.CheckCRC(packetBuffer, pkt.GetLength() - 1);
            Assert::IsFalse(crcValid);

            delete[] packetBuffer;
        }

        // Verifies that the header and CRC are copied correctly.
        TEST_METHOD(GenPacketTest)
        {
            PktDef pkt;
            pkt.SetPktCount(10);
            pkt.SetCmd(PktDef::DRIVE);
            char sampleData[4] = { static_cast<char>(0x11), static_cast<char>(0x22), static_cast<char>(0x33), static_cast<char>(0x44) };
            pkt.SetBodyData(sampleData, 4);
            pkt.CalcCRC();
            char* packetBuffer = pkt.GenPacket();

            // Verify header copying.
            PktDef::Header header;
            memcpy(&header, packetBuffer, sizeof(PktDef::Header));
            Assert::AreEqual(10, (int)header.PktCount);
            Assert::AreEqual(1, (int)header.Ack);
            // Header.Length should equal BASEPKTSIZE (6) + 4 (body size) = 10.
            Assert::AreEqual((unsigned short)10, header.Length);

            // Verify that the CRC stored at the end of the packet matches the value in the object.
            char crcFromPacket;
            memcpy(&crcFromPacket, packetBuffer + header.Length - 1, sizeof(crcFromPacket));
            Assert::AreEqual(pkt.GetCRC(), (int)crcFromPacket);

            delete[] packetBuffer;
        }

        // Test that multiple calls to GenPacket produce consistent packets and do not leak memory.
        TEST_METHOD(RepeatedGenPacketTest)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            char sampleData[2] = { static_cast<char>(0x10), static_cast<char>(0x20) };
            pkt.SetBodyData(sampleData, 2);
            pkt.CalcCRC();

            // Call GenPacket twice.
            char* packet1 = pkt.GenPacket();
            int length = pkt.GetLength();

            // Make a copy of the first packet's data
            std::vector<char> copy1(packet1, packet1 + length);
            char* packet2 = pkt.GenPacket();

            // Verify that the content remains identical.
            bool sameContent = (memcmp(copy1.data(), packet2, length) == 0);
            Assert::IsTrue(sameContent);

            delete[] packet2;
        }

        // Test the behavior of GenPacket when no body data has been set.
        TEST_METHOD(GenPacketNoBodyTest)
        {
            PktDef pkt;
            pkt.SetCmd(PktDef::DRIVE);
            // Without calling SetBodyData, the packet length should remain 0.
            char* packetBuffer = pkt.GenPacket();
            Assert::AreEqual(6, pkt.GetLength());
            delete[] packetBuffer;
        }

    };
}

