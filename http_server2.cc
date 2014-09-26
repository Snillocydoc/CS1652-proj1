
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


#define BUFSIZE 1024
#define FILENAMESIZE 100

int handle_connection(int sock);

int main(int argc, char * argv[]) {
	struct sockaddr_in saddr;
	char buf[BUFSIZE];
	int listen_fd=-1;
	int total_fds=0;
	fd_set descriptors;
    int server_port = -1;
    int rc          =  0;
    int sock        = -1;

    /* parse command line args */
    if (argc != 3) {
	fprintf(stderr, "usage: http_server1 k|u port\n");
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
    /* connection handling loop: wait to accept connection */

    while (1) {
	
	/* create read list */
	
	int counter=0;
	int new_fd=0;
	/* do a select */
	FD_SET(listen_fd,&descriptors);
	if(minet_select(total_fds+1,&descriptors,NULL,NULL,NULL)<0){
		fprintf(stderr,"Select error");
		exit(-1);
	}

	

	/* process sockets that are ready */
	for(counter=0;counter<=total_fds;counter++){
		if(minet_select(total_fds+1,&descriptors,NULL,NULL,NULL)<0){
			fprintf(stderr,"Select error");
			exit(-1);
		}
		/* for the accept socket, add accepted connection to connections */
		if(FD_ISSET(counter,&descriptors)&&counter==listen_fd){
printf("New socket!");
fflush(stdout);
			new_fd=minet_accept(listen_fd,NULL);
			FD_SET(new_fd,&descriptors);
			
			if(new_fd>total_fds)
				total_fds=new_fd;
		}
		/* for a connection socket, handle the connection */
		else if(FD_ISSET(counter,&descriptors)&&counter!=listen_fd) {

			rc = handle_connection(counter);

			FD_CLR(counter,&descriptors);
			
		}
	}



	
	
	
	
	
    }
}

int handle_connection(int sock) {
    bool ok = false;
	char buf[BUFSIZE];
	char *content;
	char* filename=(char*)malloc(FILENAMESIZE);
	FILE * fd;
	int f_size=0;
	int counter=0;
	int bytes_read=0;

    const char * ok_response_f = "HTTP/1.0 200 OK\r\n"	\
	"Content-type: text/plain\r\n"			\
	"Content-length: %d \r\n\r\n";
    
    const char * notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"	\
	"Content-type: text/html\r\n\r\n"			\
	"<html><body bgColor=black text=white>\n"		\
	"<h2>404 FILE NOT FOUND</h2>\n"				\
	"</body></html>\n";
   
    /* first read loop -- get request and headers*/
	if(minet_read(sock,buf,BUFSIZE)<0){
		fprintf(stderr, "Read failed\n");
		exit(-1);
	}

    /* parse request to get file name */
    /* Assumption: this is a GET request and filename contains no spaces*/
	filename=strtok(buf,"GET ");
    /* try opening the file */
	if((fd = fopen(filename,"rb"))>0){
		ok=true;

	}
	else{
		ok=false;
	}
    //* send response */
    if (ok) {
	int total_written=0;
	/* send headers */
	if(minet_write(sock,(char *)ok_response_f,strlen(ok_response_f))<0){
		fprintf(stderr,"Write failed\n");
		exit(-1);
	}
	
	/* send file */
	//get file size
	fseek(fd,0,SEEK_END);
	f_size=ftell(fd);
	fseek(fd,0,SEEK_SET);
	
	
	
	//read content from file to content buffer
	if((content=(char*)malloc(f_size))==NULL){
		fprintf(stderr,"Malloc failed\n");
		exit(-1);
	}
	if(fread(content,f_size,1,fd)<0){
		fprintf(stderr,"File read failed\n");
		exit(-1);
	}
	fclose(fd);	

	//write from content buffer to the current connection socket
	while(total_written<f_size){
		int change=total_written;
		total_written+=minet_write(sock,content+total_written,BUFSIZE);
		if(total_written<change){
			fprintf(stderr,"Write to connection socket failed\n");
			exit(-1);
		}
	}
	

    } else {
	// send error response
	if(minet_write(sock,(char*)notok_response,strlen(notok_response))<0){
		fprintf(stderr,"Write failed\n");
		exit(-1);
	}
    }

    /* close socket and free space */
	
	minet_close(sock);
  	//free(content);
    if (ok) {
	return 0;
    } else {
	return -1;
    }
}
