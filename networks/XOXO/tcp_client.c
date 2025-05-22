#include "headers.h"
// #define ip "172.30.95.146"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <IP address>\n", argv[0]);
        return 1;
    }
    char *ip = argv[1];

    int socket_fd = 0;
    struct sockaddr_in server_address;
    char buffer[1024] = {0};
    char input[1024] = {0};

    char *wait_msg = "Wait for the other client to connect,please don't type anything\0";

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("\nSocket creation error\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(34567);
    if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    ssize_t bytes_received = 0;
    while (1)
    {
        memset(buffer, '\0', sizeof(buffer));
        // read(socket_fd, buffer, sizeof(buffer));
        bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0); // -1 to leave space for null terminator
        if (bytes_received < 0)
        {
            perror("recv failed");
        }
        else if (bytes_received == 0)
        {
            printf("Client closed the connection.\n");
        }
        else
        {
            buffer[bytes_received] = '\0'; // Null-terminate the string
            printf("Received message:\n%s\n", buffer);
        }
        if (strcmp(buffer, wait_msg) != 0 && strstr(buffer, "type") != NULL)
        {
            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                int len = strlen(input);
                if (len > 0 && input[len - 1] == '\n')
                {
                    input[len - 1] = '\0';
                }
            }
            send(socket_fd, input, strlen(input), 0);
            if (strcmp(input, "no") == 0)
            {
                break;
            }
        }
        else if (strstr(buffer, "sorry") != NULL)
        {
            break;
        }
    }

    close(socket_fd);

    return 0;
}
