/*
 *
 *  COPYRIGHT (c) 1989, 1990, 1991, 1992, 1993, 1994.
 *  On-Line Applications Research Corporation (OAR).
 *  All rights assigned to U.S. Government, 1994.
 *
 *  This material may be reproduced by or for the U.S. Government pursuant
 *  to the copyright license under the clause at DFARS 252.227-7013.  This
 *  notice must appear in all copies of this file and its derivatives.
 *
 *  $Id$
 */

#include "system.h"
#undef EXTERN
#define EXTERN
#include "conftbl.h"
#include "gvar.h"

rtems_id         Task_id[OPERATION_COUNT+1];
rtems_unsigned32 Task_index;

rtems_task High_task(
  rtems_task_argument argument
);

rtems_task Middle_tasks(
  rtems_task_argument argument
);

rtems_task Low_task(
  rtems_task_argument argument
);

void test_init();

rtems_task Init(
  rtems_task_argument argument
)
{
  rtems_status_code status;

  puts( "\n\n*** TIME TEST 5 ***" );

  test_init();

  status = rtems_task_delete( RTEMS_SELF );
  directive_failed( status, "rtems_task_delete of RTEMS_SELF" );
}

void test_init()
{
  rtems_status_code   status;
  rtems_task_entry    task_entry;
  rtems_task_priority priority;
  rtems_unsigned32    index;

  priority = 250;

  for( index = 0; index <= OPERATION_COUNT ; index++ ) {

    status = rtems_task_create(
      rtems_build_name( 'T', 'I', 'M', 'E' ),
      priority,
      1024,
      RTEMS_DEFAULT_MODES,
      RTEMS_DEFAULT_ATTRIBUTES,
      &Task_id[ index ]
    );
    directive_failed( status, "rtems_task_create loop" );

    priority--;

    if ( index==0 )                    task_entry = Low_task;
    else if ( index==OPERATION_COUNT ) task_entry = High_task;
    else                               task_entry = Middle_tasks;

    status = rtems_task_start( Task_id[ index ], task_entry, 0 );
    directive_failed( status, "rtems_task_start loop" );
  }
}

rtems_task High_task(
  rtems_task_argument argument
)
{
  Timer_initialize();

  (void) rtems_task_suspend( RTEMS_SELF );

  end_time = Read_timer();

  put_time(
    "rtems_task_resume (causing preempt)",
    end_time,
    OPERATION_COUNT,
    0,
    CALLING_OVERHEAD_TASK_RESUME
  );

  exit( 0 );
}

rtems_task Middle_tasks(
  rtems_task_argument argument
)
{
  (void) rtems_task_suspend( RTEMS_SELF );

  Task_index++;
  (void) rtems_task_resume( Task_id[ Task_index ] );
}

rtems_task Low_task(
  rtems_task_argument argument
)
{

  end_time = Read_timer();

  put_time(
    "rtems_task_suspend self",
    end_time,
    OPERATION_COUNT,
    0,
    CALLING_OVERHEAD_TASK_SUSPEND
  );

  Task_index = 1;
  Timer_initialize();
  (void) rtems_task_resume( Task_id[ Task_index ] );
}
