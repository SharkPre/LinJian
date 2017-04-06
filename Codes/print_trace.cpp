/*
	当进程接收到 SIGINT/SIGKILL/SIGTERM 时，调用该函数可以打印进程结束前的堆栈信息 
*/
void print_trace(void)
{
	int i;   
	const int MAX_CALLSTACK_DEPTH = 32;    /* 需要打印堆栈的最大深度 */  
	void *traceback[MAX_CALLSTACK_DEPTH];  /* 用来存储调用堆栈中的地址 */  
	/* 利用 addr2line 命令可以打印出一个函数地址所在的源代码位置   
	* 调用格式为： addr2line -f -e /tmp/a.out 0x400618  
	* 使用前，源代码编译时要加上 -rdynamic -g 选项  
	*/  
	char cmd[512] = "addr2line -f -e ";   
	char *prog = cmd + strlen(cmd);   
	/* 得到当前可执行程序的路径和文件名 */  
	int r = readlink("/proc/self/exe",prog,sizeof(cmd)-(prog-cmd)-1);   
	/* popen会fork出一个子进程来调用/bin/sh, 并执行cmd字符串中的命令，  
	* 同时，会创建一个管道，由于参数是'w', 管道将与标准输入相连接，  
	* 并返回一个FILE的指针fp指向所创建的管道，以后只要用fp往管理里写任何内容，  
	* 内容都会被送往到标准输入，  
	* 在下面的代码中，会将调用堆栈中的函数地址写入管道中，  
	* addr2line程序会从标准输入中得到该函数地址，然后根据地址打印出源代码位置和函数名。  
	*/  
	/* 得到当前调用堆栈中的所有函数地址，放到traceback数组中 */  
	int depth = backtrace(traceback, MAX_CALLSTACK_DEPTH);
	char **strings;
	strings = backtrace_symbols (traceback, depth);
	GT_INFO("-------------------------------[begin]");
	for (i = 0; i < depth; i++)   
	{   
		if (strings)
		{
			GT_INFO("%s", strings[i]);
		}
		///* 得到调用堆栈中的函数的地址，然后将地址发送给 addr2line */  
		//printf("%p\n", traceback[i]);   
		///* addr2line 命令在收到地址后，会将函数地址所在的源代码位置打印到标准输出 */  
	} 
	GT_INFO("-------------------------------[end]");
}