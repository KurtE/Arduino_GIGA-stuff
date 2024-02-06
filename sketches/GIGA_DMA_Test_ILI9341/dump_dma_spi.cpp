#include <Arduino.h>
#include "DMADefines.h"
void dump_dma_and_spi_settings(SPI_TypeDef *pgigaSpi) {
  Serial.println("\n**SPI Settings**");
  uint32_t reg = pgigaSpi->CR1;
  Serial.print("  CR1:  ");
  Serial.println(reg, HEX); /*!< SPI/I2S Control register 1,                      Address offset: 0x00 */
  if (reg & SPI_CR1_SPE_Msk) Serial.println("\tSPE");
  if (reg & SPI_CR1_MASRX_Msk) Serial.println("\tMASRX");
  if (reg & SPI_CR1_CSTART_Msk) Serial.println("\tCSTART");
  if (reg & SPI_CR1_CSUSP_Msk) Serial.println("\tCSUSP");
  if (reg & SPI_CR1_HDDIR_Msk) Serial.println("\tHDDIR");
  if (reg & SPI_CR1_SSI_Msk) Serial.println("\tSSI");
  if (reg & SPI_CR1_CRC33_17_Msk) Serial.println("\tCRC33_17");
  if (reg & SPI_CR1_RCRCINI_Msk) Serial.println("\tRCRCINI");
  if (reg & SPI_CR1_TCRCINI_Msk) Serial.println("\tTCRCINI");
  if (reg & SPI_CR1_IOLOCK_Msk) Serial.println("\tIOLOCK");

  reg = pgigaSpi->CR2;
  Serial.print("  CR2:  ");
  Serial.println(reg, HEX);
  if (reg & SPI_CR2_TSER_Msk) {
    Serial.print("\tTSER: ");
    Serial.println((reg & SPI_CR2_TSER_Msk) << SPI_CR2_TSER_Pos, DEC);
  }
  if (reg & SPI_CR2_TSIZE_Msk) {
    Serial.print("\tTSIZE: ");
    Serial.println((reg & SPI_CR2_TSIZE_Msk) << SPI_CR2_TSIZE_Pos, DEC);
  }

  reg = pgigaSpi->CFG1;
  Serial.print("  CFG1:  ");
  Serial.println(reg, HEX); /*!< SPI Configuration register 1,                    Address offset: 0x08 */
  if (reg & SPI_CFG1_DSIZE_Msk) {
    Serial.print("\tDSIZE: ");
    Serial.println((reg & SPI_CFG1_DSIZE_Msk) >> SPI_CFG1_DSIZE_Pos, DEC);
  }
  if (reg & SPI_CFG1_FTHLV_Msk) {
    Serial.print("\tFTHLV: ");
    Serial.println((reg & SPI_CFG1_FTHLV_Msk) >> SPI_CFG1_FTHLV_Pos, DEC);
  }
  if (reg & SPI_CFG1_UDRCFG_Msk) {
    Serial.print("\tUDRCFG: ");
    Serial.println((reg & SPI_CFG1_UDRCFG_Msk) >> SPI_CFG1_UDRCFG_Pos, DEC);
  }
  if (reg & SPI_CFG1_UDRDET_Msk) {
    Serial.print("\tUDRDET: ");
    Serial.println((reg & SPI_CFG1_UDRDET_Msk) >> SPI_CFG1_UDRDET_Pos, DEC);
  }
  if (reg & SPI_CFG1_RXDMAEN_Msk) {
    Serial.print("\tRXDMAEN: ");
    Serial.println((reg & SPI_CFG1_RXDMAEN_Msk) >> SPI_CFG1_RXDMAEN_Pos, DEC);
  }
  if (reg & SPI_CFG1_TXDMAEN_Msk) {
    Serial.print("\tTXDMAEN: ");
    Serial.println((reg & SPI_CFG1_TXDMAEN_Msk) >> SPI_CFG1_TXDMAEN_Pos, DEC);
  }
  if (reg & SPI_CFG1_CRCSIZE_Msk) {
    Serial.print("\tCRCSIZE: ");
    Serial.println((reg & SPI_CFG1_CRCSIZE_Msk) >> SPI_CFG1_CRCSIZE_Pos, DEC);
  }
  if (reg & SPI_CFG1_CRCEN_Msk) {
    Serial.print("\tCRCEN: ");
    Serial.println((reg & SPI_CFG1_CRCEN_Msk) >> SPI_CFG1_CRCEN_Pos, DEC);
  }
  if (reg & SPI_CFG1_MBR_Msk) {
    Serial.print("\tMBR: ");
    Serial.println((reg & SPI_CFG1_MBR_Msk) >> SPI_CFG1_MBR_Pos, DEC);
  }

  reg = pgigaSpi->CFG2;
  Serial.print("  CFG2:  ");
  Serial.println(reg, HEX); /*!< SPI Configuration register 2,                    Address offset: 0x0C */
  if (reg & SPI_CFG2_MSSI_Msk) {
    Serial.print("\tMSSI: ");
    Serial.println((reg & SPI_CFG2_MSSI_Msk) >> SPI_CFG2_MSSI_Pos, DEC);
  }
  if (reg & SPI_CFG2_MIDI_Msk) {
    Serial.print("\tMIDI: ");
    Serial.println((reg & SPI_CFG2_MIDI_Msk) >> SPI_CFG2_MIDI_Pos, DEC);
  }
  if (reg & SPI_CFG2_IOSWP_Msk) {
    Serial.print("\tIOSWP: ");
    Serial.println((reg & SPI_CFG2_IOSWP_Msk) >> SPI_CFG2_IOSWP_Pos, DEC);
  }
  if (reg & SPI_CFG2_COMM_Msk) {
    Serial.print("\tCOMM: ");
    Serial.println((reg & SPI_CFG2_COMM_Msk) >> SPI_CFG2_COMM_Pos, DEC);
  }
  if (reg & SPI_CFG2_SP_Msk) {
    Serial.print("\tSP: ");
    Serial.println((reg & SPI_CFG2_SP_Msk) >> SPI_CFG2_SP_Pos, DEC);
  }
  if (reg & SPI_CFG2_MASTER_Msk) {
    Serial.print("\tMASTER: ");
    Serial.println((reg & SPI_CFG2_MASTER_Msk) >> SPI_CFG2_MASTER_Pos, DEC);
  }
  if (reg & SPI_CFG2_LSBFRST_Msk) {
    Serial.print("\tLSBFRST: ");
    Serial.println((reg & SPI_CFG2_LSBFRST_Msk) >> SPI_CFG2_LSBFRST_Pos, DEC);
  }
  if (reg & SPI_CFG2_CPHA_Msk) {
    Serial.print("\tCPHA: ");
    Serial.println((reg & SPI_CFG2_CPHA_Msk) >> SPI_CFG2_CPHA_Pos, DEC);
  }
  if (reg & SPI_CFG2_CPOL_Msk) {
    Serial.print("\tCPOL: ");
    Serial.println((reg & SPI_CFG2_CPOL_Msk) >> SPI_CFG2_CPOL_Pos, DEC);
  }
  if (reg & SPI_CFG2_SSM_Msk) {
    Serial.print("\tSSM: ");
    Serial.println((reg & SPI_CFG2_SSM_Msk) >> SPI_CFG2_SSM_Pos, DEC);
  }
  if (reg & SPI_CFG2_SSIOP_Msk) {
    Serial.print("\tSSIOP: ");
    Serial.println((reg & SPI_CFG2_SSIOP_Msk) >> SPI_CFG2_SSIOP_Pos, DEC);
  }
  if (reg & SPI_CFG2_SSOE_Msk) {
    Serial.print("\tSSOE: ");
    Serial.println((reg & SPI_CFG2_SSOE_Msk) >> SPI_CFG2_SSOE_Pos, DEC);
  }
  if (reg & SPI_CFG2_SSOM_Msk) {
    Serial.print("\tSSOM: ");
    Serial.println((reg & SPI_CFG2_SSOM_Msk) >> SPI_CFG2_SSOM_Pos, DEC);
  }
  if (reg & SPI_CFG2_AFCNTR_Msk) {
    Serial.print("\tAFCNTR: ");
    Serial.println((reg & SPI_CFG2_AFCNTR_Msk) >> SPI_CFG2_AFCNTR_Pos, DEC);
  }
  reg = pgigaSpi->IER;
  Serial.print("  IER:  ");
  Serial.println(reg, HEX); /*!< SPI/I2S Interrupt Enable register,               Address offset: 0x10 */
  if (reg & SPI_IER_RXPIE_Msk) { Serial.println("\tRXPIE"); }
  if (reg & SPI_IER_TXPIE_Msk) { Serial.println("\tTXPIE"); }
  if (reg & SPI_IER_DXPIE_Msk) { Serial.println("\tDXPIE"); }
  if (reg & SPI_IER_EOTIE_Msk) { Serial.println("\tEOTIE"); }
  if (reg & SPI_IER_TXTFIE_Msk) { Serial.println("\tTXTFIE"); }
  if (reg & SPI_IER_UDRIE_Msk) { Serial.println("\tUDRIE"); }
  if (reg & SPI_IER_OVRIE_Msk) { Serial.println("\tOVRIE"); }
  if (reg & SPI_IER_CRCEIE_Msk) { Serial.println("\tCRCEIE"); }
  if (reg & SPI_IER_TIFREIE_Msk) { Serial.println("\tTIFREIE"); }
  if (reg & SPI_IER_MODFIE_Msk) { Serial.println("\tMODFIE"); }
  if (reg & SPI_IER_TSERFIE_Msk) { Serial.println("\tTSERFIE"); }

  reg = pgigaSpi->SR;
  Serial.print("  SR:  ");
  Serial.println(reg, HEX); /*!< SPI/I2S Status register,                         Address offset: 0x14 */
  if (reg & SPI_SR_RXP_Msk) { Serial.println("\tRXP"); }
  if (reg & SPI_SR_TXP_Msk) { Serial.println("\tTXP"); }
  if (reg & SPI_SR_DXP_Msk) { Serial.println("\tDXP"); }
  if (reg & SPI_SR_EOT_Msk) { Serial.println("\tEOT"); }
  if (reg & SPI_SR_TXTF_Msk) { Serial.println("\tTXTF"); }
  if (reg & SPI_SR_UDR_Msk) { Serial.println("\tUDR"); }
  if (reg & SPI_SR_OVR_Msk) { Serial.println("\tOVR"); }
  if (reg & SPI_SR_CRCE_Msk) { Serial.println("\tCRCE"); }
  if (reg & SPI_SR_TIFRE_Msk) { Serial.println("\tTIFRE"); }
  if (reg & SPI_SR_MODF_Msk) { Serial.println("\tMODF"); }
  if (reg & SPI_SR_TSERF_Msk) { Serial.println("\tTSERF"); }
  if (reg & SPI_SR_SUSP_Msk) { Serial.println("\tSUSP"); }
  if (reg & SPI_SR_TXC_Msk) { Serial.println("\tTXC"); }
  if (reg & SPI_SR_RXPLVL_Msk) {
    Serial.print("\tRXPLVL:");
    Serial.println((reg & SPI_SR_RXPLVL_Msk) >> SPI_SR_RXPLVL_Pos, DEC);
  }
  if (reg & SPI_SR_RXWNE_Msk) { Serial.println("\tRXWNE"); }
  if (reg & SPI_SR_CTSIZE_Msk) { Serial.println("\tCTSIZE"); }

  //-----------------------------
  Serial.println("** DMA1 **");
  Serial.print("  LISR: ");
  reg = DMA1->LISR;
  Serial.println(reg, HEX);
  dump_LISR_DISR_bits(0, reg);
  dump_LISR_DISR_bits(1, reg >> 6);
  dump_LISR_DISR_bits(2, reg >> 16);
  dump_LISR_DISR_bits(3, reg >> 22);
  Serial.print("  HISR: ");
  Serial.println(DMA1->HISR, HEX);
  reg = DMA1->HISR;
  dump_LISR_DISR_bits(4, reg);
  dump_LISR_DISR_bits(5, reg >> 6);
  dump_LISR_DISR_bits(6, reg >> 16);
  dump_LISR_DISR_bits(7, reg >> 22);
  // write only registers
  //  Serial.print("  LIFCR: ");
  //  Serial.println(DMA1->LIFCR, HEX);
  //  Serial.print("  HIFCR: ");
  //  Serial.println(DMA1->HIFCR, HEX);


  dump_dma_stream_settings("DMA1_Stream1:", DMA1_Stream1);

  dump_dmamux("DMAMUX1_Channel1", DMAMUX1_Channel1);
}
void dump_LISR_DISR_bits(uint8_t index, uint32_t reg_bits) {
  if ((reg_bits & 0x3f) == 0) return;
  Serial.print("\t");
  Serial.print(index, DEC);
  Serial.print(":");
  if (reg_bits & 0x01) Serial.print(" FEIF");
  if (reg_bits & 0x04) Serial.print(" DMEIF");
  if (reg_bits & 0x08) Serial.print(" TEIF");
  if (reg_bits & 0x10) Serial.print(" HTIF");
  if (reg_bits & 0x20) Serial.print(" TCIF");
  Serial.println();
}

