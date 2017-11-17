/* Tran Thi Dung 14020580
Server open port 8888
Client send downloading file_name to server
Server send back file to client
replay of sending
End of process if client enter @
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv){
	int sockfd;
	struct sockaddr_in serv_addr;

	/*Check file and ip*/
	if(argc != 2){
		printf("USAGE : <ip>");
		return 1;
	}
	
	/*create endpoint for communication*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8888);
	
	/*Convert IPv4 address from text to binary form*/
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) < 0){
        perror("inet_pton");
        return 1;
	}
		
	/*initiate a connection on a socket*/
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
		printf("Error: Connection failed\n");
		return 1;
	}
	
	while(1){
		char file_name[256];
		char recvBuff[1024];/*receive-buffer*/
		memset(file_name, '0', sizeof(file_name));
		memset(recvBuff, '0', sizeof(recvBuff));
		
		/*enter file name*/
		printf("Nhap ten file: ");
		scanf("%s", file_name);
		
		if(file_name[0]=='@') return 1; 	
		/*Send to server*/
		if(write(sockfd, file_name, strlen(file_name)) < 0){
			perror("Send message failure");
			return 1;
		}
				
		/***Recieve size of file***/
		read(sockfd, recvBuff, sizeof(long));
		int fsize = atoi(recvBuff);
		printf("Length of file: %d\n", fsize);
		if(fsize == 0){
			printf("Can't be download\n");
			memset(recvBuff, '0', sizeof(recvBuff));
		}
		/******/
		else{
			/***Create file to store data***/
			FILE *rf;
			rf = fopen(file_name, "ab"); 
			if(rf == NULL){
				perror("Open file");
				return 1;
			}	
			/******/

			int nRead;
			/**** Read & write data****/
			while(1){
				nRead = read(sockfd, recvBuff, 1024);
				if(nRead > 0){
					printf("nRead: %d\n", nRead);
					fwrite(recvBuff, 1, nRead, rf);
					fsize -= nRead;
					if(fsize == 0) break;
				}
			}
			printf("Done\n");
			fclose(rf);
		}
	}
	close(sockfd);
	return 0;
}




























