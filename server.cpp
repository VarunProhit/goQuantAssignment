#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <set>
#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <chrono>

using json = nlohmann::json;

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

std::string getEnvVariable(const std::string &key) {
    std::ifstream envFile(".env");
    if (!envFile.is_open()) {
        std::cerr << "Error: Could not open .env file.\n";
        return "";
    }

    std::string line;
    while (std::getline(envFile, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string currentKey = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            currentKey = trim(currentKey);
            value = trim(value); // Trim whitespace and carriage return
            if (currentKey == key) {
                return value;
            }
        }
    }
    return "";
}

using json = nlohmann::json;
using websocketpp::connection_hdl;

class WebSocketServer {
public:
    WebSocketServer() {
        server.init_asio();

        server.set_open_handler([this](connection_hdl hdl) { onOpen(hdl); });
        server.set_close_handler([this](connection_hdl hdl) { onClose(hdl); });
        server.set_message_handler([this](connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
            onMessage(hdl, msg);
        });

        // Load .env file
        
        env_client_id = getEnvVariable("CLIENT_ID");
        env_client_secret = getEnvVariable("CLIENT_SECRET");

        if (env_client_id.empty() || env_client_secret.empty()) {
            throw std::runtime_error("CLIENT_ID or CLIENT_SECRET missing in .env file");
        }
    }

    void run(uint16_t port) {
        server.listen(port);
        server.start_accept();
        server.run();
    }

    void broadcast(const std::string &symbol, const std::string &message) {
        std::lock_guard<std::mutex> lock(mutex);
        if (subscriptions.find(symbol) != subscriptions.end()) {
            for (auto &hdl : subscriptions[symbol]) {
                try {
                    server.send(hdl, message, websocketpp::frame::opcode::text);
                } catch (const std::exception &e) {
                    std::cerr << "Broadcast error: " << e.what() << std::endl;
                }
            }
        }
    }

    void addSymbol(const std::string &symbol) {
        std::lock_guard<std::mutex> lock(mutex);
        subscriptions[symbol] = std::set<connection_hdl, std::owner_less<connection_hdl>>();
    }

private:
    websocketpp::server<websocketpp::config::asio> server;
    std::unordered_map<std::string, std::set<connection_hdl, std::owner_less<connection_hdl>>> subscriptions;
    std::mutex mutex;

    std::string env_client_id;
    std::string env_client_secret;

    void onOpen(connection_hdl hdl) {
        std::cout << "Client connected." << std::endl;
    }

    void onClose(connection_hdl hdl) {
        std::cout << "Client disconnected." << std::endl;

        // Remove the connection from all subscriptions
        std::lock_guard<std::mutex> lock(mutex);
        for (auto &[symbol, clients] : subscriptions) {
            clients.erase(hdl);
        }
    }

    void onMessage(connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
        try {
            auto payload = msg->get_payload();
            auto parsed = json::parse(payload);

            if (parsed["action"] == "authenticate") {
                std::string client_id = parsed["client_id"];
                std::string client_secret = parsed["client_secret"];
                
                if (authenticate(client_id, client_secret)) {
                    server.send(hdl, R"({"status":"authenticated"})", websocketpp::frame::opcode::text);
                } else {
                    server.send(hdl, R"({"status":"error","error":"Invalid credentials"})", websocketpp::frame::opcode::text);
                    server.close(hdl, websocketpp::close::status::policy_violation, "Authentication failed");
                }
            } else if (parsed["action"] == "subscribe") {
                std::string symbol = parsed["symbol"];
                std::lock_guard<std::mutex> lock(mutex);
                subscriptions[symbol].insert(hdl);
                std::cout << "Client subscribed to " << symbol << std::endl;
            } else if (parsed["action"] == "unsubscribe") {
                std::string symbol = parsed["symbol"];
                std::lock_guard<std::mutex> lock(mutex);
                subscriptions[symbol].erase(hdl);
                std::cout << "Client unsubscribed from " << symbol << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Message handling error: " << e.what() << std::endl;
        }
    }

    bool authenticate(const std::string &client_id, const std::string &client_secret) {
        return client_id == env_client_id && client_secret == env_client_secret;
    }
};

void streamMarketData(WebSocketServer &server, const std::string &symbol) {
    while (true) {
        json update = {
            {"symbol", symbol},
            {"best_bid", rand() % 100 + 1},
            {"best_ask", rand() % 100 + 50},
            {"timestamp", time(nullptr)}};

        server.broadcast(symbol, update.dump());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    try {
        WebSocketServer server;
        server.addSymbol("ETH-PERPETUAL");

        std::thread serverThread([&server]() {
            server.run(9000);
        });

        std::thread dataThread([&server]() {
            streamMarketData(server, "ETH-PERPETUAL");
        });

        serverThread.join();
        dataThread.join();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
