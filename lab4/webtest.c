#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <sys/epoll.h>
#include <sys/time.h>
#include <math.h>

using namespace std;

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ      1024

// Determine if the character is whitespace
bool isWhitespace(char c)
{ switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}
    
// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
    
}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") || 
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, "HTTP_");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);                
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

#define NSTD 3
int NSOCKETS;

int  main(int argc, char* argv[])
{

    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    char strFileName[HOST_NAME_SIZE];

	vector<char *> headerLines;
	char buffer[MAX_MSG_SZ];
	char contentType[MAX_MSG_SZ];

	extern char *optarg;
	int c, times_to_download = 1, err = 0;
	bool debug = false;
	bool repeat = false;
	while ((c = getopt( argc, argv, "c:d" )) != -1)
	{
		switch(c)
		{
            case 'c':
                break;
			case 'd':
				debug = true;
				break;
			case '?':
				err = 1;
				break;
		}
	}

	char *port = argv[ optind + 1];
	while (*port){
		if(!isdigit(*port++)){
		printf("\nERROR: The port must be a number\n");
		 return 0;
		}
	}

    strcpy(strHostName,argv[ optind ]);
    nHostPort=atoi(argv[ optind + 1 ]);
    strcpy(strFileName,argv[ optind + 2 ]);
    NSOCKETS = atoi(argv[ optind + 3 ]);


    int hSocket[NSOCKETS];                 /* handle to socket */
    struct timeval oldtime[NSOCKETS+NSTD];
 

    //Construct loop
    for (int i = 0; i < NSOCKETS; i++){

        /* make a socket */
        hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(hSocket[i] == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            return 0;
        }
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
	
	if(pHostInfo == NULL) {
		printf("\nERROR: Host name not found.\n\n");
		return 0;
	}

    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);


    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;
	
	
    if(!repeat){printf("\nConnecting to %s (%X) on port %d",strHostName,nHostAddress,nHostPort);}


    int epollFD = epoll_create(1);
    // Send the requests and set up the epoll data
    for (int i = 0; i < NSOCKETS; i++){
        /* connect to host */
        if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) 
           == SOCKET_ERROR)
        {
            printf("\nCould not connect to host\n");
            return 0;
        }

        // Write message
        #define MAXMSG 1024
        char *message = (char *)malloc(MAXMSG);
        sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",strFileName,strHostName, nHostPort);
        write(hSocket[i],message,strlen(message));
        free(message);

        // Keep track of the time when we sent the request
        gettimeofday(&oldtime[hSocket[i]],NULL);
        // Tell epoll that we want to know when this socket has data
        struct epoll_event event;
        event.data.fd = hSocket[i];
        event.events = EPOLLIN;
        int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSocket[i],&event);
        if(ret)
            perror ("epoll_ctl");
    }
	
	int times[NSOCKETS];

    //Response loop
    for (int i = 0; i < NSOCKETS; i++){

        struct epoll_event event;
        int rval = epoll_wait(epollFD,&event,1,-1);
        if(rval < 0)
            perror("epoll_wait");

        char *startline = GetLine(event.data.fd);
        printf("\nStatus line %s\n", startline);
        // Read the header lines
        GetHeaderLines(headerLines, event.data.fd , false);
		int data = 0;
        while((data  = read(event.data.fd,buffer,MAX_MSG_SZ)) > 0) {
            //write(1,buffer,rval);
			rval += data;
        }

        struct timeval newtime;
        // Get the current time and subtract the starting time for this request.
        gettimeofday(&newtime,NULL);
        double usec = (newtime.tv_sec - oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec);
        times[i] = usec;
		if(debug)
			std::cout << "Time "<<usec/1000000<<std::endl;
        printf("got %d from %d\n",rval,event.data.fd);
        // Take this one out of epoll
        epoll_ctl(epollFD,EPOLL_CTL_DEL,event.data.fd,&event);
    }

	
	// Now close the sockets
    printf("\nClosing socket\n");
    for(int i = 0; i < NSOCKETS; i++) {
        /* close socket */                       
        if(close(hSocket[i]) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }

	//Analyze times
	double average = 0.0;
	double  std_dev = 0.0;
	double high = 0.0;
	double low = 1000000000.0;
	for (int i = 0; i < NSOCKETS; i++){
		average += times[i];
		if (times[i] > high){
			high = times[i];
		}
		if(times[i] < low){
			low = times[i];
		}
	}
	average = average / NSOCKETS;

	//STD dev
	double dev_ave = 0.0;
	double devs[NSOCKETS];
	for (int i = 0; i < NSOCKETS; i++){
		devs[i] = times[i]-average;
		devs[i] = devs[i]*devs[i];
		dev_ave += devs[i];
	}
	dev_ave = dev_ave / (double)NSOCKETS;
	std_dev = sqrt(dev_ave);

	//convert to seconds
	average = average / 1000000;
	high = high/(double)1000000;
	low = low/(double)1000000;
	std_dev = std_dev/(double)1000000;
	
	std::cout<<"\nMetrics:\n---------------------------\n\nAverage time: " << average << std::endl;
	std::cout<<"Standard Deviation: "<<std_dev<<"\n";
	std::cout<<"Slowest time: "<<high<<"\n";
	std::cout<<"Fastest time: " << low << "\n";
}


