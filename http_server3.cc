
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
	
	fd_set read_fds;
	fd_set write_fds;
	connection *listen=(connection *)malloc(sizeof(connection));
	connection *max;
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
	if((listen->sock=minet_socket(SOCK_STREAM))<0){
		fprintf(stderr, "Socket creation failed\n");
		exit(-1);
	}
	
	max=listen;
	FD_SET(listen->sock,&read_fds);

    /* set server address*/
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_addr.s_addr=htonl(INADDR_ANY);
	saddr.sin_port=htons(server_port);


    /* bind listening socket */
	if(minet_bind(listen->sock,(struct sockaddr_in*)&saddr)<0){
		fprintf(stderr, "Binding failed\n");
		exit(-1);
	}

    /* start listening */
	if(minet_listen(listen->sock,32)<0){
		fprintf(stderr, "Listen failed\n");
		exit(-1);
	}

    /* connection handling loop */

    while (1) {
	/* create read and write lists */
	
	int counter=0;
	connection* node_conn;
	connection *new_conn;
	FD_SET(listen->sock,&read_fds);
	/* do a select */
	minet_select(max->sock+1,&read_fds,&write_fds,NULL,NULL);
	
	/* process sockets that are ready */
	node_conn=listen;
	while(node_conn!=NULL)
		if(FD_ISSET(node_conn->sock,&read_fds)&&node_conn->sock==listen->sock){
			new_conn=(connection*)malloc(sizeof(connection));
			max->next=new_conn;
			max=max->next;
			new_conn->sock=minet_accept(listen->sock,NULL);
			FD_SET(new_conn->sock,&read_fds);
			FD_SET(new_conn->sock,&write_fds);
			
		}
		else if(FD_ISSET(node_conn->sock,&read_fds)&&node_conn!=listen){
			read_headers(node_conn);
			FD_CLR(node_conn->sock,&read_fds);
		}
	}
	
    }
}

void read_headers(connection * con) {


    /* first read loop -- get request and headers*/
	if(minet_read(con->sock,con->buf,BUFSIZE)<0){
		fprintf(stderr, "Read failed\n");
		exit(-1);
	}
    
	/* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/
    /* get file name and size, set to non-blocking */
    /* get name */
	con->filename=strtok(con->buf,"GET ");	
     /* try opening the file */
	if((con->fd = fopen(con->filename,"rb"))>0){
		con->ok=true;
		 /* set to non-blocking, get size */
		fseek(con->fd,0,SEEK_END);
		con->filelen=ftell(fd);
		fseek(con->fd,0,SEEK_SET);
		

	}
	else{
		con->ok=false;
	}
    
   

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
	if(con->ok==true){
		if(minet_write(con->sock,(char *)ok_response_f,strlen(ok_response_f))<0){
			fprintf(stderr,"Write failed\n");
			exit(-1);
		}
	}
	else{
		// send error response
		if(minet_write(con->sock,(char*)notok_response,strlen(notok_response))<0){
			fprintf(stderr,"Write failed\n");
			exit(-1);
		}
	}
  
    /* send headers */
}

void read_file(connection * con) {

}

void write_file(connection * con) {
 
}
