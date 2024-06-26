/* SPDX-License-Identifier: BSD-2-Clause */

/**
 * @file
 *
 * @ingroup RTEMSBSPsARMTMS570
 *
 * @brief This source file contains the MibSPI module parity based protection
 *   support.
 *
 * Algorithms are based on Ti manuals and Ti HalCoGen generated
 * code.
 */

/*
 * Copyright (C) 2016 Pavel Pisa <pisa@cmp.felk.cvut.cz>
 *
 * Czech Technical University in Prague
 * Zikova 1903/4
 * 166 36 Praha 6
 * Czech Republic
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

#include <stdint.h>
#include <stddef.h>
#include <bsp/tms570.h>
#include <bsp/tms570_selftest.h>
#include <bsp/tms570_selftest_parity.h>

/**
 * @brief run test to check that parity protection works for MibSPI modules RAM
 *
 * @param[in] desc module registers addresses end ESM channels descriptor
 *
 * @return Void, in the case of error invokes bsp_selftest_fail_notification()
 *
 * The descriptor provides address of the module registers and address
 * of internal RAM memory and corresponding parity area test access window.
 */
void tms570_selftest_par_check_mibspi( const tms570_selftest_par_desc_t *desc )
{
  volatile uint32_t      test_read_data;
  volatile tms570_spi_t *spi_regs = (volatile tms570_spi_t *) desc->fnc_data;
  uint32_t               mibspie_bak;
  uint32_t               uerrctl_bak;
  int                    perr;
  int                    wait_timeout = 10000;

  /* wait for MibSPI RAM to complete initialization */
  while ( ( spi_regs->FLG & TMS570_SPI_FLG_BUFINITACTIVE ) ==
          TMS570_SPI_FLG_BUFINITACTIVE ) {
    if ( !wait_timeout-- ) {
      bsp_selftest_fail_notification( desc->fail_code );
    }
  }

  /* Store previous configuration of MibSPI */
  mibspie_bak = spi_regs->MIBSPIE;
  uerrctl_bak = spi_regs->UERRCTRL;

  /* enable multi-buffered mode */
  spi_regs->MIBSPIE = TMS570_SPI_MIBSPIE_MSPIENA;

  /* enable parity error detection */
  spi_regs->UERRCTRL = TMS570_SPI_UERRCTRL_EDEN_SET( spi_regs->UERRCTRL,
                                                    TMS570_SELFTEST_PAR_CR_KEY );

  /* enable parity test mode */
  spi_regs->UERRCTRL |= TMS570_SPI_UERRCTRL_PTESTEN;

  /* flip parity bit */
  *desc->par_loc ^= desc->par_xor;

  /* disable parity TEST mode */
  spi_regs->UERRCTRL &= ~TMS570_SPI_UERRCTRL_PTESTEN;

  /* read to cause parity error */
  test_read_data = *desc->ram_loc;
  (void) test_read_data;

  /* check if ESM channel is flagged */
  perr = tms570_esm_channel_sr_get( desc->esm_prim_grp, desc->esm_prim_chan );

  if ( !perr ) {
    /* RAM parity error was not flagged to ESM. */
    bsp_selftest_fail_notification( desc->fail_code );
  } else {
    /* clear parity error flags */
    spi_regs->UERRSTAT = TMS570_SPI_UERRSTAT_EDFLG1 |
                         TMS570_SPI_UERRSTAT_EDFLG0;

    /* clear ESM flag */
    tms570_esm_channel_sr_clear( desc->esm_prim_grp, desc->esm_prim_chan );

    /* enable parity test mode */
    spi_regs->UERRCTRL |= TMS570_SPI_UERRCTRL_PTESTEN;

    /* Revert back to correct data by flipping parity location */
    *desc->par_loc ^= desc->par_xor;
  }

  /* Restore MIBSPI control registers */
  spi_regs->UERRCTRL = uerrctl_bak;
  spi_regs->MIBSPIE = mibspie_bak;
}
