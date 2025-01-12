#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>  // for inet_pton, htons
#include <string.h>
#include <sys/socket.h>  // for socket (it's already in unistd.h)
#include <unistd.h>  // for close()
#include <errno.h>  // for errno
#include "solver.h"
#include <fcntl.h>  // for F_SETFL

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

int main(int argc, char *argv[])
{
    bool connection_type = 0;  // 0 - TCP, 1 - UDP
    if (argc > 1) {
        if (strcmp(argv[1], "-u") == 0)
            connection_type = 1;  // 0 - TCP, 1 - UDP
    }

    if (connection_type == 0)
        TCP_connection();
    else
        UDP_connection();

    return 0;
}

void TCP_connection()
{
    // Configuring server address:
    struct in_addr local_ip;
    inet_pton(AF_INET, "0.0.0.0", &local_ip);  // convert our ip into local_ip, AF_INET for IPv4
    struct sockaddr_in local_addr_in;  // struct for ip & port
    memset(&local_addr_in, 0, sizeof(struct sockaddr_in));  // memory set, fill the struct with 0
    local_addr_in.sin_addr = local_ip;
    local_addr_in.sin_port = htons(LOCAL_PORT);  // Host To Network Short
    local_addr_in.sin_family = AF_INET;  // for IPv4
    struct sockaddr* local_addr = (struct sockaddr*)&local_addr_in;
    int local_addrlen = sizeof(struct sockaddr_in);  // length of struct

    int connector = socket(AF_INET, SOCK_STREAM, 0);  // socket to establish the connection. IPv4, TCP (type), protocol for sock_stream (0 or IPPROTO_TCP)
    if (connector < 0)
    {
        printf("# Error creating socket-connector.\n");
        return;
    }

    /*
    // Allows SO_REUSEADDR for the connector,
    // so this socket will be able to work on closed ports:
    int opt_val = 1;
    setsockopt(connector, SOL_SOCKET, SO_REUSEADDR, &opt_val);
    */

    short int bind_error = bind(connector, local_addr, local_addrlen);  // 0 if no errors
    if (bind_error != 0)
    {
        printf("# Error binding the socket: %d.\n", errno);
        close(connector);
        return;
    }

    printf("# TCP server is listening on port %d\n", LOCAL_PORT);

    while (1)
    {
        printf("# Listening for connections...\n");
        listen(connector, MAX_CONN);  // listening for new connections, like bluetooth devices

        struct sockaddr client_addr;
        int client_addrlen = sizeof(struct sockaddr);  //sockaddr_in?
        short int tcp_socket = accept(connector, &client_addr, &client_addrlen);  // connecting to the client using his address
        // through this socket-connection we will send/receive messages

        if (tcp_socket <= 0)
        {
            printf("# Error establishing connection with code: %d.\n", errno);
            break;
        }
        else
            printf("# Connection established.\n");

        while (1)
        {
            // Receiving a task from the client
            char msg[31];

            int msg_len = recv(tcp_socket, msg, sizeof(msg) - 1, 0);  // receiving the [msg, max msg length, flag] from the client
            if (msg_len <= 0)
            {
                printf("# Error receiving the message (client disconnected): %d.\n", errno);
                break;
            }

            msg[msg_len] = '\0';
            printf("# Received task from the client: %s\n", msg);

            // Solving the task
            struct SolverData data = call_solver(msg);
            double res = data.result;
            unsigned short error = data.error;
            if (error == 0)
            {
                printf("Answer is: %f\n", res);

                char str_res[31];
                sprintf(str_res, "%f", res);  // double to string
                str_res[sizeof(str_res) - 1] = '\0';

                // Sending the answer to the client
                if (send(tcp_socket, str_res, strlen(str_res), 0) == -1)  // 0 - flag
                    printf("# Error sending the message: %d.\n", errno);
                else
                    printf("# Message has been delivered to the client.\n");
            }
            else
            {
                char str_res[31];
                if (error == 1)
                {
                    printf("# Error: Wrong task format\n");
                    strcpy(str_res, "# Error: Wrong task format");
                }
                else if (error == 2)
                {
                    printf("# Error: Division by zero\n");
                    strcpy(str_res, "# Error: Division by zero");
                }

                // Sending an error to the client
                if (send(tcp_socket, str_res, strlen(str_res), 0) == -1)  // 0 - flag
                    printf("# Error sending the message: %d.\n", errno);
                else
                    printf("# Message has been delivered to the client.\n");
                continue;
            }
        }
        close(tcp_socket);
    }
    close(connector);
}

