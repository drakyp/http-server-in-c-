
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
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

    //handling the concurrent connection with a while 1 
    while (1){

        // Accept an incoming client connection
        int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        // Print a message indicating that a client has connected
        printf("Client connected\n");


        //creating child process
        pid_t pid = fork();


        //checking if it is the child we do thing 
        if(pid == 0){

            //retrieveing the request 
            char readbuffer[1024];
            int bytesReceived = recv(fd, readbuffer, sizeof(readbuffer),0);


            //parsing with strtok to get the url 
            char *readpath = strtok(readbuffer, " ");
            readpath = strtok(NULL, " ");
            int byte_sent;

            //parsing for the user-agent part 
            char *user_agent = readpath;

            //int to go to not found in case nothing is found for /files case 


            //checking if the url is a /
            if(!strcmp(readpath, "/")){
                // Define a simple HTTP 200 OK response message

                char *resp_send = "HTTP/1.1 200 OK\r\n\r\n";
                // Send the HTTP response to the client

                byte_sent = send(fd, resp_send, strlen(resp_send), 0);
            }


            //handling the echo case
            else if (!strncmp(readpath, "/echo", strlen("/echo"))){
                //advance the readpath for the thing to echo
                readpath = readpath + strlen("/echo/");

                int cont_len = strlen(readpath);

                //creating a buffer that will be used for snprintf
                char resp_send[1024];

                //using snprintf(buf, max, "%s\n"%s) to try to print and save the different information
                snprintf(resp_send, sizeof(resp_send), "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: %d\r\n\r\n%s", cont_len, readpath);

                byte_sent = send(fd, resp_send, strlen(resp_send), 0);

            }

            //handling the user-agent case 
            else if (!strncmp(readpath, "/user-agent", strlen("/user-agent"))){
                user_agent = strtok(NULL, "\r\n");
                user_agent = strtok(NULL, "\r\n");
                user_agent = strtok(NULL, "\r\n");

                user_agent = user_agent + strlen("User-Agent: ");
                int user_len = strlen(user_agent);

                char user_rep[1024];

                snprintf(user_rep, sizeof(user_rep), "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: %d\r\n\r\n%s", user_len, user_agent );



                byte_sent = send(fd, user_rep, strlen(user_rep), 0);
            }


            //handling the return of a files
            else if (!strncmp(readpath, "/files", strlen("/files")))
            {
                // Check for --directory flag and get the directory path
                const char *directory = NULL;
                if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
                    directory = argv[2];
                } else {
                    fprintf(stderr, "Usage: %s --directory <path>\n", argv[0]);
                    return 1;
                }


                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", directory, readpath + strlen("/files/"));


                //need to be careful when giving the filepath and not the readpath 
                

                //check if the file exist and is readable 
                if(access(filepath, R_OK) == 0)
                {
                    //opening the file and reading it 
                    FILE *file = fopen(filepath, "r");
                    if (file == NULL) 
                        perror("Error opening file");

                    // Determine the file size
                    fseek(file, 0, SEEK_END);// move the pointer at the end of the file to know how many byte are in the file 
                    size_t size = ftell(file);// tell the current position of the file pointer 
                    fseek(file, 0, SEEK_SET);// put the file pointer at the start as it was


                   //buffer to read what is in the file, reading what is in the file and putting it into the buffer 
                    char rep[1024];

                    //putting what is in the file into the buffer
                    size_t byteRead = fread( rep, 1, sizeof(rep), file );
                    fclose(file);


                    //checking if we could read the file 
                    if( byteRead > 0){
                    //creating a buffer for the response to send into the right format
                    char rep_send[1024];

                    //trying to printf and putting everything together    
                    snprintf(rep_send, sizeof(rep_send) ,  "HTTP/1.1 200 OK\r\n" "Content-Type: application/octet-stream\r\n" "Content-Length: %zu\r\n\r\n%s",size, rep );


                    byte_sent = send(fd, rep_send, strlen(rep_send), 0);
                    }

                }


                //given by chatgpt


/*              // Check for --directory flag and get the directory path
                const char *directory = NULL;
                if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
                    directory = argv[2];
                } else {
                    fprintf(stderr, "Usage: %s --directory <path>\n", argv[0]);
                    return 1;
                }

                // Construct the full file path
                char filepath[1024];
                snprintf(filepath, sizeof(filepath), "%s/%s", directory, readpath + strlen("/files/"));

                // Check if the file exists and is readable
                if (access(filepath, R_OK) == 0) {
                    FILE *file = fopen(filepath, "r");
                    if (file == NULL) {
                        perror("Error opening file");
                        close(fd);
                        continue;
                    }

                    // Determine the file size
                    fseek(file, 0, SEEK_END);
                    size_t size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    // Allocate memory and read the file content
                    char *rep = malloc(size);
                    if (rep == NULL) {
                        perror("Error allocating memory");
                        fclose(file);
                        close(fd);
                        continue;
                    }
                    size_t byteRead = fread(rep, 1, size, file);
                    fclose(file);

                    if (byteRead > 0) {
                        // Send HTTP response headers
                        dprintf(fd, "HTTP/1.1 200 OK\r\n");
                        dprintf(fd, "Content-Type: application/octet-stream\r\n");
                        dprintf(fd, "Content-Length: %zu\r\n", byteRead);
                        dprintf(fd, "\r\n");

                        // Send the file content
                        send(fd, rep, byteRead, 0);

                        // Print the file content to stdout (for debugging)
                        printf("File content:\n%.*s\n", (int)byteRead, rep);
                    }
                    free(rep);
                }*/
                else {
                    char *resp_error = "HTTP/1.1 404 Not Found\r\n\r\n";

                    byte_sent = send(fd, resp_error, strlen(resp_error), 0 );


                }

            }

            //in case nothing is found return error 404
            else{
                char *resp_error = "HTTP/1.1 404 Not Found\r\n\r\n";

                byte_sent = send(fd, resp_error, strlen(resp_error), 0 );
            }
        }
        //parent process 
        else if ( pid > 0 ){
            close(fd); // closing the parent process
        }
        else{
            printf(" fork failed bruh");
        }

    }


    // Close the server socket
    close(server_fd);

    // Exit the program successfully
    return 0;
}
