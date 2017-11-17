/* Tran Thi Dung 14020580
Server open port 8888
Client send download file_name to server
Server send back file to client
replay of sending
End of process if client enter @
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int ndownload = 0;
int ndone = 0;

int main(int argc, char **argv){
	int listenfd; //Socket descriptor for server
	int connfd;
	struct sockaddr_in serv_addr; //server address
	struct sockaddr_in cli_addr; //client address
	socklen_t  cliaddr_len = sizeof(cli_addr); //length of client address

	fd_set afds; //active file descriptor set
	fd_set rfds; //temp file descriptor list for select()

	int fdmax;
	int i;
	int client[FD_SETSIZE];
	int nsent = 0;
		int nsending = 0;

	/*clear the active sets*/
	FD_ZERO(&afds);

	/*Create socket for incoming connections*/
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	/*Construct address structure*/
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8888);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 	

	/*Bind to address*/
	bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	
	/*Mark the socket so it will listen for incoming connections*/
	listen(listenfd, 5);
	
	fdmax = listenfd; //initialize
	
	//initialise all client[] to -1 so not checked
	for(i = 0; i < FD_SETSIZE; i++){
		client[i] = -1; //-1 indicates available entry
	}
	
	/*add listen fd to master*/
	FD_SET(listenfd, &afds);

	while(1){
		rfds = afds;

		int nready = select(fdmax + 1, &rfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("select");
			exit(1);
		}

		if(FD_ISSET(listenfd, &rfds)){
			connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &cliaddr_len);
			for (i = 0; i <= FD_SETSIZE; ++i){
				if(client[i] < 0){
					client[i] = connfd;
					break;
				}
			}

			if(i == FD_SETSIZE) printf("Too many clients\n");

			FD_SET(connfd, &afds);

			if(connfd > fdmax)
				fdmax = connfd;

			printf("address %s:%d\t socket: %d online\n", inet_ntoa(cli_addr.sin_addr), (int) ntohs(cli_addr.sin_port), connfd);

			if(--nready <= 0) continue;
		}

		char sendBuff[1024];
		char fileSize[1024]; //message contain size of file
		char fileName[256]; //message contain name of file
		int fsize;
		int t; //receive filename from client
		
		memset(sendBuff, '0', sizeof(sendBuff));
		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &rfds)){
				t = read(i, fileName, sizeof(fileName));
				if(t <= 0){
					close(i);
					FD_CLR(i, &afds);
					printf("socket: %d hung up\n", i);
				}
				else{
					
					/*receive filename from client*/
					fileName[t] = '\0'; 
					if(fileName[0] == '@'){
						exit(-1);
					}
					
					else{	
						printf("------------------\n");
						printf("Socket %d requires file: %s\n", i, fileName);
				
						/*Open file which is sent to client*/
						FILE *rf;
						rf = fopen(fileName, "rb");
						if(rf == NULL){
							printf("File doesnt exist\n");
							printf("------------------\n");
							sprintf(fileSize, "0");
							send(i, fileSize, sizeof(long), 0);
						}	
						/*****/
						else{

							memset(fileSize, '0', sizeof(fileSize));
							/*Check length of file*/
							fseek(rf, 0, SEEK_END);
							fsize = ftell(rf);
							rewind(rf);
							/***/
					
							/**Send length of file to client***/
							sprintf(fileSize, "%d", fsize);
							printf("%d\n", fsize);
							int len = send(i, fileSize, sizeof(long), 0);
							if(len < 0){
								perror("File size");
								exit(1);
							}
							/****/
							memset(fileSize, '0', sizeof(fileSize));
							
							int nRead;
							/*Send file*/
							nsending++;
							while((	nRead = fread(sendBuff, 1, 1024, rf)) > 0){
								printf("sending file %s\n", fileName);
								write(i, sendBuff, nRead);
							}	
							memset(sendBuff, '0', sizeof(sendBuff));
							fclose(rf);

							printf("Number of sending: %d\n",nsending);
							nsending--;
							nsent++;
							printf("Number of sent: %d\n", nsent);
							
							printf("Done---------------\n");
						}
					}
				}				
			}
		}
	}				
	close(listenfd);
	return 0;
}

				

