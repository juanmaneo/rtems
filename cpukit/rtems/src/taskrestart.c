/**
 * @file
 *
 * @ingroup RTEMSImplClassicTask
 *
 * @brief This source file contains the implementation of
 *   rtems_task_restart().
 */

/*
 *  COPYRIGHT (c) 1989-2014.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/rtems/tasks.h>
#include <rtems/rtems/statusimpl.h>
#include <rtems/score/threadimpl.h>

rtems_status_code rtems_task_restart(
  rtems_id            id,
  rtems_task_argument argument
)
{
  Thread_Control           *the_thread;
  ISR_lock_Context          lock_context;
  Thread_Entry_information  entry;
  Status_Control            status;

  the_thread = _Thread_Get( id, &lock_context );

  if ( the_thread == NULL ) {
#if defined(RTEMS_MULTIPROCESSING)
    if ( _Thread_MP_Is_remote( id ) ) {
      return RTEMS_ILLEGAL_ON_REMOTE_OBJECT;
    }
#endif

    return RTEMS_INVALID_ID;
  }

  entry = the_thread->Start.Entry;
  entry.Kinds.Numeric.argument = argument;
  status = _Thread_Restart( the_thread, &entry, &lock_context );

  return _Status_Get( status );
}
