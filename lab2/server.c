#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <fstream>
#include <stdlib.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1000
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5

using namespace std;

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 3)
      {
        printf("\nUsage: server host-port dir\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
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
	//copy clement code here
	int optval = 1;
	setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as hSocket (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

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

    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
        /* read from socket into buffer */


		char readBuffer[BUFFER_SIZE];
	
        memset(pBuffer,0,sizeof(pBuffer));

		int rval = read(hSocket, pBuffer, BUFFER_SIZE);
		printf("\ngot from browser %d\n%s\n", rval, pBuffer);

		if(rval >=1){

		printf("\nrval: %d\n", rval);
		printf("\nbuffer: \n%s\n", pBuffer);
		printf("\nsocket: %d\n",hSocket);

#define MAXPATH 1000		
		char path[MAXPATH];
		rval = sscanf(pBuffer, "GET %s HTTP/1.1", path);
		printf("Got rval %d, path %s\n", rval, path);
		char fullpath[MAXPATH];
		sprintf(fullpath, "%s%s", argv[2], path);
		printf("fullpath: %s\n", fullpath);
		
	//check to see if request is file or directory

		struct stat filestat;
		int len;
		DIR *dirp;
		struct dirent *dp;

		if (stat(fullpath, &filestat)){
			cout << "ERROR in stat\n";

			//give 404
			memset(pBuffer, 0, sizeof(pBuffer));
			sprintf(pBuffer, "HTTP/1.1 404 Not Found\r\n\
Content_type: text/html\r\n\r\n\
<html>\
<head>\
<title>404 Not Found</title>\
</head><body>\
<h1>Not Found</h1>\
<p>The requested URL was not found on this server.</p>\
</body></html>\n");
			write(hSocket, pBuffer, strlen(pBuffer));	
		}
		else if(S_ISREG(filestat.st_mode)){
			cout << fullpath << " is a regular file \n";
			cout << "file size = " << filestat.st_size << "\n";
			cout << "this\n";

//create header
	//instantiate objects
			char *contentType = "text/html";
			int contentLength = filestat.st_size;
	//get file type
			char type[10];
			memset(type, '\0', sizeof(type));
			int pathlen = (int)strlen(path);
			for (int i = pathlen-1; i >= 0; i--){
				if (path[i] == '.'){

					strncpy(type, path+i, pathlen-i);
					printf("\nType: %s\n",type);
					type[10]='\0';
					break;
				}
			}
			if(strcmp(type, ".htm") == 0|| strcmp(type,".html")==0){
				printf("TYPE: HTML\n");
				contentType = "text/html";
			}else if(strcmp(type,".jpg")==0){
				printf("TYPE : JPG\n");
				contentType = "image/jpg";
			}else if( strcmp( type, ".gif")==0 ){
				printf("TYPE: GIF\n");
				contentType = "image/gif";
			}else{
				printf("TYPE: ELSE\n");
				contentType = "text/plain";
			}

			printf("HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", contentType, contentLength);

			memset(pBuffer, 0, sizeof(pBuffer));
			sprintf(pBuffer, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", contentType, contentLength);
			write(hSocket, pBuffer, strlen(pBuffer));
	


			FILE *fp = fopen(fullpath, "r");
			char *buffer = (char *)malloc(contentLength+1);

			fread(buffer, contentLength, 1, fp);
//			printf("File:\n%s\n", buffer);
			int bytes = write(hSocket, buffer, contentLength);
			printf("Bytes: %d\n",bytes);
			free(buffer);
			fclose(fp);
		}
		else if(S_ISDIR(filestat.st_mode)){
			cout << fullpath << " is a directory \n";

			char index[MAXPATH];
			sprintf(index, "%s/index.html",argv[2]);

			if(stat(index, &filestat)){
	
				dirp = opendir(fullpath);
				
				memset(pBuffer, 0, sizeof(pBuffer));
				sprintf(pBuffer,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>Name<hr />");
				write(hSocket, pBuffer, strlen(pBuffer));

				while ((dp = readdir(dirp))!=NULL){
					printf("name %s\n", dp->d_name);
					memset(pBuffer, 0, sizeof(pBuffer));
					
					char relpath[MAXPATH];
					int pos = strlen(path) -1;
					if(path[pos] == '/'){
						sprintf(relpath, "%s", dp->d_name);
					}else{
					sprintf(relpath, "%s/%s",path, dp->d_name);
					}
					printf("Relative Path: %s\n", relpath);

					sprintf(pBuffer, "<a href=\"%s\">%s</a><br />\n",relpath, dp->d_name);
					write(hSocket, pBuffer, strlen(pBuffer));
				}
				
				memset(pBuffer, 0, sizeof(pBuffer));
				sprintf(pBuffer, "</body></html>\n");
				write(hSocket, pBuffer, strlen(pBuffer));
				(void)closedir(dirp);			
			}
			else{
				memset(pBuffer, 0, sizeof(pBuffer));
				sprintf(pBuffer, "HTTP/1.1 200 OK\r\nContent-Type\r\n\r\n");
				write(hSocket, pBuffer, strlen(pBuffer));
			
				FILE *fp = fopen(index, "r");
				char *buffer = (char *)malloc(filestat.st_size+1);

				fread(buffer, filestat.st_size, 1, fp);
				write(hSocket, buffer, strlen(buffer));
				free(buffer);
				fclose(fp);
	
			}

		}


		}

/*
		FILE *fp = fopen(fullpath,"r");
		char *buffer = (char *)malloc(filestat.st_size +1);
		fread(buffer, contentLength, 1, fp);
		write(hSocket, buffer, contentLength);
		free(buffer);
		fclose(fp);
*/
        /* close socket */
//#ifdef notdef
linger lin;
unsigned int y=sizeof(lin);
lin.l_onoff=1;
lin.l_linger=10;
setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
shutdown(hSocket, SHUT_RDWR);
//#endif		
	
		printf("\nClosing the socket\n\n");
		if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
		
    }
}
