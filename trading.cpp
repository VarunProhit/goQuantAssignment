#include <iostream>
#include <string>
#include <curl/curl.h>
#include "include/json.hpp"
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

// Function to handle the response from the cURL request
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// General function to send a cURL request with optional access token
std::string sendRequest(const std::string &url, const json &payload, const std::string &accessToken = "")
{
    std::string readBuffer;
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Set the HTTP method to POST

        // Set the request payload
        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        // Set headers, including Authorization if accessToken is provided
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty())
        {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Free Resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

// Function to get the access token
std::string getAccessToken(const std::string &clientId, const std::string &clientSecret)
{
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {{"grant_type", "client_credentials"}, {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"}, {"client_id", clientId}, {"client_secret", clientSecret}}},
        {"jsonrpc", "2.0"}};

    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);

    if (responseJson.contains("result") && responseJson["result"].contains("access_token"))
    {
        // std::cout << "Payload: " << payload.dump(4) << std::endl;
        return responseJson["result"]["access_token"];
    }
    else
    {
        std::cerr << "Failed to retrieve access token." << std::endl;
        return "";
    }
}

// Function to place an order
void placeOrder(const std::string &price, const std::string &accessToken, const std::string &amount, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/buy"},
        {"params", {{"instrument_name", instrument}, {"type", "limit"}, {"price", price}, {"amount", amount}}},
        {"id", 1}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    std::cout << "Place Order Response: " << response << std::endl;
    std::cout << "Order placement latency: " << latency.count() << " seconds." << std::endl;
}

// Function to cancel an order
void cancelOrder(const std::string &accessToken, const std::string &orderID)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderID}}},
        {"id", 6}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload, accessToken);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    std::cout << "Cancel Order Response: " << response << std::endl;
    std::cout << "Order cancellation latency: " << latency.count() << " seconds." << std::endl;
}

// Function to modify an order
void modifyOrder(const std::string &accessToken, const std::string &orderID, int amount, double price)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/edit"},
        {"params", {{"order_id", orderID}, {"amount", amount}, {"price", price}}},
        {"id", 11}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", payload, accessToken);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    std::cout << "Modify Order Response: " << response << std::endl;
    std::cout << "Order modification latency: " << latency.count() << " seconds." << std::endl;
}

// Function to retrieve the order book
void getOrderBook(const std::string &accessToken, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, accessToken);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    auto responseJson = json::parse(response);
    std::cout << "Order Book for " << instrument << ":\n\n";
    std::cout << "Best Bid Price: " << responseJson["result"]["best_bid_price"] << ", Amount: " << responseJson["result"]["best_bid_amount"] << '\n';
    std::cout << "Best Ask Price: " << responseJson["result"]["best_ask_price"] << ", Amount: " << responseJson["result"]["best_ask_amount"] << '\n';

    std::cout << "Market data processing latency: " << latency.count() << " seconds." << std::endl;
}

// Function to get position details of a specific instrument
void getPosition(const std::string &accessToken, const std::string &instrument)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_position"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 20}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
    auto responseJson = json::parse(response);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    if (responseJson.contains("result"))
    {
        std::cout << "Position Details for " << instrument << ":\n\n";
        auto result = responseJson["result"];
        std::cout << "Estimated Liquidation Price: " << result["estimated_liquidation_price"] << '\n';
        std::cout << "Size Currency: " << result["size_currency"] << '\n';
        std::cout << "Realized Funding: " << result["realized_funding"] << '\n';
        std::cout << "Total Profit Loss: " << result["total_profit_loss"] << '\n';
        std::cout << "Realized Profit Loss: " << result["realized_profit_loss"] << '\n';
        std::cout << "Floating Profit Loss: " << result["floating_profit_loss"] << '\n';
        std::cout << "Leverage: " << result["leverage"] << '\n';
        std::cout << "Average Price: " << result["average_price"] << '\n';
        std::cout << "Delta: " << result["delta"] << '\n';
        std::cout << "Interest Value: " << result["interest_value"] << '\n';
        std::cout << "Mark Price: " << result["mark_price"] << '\n';
        std::cout << "Settlement Price: " << result["settlement_price"] << '\n';
        std::cout << "Index Price: " << result["index_price"] << '\n';
        std::cout << "Direction: " << result["direction"] << '\n';
        std::cout << "Open Orders Margin: " << result["open_orders_margin"] << '\n';
        std::cout << "Initial Margin: " << result["initial_margin"] << '\n';
        std::cout << "Maintenance Margin: " << result["maintenance_margin"] << '\n';
        std::cout << "Kind: " << result["kind"] << '\n';
        std::cout << "Size: " << result["size"] << '\n';
    }
    else
    {
        std::cerr << "Error: Could not retrieve position data." << std::endl;
    }

    std::cout << "End-to-end trading loop latency: " << latency.count() << " seconds." << std::endl;
}

