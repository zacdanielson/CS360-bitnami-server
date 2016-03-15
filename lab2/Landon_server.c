#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
//#include "CS360utils.h"

using namespace std;

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5



int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 2)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
	// argv[2] will be the directory
      }

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);
 
    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address))
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
    
    /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }
    
    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket, (struct sockaddr*)&Address, (socklen_t *)&nAddressSize);
        printf("\nGot a connection from %X (%d) \n", 
		Address.sin_addr.s_addr, 
		ntohs(Address.sin_port));
	
	// Get header lines
	// parse the get line from the headers
	// find the requested resource (/foo.html)
	// store path to content + requested resource
	// (file path) in variable char* rs

	memset(pBuffer,0,sizeof(pBuffer));
        int rval = read(hSocket,pBuffer,BUFFER_SIZE);
    // The above read should be get header lines like before.
	printf("Got from browser \n%s\n",pBuffer);	
#define MAXPATH 1000
	char path[MAXPATH];
	rval = sscanf(pBuffer,"GET %s HTTP/1.1",path);
	printf("Got rval %d, path %s\n",rval,path);
	char fullpath[MAXPATH];
	sprintf(fullpath,"%s%s",argv[2], path);
	
	// use the stat stuff
        // --------------------------------------------------------------------

        struct stat filestat;
	
	printf("fullpath %s\n",fullpath);
        // we will need to argv[1] to requested resource (rs)
        if(stat(fullpath, &filestat)) {
                cout <<"ERROR in stat\n";
                // return a canned 404 response
                // with 404 headers and body
        }
        if(S_ISREG(filestat.st_mode)) {
                cout << fullpath << " is a regular file \n";
                cout << "file size = "<<filestat.st_size <<"\n";

		int content_Length = filestat.st_size;
		char *content_Type = "text/plain";
		printf("Content Length: %d\n",content_Length);
		char type[10];
		memset(type,'\0',sizeof(type));
		printf("strlen of path: %d\n", strlen(path));
		int typeIndex = 0;
		for (int i = strlen(path) - 1; i >= 0; i--)
		{
			if (path[i] == '.')
			{
				strncpy(type, path + i, strlen(path) - i);
				type[strlen(path)] = '\0';
				printf("\nType: %s\n", type);
			}
		}
		
		if (strcmp(type,".htm") == 0 || strcmp(type,".html") == 0)
		{
			printf("type is html!\n");
			content_Type = "text/html";
		}
		else if (strcmp(type, ".jpg") == 0)
		{
			printf("Type is jpg\n");
			content_Type = "image/jpg";	
		}
		else if (strcmp(type, ".gif") == 0)
		{
			printf("Type is gif!");
			content_Type = "image/gif";	
		}
		else // .txt or other
		{
			printf("This is a .txt or other");
			content_Type = "text/plain";
		}

		printf("HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", content_Type, content_Length);
		
		memset(pBuffer,0,sizeof(pBuffer)); // empties the pBuffer
		sprintf(pBuffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n",content_Type, content_Length);
		
		write(hSocket, pBuffer,strlen(pBuffer));

		FILE *fp = fopen(fullpath, "r");
                char *buffer = (char *)malloc(filestat.st_size + 1);
                fread(buffer,filestat.st_size, 1, fp);
                write(hSocket, buffer, content_Length); // Where we write to the socket instead (Server)
                free(buffer);
                fclose(fp);
                // read file
                // send it to client
        }
        if(S_ISDIR(filestat.st_mode)) 
	{
                // look for index.hemt (run stat function again)
                // if ( stat( rs + "/index.html", &filestat ) {
                        // index doesn't exit!
                        // read dir listing
                        // generate html
                        // send appropriate headers
                        // and body to client
                // {
                // else {
                        // format headers
                        // read index.html
                        // send all to client
                // {
                cout << fullpath << " is a directory \n";
                DIR *dirp;
                struct dirent *dp;

                dirp = opendir(".");
                while ((dp = readdir(dirp)) != NULL)
                        printf("name %s\n", dp->d_name);

                        //  printf("<html><body>"); (Before while loop)
                        // prepend requested resource
                        // printf("<a href=\"%s\">%s</a>\n", dp->d_name, dp->d_name)
                        //  printf("</body></html>"); (After while loop)
                        // This is where you will make the references to the directory
                (void)closedir(dirp);
        }
        // --------------------------------------------------------------------
	
	/*
	memset(pBuffer,0,sizeof(pBuffer)); // empties the pBuffer
	sprintf(pBuffer,"HttP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %s\
\r\n\r\n\
<html>\
Hello</html>\n",filestat.st_size);
	write(hSocket, pBuffer,strlen(pBuffer));i
*/
//	sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\
Content-Type: image/jpg\r\n\
Content-Length: 51793\
\r\n\r\n");	

/*sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\
 Content-Type: text/html\
\r\n\r\n\
<html>\
<ul>\
<li> <a>file1.html</a></li>\
<li> file2.html</li>\
</ul>\
Hello</html>\n");
*/

/*
	write(hSocket, pBuffer, strlen(pBuffer));
	FILE *fp = fopen("test4.jpg","r");
	char *buffer = (char *)malloc(51793+1);
	fread(buffer, 51793, 1,fp);
	write(hSocket,buffer,51793);
	free(buffer);
	fclose(fp);
*/
#ifdef notdef
linger lin;
unsigned int y=sizeof(lin);
lin.l_onoff=1;
lin.l_linger=10;
setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));	
shutdown(hSocket, SHUT_RDWR);
#endif

/*
linger lin;
unsigned int y=sizeof(lin);
lin.l_onoff=1;
lin.l_linger=10;
setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
shutdown(hSocket, SHUT_RDWR);
*/

    printf("\nClosing the socket");
	/* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