#define RETRY_LIMIT 3  // limit of retry attempts for connecting to the server
#define TIMEOUT_SEC 3
#define ACK_MESSAGE "ACK"  // ACKnowledgement message
void UDP_connection()
{
    // Configuring server's address settings:
    struct in_addr local_ip;
    inet_pton(AF_INET, "0.0.0.0", &local_ip);
    struct sockaddr_in local_addr_in;
    memset(&local_addr_in, 0, sizeof(struct sockaddr_in));  // set zeros
    local_addr_in.sin_addr = local_ip;  // set local ip
    local_addr_in.sin_port = htons(LOCAL_PORT);  // set port
    local_addr_in.sin_family = AF_INET;  // IPv4

    // Creating UDP socket:
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);  // AF_INET for IPv4
    if (udp_socket < 0) {
        printf("# UDP Socket creation failed.");
        return;
    }

    // Setting nonblocking status of socket:
    // fcntl(udp_socket, F_SETFL, O_NONBLOCK);

    // Binding the socket:
    struct sockaddr* local_addr = (struct sockaddr*)&local_addr_in;
    int local_addrlen = sizeof(struct sockaddr_in);  // length of struct
    if (bind(udp_socket, local_addr, local_addrlen) < 0) {
        printf("# Bind failed.");
        close(udp_socket);
        return;
    }

    printf("# UDP server is listening on port %d.\n", LOCAL_PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        char msg[31];

        // Receiving the task from the client:
        printf("\n# Waiting for the task...\n");
        int msg_len = recvfrom(udp_socket, msg, sizeof(msg), 0, (struct sockaddr*)&client_addr, &client_addrlen);
        if (msg_len < 0) {
            printf("# Receiving failed.\n");
            continue;
        }
        msg[msg_len] = '\0';
        printf("# New task from the client: %s\n", msg);

        // Sending the confirm msg to the client:
        int sendto_result = sendto(udp_socket, ACK_MESSAGE, strlen(ACK_MESSAGE), 0, (struct sockaddr*)&client_addr, client_addrlen);
        if (sendto_result == -1)
        {
            printf("# Error sending the ACK msg to the client (sendto error).\n");
            break;
        }
        printf("# Confirmation has been sent to the client.\n", msg);

        // Solving the task:
        struct SolverData data = call_solver(msg);
        double res = data.result;
        unsigned short error = data.error;
        char response[31];

        if (error == 0)
        {
            snprintf(response, sizeof(response), "%f", res);
            printf("# Answer is: %f\n", res);
        }
        else
        {
            if (error == 1)
            {
                printf("# Error: Wrong task format\n");
                strncpy(response, "# Error: Wrong task format", sizeof(response) - 1);
            }
            else if (error == 2)
            {
                printf("# Error: Division by zero\n");
                strncpy(response, "# Error: Division by zero", sizeof(response) - 1);
            }
            response[sizeof(response) - 1] = '\0';
        }

        // Sending the answer to the client:
        int flag_sendto_error = false;
        printf("# Trying to send the answer to the client...\n");
        int attempts = 0;
        while (attempts < RETRY_LIMIT)
        {
            int sendto_result = sendto(udp_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, client_addrlen);
            if (sendto_result == -1)  // how many bytes have been sent
            {
                printf("# Error sending the answer to the client (sendto error).\n");
                flag_sendto_error = true;
                break;
            }

            usleep(10);  // waiting 10 ms before trying to receive the confirm msg

            // Receiving the confirmation from the client:
            char confirm_msg[5];
            memset(confirm_msg, 0, sizeof(confirm_msg));

            int msg_len = recvfrom(udp_socket, confirm_msg, sizeof(confirm_msg), 0, NULL, NULL);
            confirm_msg[msg_len] = '\0';

            if (msg_len > 0 && strncmp(confirm_msg, ACK_MESSAGE, msg_len) == 0)  // msg_len > 0 and msg equals the confirmation msg (ACK_MESSAGE)
            {
                printf("# Client has received the answer.\n");
                attempts = 0;
                break;  // msg has been received, going to the next step
            }
            else
            {
                attempts++;
                printf("# Client hasn't received the msg. Retrying... (%d/3)\n", attempts);
                sleep(1);  // waiting 1 sec
                continue;  // trying to receive the confirmation msg again
            }
        }
        if (attempts >= RETRY_LIMIT)
        {
            printf("# Client has been disconnected.\n");
            continue;  // waiting for the new task
        }
        if (flag_sendto_error == true)  // error, msg hasn't been sent
            continue;
    }
    close(udp_socket);
}
