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
 * Synchronization primitives.
 * The specifications of the functions are in synch.h.
 */

#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *name, int initial_count)
{
        struct semaphore *sem;

        KASSERT(initial_count >= 0);

        sem = kmalloc(sizeof(struct semaphore));
        if (sem == NULL) {
                return NULL;
        }

        sem->sem_name = kstrdup(name);
        if (sem->sem_name == NULL) {
                kfree(sem);
                return NULL;
        }

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
        sem->sem_count = initial_count;

        return sem;
}

void
sem_destroy(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
        kfree(sem->sem_name);
        kfree(sem);
}

void 
P(struct semaphore *sem)
{
        KASSERT(sem != NULL);

        /*
         * May not block in an interrupt handler.
         *
         * For robustness, always check, even if we can actually
         * complete the P without blocking.
         */
        KASSERT(curthread->t_in_interrupt == false);

	spinlock_acquire(&sem->sem_lock);
	//Pandhari : Do remember it is while loop not if loop
        while (sem->sem_count == 0) {
		/*
		 * Bridge to the wchan lock, so if someone else comes
		 * along in V right this instant the wakeup can't go
		 * through on the wchan until we've finished going to
		 * sleep. Note that wchan_sleep unlocks the wchan.
		 *
		 * Note that we don't maintain strict FIFO ordering of
		 * threads going through the semaphore; that is, we
		 * might "get" it on the first try even if other
		 * threads are waiting. Apparently according to some
		 * textbooks semaphores must for some reason have
		 * strict ordering. Too bad. :-)
		 *
		 * Exercise: how would you implement strict FIFO
		 * ordering? To impose fifo ordering we might put spinlock_release at the end
		 */
		wchan_lock(sem->sem_wchan);
		spinlock_release(&sem->sem_lock);
                wchan_sleep(sem->sem_wchan);

		spinlock_acquire(&sem->sem_lock);
        }
        KASSERT(sem->sem_count > 0);
        sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

void
V(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

        sem->sem_count++;
        KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan);

	spinlock_release(&sem->sem_lock);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
        struct lock *lock;

        lock = kmalloc(sizeof(struct lock));
        if (lock == NULL) {
                return NULL;
        }

        lock->lk_name = kstrdup(name);
        if (lock->lk_name == NULL) {
                kfree(lock);
                return NULL;
        }
        
        // add stuff here as needed
	lock->lk_th_holder = NULL;
	//we can't initilize the holder as this routine might be created by some other thread eg.  0x80033fe0 "<boot #0>"
	spinlock_init(&lock->lk_th_lock);
	lock->lk_wchan = wchan_create(lock->lk_name);
	if (lock->lk_wchan == NULL) {
		kfree(lock->lk_name);
		kfree(lock);
		return NULL;
	}
        return lock;
}

void
lock_destroy(struct lock *lock)
{
        KASSERT(lock != NULL);

        // add stuff here as needed
        
	spinlock_cleanup(&lock->lk_th_lock);
	if(lock->lk_wchan != NULL)
		wchan_destroy(lock->lk_wchan);

        kfree(lock->lk_name);
        kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
        // Write this

       // (void)lock;  // suppress warning until code gets 	
	KASSERT(lock != NULL);	

	KASSERT(curthread->t_in_interrupt == false);

	spinlock_acquire(&lock->lk_th_lock);
	//Avoid self-recurrence lock
	KASSERT(lock->lk_th_holder != curthread);	
	//KASSERT(!lock_do_i_hold(lock));
	while(NULL  != lock->lk_th_holder)	{
		wchan_lock(lock->lk_wchan);
		spinlock_release(&lock->lk_th_lock);
                wchan_sleep(lock->lk_wchan);
		spinlock_acquire(&lock->lk_th_lock);
	}
	KASSERT(lock->lk_th_holder == NULL);	
	lock->lk_th_holder = curthread;
	spinlock_release(&lock->lk_th_lock);

}

void
lock_release(struct lock *lock)
{
        // Write this

        KASSERT(lock != NULL);
	spinlock_acquire(&lock->lk_th_lock);
	lock->lk_th_holder = NULL;
	//KASSERT(lock->lk_th_holder != NULL);
	wchan_wakeone(lock->lk_wchan);
	spinlock_release(&lock->lk_th_lock);

        //(void)lock;  // suppress warning until code gets written
}

