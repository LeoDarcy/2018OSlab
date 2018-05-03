#include "device.h"
#include "x86.h"
#include <sys/syscall.h>
#include <string.h>

#define SYS_sleep 200 //sleep没有在上述的头文件定义
//用于全局变量的打印行列，提示现在打到哪一行了
int row=0;
int col=0;

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void TimerHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	__asm__ __volatile__("movl %0, %%eax" ::"r"(KSEL(SEG_KDATA)));
	__asm__ __volatile__("movw %ax, %ds");
  	__asm__ __volatile__("movw %ax, %fs");
  	__asm__ __volatile__("movw %ax, %es");
    	__asm__ __volatile__("movl %0, %%eax" ::"r"(KSEL(SEG_VIDEO)));
    	__asm__ __volatile__("movw %ax, %gs");

	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
            		TimerHandle(tf);
            		break;
		case 0x80:
			syscallHandle(tf);
			break;
		//default:assert(0);//未填充
	}
}

void TimerHandle(struct TrapFrame *tf) {
     	putChar('T');
	//assert(0);
	struct ProcessTable*blk=pcb_blocked;
	while(blk!=NULL)
	{
		blk->sleepTime=blk->sleepTime-1;
		if(0==blk->sleepTime)
		{
			struct ProcessTable*bk=blk;
			blk=blk->next;
			bk->state=PROCESS_RUNNABLE;
			bk->timeCount=PROCESS_TIME;
			bk->next=NULL;
			struct ProcessTable*tmp=pcb_run;
			if(tmp==NULL)
				pcb_run=bk;
			else
			{
				while(tmp->next!=NULL)
					tmp=tmp->next;
				tmp->next=bk;
			}
			tmp=pcb_blocked;
			if(tmp==bk)
				pcb_blocked=blk;
			else
			{
				while(tmp->next!=bk)
					tmp=tmp->next;
				tmp->next=blk;
			}
		}
		blk=blk->next;
	}
	//assert(0);
	if(pcb_run==NULL||pcb_run->state==PROCESS_RUNNABLE)
	{	
		//putChar('X');	
		//assert(0);
		process_switch();
		return ;
	}
	
	pcb_run->timeCount=pcb_run->timeCount-1;
	if(pcb_run->timeCount==0)
	{
		//pcb_run->contain->state=PROCESS_RUNNABLE;
		putChar('x');
		pcb_run->timeCount=PROCESS_TIME;
		process_switch();
	}
}
void cout(char a)
//void cout(int row,int col,char a)//对应于VGA的方法
{
	//tip：需要用暴力push，因为破坏描述符在执行完这句就还原了
	//初始化gs
	__asm__ __volatile__("push %%eax": : );
	__asm__ __volatile__("movl %0, %%eax":: "r"(KSEL(SEG_VIDEO)));
	__asm__ __volatile__("movw %ax, %gs");
	__asm__ __volatile__("pop %%eax": : );

	//通过lab1的方法打印
	__asm__ __volatile__("push %%edi": : );
	__asm__ __volatile__("movl %0,%%edi": :"r"((80*row+col)*2));
	__asm__ __volatile__("push %%eax": : );
	__asm__ __volatile__("movl %0,%%eax": :"r"(0x0c00|a));
	__asm__ __volatile__("movw %%ax,%%gs:(%%edi)": :); // 写入显存
	__asm__ __volatile__("pop %%eax": :);
	__asm__ __volatile__("pop %%edi": :);
	//SetCursor(row,col);
}

void sys_write(struct TrapFrame *tf)
{
	//一定要用static！！
	/*static int row=0;
	static int col=0;*/
	//这里有两种实现方法，一种是直接用全局变量，另一种是用VGA的方法
	//ecx储存字符串首地址，所以需要加上基地址
	tf->ecx += ((pcb_run - pcb) * PRO_MEM);
	__asm__ __volatile__("cli");//关中断 保护全局变量
	char*p =(char*) tf->ecx;	
	for(int i=0;i<tf->edx;i++)
	{
		//assert(0);
		if('\n'!=*p)
		{
			putChar(*p);
			//cout(row,col,*p);
			cout(*p);
			col++;
			if(col>=80)
			{
				row++;
				col=0;
			}
		}
		else
		{
			col=0;
			row++;
		}
		p++;
	}
	__asm__ __volatile__("sti");
}


