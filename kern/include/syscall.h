/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

struct trapframe; /* from <machine/trapframe.h> */

/*
 * The system call dispatcher.
 */

void syscall(struct trapframe *tf);

/*
 * Support functions.
 */

/* Helper for fork(). You write this. */
void enter_forked_process(void *tf,unsigned long adr_space);

/* Enter user mode. Does not return. */
void enter_new_process(int argc, userptr_t argv, vaddr_t stackptr,
		       vaddr_t entrypoint);


/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */
int sys_reboot(int code);
int sys___time(userptr_t user_seconds, userptr_t user_nanoseconds);

/**
 * Pandhari : File system support
 * Note : we need to return 2 things compulsorily
 * 1.return value : 1/0
 * 2.return code : listed in errno.h
 * 
 *
 **/
int init_file_system(void);
int sys_open(char *file_name, int flag, mode_t mode,int *retval);
int sys_dup2(int oldfd, int newfd,int *retval);
int sys_close(int fd,int *retval);
int sys_read(int fd, void *buf, size_t buflen,int *retval);
int sys_write(int fd, void *buf, size_t buflen,int *retval);
int sys_lseek(int fd, off_t pos, int code,off_t *retval);
int sys_chdir(char *u_path,int *retval);
int sys__getcwd(char *buf, size_t buflen,int *retval);

/**
 * Pandhari : process support
 *
 **/
int sys_fork(void (*entrypoint)(void *data1, unsigned long data2),
	     struct trapframe *data1, unsigned long data2,int *retval);
int sys_execv(char *prog, char **argv,int *retval);
int sys__exit(int *retval);
int sys_waitpid(pid_t pid, int *status, int option,int *retval);
int sys_getpid(pid_t *retval);
int sys_getppid(pid_t *retval);
int sys_sbrk(size_t change,int *retval);
int get_local_pid(void);
void init_process(int id,int ppid);

#endif /* _SYSCALL_H_ */
