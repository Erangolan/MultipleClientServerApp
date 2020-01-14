#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define BUF_SIZE 1024


int main(int argc, char* argv[]){

	int sockfd, ret, newSocket, fd, fileSize, bytesRead;;
	struct sockaddr_in serverAddr, newAddr;
	struct stat fileStat;
	socklen_t addr_size;
	char buffer[BUF_SIZE];
	pid_t childpid;
	char path[50] = "./server/Files/";

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(IP_ADDR);

	if(ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", PORT);

	if(listen(sockfd, 10) == 0)
		printf("[+]Listening\n");
	else
		printf("[-]Error in binding.\n");

	while(1)
	{
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0)
			exit(1);
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);
			bzero(buffer, sizeof(buffer));

			if(recv(newSocket, buffer, BUF_SIZE, 0) < 0){
				perror("recv");
				return 1;
			}

			if(strcmp(buffer, "list-files") == 0){
				if(send(newSocket, path, strlen(path), 0) < 0){
					perror("send");
					return 1;
				}
				return 1;
			}

			if(strcmp(buffer, "upload-file") == 0){
				bzero(buffer, sizeof(buffer));
				if(recv(newSocket, buffer, BUF_SIZE, 0) < 0){
					perror("recv");
					return 1;
				}

				if ((fd = open(buffer, O_WRONLY | O_CREAT, 0600)) < 0) {
					perror("open");
					return 1;
				}
				if(fstat(fd, &fileStat) < 0){
					perror("fstat");
					return 1;
				}

				if(recv(newSocket, &fileStat, sizeof(fileStat), 0) < 0){
					perror("recv");
					return 1;
				}

				fileSize = fileStat.st_size;
				while (fileSize) {
					if((bytesRead = recv(newSocket, buffer, sizeof(buffer), 0)) < 0){
						perror("recv");
						return 1;
					}
					if(write(fd, buffer, bytesRead) != bytesRead){
						perror("send");
						return 1;
					}
					fileSize -= bytesRead;
				}
				printf("File successfully saved in server!\n");
			}

			if(strcmp(buffer, "download-file") == 0){

				bzero(buffer, sizeof(buffer));
				if(recv(newSocket, buffer, BUF_SIZE, 0) < 0){
					perror("recv");
					return 1;
				}
				char path[] = {"./Files/"};
				strcat(path, buffer);

				if ((fd = open(path, O_RDONLY)) < 0){
					perror("open");
					return 1;
				}

				if(fstat(fd, &fileStat) < 0){
					perror("fstat");
					return 1;
				}

				if(send(newSocket, &fileStat, sizeof(fileStat), 0) < 0){
					perror("send");
					return 1;
				}

				fileSize = fileStat.st_size;
				while (fileSize) {
					if((bytesRead = read(fd, buffer, BUF_SIZE)) < 0){
						perror("read");
						return 1;
					}

					if(send(newSocket, buffer, bytesRead, 0) != bytesRead){
						perror("send");
						return 1;
					}
					fileSize -= bytesRead;
				}
				printf("File successfully sent to the client\n");
			}
		}
		printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
	}
	close(newSocket);
	return 0;
}
