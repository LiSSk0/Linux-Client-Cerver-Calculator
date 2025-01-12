#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>  // for inet_pton, htons
#include <string.h>
#include <errno.h>  // for errno
#include <unistd.h>  // for close()
#include <fcntl.h>  // for F_SETFL

#include "validation.h"  // checks validation of tasks

#define REMOTE_PORT 8888  // server's port
#define REMOTE_IP "127.0.0.1"  // server's ip

int main(int argc, char* argv[])
{
    bool connection_type = 0;  // 0 - TCP, 1 - UDP
    int server_port;
    char *server_ip;

    // Parcing console data:
    if (argc == 3) {
        server_ip = argv[1];
        server_port = atoi(argv[2]); // convert port number to int
    } else if (argc == 4) {
        if (strcmp(argv[1], "-u") == 0) {
            connection_type = 1;  // set UDP connection
            server_ip = argv[2];
            server_port = atoi(argv[3]);
        } else {
            // fprintf(stderr, "Wrong usage. Correct: %s [-u] <IP-address> <port-number>\n", argv[0]);
            printf("Wrong usage. Correct: ./main [-u] <IP-address> <port-number>\n");
            return 0;
        }
    } else {
        printf("Wrong usage. Correct: ./main [-u] <IP-address> <port-number>\n");
        return 0;
    }

    printf("# IP: %s, Port: %d\n", server_ip, server_port);
    // Start connection:
    if (connection_type == 0)
    {
        TCP_connection(server_ip, server_port);
    }
    else
    {
        UDP_connection(server_ip, server_port);
    }
    return 0;
}

void TCP_connection(char *server_ip, int server_port)
{
    // Configuring server address:
    struct in_addr remote_ip;
    struct sockaddr_in remote_addr_in;  // struct for server's ip & port
    memset(&remote_addr_in, 0, sizeof(struct sockaddr_in));  // fill the struct with 0
    //remote_addr_in.sin_addr = remote_ip;
    remote_addr_in.sin_port = htons(server_port);  // Host To Network Short
    remote_addr_in.sin_family = AF_INET;  // for IPv4
    //struct sockaddr* remote_addr = (struct sockaddr*)&remote_addr_in;
    //int remote_addrlen = sizeof(struct sockaddr_in);  // length of struct
    //inet_pton(AF_INET, server_ip, &remote_ip);  // set our ip into local_ip, AF_INET for IPv4

    // Verification of IP-address:
    if (inet_pton(AF_INET, server_ip, &(remote_addr_in.sin_addr)) <= 0)  // for IPv4
    {
        printf("Error: Incorrect IP-address.\n");
        return 0;
    }

    // Creating TCP socket:
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  // IPv4, TCP (type), protocol for sock_stream (0 or IPPOTOCOL)
    if (tcp_socket < 0) {
        printf("# Error creating TCP-socket with code: %d.\n", errno);
        // printf("Error code: %d\n", errno);  // errno - global variable for errors (socket error codes linux)
        return 0;
    }

    // Connecting to the server:
    if (connect(tcp_socket, (struct sockaddr *)&remote_addr_in, sizeof(remote_addr_in)) < 0)
    {
        printf("# Error establishing connection with code: %d.\n", errno);
        close(tcp_socket);
        return 0;
    }

    printf("# Connection established. Enter '!exit' to exit.\n");

    // Connection cycle:
    while (1)
    {
        char msg[31];
        printf("\nEnter the math task: ");
        if (fgets(msg, sizeof(msg), stdin) != NULL)
            msg[strcspn(msg, "\n")] = 0;  // remove the newline character at the end
        else
            continue;  // skipping iterration if empty input

        if (strcmp(msg, "!exit") == 0)  // comparing char massive to string
            break;

        // Validating the task:
        remove_spaces(msg);  // removes spaces from the task
        short int errors = task_solver(msg, strlen(msg));
        if (errors != 0) {
            // printf("Error: wrong task's format. Code: %d.\n", errors;
            continue;
        }

        // Sending a task to the server:
        if (send(tcp_socket, msg, strlen(msg), 0) == -1)
        {
            printf("Error sending the message: %d.\n", errno);
            continue;
        }

        // Receiving the answer from the server:
        char answer[31];
        int answer_len = recv(tcp_socket, answer, sizeof(answer) - 1, 0);  // answer, max answer length, flag
        if (answer_len > 0)
        {
            answer[answer_len] = '\0';   // last byte is 0 because string must end with 0
            printf("%s\n", answer);
        }
        else
            printf("Error receiving the message: %d.\n", errno);

        memset(msg, 0, sizeof(msg));  // clean msg msg for the next input
    }

    close(tcp_socket);
}

