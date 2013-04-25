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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

/*
 * test - create a directory.
 * Tasks : To verify following functionality
 * 	   1.open :int open(const char *filename, int flags, ...);
 * 	   2.read :int read(int filehandle, void *buf, size_t size);
 * 	   3.write:int write(int filehandle, const void *buf, size_t size);
      
 * 	   4.lseek:off_t lseek(int filehandle, off_t pos, int code);
 * 	   5.close:int close(int filehandle);
 * 	   6.dup2 :int dup2(int filehandle, int newhandle);
 * 	   7.chdir:int chdir(const char *path);
 * 	   8.getcwd:int __getcwd(char *buf, size_t buflen);
 * Usage: test : any number of parameter
 *
 */

int
main(int argc, char *argv[])
{

	//int i =0;	
	char buf[64];
	(void) argc;
	(void) argv;
	//printf("\nall the passed parameters\n");	
	//for(i=0;i<argc;i++) {
	//	printf("\n %s",argv[i]);	
	//}
	int ret = __getcwd(buf,64);
	printf("\n %d",ret);	
	return 0;
}
