#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <SDL2/SDL.h>  
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>
// const int SCREEN_HEIGHT = 690;
// const int SCREEN_WIDTH = 920;
SDL_Rect rect_screen = {0,0,920,690};
//The window we'll be rendering to
SDL_Window* gWindow = NULL;
SDL_Texture* tplayer;
SDL_Texture* teplayer;
SDL_Texture* bg;
SDL_Texture* tbullet;
SDL_Renderer* gRenderer = NULL; 
float DEGTORAD = 0.0174532925f;
unsigned int Packet;
char x[4] = {0x1,0x2,0x3,0x4};
int winlose;
SDL_Texture* loadTexture( const char* path )
{
	SDL_Texture* newTexture = NULL; 
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path ); 
	//Create texture from surface pixels 
	newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface ); 
	//Get rid of old loaded surface 
	SDL_FreeSurface( loadedSurface );  
	return newTexture; 
}
struct Ship
{
	float x,y,dx,dy;
	int mp,hp,angle;
	int MP,HP;
} player[2];
struct Bullet
{
	float x,y,dx,dy,angle;
	short state;
} bullets[2][100];
SDL_Rect rect[2] ={{0,0,40,40},{880,650,40,40}} ;
SDL_Rect rec = { 40,0,40,40};
SDL_Rect recb_ = {0,0,32,64};
SDL_Rect selfhp,selfmp,enemyhp,enemymp;
void create_bullet(unsigned int Packet)
{
	int angle = Packet%0x80;
	int y = (Packet>>7)%0x400;
	int x = (Packet>>17)%0x400;
	int j = (Packet>>27)%0x2;
	angle=3*angle+180;
	for (int i = 0; i < 100; ++i)
	{
		if (bullets[j][i].state == 0)
		{
			bullets[j][i] = (struct Bullet){x,y,cos((angle-90)*DEGTORAD)*7,sin((angle-90)*DEGTORAD)*7,angle,1};
			break;
		}
	}
}
void set_pos(unsigned int Packet)
{
	int angle = Packet&0x7f;
	int y = (Packet>>7)&0x3ff;
	int x = (Packet>>17)&0x3ff;
	int j = (Packet>>27)&0x1;	
	player[j].x=x;
	player[j].y=y;
	player[j].angle=angle*3+180;
	rect[j].x=x-20;
	rect[j].y=y-20;
	// printf("%d %d %d %x\n",angle,x,y,Packet );

}
void set_hpmp(unsigned int Packet,int ship){
	player[ship].mp = (Packet >> 14)&0x3fff; player[ship].hp = Packet&0x3fff;
	// if (!ship) printf("%d %d\n",player[ship].hp,player[ship].mp );
}
void handle(){
	while (recv(sockfd,&Packet,4,MSG_DONTWAIT) >0 ){
	// printf("%d-%x\n",Packet>>28,Packet );
		switch(Packet>>28){
			case 1:
			set_pos(Packet);break;
			case 2:
			player[0].mp = (Packet >> 14)&0x3fff; player[0].hp = Packet&0x3fff;
			printf("0: %d %d\n", player[0].mp,player[0].hp);break;
			case 3:
			player[1].mp = (Packet >> 14)&0x3fff; player[1].hp = Packet&0x3fff;
			printf("1: %d %d\n", player[1].mp,player[1].hp);break;
			case 4:
			create_bullet(Packet);break;
			case 5:
			set_hpmp(Packet,0);break;
			case 6:
			set_hpmp(Packet,1);break;
			case 7:
			printf("LOSELOSELOSE\n");
			winlose = 0;
			return;
			case 8:
			printf("WINWINWIN\n" );
			winlose = 1;
			return; //winwinwin

		}
	}
}
float distance(float x1,float x2,float y1, float y2){return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));}
void render()
{
	SDL_Rect recb;
	SDL_RenderClear( gRenderer ); 
	SDL_RenderCopy( gRenderer, bg, NULL,&rect_screen);
	SDL_RenderCopyEx( gRenderer, tplayer, &rec, &rect[0],(double)player[0].angle,NULL,SDL_FLIP_NONE); //render player ship
	SDL_RenderCopyEx( gRenderer, teplayer, &rec, &rect[1],(double)player[1].angle,NULL,SDL_FLIP_NONE); //render enemy ship
	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < 100; ++i)
		{
			if (bullets[j][i].state)
			{
				recb= (SDL_Rect){bullets[j][i].x-16,bullets[j][i].y-32,32,64};
				bullets[j][i].x+=bullets[j][i].dx;
				bullets[j][i].y+=bullets[j][i].dy;
				if (bullets[j][i].x<-16 || bullets[j][i].y<-16 || bullets[j][i].x>SCREEN_WIDTH+16 || bullets[j][i].y > SCREEN_HEIGHT+16)
					bullets[j][i].state=0;
				if (distance(bullets[j][i].x,player[j^1].x,bullets[j][i].y,player[j^1].y)<40) //collision
					bullets[j][i].state=0;
				SDL_RenderCopyEx( gRenderer, tbullet, &recb_, &recb,(double)bullets[j][i].angle,NULL,SDL_FLIP_NONE);

			}
		}
	}
	selfmp =(SDL_Rect) {10,04,player[0].mp,4};
	selfhp =(SDL_Rect) {10,8,player[0].hp,4};
	enemymp =(SDL_Rect){10,SCREEN_HEIGHT-12,player[1].mp,4};
	enemyhp =(SDL_Rect){10,SCREEN_HEIGHT-8,player[1].hp,4};
	SDL_SetRenderDrawColor( gRenderer, 255, 0, 0, 255 );
	SDL_RenderFillRect( gRenderer, &selfhp);
	SDL_RenderFillRect( gRenderer, &enemyhp);
	SDL_SetRenderDrawColor( gRenderer, 0, 255, 0, 255 );
	SDL_RenderFillRect( gRenderer, &selfmp);
	SDL_RenderFillRect( gRenderer, &enemymp);
	SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
	SDL_RenderPresent( gRenderer ); 
}

