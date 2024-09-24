#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    const char* exitMsg = "님이 나가셨습니다.";
    int pick = 0;
    while(pick != 2) {
        // Connect to the server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection Failed" << std::endl;
            return -1;
        }

        // Receive message from the server
        valread = read(sock, buffer, 1024);
        std::cout << buffer << std::endl;

        std::cout << "1. 채팅 치기" << std::endl;
        std::cout << "2. 나가기" << std::endl;

        std::cin >> pick;

        std::cout << "입력해보야요" << std::endl;

        char* msg = new char[1024];
        if(pick == 1) {
            std::cin >> msg;
            send(sock, msg, strlen(msg), 0);
        } else {
            send(sock, exitMsg, strlen(exitMsg), 0);
        }
    }

    return 0;
}