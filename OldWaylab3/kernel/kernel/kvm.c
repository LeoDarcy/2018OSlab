#include "x86.h"
#include "device.h"
#include <string.h>
SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0; 
	gdt[SEG_VIDEO] = SEG(STA_W,  0x0b8000,       0xffffffff, DPL_KERN);
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */
	//assert(0);
	//tss.esp0=0x200000;
/*	tss.esp0 = (uint32_t)&pcb[0].stack[MAX_STACK_SIZE];
	tss.ss0=KSEL(SEG_KDATA);
	__asm__ __volatile__("ltr %%ax":: "a" (KSEL(SEG_TSS)));
*/
	tss.esp0 = (uint32_t)&pcb[0].stack[MAX_STACK_SIZE];
	tss.ss0  = KSEL(SEG_KDATA);
	__asm__ __volatile__("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	__asm__ __volatile__("movl %0,%%eax"::"r"(KSEL(SEG_KDATA)));
	__asm__ __volatile__("movw %ax,%ds");
	__asm__ __volatile__("movw %ax,%es");
	__asm__ __volatile__("movw %ax,%ss");
	__asm__ __volatile__("movw %ax,%fs");
	// gs is KVIDEO=6
	__asm__ __volatile__("movl %0,%%eax"::"r"(KSEL(6)));
	__asm__ __volatile__("movw %ax, %gs");

	lLdt(0);
	//assert(0);
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	*/
	//实际上这里是把正确的eip等放入堆栈中，等着iret把正确的读入即可
	//iret依次返回eip,cs,eflag
/*	
	__asm__ __volatile__("pushl %0":: "r"(USEL(SEG_UDATA)));
	__asm__ __volatile__("pushl %0":: "r"(128 << 20));			
	__asm__ __volatile__("pushfl");	
	__asm__ __volatile__("pushl %0"::"r"(USEL(SEG_UCODE)));
	__asm__ __volatile__("pushl %0"::"r"(entry));	
	
	asm volatile("iret");

*/
	__asm__ __volatile__("push %%eax"::);
	__asm__ __volatile__("movl %0,%%eax"::"r"(USEL(SEG_UDATA)));
	__asm__ __volatile__("movw %ax,%ds");
	__asm__ __volatile__("movw %ax,%es");
	//__asm__ __volatile__("movw %ax,%ss");
	__asm__ __volatile__("movw %ax,%fs");
	__asm__ __volatile__("pop %%eax"::);

	//在lab3中我们这里需要作出改变，需要添加步骤：把这个app程序当作进程添加到pcb中
	int index=0;
	for(;index<MAX_PCB_NUM;index++)
	{
		if(pcb[index].state==PROCESS_DEAD)
			break;
	}
	pcb[index].state = PROCESS_RUNNING;
	pcb[index].timeCount = PROCESS_TIME;
	pcb[index].sleepTime = 0;
	GLOBAL_PID ++; 
	pcb[index].pid = GLOBAL_PID;
	pcb_run=&pcb[index];
	pcb[index].tf.ss = USEL(SEG_UDATA);
	pcb[index].tf.esp = STACK_BEGIN + PRO_MEM;
  	asm volatile("sti");
    	asm volatile("pushfl");  // %eflags
   	asm volatile("cli");
	asm volatile("movl (%%esp), %0" : "=r"(pcb[index].tf.eflags) :);

	pcb[index].tf.ss = USEL(SEG_UDATA);
	pcb[index].tf.esp = STACK_BEGIN + PRO_MEM;
	pcb[index].tf.eip = entry;
	pcb[index].tf.cs = USEL(SEG_UCODE);
	//pcb_run=&pcb[index];
	pcb_run->next=NULL;
	//注意这里我们需要修改一点trapframe的内容 因为要根据实际情况下的pcb改变

	//这一行可能有问题
	/*__asm__ __volatile__("pushl %0":: "r"(USEL(SEG_UDATA)));
	__asm__ __volatile__("pushl %0":: "r"(128 << 20));			
	__asm__ __volatile__("pushfl");	
	__asm__ __volatile__("movl (%%esp),%0":"=r"(pcb[index].tf.eflags));	
	__asm__ __volatile__("pushl %0"::"r"(USEL(SEG_UCODE)));
	__asm__ __volatile__("pushl %0"::"r"(entry));*/
	
	asm volatile("movl %0, %%esp" ::"r"(&pcb_run->tf.eip));
	asm volatile("iret");    


}

void loadUMain(void) {
	//assert(0);
	/*加载用户程序至内存*/
	struct ELFHeader* elf;
	struct ProgramHeader* ph;
	struct ProgramHeader* eph;
	uint8_t* buf= (uint8_t*)0x5000000;
	
	//读取磁盘中的到buf中	
	for(int i=1;i<101;i++)		
		readSect((void*)(buf+(i-1)*512),200+i);
	//get the elf 	
	elf=(void*)buf;
	//开始分析elf头得到程序地址
	ph = (void*)elf + elf->phoff;
	eph = ph + elf->phnum;
	for(; ph < eph ;ph ++)
	{
		if (1 == ph->type)//判定类型
		{
			uint32_t vaddr = ph->vaddr;
			memcpy((void*)vaddr , (void*)(buf + ph->off) , ph->filesz);
			//清空多余的位置			
			if(ph->memsz - ph->filesz > 0)
				memset((void*)vaddr + ph->filesz,0,ph->memsz-ph->filesz);
		} 
	}
	//进入UMain
	//assert(0);
	enterUserSpace(elf->entry);
}