bool
lock_do_i_hold(struct lock *lock)
{
        // Write this

	return (lock->lk_th_holder == curthread);
       // (void)lock;  // suppress warning until code gets written
       // return true; // dummy until code gets written
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
        struct cv *cv;

        cv = kmalloc(sizeof(struct cv));
        if (cv == NULL) {
                return NULL;
        }

        cv->cv_name = kstrdup(name);
        if (cv->cv_name == NULL) {
                kfree(cv);
                return NULL;
        }
        
        // add stuff here as needed
       
	cv->cv_wchan = wchan_create(cv->cv_name);
	if(cv->cv_wchan == NULL) {
		kfree(cv->cv_name);
		kfree(cv);
		return NULL;
	}
	spinlock_init(&cv->lk_cv);
        return cv;
}

void
cv_destroy(struct cv *cv)
{
        KASSERT(cv != NULL);

        // add stuff here as needed
        kfree(cv->cv_name);
	spinlock_cleanup(&cv->lk_cv);
	if(cv->cv_wchan != NULL)
		wchan_destroy(cv->cv_wchan);
        kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
        // Write this
       // (void)cv;    // suppress warning until code gets written
       // (void)lock;  // suppress warning until code gets written

	KASSERT(curthread->t_in_interrupt == false);
	//KASSERT(lock->lk_th_holder == curthread);
	lock_release(lock);
	//spinlock_acquire(&cv->lk_cv);
	wchan_lock(cv->cv_wchan);
	//spinlock_release(&cv->lk_cv);
	wchan_sleep(cv->cv_wchan);
	//spinlock_acquire(&cv->lk_cv);
	lock_acquire(lock);
	//spinlock_release(&cv->lk_cv);
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
        // Write this
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written

	KASSERT(lock != NULL);	
	KASSERT(cv != NULL);	
	if(lock_do_i_hold(lock)) {
		wchan_wakeone(cv->cv_wchan);		
	}
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
	KASSERT(lock != NULL);	
	KASSERT(cv != NULL);	
	
	if(lock_do_i_hold(lock)) {
		wchan_wakeall(cv->cv_wchan);		
	}
}


struct rwlock * 
rwlock_create(const char *name) {
	struct rwlock *rwlock;
	
	rwlock = kmalloc(sizeof(struct rwlock));
	if(rwlock == NULL) {
		return NULL;	
	}
	rwlock->rwlock_name = kstrdup(name);
	
	if(rwlock->rwlock_name == NULL) {
		kfree(rwlock);
		return NULL;
	}
	rwlock->sem_reader = sem_create("reader",0);
	rwlock->sem_read  = sem_create("reader unit",1);
	rwlock->sem_write = sem_create("writer unit",1);
	rwlock->sem_writer = sem_create("writer",1);

	if (rwlock->sem_reader == NULL || 
	    rwlock->sem_writer == NULL) {
		kfree(rwlock);
		return NULL;	
	}
	
	rwlock->reader_cnt = 0;
	rwlock->writer_cnt = 0;
	return rwlock;
}

void rwlock_destroy(struct rwlock *rwlock ) {
	
	KASSERT(rwlock != NULL);
	sem_destroy(rwlock->sem_reader);
	sem_destroy(rwlock->sem_writer);
	sem_destroy(rwlock->sem_read);
	sem_destroy(rwlock->sem_write);
	kfree(rwlock->rwlock_name);
	kfree(rwlock);
}

void 
rwlock_acquire_read(struct rwlock *rwlock ) {
	//(void) rwlock;
	//spinlock_acquire(&rwlock->lk_read_acq);
	P(rwlock->sem_read); //To provide atomicity
	if(rwlock->reader_cnt == 0) {
		P(rwlock->sem_writer);
		//If anybody is in the P and next thread try to use spinlock_acquire Deadlock exception occurs
	}
	if(rwlock->writer_cnt > 0) {
  		P(rwlock->sem_reader);	
	}
	rwlock->reader_cnt++;
	//spinlock_release(&rwlock->lk_read_acq);
	V(rwlock->sem_read);
}

void 
rwlock_release_read(struct rwlock *rwlock ) {
	//(void) rwlock;
	
	//spinlock_acquire(&rwlock->lk_read_rel);
	rwlock->reader_cnt--;	
	if(rwlock->reader_cnt == 0) {
		V(rwlock->sem_writer);
	}
	//spinlock_release(&rwlock->lk_read_rel);
}

void 
rwlock_acquire_write(struct rwlock *rwlock ) {
	//(void) rwlock;
	//spinlock_acquire(&rwlock->lk_write);	
	P(rwlock->sem_write);
	rwlock->writer_cnt ++;	
	P(rwlock->sem_writer);
	V(rwlock->sem_write);
	//spinlock_release(&rwlock->lk_write);	
}

void 
rwlock_release_write(struct rwlock *rwlock ) {
	//(void) rwlock;
	V(rwlock->sem_writer);
  	V(rwlock->sem_reader);	
}
