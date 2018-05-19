

#include"memory.h"
#include<common.h>


//16kb
#define MAX_STACK_SIZE (16<<10)


#define PROCESS_RUNNING 0
#define PROCESS_BLOCKED 1
#define PROCESS_RUNNABLE 2
#define PROCESS_DEAD 3
#define PROCESS_NEW 4

#define PROCESS_TIME 10

#define PRO_MEM (1<<16)
#define STACK_BEGIN 0x200000
uint32_t GLOBAL_PID;

struct StackFrame {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	int32_t irq, error;
	uint32_t eip, cs, eflags, esp, ss; 
};

struct ProcessTable
{
	uint8_t stack[MAX_STACK_SIZE];
	struct StackFrame tf;
	int StackTop;
	int state;
	int timeCount;
	int sleepTime;
	uint32_t pid;
	struct ProcessTable*child;
	struct ProcessTable*next;
};

#define MAX_PCB_NUM 15
//用于控制不能过多的进程
struct ProcessTable pcb[MAX_PCB_NUM];

//the line
/*struct ProcessLine
{
	struct ProcessTable* contain;
	struct ProcessLine* next;
}*/
//用于指示链表
struct ProcessTable*pcb_run;
//struct ProcessTable* pcb_running;
struct ProcessTable*pcb_blocked;


void initPcb();
void process_switch();

