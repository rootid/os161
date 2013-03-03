/*
 * Copyright (c) 2001, 2002, 2009
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
 * Driver code for whale mating problem
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * 08 Feb 2012 : GWA : Driver code is in kern/synchprobs/driver.c. We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

// 13 Feb 2012 : GWA : Adding at the suggestion of Isaac Elbaz. These
// functions will allow you to do local initialization. They are called at
// the top of the corresponding driver code.

struct semaphore *mSemaphore;
struct semaphore *fSemaphore;
struct semaphore *kSemaphore;
struct semaphore *mcSemaphore;
struct semaphore *fcSemaphore;
struct semaphore *kcSemaphore;
struct semaphore *ecSemaphore;
int male_cnt;
int female_cnt;
int matchmaker_cnt;

void whalemating_init() {
  //create the semaphore
  mSemaphore = sem_create("Male semaphore",1);
  fSemaphore = sem_create("Female semaphore",1);
  kSemaphore = sem_create("MM semaphore",1);
  mcSemaphore = sem_create("Male enter semaphore",0);
  fcSemaphore = sem_create("FeMale enter semaphore",0);
  kcSemaphore = sem_create("MM enter semaphore",0);
  ecSemaphore = sem_create("End semaphore",1);
  male_cnt = 0;
  female_cnt = 0;
  matchmaker_cnt = 0;
  return;
}

// 20 Feb 2012 : GWA : Adding at the suggestion of Nikhil Londhe. We don't
// care if your problems leak memory, but if you do, use this to clean up.

void whalemating_cleanup() {
	
//	kfree(mSemaphore);
//	kfree(fSemaphore); 
//	kfree(kSemaphore);
//	kfree(mcSemaphore); 
//	kfree(fcSemaphore);
//	kfree(kcSemaphore);
//	kfree(ecSemaphore);  
//
//	mSemaphore  = NULL;
//	fSemaphore  = NULL; 
//	kSemaphore  = NULL;
//	mcSemaphore = NULL;
//	fcSemaphore = NULL;
//	kcSemaphore = NULL;
//	ecSemaphore = NULL;

	return;
}

void
male(void *p, unsigned long which)
{
  struct semaphore * whalematingMenuSemaphore = (struct semaphore *)p;
  (void)which;	
 
  P(mSemaphore);
  
  //provide atomic operation 
  P(ecSemaphore);
  male_start();
  V(ecSemaphore);
  
  V(whalematingMenuSemaphore);

  //wait for female and matchmaker to start
  V(mcSemaphore); 
  V(mcSemaphore); 
  P(fcSemaphore);
  P(kcSemaphore);
  
  //provide atomic operation 
  P(ecSemaphore);
  male_cnt ++;
  male_end();
  V(ecSemaphore);
 
  //wait for female and matchmaker to end
  V(mcSemaphore);
  V(mcSemaphore);
  P(fcSemaphore);
  P(kcSemaphore);
 
 //if(male_cnt < 9) 
  V(mSemaphore); 

  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // whalemating driver can return to the menu cleanly.
  return;
}

void
female(void *p, unsigned long which)
{
	struct semaphore * whalematingMenuSemaphore = (struct semaphore *)p;
  (void)which;
  
  P(fSemaphore);
  
  P(ecSemaphore);
  female_start();
  V(ecSemaphore);
  
  V(whalematingMenuSemaphore);

  V(fcSemaphore); 
  V(fcSemaphore); 
  P(mcSemaphore);
  P(kcSemaphore);
  
  P(ecSemaphore);
  female_cnt++;
  female_end();
  V(ecSemaphore);

  V(fcSemaphore); 
  V(fcSemaphore); 
  P(mcSemaphore);
  P(kcSemaphore);
 
  if(female_cnt <= 9)
  V(fSemaphore);

  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // whalemating driver can return to the menu cleanly.
  return;
}

void
matchmaker(void *p, unsigned long which)
{
	struct semaphore * whalematingMenuSemaphore = (struct semaphore *)p;
  (void)which;
  
  P(kSemaphore);

  P(ecSemaphore);
  matchmaker_start();
  V(ecSemaphore);

  V(whalematingMenuSemaphore);

  V(kcSemaphore); 
  V(kcSemaphore); 
  P(mcSemaphore);
  P(fcSemaphore);
  
  P(ecSemaphore);
  matchmaker_cnt ++;
  matchmaker_end();
  V(ecSemaphore);
  
  V(kcSemaphore); 
  V(kcSemaphore); 
  P(mcSemaphore);
  P(fcSemaphore);

  if(matchmaker_cnt <= 9)
  V(kSemaphore); 
  
  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // whalemating driver can return to the menu cleanly.
  return;
}

/*
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is,
 * of course, stable under rotation)
 *
 *   | 0 |
 * --     --
 *    0 1
 * 3       1
 *    3 2
 * --     --
 *   | 2 | 
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X
 * first.
 *
 * You will probably want to write some helper functions to assist
 * with the mappings. Modular arithmetic can help, e.g. a car passing
 * straight through the intersection entering from direction X will leave to
 * direction (X + 2) % 4 and pass through quadrants X and (X + 3) % 4.
 * Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in drivers.c.
 */

