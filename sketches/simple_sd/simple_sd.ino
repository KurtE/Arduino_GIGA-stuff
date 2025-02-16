/*
 * Example use of chdir(), ls(), mkdir(), and  rmdir().
 */
#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

#define DEBUG_PIN 4

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7)
const uint8_t SD_CS_PIN = 7;
#define SPIX SPI
#define SPIX_BASE ((SPI_TypeDef *) SPI2_BASE)
#else
// GIGA
#define SPIX SPI
#define SPIX_BASE ((SPI_TypeDef *) SPI1_BASE)  // IF SPI object
//#define SPIX_BASE ((SPI_TypeDef *) SPI5_BASE)  // IF SPI1 object
const uint8_t SD_CS_PIN = 6;
#endif

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(32)

// Try to select the best SD card configuration.
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK, &SPI1)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK, &SPIX)

//------------------------------------------------------------------------------

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#endif  // SD_FAT_TYPE
extern void dump_spi_settings(SPI_TypeDef *pgigaSpi);

void setup() {
    Serial.begin(9600);
    delay(2000);

    // Initialize the SD card.

    SPIX.begin(); // make sure it is started
    SPIX.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    SPIX.endTransaction();
    Serial.println("After SPIx begin");
    dump_spi_settings(SPIX_BASE);

    Serial.println("Before SD Begin");
    bool status = sd.cardBegin(SD_CONFIG);
    dump_spi_settings(SPIX_BASE);
    if(!status)
    {
//        digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere
        Serial.print("Card Begin failed Error: ");
        Serial.println(sd.sdErrorCode());
        while (1) {}
    }

    //digitalWrite(DEBUG_PIN, LOW);  // make sure display does not interfere

    if (!sd.volumeBegin()) 
    {
        Serial.println("Volume begin failed");
        while (1) {}
    }

    Serial.println("List of files on the SD.\n");
    sd.ls(LS_R);
    Serial.println("--- End of List ---");
}

void loop() {
    // put your main code here, to run repeatedly:
}

void dump_spi_settings(SPI_TypeDef *pgigaSpi) {
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
}
