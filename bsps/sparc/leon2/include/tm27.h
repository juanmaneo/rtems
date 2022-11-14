/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 * @ingroup RTEMSBSPsSPARCLEON2
 * @brief Implementations for interrupt mechanisms for Time Test 27
 */

/*
 *  COPYRIGHT (c) 2006.
 *  Aeroflex Gaisler AB.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _RTEMS_TMTEST27
#error "This is an RTEMS internal file you must not include directly."
#endif

#ifndef __tm27_h
#define __tm27_h

/*
 *  Define the interrupt mechanism for Time Test 27
 *
 *  NOTE: Since the interrupt code for the SPARC supports both synchronous
 *        and asynchronous trap handlers, support for testing with both
 *        is included.
 */

#define SIS_USE_SYNCHRONOUS_TRAP  0

/*
 *  The synchronous trap is an arbitrarily chosen software trap.
 */

#if (SIS_USE_SYNCHRONOUS_TRAP == 1)

#define TEST_VECTOR SPARC_SYNCHRONOUS_TRAP( 0x90 )

#define MUST_WAIT_FOR_INTERRUPT 1

#define Install_tm27_vector( handler ) \
  set_vector( (handler), TEST_VECTOR, 1 );

#define Cause_tm27_intr() \
  __asm__ volatile( "ta 0x10; nop " );

#define Clear_tm27_intr() /* empty */

#define Lower_tm27_intr() /* empty */

/*
 *  The asynchronous trap is an arbitrarily chosen ERC32 interrupt source.
 */

#else   /* use a regular asynchronous trap */

#define TEST_INTERRUPT_SOURCE LEON_INTERRUPT_EXTERNAL_1
#define TEST_INTERRUPT_SOURCE2 LEON_INTERRUPT_EXTERNAL_1+1
#define MUST_WAIT_FOR_INTERRUPT 1

static inline void Install_tm27_vector(
  void ( *handler )( rtems_vector_number )
)
{
  (void) rtems_interrupt_handler_install(
    TEST_INTERRUPT_SOURCE,
    "tm27 low",
    RTEMS_INTERRUPT_SHARED,
    (rtems_interrupt_handler) handler,
    NULL
  );
  (void) rtems_interrupt_handler_install(
    TEST_INTERRUPT_SOURCE2,
    "tm27 high",
    RTEMS_INTERRUPT_SHARED,
    (rtems_interrupt_handler) handler,
    NULL
  );
}

#define Cause_tm27_intr() \
  do { \
    LEON_Force_interrupt( TEST_INTERRUPT_SOURCE+(Interrupt_nest>>1)); \
    nop(); \
    nop(); \
    nop(); \
  } while (0)

#define Clear_tm27_intr() \
  LEON_Clear_interrupt( TEST_INTERRUPT_SOURCE )

#define Lower_tm27_intr() /* empty */

#endif

#endif
