# GoQuant Assignment

# Objective
Create a high-performance order execution and management system to trade on Deribit Test ([https://test.deribit.com/](https://test.deribit.com/)) using C++.

---

## Initial Setup

1. **Create a new Deribit Test account**
   - Visit [https://test.deribit.com/](https://test.deribit.com/) and register.
2. **Generate API Keys for authentication**
   - Access your account settings and create API keys for secure access.

---

## Core Requirements

### **Order Management Functions**

1. **Place order**  
2. **Cancel order**  
3. **Modify order**  
4. **Get orderbook**  
5. **View current positions**
6. **Real-time market data streaming via WebSocket**
   - Implement WebSocket server functionality.
   - Allow clients to subscribe to symbols.
   - Stream continuous orderbook updates for subscribed symbols.

---

## Market Coverage

- **Instruments**: Spot, Futures, and Options.
- **Scope**: All supported symbols on Deribit Test.

---

## Technical Requirements

1. Implementation must be in **C++**.
2. System should demonstrate **low-latency performance**.
3. Include **proper error handling** and **logging**.
4. Implement WebSocket server for real-time data distribution:
   - Connection management.
   - Subscription handling.
   - Efficient message broadcasting.

---

## Bonus Section (Recommended): Performance Analysis and Optimization

### **Latency Benchmarking**

1. Measure and document the following metrics:
   - **Order placement latency**.
   - **Market data processing latency**.
   - **WebSocket message propagation delay**.
   - **End-to-end trading loop latency**.

### **Optimization Requirements**

1. Implement and justify optimization techniques for:
   - **Memory management**.
   - **Network communication**.
   - **Data structure selection**.
   - **Thread management**.
   - **CPU optimization**.

---

## Documentation Requirements for Bonus Section

1. Detailed analysis of **bottlenecks identified**.
2. **Benchmarking methodology** explanation.
3. **Before/after performance metrics**.
4. Justification for **optimization choices**.
5. Discussion of **potential further improvements**.

---

## Deliverables

1. **Complete source code** with documentation.
2. **Video recording** demonstrating:
   - System functionality.
   - Code review.
   - Implementation explanation.
3. If completing the bonus section:
   - **Performance analysis report**.
   - **Benchmarking results**.
   - **Optimization documentation**.

## Requirements

- **C++ Compiler**: GCC (recommended) or any modern C++ compiler with C++11 support.
- **cURL**: Library for HTTP requests.
- **JSON for Modern C++**: Header-only library for JSON parsing by `nlohmann/json`.

## Dependencies

1. **cURL**: Install via the following commands:
    ```bash
    sudo apt update
    sudo apt install libcurl4-openssl-dev
    ```

2. **JSON for Modern C++**: Download the header file from [nlohmann/json GitHub page](https://github.com/nlohmann/json) or install via a package manager:
    ```bash
    sudo apt install nlohmann-json3-dev
    ```
3. **Boost Libraries**: Install boost:
    ```bash
    sudo apt update
    sudo apt install libboost-all-dev
    ```
3. **Web socket**: Install websocket++:
    ```bash
    sudo apt install libwebsocketpp-dev
    ```
## Setup

1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/VarunProhit/goQuantAssignment
    cd goQuantAssignment
    ```

2. Ensure the necessary libraries (`curl`, `nlohmann/json` and, `websocketpp`) are installed.

3. Add the path to `json.hpp` if it's not already in your include path:
    - Place `json.hpp` in the `include/` directory within the project.

## Compilation

Use the following command to compile  and execute trading menu:

```bash
g++ trading.cpp -o trading -lcurl -I include
./trading
```
Use following command to compile client and server for Real-time market data streaming

```bash
g++ -std=c++17 server.cpp -o server -lboost_system -lpthread
./server
```
In another terminal
```bash
g++ -std=c++17 client.cpp -o client -lboost_system -lpthread
```