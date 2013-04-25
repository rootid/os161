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

#include <types.h>
#include <syscall.h>
#include <clock.h>
#include <copyinout.h>
#include<limits.h>
#include <kern/types.h>
#include <kern/errno.h>
#include <types.h>
#include <vnode.h>
#include <uio.h>
#include <copyinout.h>
#include <current.h>
#include <thread.h>
#include <lib.h>
#include <vfs.h>
#include <syscall.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <kern/fcntl.h>
#include <addrspace.h>
#include <mips/trapframe.h>
/*
 * Pandhari : Process system support
 * In case of system operations
 * 1.update 
 * 2.maintain 
 *
 */

static struct thread_book proc[PID_MAX];

int sys_fork(void (*entrypoint)(void *data1, unsigned long data2),
		void *data1, unsigned long data2,int *ret_pid){

	//TODO : cross chec about the data2
	//(void) data2;
	struct thread *self;
	struct addrspace *new_addrspace;
	int i = 0;	
	int result;

	for(;i<PID_MAX;i++) {
	 	if (proc[i].valid != IN_USE) {
			proc[i].valid = IN_USE;
			proc[i].ppid = curthread->pid;
			*ret_pid = i;
			break;	
		}
	
	}

	if(i > PID_MAX) {
		return EMPROC;	
	}

	struct trapframe *new_tf = (struct trapframe*)kmalloc(sizeof(struct trapframe));	


	//1.Copy parent’s trap frame, and pass it to child thread
	memcpy(new_tf,data1,sizeof(struct trapframe));
	//copy the pid
	new_tf->tf_a0 = i;

	//2.Copy parent’s address space
	result = as_copy((struct addrspace*)data2,&new_addrspace);
	if(result) {
		return result;	
	}
	//3.Create child thread
	result = thread_fork("new_proc",entrypoint,new_tf,(unsigned long)new_addrspace,&self);
	if(result) {
		proc[i].valid = NOT_IN_USE;
		return result;	
	}
	//4.Copy parent’s file table into child
	
	
	//5.Parent returns with child’s pid immediately
	
	//6.Child returns with 0
	return (0);

}

int sys_execv(const char *prog, char *const *args) {
	(void) prog;
	(void) args;
	return (0);
}

int sys__exit(int code) {
	(void) code;
	return (0);
}

int sys_waitpid(pid_t pid, int *returncode, int flags) {
	(void)pid;
	(void)returncode;
	(void)flags;

	return (0);
}

int sys_getpid(int *ret_pid) {
	*ret_pid = curthread->pid;
	return (0);
}

int sys_getppid(int *ret_ppid) {
	(void) ret_ppid;
	return (0);	

}

int sys_sbrk(size_t change) {
	(void) change;
	return (0);
}
