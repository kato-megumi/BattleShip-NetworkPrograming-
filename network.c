#include "client.h"

#define MAXLINE 4096   /*max text line length*/


void lostConnect(){
	showMessage(NULL,GTK_MESSAGE_ERROR,"Error","Mất kết nối đến server");
	exit(1);
}
void sendPacket(int len){
	int re  = send(sockfd,packet,len,0);
	if (errno == EPIPE)	lostConnect();
}
void login(char *u, char *p)
{
	bzero(packet,100);
	packet[0]=LOGIN;
	strcpy(packet+1,u);
	strcpy(packet+33,p);
	sendPacket(65);
}
void signup(char *u, char *p)
{
	bzero(packet,100);
	packet[0]=SIGNUP;
	strcpy(packet+1,u);
	strcpy(packet+33,p);
	sendPacket(65);
}
int init_network(char * ip)
{
	struct sockaddr_in servaddr;  
	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}
	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(ip);
	servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order
	
	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server");
		exit(3);
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	return sockfd;
}
int recv_loop(void *a)
{
	char p;
	int re = recv(sockfd,&p,1,MSG_DONTWAIT); // or should we wait
	if (re>0)
	{
		// printf("%d.",p);
		if ((p < UPGRADE_F) && (logined) ){return 1;}

		switch(p){
			case LOGIN_F:
			showMessage(NULL,GTK_MESSAGE_WARNING, sLOGIN_FAILED, sLOGIN_FAILED_);
			break;
			case LOGIN_S:
			showMessage(NULL,GTK_MESSAGE_WARNING, sLOGIN_SUCCESSED, "");
			switchMain();
			break;
			case SIGNUP_F:
			showMessage(NULL,GTK_MESSAGE_WARNING,sSIGNUP_FAILED,"");
			break;
			case SIGNUP_S:
			showMessage(NULL,GTK_MESSAGE_WARNING,sSIGNUP_SUCCESSED,"");
			switchMain();
			break;
			case UPGRADE_F:
			break;
			case UPGRADE_S:
			break;
			case INFO:
			// printf("info %d\n" ,sizeof User);
			recv(sockfd,&User,sizeof User,MSG_WAITALL);
			// int re = recv(sockfd,&User,sizeof User,0);
			// printf("re %d\n", re);
			updateView();
			break;
			case GAME_ACK:
			switchGame();
			break;


		}
	}
	return 1;
}
void sendUpdate(int a)
{
	packet[0] = UPGRADE;
	packet[1] =a ;
	sendPacket(2);
}