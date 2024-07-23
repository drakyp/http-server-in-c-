
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
    // Disable output buffering for stdout and stderr
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    // Declare variables for server socket and client address length
    int server_fd, client_addr_len;
    // Declare structure to store client address
    struct sockaddr_in client_addr;
    
    // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Check if socket creation was successful
    if (server_fd == -1) {
        // Print error message and exit if socket creation failed
        printf("Socket creation failed: %s...\n", strerror(errno));
        return 1;
    }
    
    // Setting the SO_REUSEADDR option to avoid "Address already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        // Print error message and exit if setting socket option failed
        printf("SO_REUSEADDR failed: %s \n", strerror(errno));
        return 1;
    }
    
    // Define server address structure and initialize it
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET, // IPv4
        .sin_port = htons(4221), // Port number 4221, converted to network byte order
        .sin_addr = { htonl(INADDR_ANY) }, // Accept connections from any IP address
    };
    
    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        // Print error message and exit if binding failed
        printf("Bind failed: %s \n", strerror(errno));
        return 1;
    }
    
    // Set the socket to listen for incoming connections with a backlog of 5
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        // Print error message and exit if listening failed
        printf("Listen failed: %s \n", strerror(errno));
        return 1;
    }
    
    // Print a message indicating that the server is waiting for a client to connect
    printf("Waiting for a client to connect...\n");
    // Set client address length
    client_addr_len = sizeof(client_addr);
    
    // Accept an incoming client connection
    int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
    // Print a message indicating that a client has connected
    printf("Client connected\n");

    //parsing the url 
    char readbuffer[1024];
    char path[512];
    int bytesReceived = recv(fd, readbuffer, sizeof(readbuffer),0);

    //parsing with strtok
    char *readpath = strtok(readbuffer, " ");
    readpath = strtok(NULL, " ");
    int byte_sent;

    if(!strcmp(readpath, "/")){
        // Define a simple HTTP 200 OK response message

        char *resp_send = "HTTP/1.1 200 OK\r\n\r\n";
        // Send the HTTP response to the client

        byte_sent = send(fd, resp_send, strlen(resp_send), 0);
    }
    else{
        char *resp_error = "HTTP/1.1 404 Not Found\r\n\r\n";
        
        byte_sent = send(fd, resp_error, strlen(resp_error), 0 );
    }

          
    // Close the server socket
    close(server_fd);

    // Exit the program successfully
    return 0;
}
