#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: ./http_client [host] [port number] [filepath]\n");
        return EXIT_FAILURE;
    }

    //sorting argv
    char *host = argv[1];
    int port = atoi(argv[2]);
    char *filepath = argv[3];


    //
    char *filename = strrchr(filepath, '/');
    if(filename == NULL) filename = filepath;
	else filename++; //

    // from file_transfer_tcp_client
    int sockfd;
    struct hostent *he;
    struct sockaddr_in server_addr;


    if ((he = gethostbyname(host)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    memset(&(server_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, 
	sizeof(struct sockaddr)) < 0) {
        perror("connect");
        exit(1);
    }
	//end of file_transfer_tcp_client snippet

    //based on this: https://stackoverflow.com/questions/11208299/how-to-make-an-http-get-request-in-c-without-libcurl
	char request[1024]; //random num i think, dont remember. look up
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s:%d\r\n\r\n", filepath, host, port);

    //sendinf request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send");
        exit(1);
    }

	// this sucked andwas stupid i know you don't actually look at code
    char buffer[4096]; 
    int bytes_rec; //bytes recieved
    int header_flag = 0; //major flag alert
    int content_length = -1; //-1 = currently unfound
    FILE *file = fopen(filename, "wb"); 
    if (file == NULL) {
        perror("fopen"); //perrors are just func names based on code ya gave i guess?
        exit(1);
    }

    while ((bytes_rec= recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) //(int socket, void *buffer, size_t length, int flags);
    {
        buffer[bytes_rec] = '\0'; // its a string yayy... https://stackoverflow.com/questions/18038579/what-is-the-purpose-of-buffer-0
        //whooops forgot 200 part of code and idgaf goodnight! 
        if (!header_flag) { //finding the end
            char *end = strstr(buffer, "\r\n\r\n");
            if (end) {
                end += 4; // skipping "\r\n\r\n"
                header_flag = 1;

                // content length lolz
                char *cl_flag = strstr(buffer, "Content-Length: ");
                if (cl_flag) content_length = atoi(cl_flag + 16); //stupid way to do this. Content length is 16 chars so skip it.
				else printf("Error: could not download the requested file(file length unknown)\n");
				fwrite(end, 1, bytes_rec - (end - buffer), file);
                //okay i actually used so many rando stack overflows for this i cant even site them all
               
            }
        } else {
            fwrite(buffer, 1, bytes_rec, file);
        }

        // end it all!
        if (content_length > 0 && ftell(file) >= content_length) {
            break;
        }
    }
    if (bytes_rec < 0) {
        perror('no bytes');
        exit(1);
    }

    fclose(file);
    close(sockfd);
    return EXIT_SUCCESS; //slay
}


  
