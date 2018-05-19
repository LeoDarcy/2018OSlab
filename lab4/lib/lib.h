#ifndef __lib_h__
#define __lib_h__
#include"types.h"
void printf(const char *format,...);
//lab3
int fork();
void sleep(uint32_t time);
void exit();


int sem_init(sem_t *sem, int value);

void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);
void sem_destroy(sem_t *sem);
#endif
