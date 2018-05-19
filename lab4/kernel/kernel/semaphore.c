#include"device.h"
#include"x86.h"

void initSema()
{
	for(int i=0;i<MAX_SEMA_NUM;i++)
	{
		semaphore[i].state=SEMA_DEAD;
		for(int j=0;j<MAX_PCB_NUM;j++)
			semaphore[i].sema_pcb_blocked[j]=SEMA_DEAD;
	}
}


