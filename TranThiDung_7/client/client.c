/* Tran Thi Dung 14020580
Server open port 8888
All of client connect with server
Client login by username
Client enter messages
Server broadcast messages to others
End of process if client enter @
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#define MAXDATASIZE 1024 //max number of bytes we can get at once
#define MAXNAMESIZE 10
char username[MAXNAMESIZE];

void send_recv(int i, int sockfd){
	char sendBuff[MAXDATASIZE]; //buffer is used to send
	char recvBuff[MAXDATASIZE]; //buffer is used to receive
	int nread;
	memset(sendBuff, 0, MAXDATASIZE);
	memset(recvBuff, 0, MAXDATASIZE);
	/*Send message to server*/
	if(i == 0){
		fgets(sendBuff, MAXDATASIZE, stdin);
		if(sendBuff[0] == '@'){
			sprintf(sendBuff, "%s shutdown\n", username);
			write(sockfd, sendBuff, sizeof(sendBuff));
			exit(1);
		}
		else{ 
			send(sockfd, sendBuff, strlen(sendBuff), 0);
		}
	}
	/*Receive message from server*/
	else{
		nread = recv(sockfd, recvBuff, MAXDATASIZE, 0);
		recvBuff[nread] = '\0';
		printf("%s", recvBuff);
		fflush(stdout);	
	}

}

int main(int argc, char **argv){
	int sockfd;
	char sendUsername[10];
	int fdmax, i;
	struct sockaddr_in serv_addr;
	memset(&username, 0, sizeof(username));

	fd_set rfds;
	fd_set afds;

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
	
	/*Log in*/
	printf("Username:");
	scanf("%s", username);
	sprintf(sendUsername, "%s online\n", username);
	write(sockfd, sendUsername, strlen(sendUsername));
	printf("[Type @ to quit]\n");
	/****/

	FD_ZERO(&afds);
	FD_ZERO(&rfds);
	FD_SET(0, &afds);
	FD_SET(sockfd, &afds);
	fdmax = sockfd;

	while(1){
		rfds = afds;
		if(select(fdmax + 1, &rfds, NULL, NULL, NULL) < 0){
			perror("SELECT");
			exit(1);
		}

		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &rfds)){
				send_recv(i, sockfd);
			}
		}
	}

	close(sockfd);
	return 0;
}




