// 13 Feb 2012 : GWA : Adding at the suggestion of Isaac Elbaz. These
// functions will allow you to do local initialization. They are called at
// the top of the corresponding driver code.

struct semaphore *zeroSemaphore;
struct semaphore *oneSemaphore;
struct semaphore *twoSemaphore;
struct semaphore *threeSemaphore;


//helper functions

int 
get_self(int dir) {
	//As modular is costlier operation,just return direction
	return dir;
}

int 
get_self_front(int dir) {
	return (dir + 3) % 4;
}
int 
get_self_left(int dir) {
	return (dir + 1) % 4;
}
int 
get_self_cross(int dir) {
	return (dir + 2) % 4;
}

struct semaphore *
get_dir_semaphore(int dir) {

	if(0 ==  dir) {
	  return  zeroSemaphore;  
	}
	else if(1 ==  dir) {
	  return oneSemaphore;  
	}
	else if(2 ==  dir) {
	  return  twoSemaphore;  
	}
	else if(3 ==  dir) {
	  return  threeSemaphore;  
	}
	return NULL;

}

struct lock *lk_strt;
struct lock *lk_lt;
struct lock *lk_rt;

void stoplight_init() {
  zeroSemaphore  = sem_create("zero",1);
  oneSemaphore   = sem_create("one",1);
  twoSemaphore   = sem_create("two",1);
  threeSemaphore = sem_create("three",1);
  lk_strt = lock_create("straight");
  lk_lt = lock_create("left");
  lk_rt = lock_create("right");
  return;
}

// 20 Feb 2012 : GWA : Adding at the suggestion of Nikhil Londhe. We don't
// care if your problems leak memory, but if you do, use this to clean up.

void stoplight_cleanup() {
  return;
}

void
gostraight(void *p, unsigned long direction)
{
  struct semaphore * stoplightMenuSemaphore = (struct semaphore *)p;
  
  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // stoplight driver can return to the menu cleanly.
  
  // To go straight follow self,self_front
  lock_acquire(lk_strt);
  V(stoplightMenuSemaphore);
  int iDir = (int)direction;
  int x = get_self(iDir);	
  int y = get_self_front(iDir);
  struct semaphore * x_sem = get_dir_semaphore(x);
  struct semaphore * y_sem = get_dir_semaphore(y);
  P(x_sem);
  inQuadrant(x);
  P(y_sem);
  inQuadrant(y);
  V(x_sem);
  leaveIntersection();
  V(y_sem);
  lock_release(lk_strt);
  return;
}

void
turnleft(void *p, unsigned long direction)
{
  struct semaphore * stoplightMenuSemaphore = (struct semaphore *)p;
  
  
  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // stoplight driver can return to the menu cleanly.
  
  // To go left follow self,self_front,self_cross
  lock_acquire(lk_lt);
  V(stoplightMenuSemaphore);
  int iDir = (int)direction;
  int x = get_self(iDir);	
  int y = get_self_front(iDir);
  int z = get_self_cross(iDir);
  struct semaphore * x_sem = get_dir_semaphore(x);
  struct semaphore * y_sem = get_dir_semaphore(y);
  struct semaphore * z_sem = get_dir_semaphore(z);
  P(x_sem);
  inQuadrant(x);
  P(y_sem);
  inQuadrant(y);
  V(x_sem);
  P(z_sem);
  inQuadrant(z);
  V(y_sem);
  leaveIntersection();
  V(z_sem);
  lock_release(lk_lt);
  return;
}

void
turnright(void *p, unsigned long direction)
{
  struct semaphore * stoplightMenuSemaphore = (struct semaphore *)p;
  
  // 08 Feb 2012 : GWA : Please do not change this code. This is so that your
  // stoplight driver can return to the menu cleanly.
  
  // To go right follow self
  lock_acquire(lk_rt);
  V(stoplightMenuSemaphore);
  int iDir = (int)direction;
  int x = get_self(iDir);	
  struct semaphore * x_sem = get_dir_semaphore(x);
  P(x_sem);
  inQuadrant(x);
  leaveIntersection();
  V(x_sem);
  lock_release(lk_rt);
  return;
}
