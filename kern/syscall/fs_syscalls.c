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
/*
 * Pandhari : File system support
 * In case of file operations
 * 1.update file offfset
 * 2.maintain file descriptor count
 * When you use SEEK_END always use VOP_STAT to get the file size
 *
 * off_t : 64 bit
 */

int
init_file_system(void) {
	
	struct vnode *vn = 0;
	int result;
	char console[5] = {0};
	//this is the weird thing to copy each time but when we invoke the vfs_open content at console gets garbled smells breaking.....
	strcpy(console,"con:");
	//open stdin
	result = vfs_open(console,O_WRONLY,0,&vn);
	if(result) {
		vfs_close(vn);		
		return 1;
	}
	curthread->pt_fd[0] = (struct file_table *)kmalloc(sizeof(struct file_table));
	curthread->pt_fd[0]->pv_node = vn;
	strcpy(curthread->pt_fd[0]->file_name,"con:");

	//open stdout
	strcpy(console,"con:");
	result = vfs_open(console,O_RDONLY,0,&vn);
	if(result) {
		vfs_close(vn);		
		return 1;
	}
	curthread->pt_fd[1] = (struct file_table *)kmalloc(sizeof(struct file_table));
	curthread->pt_fd[1]->pv_node = vn;
	strcpy(curthread->pt_fd[1]->file_name,"con:");
	
	//open stderror
	strcpy(console,"con:");
	result = vfs_open(console,O_WRONLY,0,&vn);
	if(result) {
		vfs_close(vn);		
		return 1;
	}
	curthread->pt_fd[2] = (struct file_table *)kmalloc(sizeof(struct file_table));
	curthread->pt_fd[2]->pv_node = vn;
	strcpy(curthread->pt_fd[2]->file_name,"con:");
	return (0);

}
int 
sys_open(char* u_path, int flag, mode_t mode,int *retval){

	//(void) u_path;
	//(void) flag;	//RWX 
	//(void) mode; //USER_MODE,GROUP
	int result = 0;
	//0 : STDIN 1:STDOUT 2:STDERROR
	int index = 3;
	char dest[PATH_MAX + 1];
	size_t actual_len;
	//NULL user path
	if (copyinstr((const_userptr_t)u_path,dest,PATH_MAX, &actual_len) != 0) {
		kprintf("not able to copy the string from the user space\n");
		return 1;
	}
	//To avoid garbage values
	dest[actual_len] = 0;
	if(u_path == 0)	{
		*retval = EFAULT;
		return (1);
	}
	else if (strlen(dest) > PATH_MAX ) {
		*retval = ENAMETOOLONG;
		return (1);
	}

	//TODO : initialize 0,1,2 i.e. console,stdin and stdout here
	
	if(0 == curthread->pt_fd[0])  {
		result = init_file_system();	
	}
	if(result) {
		*retval = result;
		return (1);
	}
	for(;index<MAX_FD_SIZE;index++) {
		if(NULL == curthread->pt_fd[index]) {
			curthread->pt_fd[index] = (struct file_table *)kmalloc(sizeof(struct file_table));
			curthread->pt_fd[index]->pv_node = NULL;
			strcpy(curthread->pt_fd[index]->file_name,dest);
			break;
		}
	}
	if(index > MAX_FD_SIZE) {
		//too many open files
		*retval = EMFILE;
		return (1);
	}

	result = vfs_open(u_path,flag,mode,&curthread->pt_fd[index]->pv_node);
	if(result) {
		//free the allocated memory and null the pointer to avoid dangling memory references
		kfree(curthread->pt_fd[index]);
		curthread->pt_fd[index] = NULL;
		return result;
	}
	*retval = index;
	return(0);
}

int 
sys_dup2(int oldfd, int newfd,int *retval) {
   	int result = 0; 

	if (oldfd > MAX_FD_SIZE || newfd > MAX_FD_SIZE || newfd < 0 ||oldfd  < 0 || (curthread->pt_fd[oldfd] == 0) ) {
   	*retval = EBADF;
   	return (1);
    }

    if(curthread->pt_fd[newfd] != 0) {
	    int ret;
	    result = sys_close(newfd,&ret);
	    if(result) {
	   	*retval = result;
	       	 return (1);	
	    }
    } else {
    	curthread->pt_fd[newfd] = (struct file_table *)kmalloc(sizeof(struct file_table));
    }
    curthread->pt_fd[newfd]->pv_node = 0;
    struct vnode *vn = 0;
    //vfs_open helps to increase the open_ref count
    result = vfs_open(curthread->pt_fd[oldfd]->file_name,O_WRONLY,0,&vn);
    if(result) {
	vfs_close(vn);		
	return 1;
     }
    curthread->pt_fd[newfd]->pv_node = vn;
    strcpy(curthread->pt_fd[newfd]->file_name,curthread->pt_fd[oldfd]->file_name);
    curthread->pt_fd[newfd]->offset = curthread->pt_fd[oldfd]->offset;
    *retval = newfd;
    return(0);
}

