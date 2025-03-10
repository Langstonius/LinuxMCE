/*
 * pthread_cond_signal.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2002 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@ise.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * -------------------------------------------------------------
 * Algorithm:
 * The algorithm used in this implementation is that developed by
 * Alexander Terekhov in colaboration with Louis Thomas. The bulk
 * of the discussion is recorded in the file README.CV, which contains
 * several generations of both colaborators original algorithms. The final
 * algorithm used here is the one referred to as
 *
 *     Algorithm 8a / IMPL_SEM,UNBLOCK_STRATEGY == UNBLOCK_ALL
 * 
 * presented below in pseudo-code as it appeared:
 *
 *
 * given:
 * semBlockLock - bin.semaphore
 * semBlockQueue - semaphore
 * mtxExternal - mutex or CS
 * mtxUnblockLock - mutex or CS
 * nWaitersGone - int
 * nWaitersBlocked - int
 * nWaitersToUnblock - int
 * 
 * wait( timeout ) {
 * 
 *   [auto: register int result          ]     // error checking omitted
 *   [auto: register int nSignalsWasLeft ]
 *   [auto: register int nWaitersWasGone ]
 * 
 *   sem_wait( semBlockLock );
 *   nWaitersBlocked++;
 *   sem_post( semBlockLock );
 * 
 *   unlock( mtxExternal );
 *   bTimedOut = sem_wait( semBlockQueue,timeout );
 * 
 *   lock( mtxUnblockLock );
 *   if ( 0 != (nSignalsWasLeft = nWaitersToUnblock) ) {
 *     if ( bTimeout ) {                       // timeout (or canceled)
 *       if ( 0 != nWaitersBlocked ) {
 *         nWaitersBlocked--;
 *       }
 *       else {
 *         nWaitersGone++;                     // count spurious wakeups.
 *       }
 *     }
 *     if ( 0 == --nWaitersToUnblock ) {
 *       if ( 0 != nWaitersBlocked ) {
 *         sem_post( semBlockLock );           // open the gate.
 *         nSignalsWasLeft = 0;                // do not open the gate
 *                                             // below again.
 *       }
 *       else if ( 0 != (nWaitersWasGone = nWaitersGone) ) {
 *         nWaitersGone = 0;
 *       }
 *     }
 *   }
 *   else if ( INT_MAX/2 == ++nWaitersGone ) { // timeout/canceled or
 *                                             // spurious semaphore :-)
 *     sem_wait( semBlockLock );
 *     nWaitersBlocked -= nWaitersGone;     // something is going on here
 *                                          //  - test of timeouts? :-)
 *     sem_post( semBlockLock );
 *     nWaitersGone = 0;
 *   }
 *   unlock( mtxUnblockLock );
 * 
 *   if ( 1 == nSignalsWasLeft ) {
 *     if ( 0 != nWaitersWasGone ) {
 *       // sem_adjust( semBlockQueue,-nWaitersWasGone );
 *       while ( nWaitersWasGone-- ) {
 *         sem_wait( semBlockQueue );       // better now than spurious later
 *       }
 *     } sem_post( semBlockLock );          // open the gate
 *   }
 * 
 *   lock( mtxExternal );
 * 
 *   return ( bTimedOut ) ? ETIMEOUT : 0;
 * }
 * 
 * signal(bAll) {
 * 
 *   [auto: register int result         ]
 *   [auto: register int nSignalsToIssue]
 * 
 *   lock( mtxUnblockLock );
 * 
 *   if ( 0 != nWaitersToUnblock ) {        // the gate is closed!!!
 *     if ( 0 == nWaitersBlocked ) {        // NO-OP
 *       return unlock( mtxUnblockLock );
 *     }
 *     if (bAll) {
 *       nWaitersToUnblock += nSignalsToIssue=nWaitersBlocked;
 *       nWaitersBlocked = 0;
 *     }
 *     else {
 *       nSignalsToIssue = 1;
 *       nWaitersToUnblock++;
 *       nWaitersBlocked--;
 *     }
 *   }
 *   else if ( nWaitersBlocked > nWaitersGone ) { // HARMLESS RACE CONDITION!
 *     sem_wait( semBlockLock );                  // close the gate
 *     if ( 0 != nWaitersGone ) {
 *       nWaitersBlocked -= nWaitersGone;
 *       nWaitersGone = 0;
 *     }
 *     if (bAll) {
 *       nSignalsToIssue = nWaitersToUnblock = nWaitersBlocked;
 *       nWaitersBlocked = 0;
 *     }
 *     else {
 *       nSignalsToIssue = nWaitersToUnblock = 1;
 *       nWaitersBlocked--;
 *     }
 *   }
 *   else { // NO-OP
 *     return unlock( mtxUnblockLock );
 *   }
 * 
 *   unlock( mtxUnblockLock );
 *   sem_post( semBlockQueue,nSignalsToIssue );
 *   return result;
 * }
 * -------------------------------------------------------------
 *
 */