void dump_dma_stream_settings(const char *sz, DMA_Stream_TypeDef *dmas) {
  Serial.println(sz);
  Serial.print("  CR: ");
  uint32_t reg = dmas->CR;
  Serial.println(reg, HEX);
  if (reg & DMA_SxCR_MBURST_Msk) {
    Serial.print(" MBURST:");
    Serial.print((reg & DMA_SxCR_MBURST_Msk) >> DMA_SxCR_MBURST_Pos, HEX);
  }
  if (reg & DMA_SxCR_PBURST_Msk) {
    Serial.print(" tPBURST:");
    Serial.print((reg & DMA_SxCR_PBURST_Msk) >> DMA_SxCR_PBURST_Pos, HEX);
  }
  if (reg & DMA_SxCR_TRBUFF_Msk) {
    Serial.print(" TRBUFF:");
    Serial.print((reg & DMA_SxCR_TRBUFF_Msk) >> DMA_SxCR_TRBUFF_Pos, HEX);
  }
  if (reg & DMA_SxCR_CT_Msk) {
    Serial.print(" CT:");
    Serial.print((reg & DMA_SxCR_CT_Msk) >> DMA_SxCR_CT_Pos, HEX);
  }
  if (reg & DMA_SxCR_DBM_Msk) {
    Serial.print(" DBM:");
    Serial.print((reg & DMA_SxCR_DBM_Msk) >> DMA_SxCR_DBM_Pos, HEX);
  }
  if (reg & DMA_SxCR_PL_Msk) {
    Serial.print(" PL:");
    Serial.print((reg & DMA_SxCR_PL_Msk) >> DMA_SxCR_PL_Pos, HEX);
  }
  if (reg & DMA_SxCR_PINCOS_Msk) {
    Serial.print(" PINCOS:");
    Serial.print((reg & DMA_SxCR_PINCOS_Msk) >> DMA_SxCR_PINCOS_Pos, HEX);
  }
  if (reg & DMA_SxCR_MSIZE_Msk) {
    Serial.print(" MSIZE:");
    Serial.print((reg & DMA_SxCR_MSIZE_Msk) >> DMA_SxCR_MSIZE_Pos, HEX);
  }
  if (reg & DMA_SxCR_PSIZE_Msk) {
    Serial.print(" PSIZE:");
    Serial.print((reg & DMA_SxCR_PSIZE_Msk) >> DMA_SxCR_PSIZE_Pos, HEX);
  }
  if (reg & DMA_SxCR_MINC_Msk) {
    Serial.print(" MINC:");
    Serial.print((reg & DMA_SxCR_MINC_Msk) >> DMA_SxCR_MINC_Pos, HEX);
  }
  if (reg & DMA_SxCR_PINC_Msk) {
    Serial.print(" PINC:");
    Serial.print((reg & DMA_SxCR_PINC_Msk) >> DMA_SxCR_PINC_Pos, HEX);
  }
  if (reg & DMA_SxCR_CIRC_Msk) {
    Serial.print(" CIRC:");
    Serial.print((reg & DMA_SxCR_CIRC_Msk) >> DMA_SxCR_CIRC_Pos, HEX);
  }
  if (reg & DMA_SxCR_DIR_Msk) {
    Serial.print(" DIR:");
    Serial.print((reg & DMA_SxCR_DIR_Msk) >> DMA_SxCR_DIR_Pos, HEX);
  }
  if (reg & DMA_SxCR_PFCTRL_Msk) {
    Serial.print(" PFCTRL:");
    Serial.print((reg & DMA_SxCR_PFCTRL_Msk) >> DMA_SxCR_PFCTRL_Pos, HEX);
  }
  if (reg & DMA_SxCR_TCIE_Msk) {
    Serial.print(" TCIE:");
    Serial.print((reg & DMA_SxCR_TCIE_Msk) >> DMA_SxCR_TCIE_Pos, HEX);
  }
  if (reg & DMA_SxCR_HTIE_Msk) {
    Serial.print(" HTIE:");
    Serial.print((reg & DMA_SxCR_HTIE_Msk) >> DMA_SxCR_HTIE_Pos, HEX);
  }
  if (reg & DMA_SxCR_TEIE_Msk) {
    Serial.print(" TEIE:");
    Serial.print((reg & DMA_SxCR_TEIE_Msk) >> DMA_SxCR_TEIE_Pos, HEX);
  }
  if (reg & DMA_SxCR_DMEIE_Msk) {
    Serial.print(" DMEIE:");
    Serial.print((reg & DMA_SxCR_DMEIE_Msk) >> DMA_SxCR_DMEIE_Pos, HEX);
  }
  if (reg & DMA_SxCR_EN_Msk) {
    Serial.print(" EN:");
    Serial.print((reg & DMA_SxCR_EN_Msk) >> DMA_SxCR_EN_Pos, HEX);
  }


  Serial.print("\n  NDTR: ");
  Serial.print(dmas->NDTR, DEC);
  Serial.print("(0x");
  Serial.print(dmas->NDTR, HEX);
  Serial.print(")\n  PAR: ");
  Serial.println(dmas->PAR, HEX);
  Serial.print("  M0AR: ");
  Serial.println(dmas->M0AR, HEX);
  Serial.print("  M1AR: ");
  Serial.println(dmas->M1AR, HEX);
  Serial.print("  FCR: ");
  Serial.print(dmas->FCR, HEX);
  reg = dmas->FCR;
  if (reg & DMA_SxFCR_FEIE_Msk) {
    Serial.print(" FEIE:");
    Serial.print((reg & DMA_SxFCR_FEIE_Msk) >> DMA_SxFCR_FEIE_Pos, HEX);
  }
  if (reg & DMA_SxFCR_FS_Msk) {
    Serial.print(" FS:");
    Serial.print((reg & DMA_SxFCR_FS_Msk) >> DMA_SxFCR_FS_Pos, HEX);
  }
  if (reg & DMA_SxFCR_DMDIS_Msk) {
    Serial.print(" DMDIS:");
    Serial.print((reg & DMA_SxFCR_DMDIS_Msk) >> DMA_SxFCR_DMDIS_Pos, HEX);
  }
  if (reg & DMA_SxFCR_FTH_Msk) {
    Serial.print(" FTH:");
    Serial.print((reg & DMA_SxFCR_FTH_Msk) >> DMA_SxFCR_FTH_Pos, HEX);
  }
  Serial.println();
}

