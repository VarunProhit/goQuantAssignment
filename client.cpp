#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>

using json = nlohmann::json;

std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

std::string getEnvVariable(const std::string &key)
{
    std::ifstream envFile(".env");
    if (!envFile.is_open())
    {
        std::cerr << "Error: Could not open .env file.\n";
        return "";
    }

    std::string line;
    while (std::getline(envFile, line))
    {
        size_t pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string currentKey = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            currentKey = trim(currentKey);
            value = trim(value); // Trim whitespace and carriage return
            if (currentKey == key)
            {
                return value;
            }
        }
    }
    return "";
}
using json = nlohmann::json;
using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

void onMessage(client *c, websocketpp::connection_hdl hdl, client::message_ptr msg)
{
    auto payload = msg->get_payload();
    std::cout << "Received: " << payload << std::endl;
}

void onOpen(client *c, websocketpp::connection_hdl hdl)
{
    std::cout << "Connected to server." << std::endl;
    std::string env_client_id = getEnvVariable("CLIENT_ID");
    std::string env_client_secret = getEnvVariable("CLIENT_SECRET");
    // Authenticate
    json authMessage = {
        {"action", "authenticate"},
        {"client_id", env_client_id},
        {"client_secret", env_client_secret}};
    c->send(hdl, authMessage.dump(), websocketpp::frame::opcode::text);

    // Subscribe to a symbol
    json subscribeMessage = {
        {"action", "subscribe"},
        {"symbol", "ETH-PERPETUAL"}};
    c->send(hdl, subscribeMessage.dump(), websocketpp::frame::opcode::text);
}

void onClose(client *c, websocketpp::connection_hdl hdl)
{
    std::cout << "Disconnected from server." << std::endl;
}

void onError(client *c, websocketpp::connection_hdl hdl)
{
    std::cerr << "WebSocket error." << std::endl;
}

int main()
{
    client c;
    c.init_asio();

    c.set_message_handler(std::bind(&onMessage, &c, std::placeholders::_1, std::placeholders::_2));
    c.set_open_handler(std::bind(&onOpen, &c, std::placeholders::_1));
    c.set_close_handler(std::bind(&onClose, &c, std::placeholders::_1));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection("ws://localhost:9000", ec);
    if (ec)
    {
        std::cerr << "Connection error: " << ec.message() << std::endl;
        return 1;
    }

    c.connect(con);
    c.run();

    return 0;
}
