#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>


#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define NCONNECTIONS		20
int  main(int argc, char* argv[])
{
	struct timeval oldtime[NCONNECTIONS+10];

	int hSocket[NCONNECTIONS];                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;

    if(argc < 3)
      {
        printf("\nUsage: client host-name host-port\n");
        return 0;
      }
    else
      {
        strcpy(strHostName,argv[1]);
        nHostPort=atoi(argv[2]);
      }

    printf("\nMaking a socket");
		/* make a socket */

	
	int epollfd=epoll_create(1);
	for (int i = 0; i < NCONNECTIONS; i++){

		hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(hSocket[i] == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return 0;
		}

		/* get IP address from name */
		pHostInfo=gethostbyname(strHostName);
		/* copy address into long */
		memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

		/* fill address struct */
		Address.sin_addr.s_addr=nHostAddress;
		Address.sin_port=htons(nHostPort);
		Address.sin_family=AF_INET;


		/* connect to host */
		if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) 
		   == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			return 0;
		}

		struct epoll_event event;
		event.data.fd = hSocket[i];
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, hSocket[i], &event);

		char request[] = "GET /test1.txt HTTP/1.0\r\n\r\n";
		
		/* write what we received back to the server */
		write(hSocket[i],request,strlen(request));

		gettimeofday(&oldtime[event.data.fd], NULL);
	}


	for (int i = 0; i < NCONNECTIONS; i++){
		//create epoll interface
		//when a socket has data ready, read it
		struct epoll_event event;
		int nr_events = epoll_wait(epollfd, &event, 1, -1);
		char buffer[100000];
		int rval = read(event.data.fd, buffer, 100000);

   		printf("Got %d bytes from socket %d\nClosing socket\n", rval, event.data.fd);
		struct timeval newtime;
		gettimeofday(&newtime, NULL);

		double usec = (newtime.tv_sec - oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec);
	    std::cout << "Time "<<usec/1000000<<std::endl;

		/* close socket */                       
		if(close(hSocket[i]) == SOCKET_ERROR)
		{
			printf("\nCould not close socket\n");
			return 0;
		}
	}
}