#include "pthread.h"
#include "implement.h"


static INLINE int
ptw32_cond_unblock (pthread_cond_t * cond,
                    int unblockAll)
     /*
      * Notes.
      *
      * Does not use the external mutex for synchronisation,
      * therefore semBlockLock is needed.
      * mtxUnblockLock is for LEVEL-2 synch. LEVEL-2 is the
      * state where the external mutex is not necessarily locked by
      * any thread, ie. between cond_wait unlocking and re-acquiring
      * the lock after having been signaled or a timeout or
      * cancellation.
      *
      * Uses the following CV elements:
      *   nWaitersBlocked
      *   nWaitersToUnblock
      *   nWaitersGone
      *   mtxUnblockLock
      *   semBlockLock
      *   semBlockQueue
      */
{
  int result;
  pthread_cond_t cv;
  int nSignalsToIssue;

  if (cond == NULL || *cond == NULL)
    {
      return EINVAL;
    }

  cv = *cond;

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   * Assuming that any race condition is harmless.
   */
  if (cv == PTHREAD_COND_INITIALIZER)
    {
      return 0;
    }

  if ((result = pthread_mutex_lock(&(cv->mtxUnblockLock))) != 0)
    {
      return result;
    }

  if ( 0 != cv->nWaitersToUnblock )
    {
      if ( 0 == cv->nWaitersBlocked )
        {
          return pthread_mutex_unlock( &(cv->mtxUnblockLock) );
        }
      if (unblockAll)
        {
          cv->nWaitersToUnblock += (nSignalsToIssue = cv->nWaitersBlocked);
          cv->nWaitersBlocked = 0;
        }
      else
        {
          nSignalsToIssue = 1;
          cv->nWaitersToUnblock++;
          cv->nWaitersBlocked--;
        }
    }
  else if ( cv->nWaitersBlocked > cv->nWaitersGone ) 
    {
      if (sem_wait( &(cv->semBlockLock) ) != 0)
        {
          result = errno;
          (void) pthread_mutex_unlock( &(cv->mtxUnblockLock) );
          return result;
        }
      if ( 0 != cv->nWaitersGone )
        {
          cv->nWaitersBlocked -= cv->nWaitersGone;
          cv->nWaitersGone = 0;
        }
      if (unblockAll)
        {
          nSignalsToIssue = cv->nWaitersToUnblock = cv->nWaitersBlocked;
          cv->nWaitersBlocked = 0;
        }
      else
        {
          nSignalsToIssue = cv->nWaitersToUnblock = 1;
          cv->nWaitersBlocked--;
        }
    }
  else
    {
      return pthread_mutex_unlock( &(cv->mtxUnblockLock) );
    }

  if ((result = pthread_mutex_unlock( &(cv->mtxUnblockLock) )) == 0)
    {
      if (sem_post_multiple( &(cv->semBlockQueue), nSignalsToIssue ) != 0)
        {
          result = errno;
        }
    }

  return result;

}                               /* ptw32_cond_unblock */

int
pthread_cond_signal (pthread_cond_t * cond)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function signals a condition variable, waking
      *      one waiting thread.
      *      If SCHED_FIFO or SCHED_RR policy threads are waiting
      *      the highest priority waiter is awakened; otherwise,
      *      an unspecified waiter is awakened.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function signals a condition variable, waking
      *      one waiting thread.
      *      If SCHED_FIFO or SCHED_RR policy threads are waiting
      *      the highest priority waiter is awakened; otherwise,
      *      an unspecified waiter is awakened.
      *
      *      NOTES:
      *
      *      1)      Use when any waiter can respond and only one need
      *              respond (all waiters being equal).
      *
      * RESULTS
      *              0               successfully signaled condition,
      *              EINVAL          'cond' is invalid,
      *
      * ------------------------------------------------------
      */
{
  /*
   * The '0'(FALSE) unblockAll arg means unblock ONE waiter.
   */
  return (ptw32_cond_unblock(cond, 0));

}                               /* pthread_cond_signal */

int
pthread_cond_broadcast (pthread_cond_t * cond)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function broadcasts the condition variable,
      *      waking all current waiters.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function signals a condition variable, waking
      *      all waiting threads.
      *
      *      NOTES:
      *
      *      1)      Use when more than one waiter may respond to
      *              predicate change or if any waiting thread may
      *              not be able to respond
      *
      * RESULTS
      *              0               successfully signalled condition to all
      *                              waiting threads,
      *              EINVAL          'cond' is invalid
      *              ENOSPC          a required resource has been exhausted,
      *
      * ------------------------------------------------------
      */
{
  /*
   * The '1'(TRUE) unblockAll arg means unblock ALL waiters.
   */
  return (ptw32_cond_unblock(cond, 1));

}                               /* pthread_cond_broadcast */