int 
sys_close(int fd,int *retval) {
	//(void)u_path;
	struct vnode *vn = NULL;
   	if (fd > MAX_FD_SIZE || (curthread->pt_fd[fd] == 0)) {
   	        *retval = EBADF;
   	        return (1);
   	}

	if(curthread->pt_fd[fd] != 0) 
	{
		vn = curthread->pt_fd[fd]->pv_node;
   		vfs_close(vn); 
		kfree(curthread->pt_fd[fd]);
		curthread->pt_fd[fd] = 0;
	}
	return(0);
}

int 
sys_read(int fd, void *buf, size_t buflen,int *retval) {
   //(void) fd;
   //(void) bufi;
   //(void) buflen;
   struct vnode *_vn = NULL;
   
   //check invalid fd
   if (fd > MAX_FD_SIZE || (curthread->pt_fd[fd] == 0)) {
	   *retval = EBADF;
	   return (1);
   }

   _vn = curthread->pt_fd[fd]->pv_node;
   struct uio _uio;
   struct iovec _iov;
   int result;
   void *kbuf = kmalloc(sizeof(*buf)*buflen);
   uio_kinit(&_iov,&_uio,kbuf,buflen,curthread->pt_fd[fd]->offset,UIO_READ);
   result = VOP_READ(_vn,&_uio); 	
   if(result) {
	   *retval = EIO;
	   return result;
   }
   result = copyout(kbuf,buf,buflen);
   if(result) {
	   *retval = EFAULT;
	   return result;
   }
   *retval = buflen;
   return(0);
}

int
sys_write(int fd, void *buf, size_t buflen,int *retval) {
    //(void)fd;
    //(void)buf;
    //(void)buflen;
    struct uio _uio;
    struct iovec _iov;
    void *kbuf = 0;
    int result = 0;
    struct vnode *vn = 0;

    //Initialize STDIN/OUT/ERROR
    if(0 == curthread->pt_fd[0])  {
		result = init_file_system();	
    }

    //check invalid fd
    if (fd > MAX_FD_SIZE || (curthread->pt_fd[fd] == 0)) {
            *retval = EBADF;
            return (1);
    }
    //Allocate the structure
    kbuf = kmalloc(sizeof(*buf) * buflen);

    if(kbuf == 0) {
	*retval = ENOMEM;
   	return (1); 
    }
    //copy in the data from the user space
    result = copyin((const_userptr_t)buf,kbuf,buflen);

    //no append functiality
    uio_kinit(&_iov,&_uio,kbuf,buflen,curthread->pt_fd[fd]->offset,UIO_WRITE);
    
    vn = curthread->pt_fd[fd]->pv_node;
   
    result = VOP_WRITE(vn,&_uio);
    
    kfree(kbuf);
    kbuf = NULL;

    if(result) {
	*retval = result;
   	return (1); 
    } 
   
    //update offset
    curthread->pt_fd[fd]->offset += buflen;
    *retval = buflen;
    return(0);
}

int 
sys_lseek(int fd, off_t pos,int code,off_t *retval) {
    //(void)fd;
    //(void)pos;
    //(void)whence;
    struct vnode *vn = 0;
    struct stat st;
    int result = 0;
    off_t file_size = 0;
    if(curthread->pt_fd[fd] == 0) {
   	*retval =  EBADF;
	return (1);
    }
    vn = curthread->pt_fd[fd]->pv_node;
    //get the size of  
    VOP_STAT(vn,&st);
    file_size  = st.st_size;
    if(pos < 0) {
	 *retval = EINVAL;
   	 return	(1);
    }
    switch(code) {
	    case SEEK_CUR:
		 curthread->pt_fd[fd]->offset += pos;
		break;
	    case SEEK_SET:
		 curthread->pt_fd[fd]->offset = pos;
		break;
	    case SEEK_END:
		 curthread->pt_fd[fd]->offset = file_size + pos;
		break;
    }
    result = VOP_TRYSEEK(vn,curthread->pt_fd[fd]->offset);
    if(result) {
	    return (result);
    }
    *retval = curthread->pt_fd[fd]->offset;
    return(0);
}

int 
sys_chdir(char *u_path,int *retval) {
	//(void)u_path;
	int result;
	char dest[NAME_MAX + 1];
	size_t actual_len;
	//Use copyinstr to copy from user space string
	//else you will get address out of bound error
	if (copyinstr((const_userptr_t)u_path,dest,NAME_MAX, &actual_len) != 0) {
		kprintf("not able to copy the string from the user space\n");
		return 1;
	}
	//To avoid garbage values
	dest[actual_len] = 0;
	
	result = vfs_chdir(dest);

	if(result) {
		*retval = ENOENT ;
		return (1);
	}
	return(0);
}


int 
sys__getcwd(char *buf,size_t size,int *retval) {
     //call vfs routine
     //(void)buf; : user data structure
     //(void)buflen;
     // To initialize kernel buffer we have to use uio and iovec
     struct uio uio;
     struct iovec iov;
     int result;
     //Initialize iovec and uio
     uio_kinit(&iov, &uio, buf, size-1, 0, UIO_READ);
     result = vfs_getcwd(&uio);
     if(result) {
	*retval = result;
	return (1);
     }
     buf[sizeof(buf)-1 - uio.uio_resid] = 0;
     *retval = strlen(buf);
     return(0);
}
