#include "global.h"
#define MAXLINE 100
#define USER_NUM_MAX 200
double DEGTORAD = 0.0174532925f*3;

//--------------------------------
struct UserInfo
{
	char user[32];
	char pass[32];
	int hp,mp,atk,def;
	int level,point,win,lose;
} UserInfo[256]; //do not use first item
int UserNumber;
struct Ship
{
	int player_id;
	double x,y,dx,dy;
	int mp,hp;
	unsigned int angle;
	int atk,def;
};
struct Bullet
{
	double x,y,dx,dy;
	short state;
};
struct Game
{
	int state; //0: free 1:wait 2:playing
	int fd[2];
	struct Bullet bullet[2][100];
	struct Ship ship[2];
} Game[256];
struct Connection
{
	int userId; //fd -> userId
	int game;
	int state; //0: free 1:wait 2:ingame
} Connection[1000];
int counter;
//------------------------------
int handleLogout(int fd);
int handleLogin(int fd,char *u,char *p);
int handleSignup(int fd,char *u,char *p);
int handleUpgrade(int fd,char a);

void send_mphp(int room);
void sendInfo(int fd);
void shot(int room, int ship);
void run(int room);
void send_shot(int room,int ship);
void gameStart(int game);
void left(int room, int ship);
void right(int room, int ship);
void go(int room, int ship);
//---------------------------------;
fd_set readfds, master;
//---------------------------------------------
float distance(float x1,float x2,float y1, float y2){return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));}

