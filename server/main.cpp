// server.cpp
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>

#define IPADDR INADDR_ANY
#define PORT 8080

struct client {
    int fd;
    char ip[INET_ADDRSTRLEN]; // 클라이언트 IP
};

std::vector<client> clients; // 채팅에 참여중인 유저들
std::mutex clients_mutex; // 유저가 동시에 등록되고 메시지를 전송 및 전달 받는데 필요한 lock

int main() {
    // TODO: IP와 PORT를 입력 받아 서버를 실행시킬 수 있도록 해야 함
    int socket_fd;
    sockaddr_in address;
    int opt = 1;
    
    // Creating socket file descriptor
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    // Setting socket's options
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = IPADDR; // IP 수정

    // Binding
    if (bind(socket_fd, (sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to bind to port.\n";
        return -1;
    }

    // Listen
    if (listen(socket_fd, 3) == -1) {
        std::cerr << "Failed to listen on socket.\n";
        return -1;
    }

    std::cout << "[LOG] Server started on port " << PORT << "...\n"; // 수정 필요

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 1초 sleep, cpu 잠유율을 줄이기 위함

        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        
        char client_ip[INET_ADDRSTRLEN]; // 클라이언트 IP
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        // 해당 아이피가 이미 존재하는지 확인
        for(client c : clients)
            if(strcmp(c.ip, client_ip) == 0) {
                std::cerr << "[ERROR] IP " << client_ip << " is already exist in our server\n";
                continue;
            }
        
        client new_client;
        new_client.fd = accept(socket_fd, (sockaddr*)&client_addr, &client_size);
        strcpy(new_client.ip, client_ip);
        
        if(new_client.fd == -1) {
            std::cerr << "[ERROR] Failed to accept client(" << client_ip << ")";
            continue;
        }
        clients.push_back(new_client);

        std::cout << "[LOG] Client connected: " << client_ip << "\n";

        char* msg = new char[strlen(client_ip) + 10];

        // 첫 번째 문자열 복사
        strcpy(msg, new_client.ip);
        strcat(msg, "님이 참여했습니다.");

        for(auto &c : clients)
            send(c.fd, msg, strlen(msg), 0);
    }

    close(socket_fd);
    return 0;
}
