/*=======================================================================

  Program:   NDI Combined API C Interface Library
  Module:    $RCSfile: ndicapi_thread.c,v $
  Creator:   David Gobbi <dgobbi@atamai.com>
  Language:  C
  Author:    $Author: dgobbi $
  Date:      $Date: 2002/11/04 02:09:39 $
  Version:   $Revision: 1.1 $

==========================================================================
Copyright 2000,2001 Atamai, Inc.

Redistribution of this source code and/or any binary applications created
using this source code is prohibited without the expressed, written
permission of the copyright holders.  

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=======================================================================*/

#include "ndicapi_thread.h"
#include <stdlib.h>

/* The interface is modelled after the Windows threading interface,
   but the only real difference from POSIX threads is the "Event"
   type which does not exists in POSIX threads (more information is
   provided below) */

#if defined(WIN32) || defined(_WIN32)

HANDLE ndiMutexCreate()
{
  return CreateMutex(0,FALSE,0);
}

void ndiMutexDestroy(HANDLE mutex)
{
  CloseHandle(mutex);
}

void ndiMutexLock(HANDLE mutex)
{
  WaitForSingleObject(mutex,INFINITE);
}

void ndiMutexUnlock(HANDLE mutex)
{
  ReleaseMutex(mutex);
}

#elif defined(unix) || defined(__unix__)

pthread_mutex_t *ndiMutexCreate()
{
  pthread_mutex_t *mutex;
  mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex,0);
  return mutex;
}

void ndiMutexDestroy(pthread_mutex_t *mutex)
{
  pthread_mutex_destroy(mutex);
  free(mutex);
}

void ndiMutexLock(pthread_mutex_t *mutex)
{
  pthread_mutex_lock(mutex);
}

void ndiMutexUnlock(pthread_mutex_t *mutex)
{
  pthread_mutex_unlock(mutex);
}

#endif

#if defined(WIN32) || defined(_WIN32)

HANDLE ndiEventCreate()
{
  return CreateEvent(0,FALSE,FALSE,0);
}

void ndiEventDestroy(HANDLE event)
{
  CloseHandle(event);
}

void ndiEventSignal(HANDLE event)
{
  SetEvent(event);
}

int ndiEventWait(HANDLE event, int milliseconds)
{
  if (milliseconds < 0) {
    WaitForSingleObject(event, INFINITE);
  }
  else {
    if (WaitForSingleObject(event, milliseconds) == WAIT_TIMEOUT) {
      return 1;
    }
  }
  return 0;
}

#elif defined(unix) || defined(__unix__)

/* There is no equivalent of an 'event' in POSIX threads, so we define
   our own event type consisting of a boolean variable (to say whether
   an event has occurred), a cond, an a mutex for locking the cond
   and the variable.
*/

pl_cond_and_mutex_t *ndiEventCreate()
{
  pl_cond_and_mutex_t *event;
  event = (pl_cond_and_mutex_t *)malloc(sizeof(pl_cond_and_mutex_t));
  event->signalled = 0;
  pthread_cond_init(&event->cond, 0);
  pthread_mutex_init(&event->mutex, 0);
  return event;
}

void ndiEventDestroy(pl_cond_and_mutex_t *event)
{
  pthread_cond_destroy(&event->cond);
  pthread_mutex_destroy(&event->mutex);
  free(event);
}

/* Setting the event is simple: lock, set the variable, signal the cond,
   and unlock.
*/

void ndiEventSignal(pl_cond_and_mutex_t *event)
{
  pthread_mutex_lock(&event->mutex);
  event->signalled = 1;
  pthread_cond_signal(&event->cond);
  pthread_mutex_unlock(&event->mutex);
}

/* Waiting for the event is simple if we don't want a timed wait:
   lock, check the variable, wait until the variable becomes set,
   unset the variable, unlock.
   Note that the event can be received by only one thread.
   
   If a timed wait is needed, then there is a little bit of
   hassle because the pthread_cond_timedwait() wait is until
   an absolute time, so we must get the current time and then
   do a little math as well as a conversion from one time structure
   to another time structure.
*/

int ndiEventWait(pl_cond_and_mutex_t *event, int milliseconds)
{
  int timedout = 0;

  if (milliseconds < 0) { /* do infinite wait */
    pthread_mutex_lock(&event->mutex);
    if (event->signalled == 0) {
      pthread_cond_wait(&event->cond, &event->mutex);
    }
    event->signalled = 0;
    pthread_mutex_unlock(&event->mutex);
  }
  else { /* do timed wait */
    struct timeval tv;
    struct timespec ts;

    pthread_mutex_lock(&event->mutex);
    if (event->signalled == 0) {
      /* all the time stuff is used to check for timeouts */
      gettimeofday(&tv, 0);
      tv.tv_sec += milliseconds/1000; /* msec to sec */ 
      tv.tv_usec += (milliseconds % 1000)*1000; /* msec to usec */
      if (tv.tv_usec >= 1000000) { /* if usec overflow */
        tv.tv_usec -= 1000000;
        tv.tv_sec += 1;
      }
      /* convert timeval to timespec */
      ts.tv_sec = tv.tv_sec;
      ts.tv_nsec = tv.tv_usec * 1000; 

#ifdef PTHREAD_COND_TIMEDWAIT_USES_TIMEVAL
      timedout = (pthread_cond_timedwait(&event->cond, &event->mutex,
                                         &tv) == ETIMEDOUT);
#else 
      timedout = (pthread_cond_timedwait(&event->cond, &event->mutex,
                                         &ts) == ETIMEDOUT);
#endif
    }
    if (!timedout) {
      event->signalled = 0;
    }
    pthread_mutex_unlock(&event->mutex);
  }

  return timedout;
}

#endif

#if defined(WIN32) || defined(_WIN32)

HANDLE ndiThreadSplit(void *thread_func(void *userdata), void *userdata)
{
  DWORD thread_id; /* we throw the thread id away */
  return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&thread_func,
                      userdata, 0, &thread_id);
}

void ndiThreadJoin(HANDLE thread)
{
  WaitForSingleObject(thread, INFINITE);
}

#elif defined(unix) || defined(__unix__)

pthread_t ndiThreadSplit(void *thread_func(void *userdata), void *userdata)
{
  pthread_t thread;
  if (pthread_create(&thread, 0, thread_func, userdata)) {
    return 0;
  }
  return thread;
}

void ndiThreadJoin(pthread_t thread)
{
  pthread_join(thread, 0);
}

#endif