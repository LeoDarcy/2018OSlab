#include "lib.h"
#include "types.h"
#include <string.h>
#include <stdarg.h>
#include <sys/syscall.h>

/*
 * io lib here
 * 库函数写在这
 */

#define SYS_sleep 200

int32_t process_syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
	int32_t ret = 0;

	asm volatile("int $0x80": "=a"(ret) : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx));

	return ret;
}

int fork()
{ 
	//assert(0);
	//return 0;
	//putChar('k');
	return process_syscall(SYS_fork,1,1,1);
}

void sleep(uint32_t time)
{
	process_syscall(SYS_sleep,time,1,1);
}


void exit()
{
	process_syscall(SYS_exit,1,1,1);
}




int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;

	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/
	//assert(0);
	/*__asm__ __volatile__("movl %0,%%eax"::"r"(num));
	__asm__ __volatile__("movl %0,%%ebx"::"r"(a1));
	__asm__ __volatile__("movl %0,%%ecx"::"r"(a2));
	__asm__ __volatile__("movl %0,%%edx"::"r"(a3));
	__asm__ __volatile__("movl %0,%%edi"::"r"(a4));
	__asm__ __volatile__("movl %0,%%esi"::"r"(a5));
	asm volatile("int $0x80");*/
	asm volatile("int $0x80":"=a"(ret):"a"(num),"b"(a1),"c"(a2),"d"(a3),"D"(a4),"S"(a5));	
	return ret;
}


void printfc(char a)
{
	//其中 eax = 4 指出系统调用号为 4，含义是 SYS_Write；ebx = 1 是文件描述符，指出写的目标是标准输出 stdout；ecx 中保存的是待输出字符串的首地址；而 edx 中则保存待输出字符串的长度。
	//syscall(SYS_write,1,(uint32_t)&a,1,0,0);
	process_syscall(SYS_write,1,(uint32_t)&a,1);
}

void printfs(char* a)
{
	int len=0;
	char*tmp=a;
	while('\0'!=*tmp)
	{
		len++;
		tmp++;
	}
	//syscall(SYS_write,1,(uint32_t)a,len,0,0);
	process_syscall(SYS_write,1,(uint32_t)a,len);
}

void printfd(int a)
{
	if(a>=0)
	{
		int len=0;
		int tmp=a;
		while(tmp!=0)
		{
			len++;
			tmp/=10;
		}
		if(len==0)
		{
			/*char o='0';
			syscall(SYS_write,1,(uint32_t)&o,1,0,0);*/
			char*out="0";
			printfs(out);
			return ;
		}
		char out[14];
		/*for(int i=0;i<len;i++)
		{
			tmp=(a/10);
			tmp*=10;
			//out[i]=(char)('0'+a-tmp);
			out[i]=('0'+a%10);
			a/=10;
		}
		for(int i=len-1;i>=0;i--)
		{
			char tp=out[i];
			//syscall(SYS_write,1,(uint32_t)&(out[i]),1,0,0);
			//syscall(SYS_write,1,(uint32_t)&tp,1,0,0);
			printfc(tp);
		}*/
		for(int i=len-1;i>=0;i--)
		{
			tmp=(a/10);
			tmp*=10;
			out[i]=(char)('0'+a-tmp);
			//out[i]=('0'+ a%10);
			a/=10;
		}
		out[len]='\0';
		printfs(out);
			
	}
	else if(0x80000000==a)
	{
		char*out="-2147483648";
		//syscall(SYS_write,1,(uint32_t)out,10,0,0);
		process_syscall(SYS_write,1,(uint32_t)out,10);
	}
	else
	{
		a=-a;
		char neg='-';
		//syscall(SYS_write,1,(uint32_t)&neg,1,0,0);
		process_syscall(SYS_write,1,(uint32_t)&neg,1);
		printfd(a);
	}
	
}

void printfx(unsigned int a)
{
	char*head="0x";
	//printfs(head);
	//syscall(SYS_write,1,(uint32_t)head,2,0,0);
	process_syscall(SYS_write,1,(uint32_t)head,2);
	int len=0;
	unsigned int tmp=a;
	while(tmp>0)
	{
		len++;
		tmp=(tmp>>4);
	}
	if(len==0)
	{
		char*out="0";
		printfs(out);
		return ;
	}
	char out[14];
	/*for(int i=0;i<len;i++)
	{
		tmp=((a>>4)<<4);
		int ot=a-tmp;
		if(tmp<10)
			out[i]=(char)('0'+ot);
		else
			out[i]=(char)('A'+ot);
		a=(a>>4);
	}
	for(int i=len-1;i>=0;i--)
		syscall(SYS_write,1,(uint32_t)&(out[i]),1,0,0);*/
	for(int i=len-1;i>=0;i--)
	{
		tmp=((a>>4)<<4);
		int ot=a%16;
		if(ot<10)
			out[i]=(char)('0'+ot);
		else
			out[i]=(char)('A'+ot-10);
		a=(a>>4);
	}
	out[len]='\0';
	printfs(out);
}
	

void printf(const char *format,...){
	//引入头文件处理变参问题
	//assert(0);
	va_list ap;
	va_start(ap,format);
	/*int len=strlen(format);
	if(len<=0)
		return ;
	for(int i=0;i<len;i++)*/
	while('\0'!=*format)
	{
		if('%'==*format)
		{
			format++;
			switch(*format)
			{
				case 'c':printfc(va_arg(ap,int));break;
				case 's':printfs(va_arg(ap,char*));break;
				case 'd':printfd(va_arg(ap,int));break;
				case 'x':printfx(va_arg(ap,int));break;
				default:;//assert(0);
			}		
		}
		else
		{
			printfc(*format);
		}
		format++;
	}
	va_end(ap);
}