void init_net(int argc, char const *argv[])
{
	struct sockaddr_in servaddr;	
	int port;
	//basic check of the arguments
	//additional checks can be inserted
	if (argc <2) {
		printf("Usage: %s <IP address of the server>\n",argv[0]); 
		exit(1);
	}
	if (argc >2)
	{
		port = atoi(argv[2]);
	} else port = 3000;
	//Create a socket for the client
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Problem in creating the socket");
		exit(2);
	}

	//Creation of the socket
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr= inet_addr(argv[1]);
	servaddr.sin_port =  htons(port); //convert to big-endian order
	
	//Connection of the client to the socket 
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
		perror("Problem in connecting to the server");
		exit(3);
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int game()
{
	winlose=-1;
	SDL_Init( SDL_INIT_VIDEO );
	//Create window
	gWindow = SDL_CreateWindow( "Battle Ship I", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	//Create renderer for window
	gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED ); 
	//Initialize renderer color 
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF ); 
	//Initialize PNG loading
	IMG_Init( IMG_INIT_PNG );
	IMG_Init( IMG_INIT_JPG );

	tplayer = loadTexture( "./spaceship.png" );
	teplayer = loadTexture( "./spaceship_enemy.png" );
	bg = loadTexture( "./background.jpg" );
	tbullet = loadTexture("./fire_blue.png");
	bzero(bullets,sizeof(bullets));
	SDL_Event e;
	int count=0;
	

	while(1)
	{	
		count++;
		if((count%=10)==0)
		{
			while( SDL_PollEvent( &e ) != 0 )
			{
				if( e.type == SDL_QUIT )
				{
					goto end;
				}
			}

			//Set texture based on current keystate
			const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

			if ( currentKeyStates[SDL_SCANCODE_UP] ) 	send(sockfd,x+0,1,0);//{player.dx+=cos((player.angle-90)*DEGTORAD)*0.7;player.dy+=sin((player.angle-90)*DEGTORAD)*0.7;}
			if ( currentKeyStates[SDL_SCANCODE_LEFT])	send(sockfd,x+1,1,0);//{player.angle=(player.angle-10)%360;}
			if ( currentKeyStates[SDL_SCANCODE_RIGHT]) 	send(sockfd,x+2,1,0);//{player.angle=(player.angle+10)%360;}
			if ( currentKeyStates[SDL_SCANCODE_SPACE]) 	send(sockfd,x+3,1,0);
		}
		handle();
		render();
		if (winlose >=0){ SDL_DestroyWindow(gWindow); return winlose;}
		usleep(10000);
	}//game loop
	end:
	SDL_DestroyWindow(gWindow);
	exit(1);
	// SDL_Quit();
}