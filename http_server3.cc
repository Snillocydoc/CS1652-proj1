
 #include "minet_socket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>


#define FILENAMESIZE 100
#define BUFSIZE 1024

typedef enum { NEW,
	       READING_HEADERS,
	       WRITING_RESPONSE,
	       READING_FILE,
	       WRITING_FILE,
	       CLOSED } states;

typedef struct connection_s connection;

struct connection_s {
    int sock;
    int fd;
    char filename[FILENAMESIZE + 1];
    char buf[BUFSIZE + 1];
    char * endheaders;
    bool ok;
    long filelen;
    states state;

    int headers_read;
    int response_written;
    int file_read;
    int file_written;

    connection * next;
};

void read_headers(connection * con);
void write_response(connection * con);
void read_file(connection * con);
void write_file(connection * con);

int main(int argc, char * argv[]) {
	struct sockaddr_in saddr;
	char buf[BUFSIZE];
	int listen_fd=-1;
	int total_fds=0;
	fd_set descriptors;
    int server_port = -1;

    /* parse command line args */
    if (argc != 3) {
	fprintf(stderr, "usage: http_server3 k|u port\n");
	exit(-1);
    }

    server_port = atoi(argv[2]);

    if (server_port < 1500) {
	fprintf(stderr, "INVALID PORT NUMBER: %d; can't be < 1500\n", server_port);
	exit(-1);
    }
    

    /* initialize */
    if (toupper(*(argv[1])) == 'K') { 
	 
	 minet_init(MINET_KERNEL);
         
    } else if (toupper(*(argv[1])) == 'U') { 
	
	 minet_init(MINET_USER);
	 
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }


    /* initialize and make socket */
	if((listen_fd=minet_socket(SOCK_STREAM))<0){
		fprintf(stderr, "Socket creation failed\n");
		exit(-1);
	}
	total_fds=listen_fd;
	FD_SET(listen_fd,&descriptors);

    /* set server address*/
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_addr.s_addr=htonl(INADDR_ANY);
	saddr.sin_port=htons(server_port);


    /* bind listening socket */
	if(minet_bind(listen_fd,(struct sockaddr_in*)&saddr)<0){
		fprintf(stderr, "Binding failed\n");
		exit(-1);
	}

    /* start listening */
	if(minet_listen(listen_fd,32)<0){
		fprintf(stderr, "Listen failed\n");
		exit(-1);
	}

    /* connection handling loop */

    while (1) {
	/* create read and write lists */
	
	/* do a select */
	
	/* process sockets that are ready */
	
    }
}

void read_headers(connection * con) {

    /* first read loop -- get request and headers*/

    /* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/

    /* get file name and size, set to non-blocking */

    /* get name */

    /* try opening the file */
    
    /* set to non-blocking, get size */

    write_response(con);
}

void write_response(connection * con) {
    const char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";
    
    const char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"				\
	"</body></html>\n";
    
    /* send response */
  
    /* send headers */
}

void read_file(connection * con) {

}

void write_file(connection * con) {
 
}
