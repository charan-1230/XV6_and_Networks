#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int decode_mask(long long int mask)
{
    int result = 0;
    while (mask > 1)
    {
        result++;
        mask = mask / 2;
    }
    return result;
}

int main(int argc, char *argv[])
{
    char sys_call_name[32][20]={"-","fork","exit","wait","pipe","read","kill","exec","fstat","chdir","dup","getpid","sbrk","sleep","uptime","open","write","mknod","unlink","link","mkdir","close","waitx","getSysCount","sigalarm","sigreturn","-","-","-","-","-","-"};

    if (argc < 3)
    {
        printf("Usage: %s <mask> <command>\n", argv[0]);
        return 1;
    }

    long long int mask = atoi(argv[1]);
    int sys_num=decode_mask(mask);
    
    int child_pid = fork();
    if (child_pid < 0)
    {
        printf("fork failed\n");
        return 0;
    }
    else if (child_pid == 0)
    {
        char *arg_exec[argc - 1];
        int ind = 0;
        arg_exec[ind++] = argv[2];
        for (int i = 3; i < (argc); i++)
        {
            arg_exec[ind++] = argv[i];
        }
        arg_exec[ind++] = 0;
        if (exec(arg_exec[0], arg_exec) == -1)
        {
            printf("exec failed\n");
        }
        exit(1);
    }
    else
    {
        wait(0);
        int count = getSysCount(sys_num);
        printf("PID %d called %s %d times\n", child_pid, sys_call_name[sys_num], count);
    }

    return 0;
}