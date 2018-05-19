
#include"device.h"
#include"x86.h"

extern SegDesc gdt[NR_SEGMENTS];
extern TSS tss;

void EnterIdle()
{
	/*putChar('I');
	putChar('d');
	putChar('l');
	putChar('e');*/
	__asm__ __volatile__("movl %0, %%esp;" ::"i"(0x200000));
	enableInterrupt();
	waitForInterrupt();
}

void process_switch()
{
	//assert(0);
	putChar('S');
	//putChar('w');
	//putChar('c');
	//putChar('h');
	if(pcb_run==NULL)
	{
		//assert(0);
		putChar('~');
		EnterIdle();
		return ;
	}
	if(pcb_run->state==PROCESS_RUNNING)
	{
		struct ProcessTable*run=pcb_run;
		while(run->next!=NULL)
		{
			run=run->next;
		}
		run->next=pcb_run;
		run=run->next;
		run->state=PROCESS_RUNNABLE;
		run->timeCount=PROCESS_TIME;
		pcb_run=pcb_run->next;
		run->next=NULL;
	}
	pcb_run->state=PROCESS_RUNNING;
	pcb_run->timeCount=PROCESS_TIME;
	//加载第一个
	//putChar('P');
	putChar('0' + pcb_run->pid);
	tss.esp0 = (uint32_t)&(pcb_run->StackTop);
        tss.ss0  = KSEL(SEG_KDATA);
	int index=0;
	for(;index<MAX_PCB_NUM;index++)
	{
		if((&pcb[index])==(pcb_run))
			break;
	}
	gdt[SEG_UCODE]=SEG(STA_X | STA_R, (index)*PRO_MEM, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA]=SEG(STA_W,         (index)*PRO_MEM, 0xffffffff, DPL_USER);
        __asm__ __volatile__("pushl %eax"); 
        __asm__ __volatile__("movl %0, %%eax" ::"r"(USEL(SEG_UDATA)));
        __asm__ __volatile__("movw %ax, %ds");
        __asm__ __volatile__("movw %ax, %es");
        __asm__ __volatile__("popl %eax");

        // 还原信息
        __asm__ __volatile__("movl %0, %%esp" ::"r"(&pcb_run->tf));
        __asm__ __volatile__("popl %gs");
        __asm__ __volatile__("popl %fs");
        __asm__ __volatile__("popl %es");
        __asm__ __volatile__("popl %ds");
        __asm__ __volatile__("popal");  
        __asm__ __volatile__("addl $4, %esp");//skip irq and error 
        __asm__ __volatile__("addl $4, %esp");

        // 进入运行
        __asm__ __volatile__("iret");
}

void initPcb()
{
	GLOBAL_PID=0;
	for(int i=0;i<MAX_PCB_NUM;i++)
	{
		pcb[i].state=PROCESS_DEAD;
		pcb[i].next=NULL;
		pcb[i].child=NULL;
	}
	pcb_run=NULL;
	pcb_blocked=NULL;
}





