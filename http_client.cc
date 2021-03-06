
/* Includes */
#include "minet_socket.h" //For MINET sockets
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

/* Function Prototypes */
bool is_OK(char*);
	
/* Defines */	
#define BUFSIZE 1024

int main(int argc, char * argv[]) {

    char * server_name = NULL;
    fd_set fd;
    struct hostent *host=(struct hostent*)malloc(sizeof(struct hostent));
    struct sockaddr_in myaddr;
    int server_port    = -1;
    int socket;
    int http_status    = -1;
    
    char * server_path = NULL;
    char * req         = NULL;
    char * token;
    char buf[BUFSIZE];
    bool ok            = false;
	
    /*parse command line args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }
	
	/* Extract command line args */
    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];

	/* Build http request from address at command line */
    req = (char *)malloc(strlen("GET  HTTP/1.0\r\n\r\n") 
			 + strlen(server_path) + 1);  

    /* Choose Kernal stack or User stack based upon command line selection */
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
	memset(&myaddr,0,sizeof(myaddr));
	myaddr.sin_family=AF_INET;
	memcpy(&myaddr.sin_addr.s_addr,host->h_addr,host->h_length);
	myaddr.sin_port=htons(server_port);
	
    /* connect to the server socket */
	if(minet_connect(socket, (struct sockaddr_in*)&myaddr)<0){
		fprintf(stderr, "Connection failed\n");
		exit(-1);
	}
	
    /* Build http request with server path */
    sprintf(req, "GET %s HTTP/1.0\r\n\r\n", server_path);
	
	/* Send request to server path */
	if(minet_write(socket,req,strlen(req))<0){
		fprintf(stderr,"Write failed\n");
		exit(-1);
	}

	
    /* wait till socket can be read. */
    /* Hint: use select(), and ignore timeout for now. */
	FD_SET(socket,&fd);
	minet_select(socket+1,&fd,NULL,NULL,NULL);

    /* first read loop -- read headers */
	minet_read(socket,buf,BUFSIZE);	

    /* examine return code */  
    //Skip "HTTP/1.0"
    //remove the '\0'
	/* Check if http request was successful */
	ok = is_OK(buf);
    // Normal reply has return code 200

    /* print first part of response: header, error code, etc. */
	printf("%s",buf);

    /* second read loop -- print out the rest of the response: real web	 content */
	if(ok){
		
		FD_SET(socket,&fd);
		
		//Continue to read data using buffer until no data left to read
		while(1){
			int read=0;
			memset(buf,0,BUFSIZE);
			if(minet_select(socket+1,&fd,NULL,NULL,NULL)<0){
				minet_perror("Error");
				exit(-1);	
			}
			
			if((read=minet_read(socket,buf,BUFSIZE))<0){
				minet_perror("Error");
				exit(-1);	
			}
			if(read>0){
				printf("%s",buf);
					
			}
			else{
				break;
			}
			
		}
		
		
	}
    /*close socket and deinitialize */
	minet_close(socket);
	minet_deinit();
	
	//Return whether or not client ran successfully
    if (ok) {
	return 0;
    } else {
	return -1;
    }
	
}//END main()



/* is_OK function
	Parameters: 
		buf - pointer to client buffer
	Returns:
		bool - States whether the buffer contains useful data (successful httpe req)
*/
bool is_OK(char * buf)
{
	//Check if buffer contains a successful http request
	if(strncmp(buf,"HTTP/1.0 200",12)==0){
		return true;
	}
	else{
		return false;
	}
	
}//END is_OK()