#define RETRY_LIMIT 3  // limit of retry attempts for connecting to the server
#define TIMEOUT_SEC 3
#define ACK_MESSAGE "ACK"  // acknowledgement message
void UDP_connection(char *server_ip, int server_port)
{
    // Configuring server's address settings:
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);  // Host To Network Short
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        printf("# Error: invalid IP address.\n");
        return;
    }

    // Creating UDP socket:
    int udp_socket, attempts = 0;
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)   // AF_INET for IPv4
    {
        printf("# UDP Socket creation failed.\n");
        return;
    }

    // Setting nonblocking status of socket:
    fcntl(udp_socket, F_SETFL, O_NONBLOCK);

    // Interaction with the server:
    int flag_sendto_error = false;
    while (1)
    {
        char msg[31];
        printf("\nEnter the math task: ");
        if (fgets(msg, sizeof(msg), stdin) != NULL)
            msg[strcspn(msg, "\n")] = 0;  // removing the newline character at the end
        else
        {
            printf("Message is empty.\n");
            continue;  // skipping iterration if empty input
        }

        if (strcmp(msg, "!exit") == 0)  // comparing char massive to string
            return;  // exitting the program

        // Validating the task:
        remove_spaces(msg);  // removes spaces from the task
        short int errors = task_solver(msg, strlen(msg));  // checking errors
        if (errors != 0) {
            // printf("# Error: wrong task's format. Code: %d.\n", errors;
            continue;  // skipping iteration if error
        }

        // Sending a task to the server:
        printf("# Trying to send the task to the server...\n");
        while (attempts < RETRY_LIMIT)
        {
            int sendto_res = sendto(udp_socket, msg, strlen(msg), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));  // socket, buffer, buffer_len, flag, to whom, len to whom
            if (sendto_res == -1)  // how many bytes have been sent
            {
                printf("# Error sending the task to the server (sendto error).\n");
                flag_sendto_error = true;
                break;
            }

            usleep(10);  // waiting 10 ms before trying to receive the confirm msg

            // Receiving the confirmation from the server:
            char confirm_msg[5];
            memset(confirm_msg, 0, sizeof(confirm_msg));

            int msg_len = recvfrom(udp_socket, confirm_msg, sizeof(confirm_msg), 0, NULL, NULL);
            confirm_msg[msg_len] = '\0';

            if (msg_len > 0 && strncmp(confirm_msg, ACK_MESSAGE, strlen(ACK_MESSAGE)) == 0)  // msg_len > 0 and msg equals the confirmation msg (ACK_MESSAGE)
            {
                printf("# Server has received the task.\n");
                attempts = 0;
                break;  // msg has been received, going to the next step
            }
            else
            {
                attempts++;
                printf("# Server hasn't received the msg. Retrying... (%d/3)\n", attempts);
                sleep(1);  // waiting 1 sec
                continue;  // trying to receive the confirmation msg again
            }
        }
        if (attempts >= RETRY_LIMIT)
        {
            printf("# Connection to the server has been lost.\n");
            close(udp_socket);
            return;
        }
        if (flag_sendto_error == true)  // error, msg hasn't been sent
            continue;

        // Receiving the answer:
        char answer[31];
        int answer_len = recvfrom(udp_socket, answer, sizeof(answer), 0, NULL, NULL);  // answer, max answer length, flag
        if (answer_len > 0)
        {
            answer[answer_len] = '\0';   // last byte is 0 because string must end with 0
            printf("%s.\n", answer);

            // Sending the confirm msg to the server:
            sendto(udp_socket, ACK_MESSAGE, strlen(ACK_MESSAGE), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
        }
        else
            printf("# Error receiving the answer (empty).\n");

        memset(msg, 0, sizeof(msg));  // clean msg msg for the next input
    }

    close(udp_socket);
}
