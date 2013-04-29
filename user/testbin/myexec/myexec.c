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

/*
 *calls to execv()
 */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

static
int
exec_common_fork(void)
{
	int pid, rv, status;

	pid = fork();
	if (pid<0) {
		warn("UH-OH: fork failed");
		return -1;
	}
	
	if (pid==0) {
		/* child */
		return 0;
	}

	rv = waitpid(pid, &status, 0);
	if (rv == -1) {
		warn("UH-OH: waitpid failed");
		return -1;
	}
	//if (!WIFEXITED(status) || WEXITSTATUS(status) != 107) {
	//	warnx("FAILURE: wrong exit code of subprocess");
	//}
	return 1;
}
static
void
exec_goodargs (char **args) {

	int rv;
	(void) args;	
	//(void) desc;
	if (exec_common_fork() != 0) {
		return;
	}
	//char t[3];
	//strcpy(t,"12");
	rv = execv("/testbin/add_test", NULL);
	//rv = execv("/bin/true", args);
	exit(107);

}

static
void
test_execv(void)
{
	char **argv = 0 ;
	//= (char**) malloc (sizeof(char));
	//argv[0] = (char*)malloc(sizeof(char)*4);
	//argv[1] = (char*)malloc(sizeof(char)*4);
	exec_goodargs(argv );
}

int 
main() {
	test_execv();	
	return (0);
}
