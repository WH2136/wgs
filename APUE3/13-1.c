#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>		// for exit
#include <signal.h>		// for sigaction
#include <syslog.h>		// for syslog
#include <sys/stat.h>	// for umask
#include <sys/resource.h>

void daemonize (const char *cmd)
{
	int					i, fd0, fd1, fd2;
	pid_t				pid;
	struct rlimit		rl;
	struct sigaction	sa;

	// �����ļ�Ȩ�������� => ������
	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		perror("getrlimit");
		exit(-1);
	}

	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(-1);
	}
	else if (pid != 0)
	{
		// �������˳�
		exit(0);
	}

	// 1. ��������ն�
	// 2. ��Ϊ�鳤����
	// 3. ��Ϊ�Ự���׸�����
	setsid();

	// ���� SIGHUP �ź�
	// ���ն��˳����ᷢ�͸��źŸ��Ự�鳤���̣�Ĭ�ϴ���ʽΪ�˳�
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGHUP, &sa, NULL) < 0)
	{
		perror("sigaction");
		exit(-1);
	}

	// �ٴ� fork ���ػ����̲��ٵ����Ự�鳤����ֹ�����´��ն�
	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(-1);
	}
	if (pid != 0)
	{
		// �������˳�
		printf("%d\n", pid);
		exit(0);
	}

	if (chdir("/") < 0)
	{
		perror("chdir");
		exit(-1);
	}
	
	// �ر������ļ�������
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i=0; i<rl.rlim_max; ++i)
		close(i);

	// �ض����׼���롢��׼�������������� /dev/null
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2)
	{
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
				fd0, fd1, fd2);
		exit(-1);
	}
	while (1)
	{
		// TODO
	}
}

int main (int argc, char *argv[])
{
	daemonize(argv[0]);
	return 0;
}