void dump_dmamux(const char *sz, DMAMUX_Channel_TypeDef *dmuxc) {
  Serial.println(sz);
  uint32_t reg = dmuxc->CCR;
  Serial.print("  CCR: ");
  Serial.print(reg, HEX);
  if (reg & (1 << 8)) Serial.print(" SOIE");
  if (reg & (1 << 9)) Serial.print(" EGE");
  if (reg & (1 << 16)) Serial.print(" SE");
  if (reg & (3 << 17)) {
    Serial.print(" SPOL(");
    switch ((reg >> 17) & 0x3) {
      default: break;
      case 1: Serial.print("Rise)"); break;
      case 2: Serial.print("Fall)"); break;
      case 3: Serial.print("Rise fall)"); break;
    }
  }
  Serial.print(" NBREQ(");
  Serial.print((reg >> 19) & 0x1f, DEC);
  Serial.print(") Sync(");
  Serial.print((reg >> 24) & 0x7, DEC);
  Serial.println(")");
  // Request ID:
  Serial.print("\tREQ ID: ");
  switch (reg & 0x7f) {
    case 1: Serial.println("DMAMUX1_REQ_GEN0"); break;
    case 2: Serial.println("DMAMUX1_REQ_GEN1"); break;
    case 3: Serial.println("DMAMUX1_REQ_GEN2"); break;
    case 4: Serial.println("DMAMUX1_REQ_GEN3"); break;
    case 5: Serial.println("DMAMUX1_REQ_GEN4"); break;
    case 6: Serial.println("DMAMUX1_REQ_GEN5"); break;
    case 7: Serial.println("DMAMUX1_REQ_GEN6"); break;
    case 8: Serial.println("DMAMUX1_REQ_GEN7"); break;
    case 9: Serial.println("ADC1_DMA"); break;
    case 10: Serial.println("ADC2_DMA"); break;
    case 11: Serial.println("TIM1_CH1"); break;
    case 12: Serial.println("TIM1_CH2"); break;
    case 13: Serial.println("TIM1_CH3"); break;
    case 14: Serial.println("TIM1_CH4"); break;
    case 15: Serial.println("TIM1_UP"); break;
    case 16: Serial.println("TIM1_TRIG"); break;
    case 17: Serial.println("TIM1_COM"); break;
    case 18: Serial.println("TIM2_CH1"); break;
    case 19: Serial.println("TIM2_CH2"); break;
    case 20: Serial.println("TIM2_CH3"); break;
    case 21: Serial.println("TIM2_CH4"); break;
    case 22: Serial.println("TIM2_UP"); break;
    case 23: Serial.println("TIM3_CH1"); break;
    case 24: Serial.println("TIM3_CH2"); break;
    case 25: Serial.println("TIM3_CH3"); break;
    case 26: Serial.println("TIM3_CH4"); break;
    case 27: Serial.println("TIM3_UP"); break;
    case 28: Serial.println("TIM3_TRIG"); break;
    case 29: Serial.println("TIM4_CH1"); break;
    case 30: Serial.println("TIM4_CH2"); break;
    case 31: Serial.println("TIM4_CH3"); break;
    case 32: Serial.println("TIM4_UP"); break;
    case 33: Serial.println("I2C1_RX_DMA"); break;
    case 34: Serial.println("I2C1_TX_DMA"); break;
    case 35: Serial.println("I2C2_RX_DMA"); break;
    case 36: Serial.println("I2C2_TX_DMA"); break;
    case 37: Serial.println("SPI1_RX_DMA"); break;
    case 38: Serial.println("SPI1_TX_DMA"); break;
    case 39: Serial.println("SPI2_RX_DMA"); break;
    case 40: Serial.println("SPI2_TX_DMA"); break;
    case 41: Serial.println("USART1_RX_DMA"); break;
    case 42: Serial.println("USART1_TX_DMA"); break;
    case 43: Serial.println("USART2_RX_DMA"); break;
    case 44: Serial.println("USART2_TX_DMA"); break;
    case 45: Serial.println("USART3_RX_DMA"); break;
    case 46: Serial.println("USART3_TX_DMA"); break;
    case 47: Serial.println("TIM8_CH1"); break;
    case 48: Serial.println("TIM8_CH2"); break;
    case 49: Serial.println("TIM8_CH3"); break;
    case 50: Serial.println("TIM8_CH4"); break;
    case 51: Serial.println("TIM8_UP"); break;
    case 52: Serial.println("TIM8_TRIG"); break;
    case 53: Serial.println("TIM8_COM"); break;
    case 54: Serial.println("RESERVED"); break;
    case 55: Serial.println("TIM5_CH1"); break;
    case 56: Serial.println("TIM5_CH2"); break;
    case 57: Serial.println("TIM5_CH3"); break;
    case 58: Serial.println("TIM5_CH4"); break;
    case 59: Serial.println("TIM5_UP"); break;
    case 60: Serial.println("TIM5_TRIG"); break;
    case 61: Serial.println("SPI3_RX_DMA"); break;
    case 62: Serial.println("SPI3_TX_DMA"); break;
    case 63: Serial.println("UART4_RX_DMA"); break;
    case 64: Serial.println("UART4_TX_DMA"); break;
    case 65: Serial.println("UART5_RX_DMA"); break;
    case 66: Serial.println("UART5_TX_DMA"); break;
    case 67: Serial.println("DAC_CH1_DMA"); break;
    case 68: Serial.println("DAC_CH2_DMA"); break;
    case 69: Serial.println("TIM6_UP"); break;
    case 70: Serial.println("TIM7_UP"); break;
    case 71: Serial.println("USART6_RX_DMA"); break;
    case 72: Serial.println("USART6_TX_DMA"); break;
    case 73: Serial.println("I2C3_RX_DMA"); break;
    case 74: Serial.println("I2C3_TX_DMA"); break;
    case 75: Serial.println("DCMI_DMA"); break;
    case 76: Serial.println("CRYP_IN_DMA"); break;
    case 77: Serial.println("CRYP_OUT_DMA"); break;
    case 78: Serial.println("HASH_IN_DMA"); break;
    case 79: Serial.println("UART7_RX_DMA"); break;
    case 80: Serial.println("UART7_TX_DMA"); break;
    case 81: Serial.println("UART8_RX_DMA"); break;
    case 82: Serial.println("UART8_TX_DMA"); break;
    case 83: Serial.println("SPI4_RX_DMA"); break;
    case 84: Serial.println("SPI4_TX_DMA"); break;
    case 85: Serial.println("SPI5_RX_DMA"); break;
    case 86: Serial.println("SPI5_TX_DMA"); break;
    case 87: Serial.println("SAI1A_DMA"); break;
    case 88: Serial.println("SAI1B_DMA"); break;
    case 89: Serial.println("SAI2A_DMA"); break;
    case 90: Serial.println("SAI2B_DMA"); break;
    case 91: Serial.println("SWPMI_RX_DMA"); break;
    case 92: Serial.println("SWPMI_TX_DMA"); break;
    case 93: Serial.println("SPDIFRX_DAT_DMA"); break;
    case 94: Serial.println("SPDIFRX_CTRL_DMA"); break;
    case 95: Serial.println("HR_REQ_1"); break;
    case 96: Serial.println("HR_REQ_2"); break;
    case 97: Serial.println("HR_REQ_3"); break;
    case 98: Serial.println("HR_REQ_4"); break;
    case 99: Serial.println("HR_REQ_5"); break;
    case 100: Serial.println("HR_REQ_6"); break;
    case 101: Serial.println("DFSDM1_DMA0"); break;
    case 102: Serial.println("DFSDM1_DMA1"); break;
    case 103: Serial.println("DFSDM1_DMA2"); break;
    case 104: Serial.println("DFSDM1_DMA3"); break;
    case 105: Serial.println("TIM15_CH1"); break;
    case 106: Serial.println("TIM15_UP"); break;
    case 107: Serial.println("TIM15_TRIG"); break;
    case 108: Serial.println("TIM15_COM"); break;
    case 109: Serial.println("TIM16_CH1"); break;
    case 110: Serial.println("TIM16_UP"); break;
    case 111: Serial.println("TIM17_CH1"); break;
    case 112: Serial.println("TIM17_UP"); break;
    case 113: Serial.println("SAI3_A_DMA"); break;
    case 114: Serial.println("SAI3_B_DMA"); break;
    case 115: Serial.println("ADC3_DMA"); break;
  }
}

