#include "headers.h"
#define PORT 34567
#define MAX_CLIENTS 2
// #define ip "172.30.95.146"

void concatenate_and_send(int client_fd[MAX_CLIENTS], char *m1, char *m2, int flag)
{
    char temp[100] = {0};
    strcpy(temp, m1);
    strcat(temp, m2);
    send(client_fd[flag], temp, strlen(temp), 0);
}
void concatenate_and_send1(int client_fd[MAX_CLIENTS], char *m1, char *m2, char *m3, int flag)
{
    char temp[200] = {0};
    strcpy(temp, m1);
    strcat(temp, m2);
    strcat(temp, "\n");
    strcat(temp, m3);
    send(client_fd[flag], temp, strlen(temp), 0);
}

int checkWinner(char *boardstr)
{
    // Check rows
    for (int i = 0; i < 3; i++)
    {
        if ((boardstr[6 * i] == boardstr[6 * i + 2]) && (boardstr[6 * i + 2] == boardstr[6 * i + 4]) && (boardstr[6 * i] != '-'))
        {
            if (boardstr[6 * i] == 'x')
                return 1;
            else
                return 2;
        }
    }
    // Check columns
    for (int j = 0; j < 3; j++)
    {
        if ((boardstr[2 * j] == boardstr[6 + 2 * j]) && (boardstr[6 + 2 * j] == boardstr[12 + 2 * j]) && (boardstr[2 * j] != '-'))
        {
            if (boardstr[2 * j] == 'x')
                return 1;
            else
                return 2;
        }
    }
    // Check diagonal (top-left to bottom-right)
    if ((boardstr[0] == boardstr[8]) && (boardstr[8] == boardstr[16]) && (boardstr[0] != '-'))
    {
        if (boardstr[0] == 'x')
            return 1;
        else
            return 2;
    }
    // Check diagonal (top-right to bottom-left)
    if ((boardstr[4] == boardstr[8]) && (boardstr[8] == boardstr[12]) && (boardstr[4] != '-'))
    {
        if (boardstr[4] == 'x')
            return 1;
        else
            return 2;
    }
    // No winner
    for (int i = 0; i < 17; i++)
    {
        if (boardstr[i] == '-')
            return 3;
    }
    // draw
    return 0;
}

void initialize_BoardString(char *boardstr)
{
    int index = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            boardstr[index++] = '-'; // Empty cell represented by '-'
            if (j < 2)
                boardstr[index++] = '|'; // Add column separator
        }
        if (i < 2)
            boardstr[index++] = '\n'; // Add row separator
    }
    boardstr[index++] = '\n';
    boardstr[index] = '\0';
}

int update_boardstr(char boardstr[18], int player_no, char buffer[1024])
{
    int r, c = 0;
    if (buffer[1] != ' ')
        return -1;
    else
    {
        if (isdigit(buffer[0]) && isdigit(buffer[2]))
        {
            r = (int)buffer[0] - 48;
            c = (int)buffer[2] - 48;
            if ((r >= 1 && r <= 3) && (c >= 1 && c <= 3))
            {
                if (player_no == 1)
                {
                    if (boardstr[6 * (r - 1) + 2 * (c - 1)] == '-')
                    {
                        boardstr[6 * (r - 1) + 2 * (c - 1)] = 'x';
                        return checkWinner(boardstr);
                    }
                    else
                        return -1;
                }
                else if (player_no == 2)
                {
                    if (boardstr[6 * (r - 1) + 2 * (c - 1)] == '-')
                    {
                        boardstr[6 * (r - 1) + 2 * (c - 1)] = 'o';
                        return checkWinner(boardstr);
                    }
                    else
                        return -1;
                }
            }
            else
                return -1;
        }
        else
            return -1;
    }
}

