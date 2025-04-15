#define CROW_MAIN
#include "PktDef.h"
#include "crow_all.h"
#include "MySocket.h"
#include "iostream"
#define CRCSIZE 1

using namespace std;
using namespace crow;

MySocket* mainSocket = nullptr;
int pktCount = 1;


void loadPage(std::string path, crow::response* res)
{
    std::ifstream in(path, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res->write(contents.str());
    }
    else
    {
        res->write("404 Not Found");
    }
    res->end();
}

void sendScript(std::string filename, crow::response* res)
{
    std::ifstream in("../public/scripts/" + filename, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res->write(contents.str());
    }
    else
    {
        res->write("404 Not Found");
    }
    res->end();
}

void sendStyle(std::string filename, crow::response* res)
{
    std::ifstream in("../public/styles/" + filename, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res->write(contents.str());
    }
    else
    {
        res->write("404 Not Found");
    }
    res->end();
}

void sendImage(std::string filename, crow::response* res)
{
    std::ifstream in("../public/images/" + filename, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res->write(contents.str());
    }
    else
    {
        res->write("404 Not Found");
    }
    res->end();
}

void loadTextFile(std::string path, crow::response* res)
{
    std::ifstream in(path, std::ifstream::in);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res->set_header("Content-Type", "text/plain");
        res->write(contents.str());
    }
    else
    {
        res->write("404 Not Found");
    }
    res->end();
}

