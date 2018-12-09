#include "client.h"

int main (int    argc, char **argv)
{
	if (argc !=2) {
		printf("Usage: %s <IP address of the server>\n",argv[0]); 
		exit(1);
	}
	logined = 0;
	init_network(argv[1]);	 // network init
	gtk_init(&argc, &argv);  // gtk init
	initLogin();
	loopEvent = g_timeout_add(100,recv_loop,NULL);
	gtk_main();
}
