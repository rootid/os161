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
#include <synch.h>
#include <kern/wait.h>
/*
 * Pandhari : Process system support
 * In case of system operations
 * 1.update 
 * 2.maintain 
 *
 */
#define MAX_ARGS 4
//Intiallize bookeeper for thread
static struct thread_book proc[PID_MAX];

void
init_process(int id,int ppid) {
	proc[id].valid = IN_USE;
	proc[id].exit_code = NOT_EXIT;
	proc[id].ppid = ppid;
	proc[id].child_cnt = 1;
	proc[ppid].child_cnt += 1;
	//create semmaphore
	proc[id].enter_sem = sem_create("process init",1);
	proc[id].exit_sem = sem_create("process init",0);
}

int 
get_local_pid(void) {
	int i = 1;
	for(;i<PID_MAX;i++) {
	 	if (proc[i].valid == NOT_IN_USE) {
			break;	
		}
	}
	return i;
}

int 
sys_fork(void (*entrypoint)(void *data1, unsigned long data2),
		struct trapframe* data1, unsigned long data2,int *retval){

	//TODO : cross chec about the data2
	(void) data2;
	//struct thread *self;
	struct addrspace *new_addrspace;
	//Note : -1 and 0 (init) are special processes refer file :wait.h
	
	struct trapframe *new_tf;
        new_tf	= kmalloc(sizeof(struct trapframe));	
	bzero(new_tf,sizeof(struct trapframe));
	//1.Copy parent’s trap frame, and pass it to child thread
	memcpy(new_tf,data1,sizeof(struct trapframe));
	
	//2.Copy parent’s address space
	//result = as_copy((struct addrspace*)data2,&new_addrspace);
	int result = 0;
	result = as_copy(curthread->t_addrspace,&new_addrspace);
	if(result) {
		return result;	
	}

	pid_t i = 2;	
	for(;i<PID_MAX;i++) {
	 	if (proc[i].valid == NOT_IN_USE) {
			if(0 == curthread->pid ){
				curthread->pid = get_local_pid();
				init_process(curthread->pid,0);
			}
			P(proc[curthread->pid].enter_sem);
			init_process(i,curthread->pid);
			V(proc[curthread->pid].enter_sem);
			//increament semaphore count
			//Use simple array to maintain the count for the parent
			break;	
		}
	
	}
	if(i > PID_MAX) {
		*retval = EMPROC;
		return (1);	
	}

	//copy the pid Send anything to new thread with the help of new_tf
	//3.Create child thread
	
	new_tf->tf_a0 = i;
	struct thread *new_t;
	result = thread_fork("new_proc",entrypoint,new_tf,(unsigned long)new_addrspace,&new_t);
	if(result) {
		proc[i].valid = NOT_IN_USE;
		//proc[i].self_thread = 0;
		return result;	
	}
	//4.Copy parent’s file table into child
	
	//5.Parent returns with child’s pid immediately
	*retval = i;
	return (0);

}

