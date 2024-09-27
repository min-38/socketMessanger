// client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>

#define IPADDR INADDR_ANY
#define PORT 8080

std::mutex mtx;

void receiveMessages(int* client_fd) {
    char buffer[1024];
    int bytes_received;
    do {
        bytes_received = recv(*client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Message> " << buffer << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(bytes_received > 0);
}

void sendMessages(int *client_fd) {
    std::string message;
    std::cout << "나> " << std::flush;
    std::getline(std::cin, message);
    send(*client_fd, message.c_str(), message.size(), 0);
}

void client(sockaddr_in *server_addr) {
    // socket 설정
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1) {
        std::cerr << "[에러] 소켓 생성 실패.\n";
        return;
    }

    if(connect(client_fd, (sockaddr*)server_addr, sizeof(*server_addr)) == -1) {
        std::cerr << "[에러] 서버와 연결이 되지 않음.\n";
        return;
    }

    std::thread receive_thread(receiveMessages, &client_fd);
    receive_thread.detach();
    std::thread* send_thread;
    while(true) {
        sendMessages(&client_fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    close(client_fd);
}

int main(int argc, char* argv[]) {
    std::string server_ip = "";
    int server_port = PORT;

    if(argc >= 2) {
        server_ip = argv[1];
        server_port = std::stoi(argv[2]);
    }

    // 서버 설정
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if(server_ip == "")
        server_addr.sin_addr.s_addr = IPADDR;
    else
        if(inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr.s_addr) <= 0) {
            std::cerr << "IP 형식과 맞지 않습니다: " << argv[1] << std::endl;
            return -1;
        }

    // client thread를 실행
    std::thread client_thread(client, &server_addr);
    client_thread.join();  // 클라이언트 스레드가 끝날 때까지 기다림

    return 0;
}