void sys_exit(struct TrapFrame *tf) {
	putChar('e');
	putChar('x');
	putChar('i');
	putChar('t');
	//assert(0);
	struct ProcessTable*father=pcb_run;
	struct ProcessTable*son=pcb_run;
	//除去子进程
	if(father->child!=NULL)
	{
		while(son->next!=NULL&&son->next!=father->child)
			son=son->next;
		if(son==NULL)
			assert(0);
		struct ProcessTable*tmp=son;
		son=tmp->next;
		tmp->next=son->next;
		son->state=PROCESS_DEAD;
	}
	//除去父进程
	pcb_run=pcb_run->next;
	father->next=NULL;
	father->state=PROCESS_DEAD;
	process_switch();
   
}

void sys_fork(struct TrapFrame *tf) {
	putChar('F');
	//assert(0);
	int index=0;
	for(;index<MAX_PCB_NUM;index++)
	{
		if(pcb[index].state==PROCESS_DEAD)
			break;
	}
	if(index>=MAX_PCB_NUM)
		assert(0);
	//修改这个新的信息
	//复制内存信息
	int now=0;
	for(;now<MAX_PCB_NUM;now++)
	{
		if((&pcb[now])==pcb_run)
			break;
	}
	memcpy((void*)(STACK_BEGIN+(index)*PRO_MEM),(void*)(STACK_BEGIN+(now)*PRO_MEM),PRO_MEM);
	for (int i = 0; i < MAX_STACK_SIZE; i++) 
	{	
        	pcb[index].stack[i] = pcb[now].stack[i];
	}

	//pcb[index]=(*pcb_run);
	pcb[index].tf=pcb[now].tf;
	pcb[index].state = PROCESS_RUNNABLE;
	pcb[index].timeCount = PROCESS_TIME;
	pcb[index].sleepTime = 0;
	GLOBAL_PID ++; 
	pcb[index].pid = GLOBAL_PID;
	//添加到pcb_run的后面
	pcb[index].next=pcb_run->next;
	pcb_run->next=&pcb[index];
	if(pcb_run->child!=NULL)
		assert(0);
	pcb_run->child=&pcb[index];//添加父子信息
	//把pcb_run调整
	pcb_run->timeCount=PROCESS_TIME;
	//设置返回信息
	pcb_run->tf.eax=pcb[index].pid;
	pcb[index].tf.eax=0;
	
	process_switch();
}

void sys_sleep(struct TrapFrame *tf) {
	//putChar('s');
	//putChar('l');
	putChar('Z');
	//putChar('p');
	//assert(0);
	pcb_run->sleepTime=tf->ebx;
	pcb_run->state=PROCESS_BLOCKED;
	struct ProcessTable* blk=pcb_run;
	pcb_run=pcb_run->next;
	blk->next=NULL;

	struct ProcessTable*bk=pcb_blocked;
	if(bk==NULL)
		pcb_blocked=blk;
	else
	{	
		while(bk!=NULL&&bk->next!=NULL)
			bk=bk->next;
		bk->next=blk;
	}
	process_switch();
}


void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch( tf->eax)
	{
		case 0:;//assert(0);
		case SYS_brk:assert(0);break;
		case SYS_open:assert(0);break;
		case SYS_read:assert(0);break;
		case SYS_write:sys_write(tf);break;
		case SYS_lseek:assert(0); break;
		case SYS_close: assert(0); break;
		case SYS_fork:sys_fork(tf);break;
		case SYS_sleep:sys_sleep(tf);break;
		case SYS_exit:sys_exit(tf);break;
		default:assert(0);
	}
	
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