int 
sys_execv(char *progname, char **argv,int *retval) {
	//(void) prog;
	//(void) argv;
	//userspace(u/space) ---> kernelspace (k/space) ---> stackptr (u/space)
	//simillar to elf,pointers are aligned by 4 in mips (32 bit)
	//1.copy the arguments into kernel buffer
	int argc = 0;
	int result = 0;
	size_t actual_len = 0;
	vaddr_t entrypoint,stackptr;
	struct vnode *vn = 0;
	char **kargv;
	//copy the file name
	char dest[PATH_MAX + 1];
	if (copyinstr((const_userptr_t)progname,dest,PATH_MAX, &actual_len) != 0) {
		kprintf("not able to copy the string from the user space\n");
		return 1;
	}
	//To avoid garbage values
	dest[actual_len] = 0;
	if(strlen(dest) == 0)	{
		*retval = ENOENT;
		return (1);	
	}
	/* Open the file. */
	result = vfs_open(dest, O_RDONLY, 0, &vn);
	if (result) {
		*retval = ENOENT;
		return 1;
	}
	kargv = (char **)kmalloc(sizeof(char));
	if(argv != 0) { //When argument is NULL
		while(argv[argc] != 0)  {
			//copyinstr
			actual_len = 0;
			argc += 1;
			if(argc >= MAX_ARGS) {
				*retval = E2BIG;	
				return (1);
			}
			char local_args [NAME_MAX] ;
			copyinstr((const userptr_t)argv[argc],local_args,NAME_MAX,&actual_len);
			local_args[actual_len] = 0;
			kargv[argc] = local_args;
		}
	}
	//2.open the new excutable,create new address space,and load elf into it
	/* We should be a new thread. */
	//KASSERT(curthread->t_addrspace == NULL);

	/* Create a new address space. */
	curthread->t_addrspace = as_create();
	if (curthread->t_addrspace==NULL) {
		vfs_close(vn);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_addrspace);
	
	result = load_elf(vn,&entrypoint);
	
	if (result) {
		/* thread_exit destroys curthread->t_addrspace */
		vfs_close(vn);
		return result;
	}
	
	/* Done with the file now. */
	vfs_close(vn);
	
	/* Define the user stack in the address space */
	stackptr = 0;
	result = as_define_stack(curthread->t_addrspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_addrspace */
		return result;
	}

	//3.copy arguments from kernel stack
	int i = 0;
	for(i=argc;i>0;i--) {
		int len =  strlen(kargv[i]) + 1;
		int l1 = len % 4;
		actual_len = 0;
		if(l1 != 0) {
			len += l1;	
		}
		copyoutstr(kargv[i],(userptr_t)stackptr,len,&actual_len);
		stackptr -= len;
	}	
	
	//4.return into use mode using enter new process	
	enter_new_process(argc,(userptr_t)stackptr,stackptr,entrypoint);
	
	return (0);
}

int 
sys__exit(int *retval) {
	//do the bookeeping stuff
	int ppid;
	int pid = curthread->pid;
	while(0 != proc[pid].child_cnt) {
		P(proc[pid].enter_sem);
		proc[pid].exit_code = *retval;
		proc[pid].child_cnt -= 1;
		ppid = proc[pid].ppid;
		proc[ppid].child_cnt -= 1;
		proc[pid].valid = NOT_IN_USE;

		//dealloc all the file handles
		int i=3;
		for(;i<=MAX_FD_SIZE;i++) {
			if(0 != curthread->pt_fd[i]) {
				//close the file
				//free the desciptors		
				vfs_close(curthread->pt_fd[i]->pv_node);
				kfree(curthread->pt_fd[i]);
				curthread->pt_fd[i] = NULL;
			}
		}
		//V(proc[pid].enter_sem);
	}
	int _exitcode = *retval;
	*retval = _MKWVAL(_exitcode) | __WEXITED;
	proc[pid].exit_code = *retval;
	//exit the thread
	V(proc[pid].exit_sem);
	thread_exit();
	return (0);
}
/**
 * Note : not using any option
 */
int 
sys_waitpid(pid_t pid, int *status, int option,int *retval) {

	//Waitpid pid : can be child / parent
	//TODO : check aligned by 4 condition
	(void) option;
	(void) status;
	if(proc[pid].valid == NOT_IN_USE) {
		*retval = ESRCH;
		return 1;	
	}
	//wait untill all it's child exits
	P(proc[pid].exit_sem);
	//*status = proc[pid].exit_code;
	*retval = proc[pid].exit_code;
	return (0);
}

int 
sys_getpid(pid_t *retval) {
	*retval = curthread->pid;
	return (0);
}

int 
sys_getppid(pid_t *retval) {
	*retval = proc[curthread->pid].ppid;
	return (0);	

}

int 
sys_sbrk(size_t change,int *retval) {
	(void) change;
	(void) retval;
	/***
	 * 
	heap        heap
	base        top
	(vaddr)     (vaddr+MEM_SIZE)
	----------------------------------------------
	|	     |                   |     |    |
	|            |                   |     |    |          
	|            |                   |     |    |          		  ------------------------------------------------
	 **/			
			
	//as_define_region();
	//			
	//if (change < 0) {
	//	*retval = EINVAL;	
	//	return (1);
	//}
	
	
	return (0);
}
