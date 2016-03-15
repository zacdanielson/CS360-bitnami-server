#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

main()
{
	int pid = fork();
	std::cout << "Fork Returned " << pid << std::endl;
	if(pid == 0) {//we are the child
		std::cout << "Child about to exec " << std::endl;
		execl("/bin/ls", "/bin/ls, (char *)0");
		std::cout << "Child done with exec " << std::endl;
	}else{ // we are the parent
		int status;
		wait(&status);
		std::cout << "Parent time "<< std::endl;
	}
}
