#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void normalizeString(char *str) {
    int len = strlen(str);
    int i, j;
    int spaceCount = 0;

    // Xóa ký tự dấu cách ở đầu xâu
    while (isspace(str[0])) {
        memmove(str, str + 1, len);
        len--;
    }

    // Xóa ký tự dấu cách ở cuối xâu
    while (isspace(str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }

    // Xóa ký tự không hợp lệ giữa các từ
    for (i = 0, j = 0; i < len; i++) {
        if (isspace(str[i])) {
            if (spaceCount == 0) {
                str[j++] = ' ';
                spaceCount++;
            }
        } else {
            str[j++] = str[i];
            spaceCount = 0;
        }
    }
    str[j] = '\0';

    // Viết hoa chữ cái đầu các từ
    int capitalizeNext = 1;
    for (i = 0; i < j; i++) {
        if (isspace(str[i])) {
            capitalizeNext = 1;
        } else {
            if (capitalizeNext) {
                str[i] = toupper(str[i]);
                capitalizeNext = 0;
            } else {
                str[i] = tolower(str[i]);
            }
        }
    }
}

int main() {
    int server_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in server_address, client_address;
    int max_clients = MAX_CLIENTS;
    int activity, i, valread, sd;
    int max_sd;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Khởi tạo mảng client_socket để lưu các socket của các client
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    // Tạo socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin địa chỉ và cổng của server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    // Gán địa chỉ và cổng cho server socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Không thể gán địa chỉ và cổng cho server socket");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe các kết nối đến
    if (listen(server_socket, 3) < 0) {
        perror("Lỗi trong quá trình lắng nghe kết nối đến");
        exit(EXIT_FAILURE);
    }

    printf("Telnet server đã sẵn sàng\n");

    while (1) {
        // Thiết lập tập file descriptors để theo dõi
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Thêm các socket của clients vào tập file descriptors
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Sử dụng hàm select để chờ sự kiện xảy ra trên các socket
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Xử lý kết nối mới
        if (FD_ISSET(server_socket, &readfds)) {
            int new_socket;
            int client_address_length = sizeof(client_address);
            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_length)) < 0) {
                perror("Lỗi trong quá trình chấp nhận kết nối mới");
                exit(EXIT_FAILURE);
            }
            printf("Kết nối mới được chấp nhận\n");

            // Thêm kết nối mới vào mảng client_socket
            for (i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    string g = "Xin chào. Hiện đang có " + i + "clients đang kết nối!\n";
                    send(sd, g, strlen(g), 0);
                    break;
                }
            }
        }

        // Xử lý dữ liệu từ các client đang kết nối
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                // Đọc dữ liệu từ client
                memset(buffer, 0, sizeof(buffer));
                valread = read(sd, buffer, sizeof(buffer));

                // Xử lý yêu cầu
                if (buffer == "exit") {
                    send(sd, "Tạm biệt!\n", strlen("Tạm biệt!\n"), 0);
                    close(sd);
                    return 0;
                }
                    
                else {
                    normalizeString(buffer);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

return 0;
}