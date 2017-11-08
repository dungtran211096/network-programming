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
#include <string.h>
#include <netinet/in.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

#define MAXLINE 256


int main(int argc, char **argv){
	int listenfd; //Socket descriptor for server
	int connfd;
	struct sockaddr_in serv_addr; //server address
	struct sockaddr_in cli_addr; //client address
	socklen_t  cliaddr_len = sizeof(cli_addr); //length of client address

	fd_set afds; //active file descriptor set
	fd_set rfds; //temp file descriptor list for select()
	
	int fdmax; //max amount of file descriptors
	char buffer[MAXLINE]; //Buffer for client data
	int i,j, maxi;
	int client[FD_SETSIZE];
	
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
	maxi = -1; //index into client[] array
	
	//initialise all client[] to -1 so not checked
	for(i = 0; i < FD_SETSIZE; i++){
		client[i] = -1; //-1 indicates available entry
	}
	
	/*add listen fd to master*/
	FD_SET(listenfd, &afds);

	while(1){
		rfds = afds; //copy it

		/*wait for activity on one of the sockets*/
		int nready = select(fdmax + 1, &rfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("select");
			exit(1);
		}

		/*New client connection*/
		if(FD_ISSET(listenfd, &rfds)){
			connfd = accept(listenfd, (struct sockaddr*) &cli_addr, &cliaddr_len);
			for (i = 0; i <= FD_SETSIZE; i++){
				if (client[i] < 0){
					client[i] = connfd; //save descriptor
					break;
				}
			}

			if(i == FD_SETSIZE) printf("Too many clients");

			FD_SET(connfd, &afds); //add new descriptor to set
			if(connfd > fdmax)
				fdmax = connfd; //for select
			
			printf("address %s:%d\t socket: %d online\n", inet_ntoa(cli_addr.sin_addr), (int) ntohs(cli_addr.sin_port), connfd);
			
			if(i > maxi)
				maxi = i; // max index in client array[]

			if (--nready <= 0) continue; //no more readable descriptors
		}
		/****/
		int n;
		/*Handle client*/
		for (i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &rfds)){
				//Check if it was for closing
				if( (n = read(i, buffer, MAXLINE)) == 0){
					close(i);
					FD_CLR(i, &afds);
					printf("socket: %d hung up\n", i);
				}
				
				/*got data from a client
				! broadcast to everyone
				*/			
				else{
					for(j = 0; j <= fdmax; j++){
						if(FD_ISSET(j, &afds)){
							if(j != listenfd && j != i){
								if(send(j, buffer, n, 0) < 0)
									perror("send");
							}
						}
					}
				} 
			} /*handle data from client*/
		} /*ENd For*/
	} /*End while*/
	close(listenfd);
	return 0;
}
