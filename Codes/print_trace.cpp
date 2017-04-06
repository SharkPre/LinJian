/*
	�����̽��յ� SIGINT/SIGKILL/SIGTERM ʱ�����øú������Դ�ӡ���̽���ǰ�Ķ�ջ��Ϣ 
*/
void print_trace(void)
{
	int i;   
	const int MAX_CALLSTACK_DEPTH = 32;    /* ��Ҫ��ӡ��ջ�������� */  
	void *traceback[MAX_CALLSTACK_DEPTH];  /* �����洢���ö�ջ�еĵ�ַ */  
	/* ���� addr2line ������Դ�ӡ��һ��������ַ���ڵ�Դ����λ��   
	* ���ø�ʽΪ�� addr2line -f -e /tmp/a.out 0x400618  
	* ʹ��ǰ��Դ�������ʱҪ���� -rdynamic -g ѡ��  
	*/  
	char cmd[512] = "addr2line -f -e ";   
	char *prog = cmd + strlen(cmd);   
	/* �õ���ǰ��ִ�г����·�����ļ��� */  
	int r = readlink("/proc/self/exe",prog,sizeof(cmd)-(prog-cmd)-1);   
	/* popen��fork��һ���ӽ���������/bin/sh, ��ִ��cmd�ַ����е����  
	* ͬʱ���ᴴ��һ���ܵ������ڲ�����'w', �ܵ������׼���������ӣ�  
	* ������һ��FILE��ָ��fpָ���������Ĺܵ����Ժ�ֻҪ��fp��������д�κ����ݣ�  
	* ���ݶ��ᱻ��������׼���룬  
	* ������Ĵ����У��Ὣ���ö�ջ�еĺ�����ַд��ܵ��У�  
	* addr2line�����ӱ�׼�����еõ��ú�����ַ��Ȼ����ݵ�ַ��ӡ��Դ����λ�úͺ�������  
	*/  
	/* �õ���ǰ���ö�ջ�е����к�����ַ���ŵ�traceback������ */  
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
		///* �õ����ö�ջ�еĺ����ĵ�ַ��Ȼ�󽫵�ַ���͸� addr2line */  
		//printf("%p\n", traceback[i]);   
		///* addr2line �������յ���ַ�󣬻Ὣ������ַ���ڵ�Դ����λ�ô�ӡ����׼��� */  
	} 
	GT_INFO("-------------------------------[end]");
}