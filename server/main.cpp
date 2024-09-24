#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>  // read() 함수가 정의된 헤더 파일

#define PORT 8080

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    const char* hello = "채팅방에 들어오셨습니다.";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen error" << std::endl;
        return -1;
    }

    std::cout << "Server is running and waiting for connections..." << std::endl;

    // Accept incoming connections
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        std::cerr << "Accept error" << std::endl;
        return -1;
    }

    // 클라이언트 IP 주소 가져오기
    char* client_ip = inet_ntoa(address.sin_addr);
    int client_port = ntohs(address.sin_port);

    std::cout << client_ip << std::endl;

    // Send message to the client
    send(new_socket, hello, strlen(hello), 0);
    std::cout << "" << std::endl;

    // Receive message from the client
    valread = read(new_socket, buffer, 1024);
    std::cout << buffer << std::endl;

    return 0;
}