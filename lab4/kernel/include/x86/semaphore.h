#include<memory.h>
#include<common.h>

#define sem_t int
#define SEMA_DEAD 0
#define SEMA_USED 1
#define SEMA_BLOCKED 3
struct Semaphore{
	int value;
	int state;
	int sema_pcb_blocked[MAX_PCB_NUM];	
};

typedef struct Semaphore Semaphore;

#define MAX_SEMA_NUM 10
Semaphore semaphore[MAX_SEMA_NUM];
void initSema();
/*
void P( Semaphore* s);
void V( Semaphore* s);
*/
int sem_init(sem_t *sem, uint32_t value);
