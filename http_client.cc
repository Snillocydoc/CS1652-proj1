/* UNCOMMENT FOR MINET 
*/
 #include "minet_socket.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>


#define BUFSIZE 1024

int main(int argc, char * argv[]) {

   
    char * server_name = NULL;
    fd_set fd;
    struct hostent *host=(struct hostent*)malloc(sizeof(struct hostent));
    struct sockaddr_in myaddr;
    int server_port    = -1;
    int socket;
    char * server_path = NULL;
    char * req         = NULL;
    char * buf         = (char *)malloc(20);
    bool ok            = false;
	
	
	
    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }
	
	
    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];

    req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

    /* initialize */
    if (toupper(*(argv[1])) == 'K') { 
         
	minet_init(MINET_KERNEL);	
    } else if (toupper(*(argv[1])) == 'U') { 
	
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }

    /* make socket */
	if((socket=minet_socket(SOCK_STREAM))<0){
		fprintf(stderr, "Socket creation failed\n");
		exit(-1);
	}
	
    /* get host IP address  */
    /* Hint: use gethostbyname() */
	if((host=gethostbyname(server_name))<0){
		fprintf(stderr, "Getting host by name failed\n");
		exit(-1);
	}
	
    /* set address */
	myaddr.sin_family=AF_INET;
	memcpy(&myaddr.sin_addr.s_addr,host->h_addr,host->h_length);
	myaddr.sin_port=htons(server_port);

    /* connect to the server socket */
	minet_connect(socket, (struct sockaddr_in*)&myaddr);
	
    /* send request message */
    sprintf(req, "GET %s HTTP/1.0\r\n\r\n", server_path);

    /* wait till socket can be read. */
    /* Hint: use select(), and ignore timeout for now. */
	FD_ZERO(&fd);
	FD_SET(socket,&fd);
	minet_select(1,&fd,NULL,NULL,NULL);
//fflush(stdout);
    /* first read loop -- read headers */
	minet_read(socket, buf, 20);
    /* examine return code */  
	 

    //Skip "HTTP/1.0"
    //remove the '\0'

    // Normal reply has return code 200

    /* print first part of response: header, error code, etc. */
	buf[19]=0;
	printf("received: %s",buf);
    /* second read loop -- print out the rest of the response: real web content */

    /*close socket and deinitialize */

    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
