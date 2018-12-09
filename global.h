#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
// #define SERV_PORT 3000
#define LISTENQ 8
#define SCREEN_HEIGHT 690
#define SCREEN_WIDTH 920

enum cHeader{LOGIN,SIGNUP,UPGRADE,GAME_REQ};
enum sHeader{LOGIN_F,LOGIN_S,SIGNUP_F,SIGNUP_S,UPGRADE_F,UPGRADE_S,INFO,GAME_ACK};
enum upHeader{HPUP,MPUP,ATKUP,DEFUP};
//-------------constant----------------
#define SERV_PORT 6969 /*port*/