int main(int argc,char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP address>\n", argv[0]);
        return 1;
    }
    char *ip = argv[1];
    
    struct sockaddr_in server_address;
    int server_fd, client_fd[MAX_CLIENTS] = {-1}, ready[MAX_CLIENTS] = {0}, addrlen = sizeof(server_address);
    char buffer_serv[1024] = {0}, boardstr[25] = {0};
    ssize_t bytes_received = 0;

    char *ready_msg = "ready\0";
    char *wait_msg = "Wait for the other client to connect,please don't type anything\0";
    char *query_msg = "If you are ready to play,please type ready\0";
    char *turn_info_msg = "Its your turn,please type a move\0";
    char *invalid_move_msg = "Invalid move,please type again\0";
    char *end_query_msg = "If u want to play again,please type yes otherwise type no\0";
    char *opponent_msg = "sorry,your opponent did not wish to play\0";
    char *waiting_msg = "waiting for other player\0";
    char *draw = "Its a Draw\0";
    char *player_1 = "Player 1 Wins!\0";
    char *player_2 = "Player 2 Wins!\0";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for %d clients to connect...\n", MAX_CLIENTS);
    int connected_clients = 0;
    while (connected_clients < MAX_CLIENTS)
    {
        int new_socket = accept(server_fd, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue; // Keep accepting until both clients are connected
        }
        // Store the client's socket file descriptor
        client_fd[connected_clients] = new_socket;
        printf("Client %d connected\n", connected_clients + 1);
        connected_clients++;
        // If only one client is connected,send wait message
        if (connected_clients == 1)
            send(client_fd[0], wait_msg, strlen(wait_msg), 0);
    }
    send(client_fd[1], "wait for my reply,please don't enter anything", 46, 0);
    printf("Both clients are connected, waiting for them to be ready...\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        while (1)
        {
            send(client_fd[i], query_msg, strlen(query_msg), 0);
            memset(buffer_serv, '\0', sizeof(buffer_serv));
            // read(client_fd[i], buffer, sizeof(buffer));
            bytes_received = recv(client_fd[i], buffer_serv, sizeof(buffer_serv) - 1, 0); // -1 to leave space for null terminator
            if (bytes_received < 0)
                perror("recv failed");
            else if (bytes_received == 0)
                printf("Client closed the connection.\n");
            else
            {
                buffer_serv[bytes_received] = '\0'; // Null-terminate the string
                printf("Received message: %s\n", buffer_serv);
            }
            if (strcmp(buffer_serv, ready_msg) == 0)
            {
                ready[i] = 1;
                printf("Client %d is ready\n", i + 1);
                break;
            }
        }
    }
    initialize_BoardString(boardstr);
    char msg_1[100] = {0};
    char msg_2[100] = {0};
    strcpy(msg_1, boardstr);
    strcpy(msg_2, boardstr);
    strcat(msg_1, "\nuse x to play\nIts your turn,please type a move\0");
    strcat(msg_2, "\nuse o to play\nwaiting for other player\0");
    send(client_fd[0], msg_1, strlen(msg_1), 0);
    send(client_fd[1], msg_2, strlen(msg_2), 0);
    int turn = 1;
    int result;
    int flag_end = 0;
    while (1)
    {
        result = -1;
        if (turn == 1)
        {
            while (result == -1)
            {
                memset(buffer_serv, '\0', sizeof(buffer_serv));
                bytes_received = recv(client_fd[0], buffer_serv, sizeof(buffer_serv) - 1, 0); // -1 to leave space for null terminator
                if (bytes_received < 0)
                    perror("recv failed");
                else if (bytes_received == 0)
                    printf("Client closed the connection.\n");
                else
                {
                    buffer_serv[bytes_received] = '\0'; // Null-terminate the string
                    printf("Received message: %s\n", buffer_serv);
                }
                result = update_boardstr(boardstr, 1, buffer_serv);
                if (result == -1)
                    send(client_fd[0], invalid_move_msg, strlen(invalid_move_msg), 0);
                else
                    break;
            }
            turn = 2;
        }
        else if (turn == 2)
        {
            while (result == -1)
            {
                memset(buffer_serv, '\0', sizeof(buffer_serv));
                bytes_received = recv(client_fd[1], buffer_serv, sizeof(buffer_serv) - 1, 0); // -1 to leave space for null terminator
                if (bytes_received < 0)
                    perror("recv failed");
                else if (bytes_received == 0)
                    printf("Client closed the connection.\n");
                else
                {
                    buffer_serv[bytes_received] = '\0'; // Null-terminate the string
                    printf("Received message: %s\n", buffer_serv);
                }
                result = update_boardstr(boardstr, 2, buffer_serv);
                if (result == -1)
                    send(client_fd[1], invalid_move_msg, strlen(invalid_move_msg), 0);
                else
                    break;
            }
            turn = 1;
        }
        if (turn == 2 && result == 3)
        {
            concatenate_and_send(client_fd, boardstr, waiting_msg, 0);
            concatenate_and_send(client_fd, boardstr, turn_info_msg, 1);
        }
        else if (turn == 1 && result == 3)
        {
            concatenate_and_send(client_fd, boardstr, turn_info_msg, 0);
            concatenate_and_send(client_fd, boardstr, waiting_msg, 1);
        }
        if (result == 0)
        {
            concatenate_and_send1(client_fd, boardstr, draw, end_query_msg, 0);
            concatenate_and_send1(client_fd, boardstr, draw, end_query_msg, 1);
            flag_end = 1;
        }
        else if (result == 1)
        {
            concatenate_and_send1(client_fd, boardstr, player_1, end_query_msg, 0);
            concatenate_and_send1(client_fd, boardstr, player_1, end_query_msg, 1);
            flag_end = 1;
        }
        else if (result == 2)
        {
            concatenate_and_send1(client_fd, boardstr, player_2, end_query_msg, 0);
            concatenate_and_send1(client_fd, boardstr, player_2, end_query_msg, 1);
            flag_end = 1;
        }
        if (flag_end == 1)
        {
            printf("Asking for one more game....\n");
            memset(msg_1, '\0', sizeof(msg_1));
            bytes_received = recv(client_fd[0], msg_1, sizeof(msg_1) - 1, 0); // -1 to leave space for null terminator
            if (bytes_received < 0)
                perror("recv failed");
            else if (bytes_received == 0)
                printf("Client closed the connection.\n");
            else
            {
                msg_1[bytes_received] = '\0'; // Null-terminate the string
                printf("Received message: %s\n", msg_1);
            }
            memset(msg_2, '\0', sizeof(msg_2));
            bytes_received = recv(client_fd[1], msg_2, sizeof(msg_2) - 1, 0); // -1 to leave space for null terminator
            if (bytes_received < 0)
                perror("recv failed");
            else if (bytes_received == 0)
                printf("Client closed the connection.\n");
            else
            {
                msg_2[bytes_received] = '\0'; // Null-terminate the string
                printf("Received message: %s\n", msg_2);
            }
            if (strcmp(msg_1, "yes") == 0 && strcmp(msg_2, "yes") == 0)
            {
                initialize_BoardString(boardstr);
                char msg_1[100] = {0};
                char msg_2[100] = {0};
                strcpy(msg_1, boardstr);
                strcpy(msg_2, boardstr);
                strcat(msg_1, "\nuse x to play\nIts your turn,please type a move\0");
                strcat(msg_2, "\nuse o to play\nwaiting for other player\0");
                send(client_fd[0], msg_1, strlen(msg_1), 0);
                send(client_fd[1], msg_2, strlen(msg_2), 0);
                turn = 1;
                flag_end = 0;
            }
            else if (strcmp(msg_1, "no") == 0 && strcmp(msg_2, "no") == 0)
            {
                printf("Both players exited. Closing the game.\n");
                close(client_fd[0]);
                close(client_fd[1]);
                break;
            }
            else if (strcmp(msg_1, "yes") == 0 && strcmp(msg_2, "no") == 0)
            {
                printf("Player 2 exited. Informing player 1.\n");
                close(client_fd[1]);
                send(client_fd[0], opponent_msg, strlen(opponent_msg), 0);
                close(client_fd[0]);
                break;
            }
            else if (strcmp(msg_1, "no") == 0 && strcmp(msg_2, "yes") == 0)
            {
                printf("Player 1 exited. Informing player 2.\n");
                close(client_fd[0]);
                send(client_fd[1], opponent_msg, strlen(opponent_msg), 0);
                close(client_fd[1]);
                break;
            }
        }
    }
    close(server_fd);
    return 0;
}