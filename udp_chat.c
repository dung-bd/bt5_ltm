#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <receiver IP> <receiver port> <listening port>\n", argv[0]);
        exit(1);
    }

    // Lấy địa chỉ IP và cổng từ dòng lệnh
    char *receiverIP = argv[1];
    int receiverPort = atoi(argv[2]);
    int listeningPort = atoi(argv[3]);

    // Khởi tạo socket
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Thiết lập địa chỉ và cổng của máy nhận
    struct sockaddr_in receiverAddr;
    memset(&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(receiverPort);
    if (inet_pton(AF_INET, receiverIP, &(receiverAddr.sin_addr)) <= 0) {
        perror("Error setting receiver address");
        exit(1);
    }

    // Thiết lập địa chỉ và cổng chờ
    struct sockaddr_in listenAddr;
    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(listeningPort);
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Gắn socket với địa chỉ và cổng chờ
    if (bind(sockfd, (struct sockaddr *)&listenAddr, sizeof(listenAddr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Khởi tạo biến lưu trữ dữ liệu và các biến liên quan đến select/poll
    char buffer[MAX_BUFFER_SIZE];
    int maxfd;
    fd_set readfds;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds); // Thêm stdin vào tập đọc
        FD_SET(sockfd, &readfds); // Thêm socket vào tập đọc
        maxfd = (fileno(stdin) > sockfd) ? fileno(stdin) : sockfd;

        // Thiết lập timeout
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000; // 0.5 giây

        // Sử dụng select để kiểm tra sự kiện trên các file descriptor
        int activity = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("Error in select");
            exit(1);
        }

        // Kiểm tra sự kiện trên stdin
        if (FD_ISSET(fileno(stdin), &readfds)) {
            fgets(buffer, MAX_BUFFER_SIZE, stdin);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&receiverAddr, sizeof(receiverAddr));
        }

        // Kiểm tra sự kiện trên socket
        if (FD_ISSET(sockfd, &readfds)) {
            int recvLen;
            struct sockaddr_in senderAddr;
            socklen_t addrLen = sizeof(senderAddr);
            if ((recvLen = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE - 1, 0, (struct sockaddr *)&senderAddr, &addrLen)) > 0) {
                buffer[recvLen] = '\0';
                printf("Received: %s", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}