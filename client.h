#ifndef DEF_GLOBAL_H
#define DEF_GLOBAL_H

#define sYOU_WIN "Chiến thắng"
#define sYOU_LOSE "Thất bại"
#define sLOGIN "Đăng nhập"
#define sSIGNUP "Đăng ký"
#define sUSERNAME "Tên đăng nhập:"
#define sPASSWORD "Mật khẩu:"
#define sREPASSWORD "Nhập lại mật khẩu:"
#define sCANCEL "Hủy"
#define sLOGIN_FAILED "Đăng nhập thất bại"
#define sLOGIN_FAILED_ "Tên đăng nhập hoặc mật khẩu sai!"
#define sSIGNUP_FAILED "Đăng ký thất bại"
#define sLOGIN_SUCCESSED "Đăng nhập thành công"
#define sSIGNUP_SUCCESSED "Đăng ký thành công"
#define sNOT_EMPTY "Không được để trống tài khoản hoặc mật khẩu!"
#define sNOT_SAME "Hai mật khẩu không đồng nhất!"
#include "global.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
int logined;
int gaming;
struct UserInfo
{
	char user[32];
	char pass[32];
	int hp,mp,atk,def;
	int level,point,win,lose;
} User;
//-------------network----------------
void signup(char *u, char *p);
void login(char *u, char *p);
int init_network(char * ip);
int recv_loop(void *a);
void sendUpdate(int a);
int sockfd;
char packet[100];

//-------------gui----------------
void showMessage(GtkWidget *parent, GtkMessageType type, char *mms, char *content);
void initLogin();
void switchMain();
void updateView();
guint loopEvent;
void switchGame();
void sendPacket(int len);

#endif