int main()
{
    SimpleApp app;

    CROW_ROUTE(app, "/")
        ([](const request& req, response& res)
            {
                loadPage("../public/index.html", &res);
            });
    CROW_ROUTE(app, "/connect/<string>/<int>/<string>").methods("POST"_method)
        ([](const request& req, response& res, string ip, int port, string type)
            {
                if(mainSocket != nullptr)
                {
                    delete mainSocket;
                }
                // code here
                CROW_LOG_DEBUG << "IP: " << ip << " PORT: " << to_string(port) << " TYPE: " << type;
                SocketType sType = CLIENT;
                ConnectionType cType = strcmp(type.c_str(), "TCP") == 0 ? TCP : UDP;

                mainSocket = new MySocket(sType, ip, port, cType, 25);

                CROW_LOG_DEBUG << "Socket Status: " << mainSocket->GetStatus() ;

                if(mainSocket->GetStatus() == -1)
                {
                    res.code = 503;
                }

                res.end();
            });
    CROW_ROUTE(app, "/telecommand").methods("PUT"_method)
        ([](const request& req, response& res)
            {
                const char* sleepParam = req.url_params.get("sleep");
                string sleep = sleepParam ? string(sleepParam) : "";

                int directionVal = req.url_params.get("direction") ? atoi(req.url_params.get("direction")) : 0;
                int speed = req.url_params.get("speed") ? atoi(req.url_params.get("speed")) : 0;
                int duration = req.url_params.get("duration") ? atoi(req.url_params.get("duration")) : 0;

                PktDef newPkt;
                newPkt.SetPktCount(pktCount);

                if (sleep == "true")
                {
                    newPkt.SetCmd(PktDef::SLEEP);
                }
                else
                {
                    struct PktDef::DriveBody db;
                    db.direction = static_cast<PktDef::Direction>(directionVal);
                    db.duration = duration;
                    db.speed = speed;
                    newPkt.SetCmd(PktDef::DRIVE);
                    newPkt.SetBodyData((char *)&db, sizeof(db));
                }

                // Calculate the CRC
                newPkt.CalcCRC();

                // Send data
                mainSocket->SendData(newPkt.GenPacket(), newPkt.GetLength());

                // Receive data
                char buffer[5];

                mainSocket->GetData(buffer);

                PktDef rxPkt(buffer);

                pktCount = rxPkt.GetPktCount() + 1;
                bool correctCRC = rxPkt.CheckCRC(buffer, rxPkt.GetLength() - CRCSIZE); // Minus 1 to not include the CRC

                if(correctCRC)
                {
                    if(!rxPkt.GetAck())
                    {
                        res.code = 400;
                    }
                }
                else
                {
                    res.code = 503;
                }

                res.end();
            });
    CROW_ROUTE(app, "/telementry_request").methods("GET"_method)
        ([](const request& req, response& res)
            {
                PktDef newPkt;
                newPkt.SetPktCount(pktCount);
                newPkt.SetCmd(PktDef::RESPONSE);
                newPkt.CalcCRC();

                mainSocket->SendData(newPkt.GenPacket(), newPkt.GetLength());

                char buffer[14];

                mainSocket->GetData(buffer);

                PktDef rxPkt(buffer);

                pktCount = rxPkt.GetPktCount() + 1;

                CROW_LOG_DEBUG << "First Pkt length " << to_string(rxPkt.GetLength());

                bool correctCRC = rxPkt.CheckCRC(buffer, rxPkt.GetLength() - CRCSIZE); // Minus 1 to not include the CRC

                if(correctCRC)
                {
                    if(!rxPkt.GetAck())
                    {
                        res.code = 400;
                    }
                    memset(buffer, 0, 14);

                    mainSocket->GetData(buffer);
                    PktDef rxPkt2(buffer);

                    pktCount = rxPkt2.GetPktCount() + 1;

                    bool correctCRC2 = rxPkt2.CheckCRC(buffer, rxPkt2.GetLength() - CRCSIZE); // Minus 1 to not include the CRC
                    if(correctCRC2)
                    {
                        if(!rxPkt2.GetStatus())
                        {
                            res.code = 400;
                        }

                        res.set_header("Content-Type", "application/json");
                        res.body.append(
                            "{\"LPC\":\"" + std::to_string(rxPkt2.TelemBody.LastPktCounter) +
                            "\", \"hitCount\":\"" + std::to_string(rxPkt2.TelemBody.HitCount) +
                            "\", \"currentGrade\":\"" + std::to_string(rxPkt2.TelemBody.CurrentGrade) +
                            "\", \"lastCMD\":\"" + std::to_string(rxPkt2.TelemBody.LastCmd) +
                            "\", \"lastCMDSpeed\":\"" + std::to_string(rxPkt2.TelemBody.LastCmdSpeed) +
                            "\", \"lastCMDValue\":\"" + std::to_string(rxPkt2.TelemBody.LastCmdValue) +
                            "\"}"
                        );
                        // Set body here
                        CROW_LOG_DEBUG << "Second Pkt length " << to_string(rxPkt2.GetLength());
                        CROW_LOG_DEBUG << "LPC: " << to_string(rxPkt2.TelemBody.LastPktCounter);
                        CROW_LOG_DEBUG << "Hit Count: " << to_string(rxPkt2.TelemBody.HitCount);
                        CROW_LOG_DEBUG << "Current Grade: " << to_string(rxPkt2.TelemBody.CurrentGrade);
                        CROW_LOG_DEBUG << "Last Cmd: " << to_string(rxPkt2.TelemBody.LastCmd);
                        CROW_LOG_DEBUG << "Last Cmd Speed: " << to_string(rxPkt2.TelemBody.LastCmdSpeed);
                        CROW_LOG_DEBUG << "Last Cmd Value: " << to_string(rxPkt2.TelemBody.LastCmdValue);
                    }
                    else
                    {
                        res.code = 503;
                    }

                }
                else
                {
                    res.code = 503;
                }


                res.end();
            });
    CROW_ROUTE(app, "/get_script/<string>")
        ([](const request& req, response& res, string filename)
            {
                sendScript(filename, &res);
            });
    CROW_ROUTE(app, "/get_style/<string>")
        ([](const request& req, response& res, string filename)
            {
                sendStyle(filename, &res);
            });
    CROW_ROUTE(app, "/get_image/<string>")
        ([](const request& req, response& res, string filename)
            {
                sendImage(filename, &res);
            });
    app.port(23500).multithreaded().loglevel(LogLevel::DEBUG).run();
    return 1;
}