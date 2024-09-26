// server.cpp
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>

#define IPADDR INADDR_ANY
#define PORT 8080

struct Client {
    int fd;
    char ip[INET_ADDRSTRLEN]; // 클라이언트 IP
};

std::vector<Client> clients; // 채팅에 참여중인 유저들

// 메시지를 다른 클라이언트들에게 전송
void broadcastMessage(const std::string& message, const int sender_fd) {
    for (const Client &c : clients)
        if (c.fd != sender_fd)
            send(c.fd, message.c_str(), message.size(), 0);
}

void handleClient() {
    while (true) {
        char buffer[1024]; // 메시지 버퍼
        for(const Client &c : clients) {
            int bytes_received = recv(c.fd, buffer, sizeof(buffer), 0); // client로부터 메시지 가져오기
            if(bytes_received > 0) {
                buffer[bytes_received] = '\0';
                std::string message = buffer;
                broadcastMessage(message, c.fd);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// 클라이언트를 수락하는 accept 스레드 함수
void acceptClients(int server_socket) {
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

        if (client_socket == -1) {
            std::cerr << "Failed to accept client.\n";
            continue;
        }

        // 클라이언트 IP 주소 추출
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        // 클라이언트 설정
        Client client;
        client.fd = client_socket;
        strcpy(client_ip, client_ip);

        clients.push_back(client);

        broadcastMessage(strcat(client_ip, "님이 채팅방에 참여하셨어요.\n"), server_socket);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// 서버 스레드 함수 - 클라이언트 accept 스레드를 실행
void server() {
    // TODO: IP와 PORT를 입력 받아 서버를 실행시킬 수 있도록 해야 함
    int socket_fd;
    
    // Creating socket file descriptor
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    // Setting socket's options
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set socket options.\n";
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = IPADDR; // IP 수정

    // Binding
    if (bind(socket_fd, (sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Failed to bind to port.\n";
        return;
    }

    // Listen
    if (listen(socket_fd, 3) == -1) {
        std::cerr << "Failed to listen on socket.\n";
        return;
    }

    std::cout << "Server started on port 8080...\n";

    // 클라이언트 accept를 처리하는 스레드 시작
    std::thread accept_thread(acceptClients, socket_fd);
    accept_thread.detach();

    // 각 클라이언트마다 메시지 처리 스레드 생성
    std::thread client_thread(handleClient);
    client_thread.detach();

    while(!accept_thread.joinable() && !client_thread.joinable())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    close(socket_fd);
}

int main() {
    // 서버 스레드를 실행
    std::thread server_thread(server);
    server_thread.join();  // 서버 스레드가 끝날 때까지 기다림

    return 0;
}