int saveSend(int fd, void *buf,size_t len,int flags)
{
	// char *p= buf;
	// printf("send %d: %d\n",fd,	p[0] );
	int re = send(fd,buf,len,flags|MSG_NOSIGNAL|MSG_DONTWAIT);
	if (errno == EPIPE) handleLogout(fd);

	return re;

}
void reply(int fd,int a)
{
	char p = a;
	saveSend(fd,&p,1,0);
}
void initAuth()
{
	bzero(UserInfo,sizeof(UserInfo));
	bzero(Connection,sizeof(Connection));
	FILE *f;
	f = fopen("info","r+");
	fread(UserInfo,sizeof(UserInfo[0]),256,f);
	fread(&UserNumber,sizeof(int),1,f);
	if (!UserNumber) UserNumber = 1; // UserNumber count from 1 ('cuz we exclude first item)
	fclose(f);
}
void saveAuth()
{
	FILE *f;
	f = fopen("info","w");
	fwrite(UserInfo,sizeof(UserInfo[0]),256,f);
	fwrite(&UserNumber,sizeof(int),1,f);
	fclose(f);
}
int existUser(char *u) //fail return 0
{
	for (int i = 0; i < UserNumber; ++i)
	{
		if (!strncmp(UserInfo[i].user,u,32)) return i;
	}
	return 0; //fail
}
int createUser(char *u,char *p)  //fail return 0
{
	if ((u[0]==0)||(p[0]==0)) return 1; //fail
	if (existUser(u)) return 1;      //fail
	strncpy(UserInfo[UserNumber].user,u,32);
	strncpy(UserInfo[UserNumber].pass,p,32);
	UserInfo[UserNumber].hp=100;
	UserInfo[UserNumber].mp=100;
	UserInfo[UserNumber].atk=10;
	UserInfo[UserNumber].point=10;
	UserNumber++;
	return 0;
}
void endGame(int game,int loserfd,int quit)//loserfd fd
{
	int winFd = (Game[game].fd[0]==loserfd)? Game[game].fd[1] : Game[game].fd[0];
	int loserID= Connection[loserfd].userId;
	int winnerID=Connection[winFd].userId;
	//clean Game obj, clean connection state,game
	bzero(&Game[game],sizeof(Game[0]));
	Connection[winFd].state=0;
	Connection[winFd].game=0;
	Connection[loserfd].state=0;
	Connection[loserfd].game=0;

	unsigned int packet=7<<28; //lose
	if (!quit) saveSend(loserfd,&packet,4,0);
	packet=8<<28;
	saveSend(winFd,&packet,4,0);

	printf("game %d end\n", game);
	printf("%s win %s lose\n",UserInfo[winnerID].user,UserInfo[loserID].user );
	UserInfo[winnerID].win++;
	UserInfo[winnerID].level++;
	if(UserInfo[winnerID].level%10==0) UserInfo[winnerID].point++;
	UserInfo[loserID].lose++;

	sendInfo(winFd);
	if (!quit) sendInfo(loserfd);
}
//----------------------handle---------------
int handleLogout(int fd)
{
	printf("Socket %d hung up\n", fd);
	if (Connection[fd].state==1) {
		bzero(&Game[Connection[fd].game],sizeof(Game[0]));
	}
	if (Connection[fd].state==2) {
		endGame(Connection[fd].game,fd,1);
	}
	bzero(&Connection[fd],sizeof(Connection[0]));
	close(fd);           // bye!
	FD_CLR(fd, &master); // remove from master set

	saveAuth();
}
int handleGame(int fd)
{
	if (Connection[fd].state) return 0;
	for (int i = 0; i < 256; ++i)
	{
		if (Game[i].state<2)
		{
			if (Game[i].state==1)
			{
				Game[i].state=2;
				Connection[fd].state=2;
				Connection[fd].game=i;
				Game[i].fd[1]=fd;
				Connection[Game[i].fd[0]].state=2;
				reply(Game[i].fd[0],GAME_ACK);
				reply(Game[i].fd[1],GAME_ACK);
				gameStart(i);
			} else
			{
				Game[i].state=1;
				Game[i].fd[0]=fd;
				Connection[fd].game=i;
				Connection[fd].state=1;
			}
			return 0;
		}
	}

}
int handleMessageGame(int fd)
{
	char result;
	int game = Connection[fd].game;
	int x = (Game[game].fd[0]==fd)? 0: 1;
	while(recv(fd,&result,1,MSG_DONTWAIT)>0){
		// printf("%d--%d\n", fd,result);
		switch (result)
		{
			case 1: go(game,x);break;
			case 2: left(game,x);break;
			case 3: right(game,x);break;
			case 4: shot(game,x);break;
		}
	}
}
int handleMessage(int fd)
{
	char p[MAXLINE];
	int re = recv(fd,p,sizeof(p),MSG_PEEK|MSG_DONTWAIT);
	if ((re<1) || errno == EPIPE) 
	{
		handleLogout(fd);
		return re;
	}
	// printf("%d-normal-%d\n", fd,p[0]);
	if (Connection[fd].state == 2) {handleMessageGame(fd);return 0;}
	switch(p[0])
	{
		case LOGIN:
		if (re >= 65) {
			recv(fd,p,65,0);
			handleLogin(fd,p+1,p+33);
		}
		break;
		case SIGNUP:
		if (re >= 65) {
			recv(fd,p,65,0);
			handleSignup(fd,p+1,p+33);
		}
		case UPGRADE:
		if (re >= 2) {
			recv(fd,p,2,0);
			handleUpgrade(fd,p[1]);
		}
		break;
		case GAME_REQ:
		recv(fd,p,1,0);
		handleGame(fd);
		break;
		default:
		recv(fd,p,re,MSG_DONTWAIT);
		break;
	}

}
int handleNewConnection(int fd)
{
	return fd;
	//TODO ???
}
void sendInfo(int fd){
	char p[200];
	if (!(Connection[fd].userId) || Connection[fd].state) return;
	memcpy(p+1,&UserInfo[Connection[fd].userId],sizeof(UserInfo[0]));
	p[0]=INFO;
	saveSend(fd,p,sizeof(UserInfo[0])+1,0);
}
int handleLogin(int fd,char *u,char *p)
{
	int re;
	if ((re=existUser(u))>=0 )
	{
		if (!strncmp(UserInfo[re].pass,p,32)) 
		{	
			Connection[fd].userId=re;
			reply(fd,LOGIN_S);
			sendInfo(fd);
			return re;
		}
	}
	// return -1;//login fail
	//TODO
	reply(fd,LOGIN_F);
	return re;
}
int handleSignup(int fd,char *u,char *p)
{
	int re;
	if (createUser(u,p)) reply(fd,SIGNUP_F);
	else 
	{
		reply(fd,SIGNUP_S);
		Connection[fd].userId=UserNumber-1;
		sendInfo(fd);
	}
	return 0;
}
int handleUpgrade(int fd,char a)
{
	int id;
	if (!(id=Connection[fd].userId) || (UserInfo[id].point==0) ) {reply(fd,UPGRADE_F) ;return 1;}
	switch(a)
	{
		case HPUP:
		UserInfo[id].hp+=8;break;
		case MPUP:
		UserInfo[id].mp+=8;break;
		case ATKUP:
		UserInfo[id].atk+=2;break;
		case DEFUP:
		UserInfo[id].def+=1;break;
		default:
		reply(fd,UPGRADE_F);return 1;
	}
	UserInfo[id].point--;
	reply(fd,UPGRADE_S);
	sendInfo(fd);
	return 0;
}
int createServer()
{
	int maxfd, listenfd, connfd, i, rv;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	//creation of the socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) >= 0)
	{
		puts("Server address is one of:");
		system("ifconfig | perl -nle'/dr:(\\S+)/ && print $1'");
		printf("Server is running at port %d\n", SERV_PORT);
	}
	else
	{
		perror("bind failed");
		return 0;
	}

	listen(listenfd, USER_NUM_MAX);

	printf("%s\n", "Server running...waiting for connections.");
	//Assign initial value for the fd_set
	maxfd = listenfd;
	FD_ZERO(&master);  //FD_ZERO works like memset 0;
	FD_ZERO(&readfds); //clear the master and temp sets
	FD_SET(maxfd, &master);

	fprintf(stdout, "Current maxfd: %d\n", maxfd);
	fflush(stdout);
	while (1)
	{
		usleep(10000); //10ms
		readfds = master;
		// // clear the set ahead of time
		//      FD_ZERO(&readfds);
		struct timeval tv = {0,0};
		rv = select(maxfd + 1, &readfds, NULL, NULL, &tv);
		if (rv == -1)
		{
			perror("error");
		}
		for (i = 0; i <= maxfd; i++)
		{
			if (FD_ISSET(i, &readfds))
			{
				if (i == listenfd)
				{
					connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
					if (connfd == -1)
					{
						perror("accept");
					}
					else
					{
						if (connfd > maxfd)
						{ // keep track of the max
							maxfd = connfd;
						}
						FD_SET(connfd, &master);
						printf("New connection on socket %d\n", connfd);
						handleNewConnection(connfd);
					}
				}
				else
				{
					int r = handleMessage(i);
				}
			}
		}
		for (int i = 0; i < 256; ++i)
		{
			if (Game[i].state==2){
				run(i);
			}
		}
		counter ++;
		counter = counter%400;
		if (counter == 0){
			for (i = 0; i <= maxfd; i++)
			{
				if (FD_ISSET(i, &readfds) && (i != listenfd))
				{
					sendInfo(i);
				}
			}
		}
	}
}
//----------------------------------------------------Game---------------------------------------------------------
void gameStart(int game)
{
	//todo
	int user0 = Connection[Game[game].fd[0]].userId;
	int user1 = Connection[Game[game].fd[1]].userId;
	Game[game].ship[0]=(struct Ship){0,20,20,0,0,UserInfo[user0].mp,UserInfo[user0].hp,0,UserInfo[user0].atk,UserInfo[user0].def}; 
	Game[game].ship[1]=(struct Ship){0,900,670,0,0,UserInfo[user1].mp,UserInfo[user1].hp,60,UserInfo[user1].atk,UserInfo[user1].def}; 
	send_mphp(game);
}
void left(int room, int ship){
	unsigned int a = Game[room].ship[ship].angle;
	if (a ==0) a+=120;a--;
	Game[room].ship[ship].angle= a;
}
void right(int room, int ship){
	unsigned int a = Game[room].ship[ship].angle;
	a++; if (a >120) a-=120;
	Game[room].ship[ship].angle= a;
}
void go(int room, int ship){
	double angle = Game[room].ship[ship].angle;
	Game[room].ship[ship].dx+=cos((angle-90)*DEGTORAD)*0.7;
	Game[room].ship[ship].dy+=sin((angle-90)*DEGTORAD)*0.7;
}
void send_(int room,int ship,int type){
	unsigned int packet=type<<28;
	int fd0 = Game[room].fd[0];
	int fd1 = Game[room].fd[1];
	unsigned int angle =  Game[room].ship[ship].angle;
	unsigned int y =  Game[room].ship[ship].y ;
	unsigned int x =  Game[room].ship[ship].x ;
	packet |= angle & 127;
	packet |= y<<7;
	packet |= x<<17;
	packet ^= (ship<<27);
	saveSend(fd0,&packet,4,0);

	packet ^= 0x8000000;
	saveSend(fd1,&packet,4,0);
}
void send_mphp(int room){
	unsigned int packet;
	for (int i = 0; i < 2; ++i)
	{
		packet =2<<28;
		packet |= ((Game[room].ship[i].mp<<14) + Game[room].ship[i].hp);
		printf("%d %d \n",Game[room].ship[i].mp,Game[room].ship[i].hp );
		saveSend(Game[room].fd[i],&packet,4,0);
		packet|=1<<28;
		saveSend(Game[room].fd[i^1],&packet,4,0);
	}
}
void send_shot(int room,int ship){
	send_(room,ship,4);
}
void send_pos(int room){
	send_(room,0,1);
	send_(room,1,1);
}
void shot(int room, int ship){
	int angle = Game[room].ship[ship].angle;
	if (Game[room].ship[ship].mp==0) return;
	Game[room].ship[ship].mp--;
	for (int i = 0; i < 100; ++i)
	{
		if (!Game[room].bullet[ship][i].state) {Game[room].bullet[ship][i] 
			= (struct Bullet){Game[room].ship[ship].x,Game[room].ship[ship].y,cos((angle-90)*DEGTORAD)*7,sin((angle-90)*DEGTORAD)*7,1};
			break;
		}
	}
	send_shot(room,ship);
	send_mphp(room);
}
void run(int room){
	int damage;
	for (int i = 0; i < 2; ++i)
	{
		Game[room].ship[i].x+=Game[room].ship[i].dx;if (Game[room].ship[i].x>SCREEN_WIDTH-20) Game[room].ship[i].x=SCREEN_WIDTH-20;		if (Game[room].ship[i].x<20) Game[room].ship[i].x=20;
		Game[room].ship[i].y+=Game[room].ship[i].dy;if (Game[room].ship[i].y>SCREEN_HEIGHT-20) Game[room].ship[i].y=SCREEN_HEIGHT-20;	if (Game[room].ship[i].y<20) Game[room].ship[i].y=20;
		Game[room].ship[i].dx*=0.95;
		Game[room].ship[i].dy*=0.95;
		for (int j = 0; j < 100; ++j)
		{
			if (Game[room].bullet[i][j].state)
			{
				Game[room].bullet[i][j].x+=Game[room].bullet[i][j].dx;
				Game[room].bullet[i][j].y+=Game[room].bullet[i][j].dy;
				if (Game[room].bullet[i][j].x<-16 || Game[room].bullet[i][j].y<-16 || Game[room].bullet[i][j].x>SCREEN_WIDTH+16 || Game[room].bullet[i][j].y > SCREEN_HEIGHT+16)
					Game[room].bullet[i][j].state=0;
				if (distance(Game[room].bullet[i][j].x,Game[room].ship[i^1].x,Game[room].bullet[i][j].y,Game[room].ship[i^1].y)<40) //collision
				{
					damage = Game[room].ship[i^1].atk-Game[room].ship[i^1].def;
					damage = (damage > 0)? damage : 1;
					Game[room].bullet[i][j].state=0; 
					Game[room].ship[i^1].hp -= damage; 
					printf("damage %d %d\n",damage,Game[room].ship[i^1].hp );
					send_mphp(room);
					if (Game[room].ship[i^1].hp < 1) endGame(room,Game[room].fd[i^1],0);
				}

			}
		}

	}
	send_pos(room);

}
//-----------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	initAuth();
	createServer();
}