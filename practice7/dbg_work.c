#include <stdio.h>
#include "dbg_lib.h"

int main(){
	int shmid = dbg_init();
	printf("shmid is %d\n", shmid);
	
	char* buffer = "It works!\n";
	printf("Writing your message!\n");
	dbg_write(shmid, buffer, sizeof(buffer));
	
	char *msg = dbg_read(shmid);
	printf("Your message is: %s", msg);
	printf("Closing...");
	dbg_close(shmid);
	
	return 0;
}
