// client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>

#define IPADDR INADDR_ANY
#define PORT 8080

std::mutex mtx;

void receiveMessages(int client_socket) {
    char buffer[1024];
    while (true) {
        std::lock_guard<std::mutex> guard(mtx);
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Message> " << buffer << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void client() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return;
    }

    if(connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to connect to server.\n";
        return;
    }

    std::thread receive_thread(receiveMessages, client_fd);
    receive_thread.detach();

    std::string message;
    while (true) {
        std::lock_guard<std::mutex> guard(mtx);

        if(!receive_thread.joinable()) {
            std::cout << "나> ";
            std::getline(std::cin, message);
            send(client_fd, message.c_str(), message.size(), 0);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    close(client_fd);
}

int main() {
    // 클라이언트 스레드를 실행
    std::thread client_thread(client);
    client_thread.join();  // 클라이언트 스레드가 끝날 때까지 기다림

    return 0;
}
