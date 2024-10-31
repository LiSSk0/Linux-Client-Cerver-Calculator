#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>  // for inet_pton, htons
#include <string.h>
#include <sys/socket.h>  // for socket. optional cuz its already in unistd.h
#include <unistd.h>  // for close()
#include <errno.h>  // for errno
#include "solver.h"

#define MAX_CONN 5  // max socket connections
#define LOCAL_PORT 8888  // server's port


struct SolverData call_solver(char msg[])
{
    remove_spaces(msg);
    int msg_len = strlen(msg);

    struct SolverData solver_data;
    solver_data = task_solver(msg, msg_len);

    return solver_data;
}

int main()
{
    struct in_addr local_ip;
    inet_pton(AF_INET, "0.0.0.0", &local_ip);  // convert our ip into local_ip, AF_INET for IPv4

    struct sockaddr_in local_addr_in;  // struct for ip & port
    memset(&local_addr_in, 0, sizeof(struct sockaddr_in));  // memory set, fill the struct with 0
    local_addr_in.sin_addr = local_ip;
    local_addr_in.sin_port = htons(LOCAL_PORT);  // Host To Network Short
    local_addr_in.sin_family = AF_INET;  // for IPv4
    struct sockaddr* local_addr = (struct sockaddr*)&local_addr_in;
    int local_addrlen = sizeof(struct sockaddr_in);  // length of struct

    int connector = socket(AF_INET, SOCK_STREAM, 0);  // socket to establish the connection. IPv4, TCP (type), protocol for sock_stream (0 or IPPROTO_TCP
    // printf("connector: %d\n", connector);  // -1 - error
    // printf("Error code: %d\n", errno);  // errno - global variable for errors. google: socket error codes linux

    /*
    // Allows SO_REUSEADDR for the connector,
    // so this socket will be able to work on closed ports:
    int opt_val = 1;
    setsockopt(connector, SOL_SOCKET, SO_REUSEADDR, &opt_val);
    */


    short int bind_error = bind(connector, local_addr, local_addrlen);  // 0 if no errors
    if (bind_error != 0)
        printf("Error code: %d.\n", errno);

    while (1)
    {
        listen(connector, MAX_CONN);  // listening for new connections, like bluetooth devices

        struct sockaddr remote_addr;
        int remote_addrlen = sizeof(struct sockaddr);  //sockaddr_in?
        short int tcp_socket = accept(connector, &remote_addr, &remote_addrlen);  // connecting to the client using his address
        // through this socket-connection we will send/receive messages

        if (tcp_socket <= 0)
        {
            printf("Error establishing connection with code: %d.\n", errno);
            break;
        }
        else
            printf("Connection established.\n");

        while (1)
        {
            // Receiving a task from the client
            char msg[100];

            int msg_len = recv(tcp_socket, msg, sizeof(msg) - 1, 0);  // receiving the [msg, max msg length, flag] from the client
            if (msg_len <= 0)
            {
                printf("Error receiving the message (client disconnected): %d.\n", errno);
                // continue;
                break;
            }

            msg[msg_len] = '\0';
            printf("Received a task from the client: %s\n", msg);

            // Solving the task
            struct SolverData data = call_solver(msg);
            double res = data.result;
            unsigned short error = data.error;
            if (error == 0)
            {
                printf("Answer is: %f\n", res);

                char str_res[100];
                sprintf(str_res, "%f", res);  // double to string
                str_res[sizeof(str_res) - 1] = '\0';

                // Sending the answer to the client
                if (send(tcp_socket, str_res, strlen(str_res), 0) == -1)  // 0 - flag
                    printf("Error sending the message: %d.\n", errno);
                else
                    printf("Message has been delivered to the client.\n");
            }
            else
            {
                char str_res[100];
                if (error == 1)
                {
                    printf("Error: Wrong task format");
                    strcpy(str_res, "Error: Wrong task format");
                }
                else if (error == 2)
                {
                    printf("Error: Division by zero");
                    strcpy(str_res, "Error: Division by zero");
                }

                // Sending an error to the client
                if (send(tcp_socket, str_res, strlen(str_res), 0) == -1)  // 0 - flag
                    printf("Error sending the message: %d.\n", errno);
                else
                    printf("Message has been delivered to the client.\n");
                continue;
            }
        }
        close(tcp_socket);
    }
    close(connector);
    return 0;
}

/*
cd coding/server
gcc main.c -o main
./srv main
*/
