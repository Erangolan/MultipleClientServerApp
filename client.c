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

void ls(char* buffer, char* path);


int main(int argc, char* argv[]){

	int clientSocket, ret, fd, fileSize, bytesRead;
	struct sockaddr_in serverAddr;
	char buffer[BUF_SIZE] = {0}, path1[50] = {0}, path2[] = {"./Files/"};
	struct stat fileStat;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		perror("socket");
		return 1;
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(IP_ADDR);

	if(ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect");
		return 1;
	}
	printf("[+]Connected to Server.\n");

	if(strcmp(argv[1], "list-files") != 0 && strcmp(argv[1], "upload-file") != 0 && strcmp(argv[1], "download-file") != 0){
		printf("bad arguments\n");
		return 1;
	}

	if(send(clientSocket, argv[1], strlen(argv[1]), 0) < 0){
		perror("send");
		return 1;
	}

	if(strcmp(argv[1], "list-files") == 0){
		if(recv(clientSocket, path1, 1024, 0) < 0){
			perror("recv");
			return 1;
		}
		ls(buffer, path1);
	}

	if(strcmp(argv[1], "upload-file") == 0){

		if(send(clientSocket, argv[2], strlen(argv[2]), 0) < 0){
			perror("send");
			return 1;
		}

		strcat(path2, argv[2]);

		if ((fd = open(path2, O_RDONLY)) < 0){
				perror("open");
				return 1;
		}

		if(fstat(fd, &fileStat) < 0){
			perror("fstat");
			return 1;
		}

		if(send(clientSocket, &fileStat, sizeof(fileStat), 0) < 0){
			perror("send");
			return 1;
		}

		fileSize = fileStat.st_size;
		while (fileSize) {
				if((bytesRead = read(fd, buffer, BUF_SIZE)) < 0){
					perror("read");
					return 1;
				}
				if(send(clientSocket, buffer, bytesRead, 0) != bytesRead){
					perror("send");
					return 1;
				}
				fileSize -= bytesRead;
		}
		printf("File %s had been successfully uploaded to the server!\n", argv[2]);
	}

	if (strcmp(argv[1], "download-file") == 0) {
		if(send(clientSocket, argv[2], strlen(argv[2]), 0) < 0){
			perror("send");
			return 1;
		}

		if(recv(clientSocket, &fileStat, sizeof(fileStat), 0) < 0){
			perror("recv");
			return 1;
		}

		if ((fd = open(argv[2], O_WRONLY | O_CREAT, 0600)) < 0) {
			perror("open");
			return 1;
		}

		fileSize = fileStat.st_size;
		while (fileSize) {
			if((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) < 0){
				perror("recv");
				return 1;
			}

			if(write(fd, buffer, bytesRead) != bytesRead){
				perror("send");
				return 1;
			}
			fileSize -= bytesRead;
		}
		close(fd);
		printf("File successfully downloaded!\n");
	}
	close(clientSocket);
	return 0;
}


void ls(char* buffer, char* path){
	struct stat fileStat;
	char tmp[50];
	bzero(tmp, sizeof(tmp));
	strcat(tmp, path);
	int fd;
	struct dirent* d;
	DIR* dir;

	if((dir = opendir(path)) != 0){
		while (d = readdir(dir)){
			if(d->d_name[0] != '.'){
				strcat(tmp, d->d_name);
				printf("%s\t", d->d_name);

				if((fd = open(tmp, O_RDONLY)) < 0){
				perror("open");
				return;
			}

			if(fstat(fd, &fileStat) < 0){
				perror("fstat");
				return;
			}
			printf("%ld\n", fileStat.st_size);
			}
			bzero(tmp, sizeof(tmp));
			strcat(tmp, path);
		}
	}
}





//