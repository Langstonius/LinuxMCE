/*
 * pthread_setcanceltype.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
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
 */

#include "pthread.h"
#include "implement.h"


int
pthread_setcanceltype (int type, int *oldtype)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      * PARAMETERS
      *      type,
      *      oldtype
      *              PTHREAD_CANCEL_DEFERRED
      *                      only deferred cancelation is allowed,
      *
      *              PTHREAD_CANCEL_ASYNCHRONOUS
      *                      Asynchronous cancellation is allowed
      *
      *
      * DESCRIPTION
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      *      NOTES:
      *      1)      Use with caution; most code is not safe for use
      *              with asynchronous cancelability.
      *
      * COMPATIBILITY ADDITIONS
      *      If 'oldtype' is NULL then the previous type is not returned
      *      but the function still succeeds. (Solaris)
      *
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'type' is invalid
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_t self = pthread_self();

  if (self == NULL
      || (type != PTHREAD_CANCEL_DEFERRED
          && type != PTHREAD_CANCEL_ASYNCHRONOUS))
    {
      return EINVAL;
    }

  /*
   * Lock for async-cancel safety.
   */
  (void) pthread_mutex_lock(&self->cancelLock);

  if (oldtype != NULL)
    {
      *oldtype = self->cancelType;
    }

  self->cancelType = type;

  /*
   * Check if there is a pending asynchronous cancel
   */
  if (self->cancelState == PTHREAD_CANCEL_ENABLE
      && type == PTHREAD_CANCEL_ASYNCHRONOUS
      && WaitForSingleObject(self->cancelEvent, 0) == WAIT_OBJECT_0)
    {
      self->state = PThreadStateCanceling;
      self->cancelState = PTHREAD_CANCEL_DISABLE;
      ResetEvent(self->cancelEvent);
      (void) pthread_mutex_unlock(&self->cancelLock);
      ptw32_throw(PTW32_EPS_CANCEL);

      /* Never reached */
    }

  (void) pthread_mutex_unlock(&self->cancelLock);

  return (result);

}                               /* pthread_setcanceltype */

