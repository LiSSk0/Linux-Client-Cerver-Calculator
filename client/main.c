#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>  // for inet_pton, htons
#include <string.h>
#include <errno.h>  // for errno
#include <unistd.h>  // for close()

#include "validation.h"  // checks validation of tasks

#define REMOTE_PORT 8888  // server's port
#define REMOTE_IP "127.0.0.1"  // server's ip

int main(int argc, char* argv[])
{
    // parcing console data
    if (argc != 3) {  // check if the number of arguments is correct
        fprintf(stderr, "Wrong usage. Correct: %s <IP-address> <port-number>\n", argv[0]);
        return 0;
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]); // convert port number to int


    struct in_addr remote_ip;
    inet_pton(AF_INET, server_ip, &remote_ip);  // set our ip into local_ip, AF_INET for IPv4

    struct sockaddr_in remote_addr_in;  // struct for ip & port
    memset(&remote_addr_in, 0, sizeof(struct sockaddr_in));  // memory set, fill the struct with 0
    remote_addr_in.sin_addr = remote_ip;
    remote_addr_in.sin_port = htons(server_port);  // Host To Network Short
    remote_addr_in.sin_family = AF_INET;  // for IPv4
    struct sockaddr* remote_addr = (struct sockaddr*)&remote_addr_in;
    int remote_addrlen = sizeof(struct sockaddr_in);  // length of struct

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  // IPv4, TCP (type), protocol for sock_stream (0 or IPPOTOCOL)
    // printf("tcp_socket: %d\n", tcp_socket);  // -1 - error
    // printf("Error code: %d\n", errno);  // errno - global variable for errors. find in google: socket error codes linux

    short int connect_error = connect(tcp_socket, remote_addr, remote_addrlen);  // connecting to the server
    if (connect_error != 0)
    {
        printf("Error establishing connection with code: %d.\n", errno);
        close(tcp_socket);
        return 0;
    }

    printf("Connection established. Enter '!exit' to exit.\n");

    while (1)
    {
        char msg[100];
        printf("Enter the math task: ");
        if (fgets(msg, sizeof(msg), stdin) != NULL)
            msg[strcspn(msg, "\n")] = 0;  // remove the newline character at the end
        else
            continue;  // skipping iterration if empty input

        if (strcmp(msg, "!exit") == 0)  // comparing char massive to string
            break;

        // Validating the task:
        remove_spaces(msg);  // removes spaces from the task
        short int errors = task_solver(msg, strlen(msg));  // checking errors
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
        char answer[100];
        int answer_len = recv(tcp_socket, answer, sizeof(answer) - 1, 0);  // answer, max answer length, flag
        if (answer_len > 0)
        {
            answer[answer_len] = '\0';   // last byte is 0 because string must end with 0
            printf("%s.\n", answer);
        }
        else
            printf("Error receiving the message: %d.\n", errno);

        memset(msg, 0, sizeof(msg));  // clean msg buffer for the next input
    }

    close(tcp_socket);
    return 0;
}
