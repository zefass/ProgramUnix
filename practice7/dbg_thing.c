#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>  
#include <sys/shm.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_NAME "my_shm"
#define SHM_SIZE 1024
#define SHM_DEVICE "/dev/shm/my_shm"
 
 
int dbg_init(void) {
    int shmid = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmid, SHM_SIZE);

    return shmid;
}

void dbg_close(int shmid) {
    close(shmid);
    shm_unlink(SHM_NAME);
}

void dbg_write(int shmid, char* buffer, int length) {
    char *ptr = (char*) mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, shmid, 0);
    sprintf(ptr, "%s", buffer);
}

char* dbg_read() {
    FILE* fp =  fopen(SHM_DEVICE, "r");
 
    if (NULL == fp) {
        return NULL;    
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    
    char* buffer = (char*) malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }
    
    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';
    
    fclose(fp);
    return buffer;
}
