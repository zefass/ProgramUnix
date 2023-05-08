#ifndef dbg_h
#define dbg_h

extern int dbg_init(void);
extern void dbg_write(int shmid, char *buffer, int length);
extern char* dbg_read();
extern void dbg_close(int shmid);

#endif