// Function to print all open orders with instrument, order ID, price, and amount
void getOpenOrders(const std::string &accessToken)
{
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/get_open_orders"},
        {"params", {{"kind", "future"}, {"type", "limit"}}},
        {"id", 25}};

    auto start = std::chrono::high_resolution_clock::now();
    std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders", payload, accessToken);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end - start;

    auto responseJson = json::parse(response);

    if (responseJson.contains("result"))
    {
        std::cout << "Open Orders:\n\n";
        for (const auto &order : responseJson["result"])
        {
            std::string instrument = order["instrument_name"];
            std::string orderId = order["order_id"];
            double price = order["price"];
            double amount = order["amount"];

            std::cout << "Instrument: " << instrument << ", Order ID: " << orderId
                      << ", Price: " << price << ", Amount: " << amount << '\n';
        }
    }

    std::cout << "Open orders latency: " << latency.count() << " seconds." << std::endl;
}

int main()
{
    std::string clientId = getEnvVariable("CLIENT_ID");
    std::string clientSecret = getEnvVariable("CLIENT_SECRET");

    if (clientId.empty() || clientSecret.empty())
    {
        std::cerr << "Error: CLIENT_ID or CLIENT_SECRET is missing in .env file.\n";
        return 1;
    }

    std::string accessToken = getAccessToken(clientId, clientSecret);

    if (!accessToken.empty())
    {
        int choice;
        do
        {
            std::cout << "\n--- Trading System Menu ---\n";
            std::cout << "0. Exit\n";
            std::cout << "1. Place Order\n";
            std::cout << "2. Cancel Order\n";
            std::cout << "3. Modify Order\n";
            std::cout << "4. Get Order Book\n";
            std::cout << "5. Get Position\n";
            std::cout << "6. Get Open Orders\n";
            std::cout << "Enter your choice: ";
            std::cin >> choice;

            switch (choice)
            {
            case 0:
                std::cout << "Exiting program.\n";
                break;
            case 1:
            {
                std::string amount, price, instrument;
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cout << "Enter price: ";
                std::cin >> price;
                std::cout << "Enter instrument (e.g., ETH-PERPETUAL): ";
                std::cin >> instrument;
                placeOrder(price, accessToken, amount, instrument);
                break;
            }
            case 2:
            {
                std::string orderId;
                std::cout << "Enter order ID to cancel: ";
                std::cin >> orderId;
                cancelOrder(accessToken, orderId);
                break;
            }
            case 3:
            {
                std::string orderId;
                double newAmount, newPrice;
                std::cout << "Enter order ID to modify: ";
                std::cin >> orderId;
                std::cout << "Enter new amount: ";
                std::cin >> newAmount;
                std::cout << "Enter new price: ";
                std::cin >> newPrice;
                modifyOrder(accessToken, orderId, newAmount, newPrice);
                break;
            }
            case 4:
            {
                std::string instrument;
                std::cout << "Enter instrument (e.g., ETH-PERPETUAL): ";
                std::cin >> instrument;
                getOrderBook(accessToken, instrument);
                break;
            }
            case 5:
            {
                std::string instrument;
                std::cout << "Enter instrument (e.g., ETH-PERPETUAL): ";
                std::cin >> instrument;
                getPosition(accessToken, instrument);
                break;
            }
            case 6:
                getOpenOrders(accessToken);
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
            }
        } while (choice != 0);
    }
    else
    {
        std::cerr << "Unable to obtain access token, aborting operations." << std::endl;
    }

    return 0;
}
