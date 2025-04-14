#define CROW_MAIN
#include "PktDef.h"
#include "crow_all.h"
#include "MySocket.h"
#include "iostream"

using namespace std;

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
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
        ([](const crow::request& req, crow::response& res)
            {
                loadPage("../public/index.html", &res);
            });
    CROW_ROUTE(app, "/get_script/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename)
            {
                sendScript(filename, &res);
            });
    CROW_ROUTE(app, "/get_style/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename)
            {
                sendStyle(filename, &res);
            });
    CROW_ROUTE(app, "/get_image/<string>")
        ([](const crow::request& req, crow::response& res, std::string filename)
            {
                sendImage(filename, &res);
            });
    app.port(23500).multithreaded().run();
    return 1;
}