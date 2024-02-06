#include "DMADefines.h"
/*************Function prototype************/
void ADC_Init();
void DMA_Config();
void VREFBUF_Init();

/*************BUFFER***********************/
struct shared_data {
  uint16_t BufferADC1[10];
};
volatile struct shared_data *const xfr_ptr = (struct shared_data *)0x38001000;
//uint16_t *temp_address = BufferADC1;
volatile uint32_t TComplete = 0;
volatile uint32_t Eosmp = 0;
volatile uint32_t Eocie = 0;
void setup() {
  Serial.begin(9600);
  delay(100);
  DMA_Config();
  VREFBUF_Init();
  ADC_Init();
  AC_TIMER1_Init();
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN_Msk);
  SET_BIT(ADC2->CR, ADC_CR_ADSTART_Msk);  //Start ADC


  delay(1000);
}
void loop() {
  Serial.print("TCDMA : ");
  Serial.print(TComplete);
  Serial.print(" SequenceConv : ");
  Serial.println(Eocie);
}

void VREFBUF_Init(void) {
  SET_BIT(RCC->APB4ENR, RCC_APB4ENR_VREFEN_Msk);
  delay(1000);
  SET_BIT(VREFBUF->CSR, VREFBUF_CSR_ENVR_Msk);
  CLEAR_BIT(VREFBUF->CSR, VREFBUF_CSR_HIZ_Msk);
  while (VREFBUF_CSR_VRR & VREFBUF_CSR_VRR_Msk != 1) {}
}

void DMA_Config(void) {
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN_Msk);
  delay(1000);
  //  DMA1_Stream1->CR = 0x00000000; //clear register, reset
  //  while((DMA1_Stream1->CR & 0x00000001 == 0x00000000));
  /***************P2M****************************/
  // 00 - Peripheral to memory
  // 01 - Memory to peripheral
  // 10 = memory to memory
  CLEAR_BIT(DMA1_Stream1->CR, DMA_SxCR_DIR_0 | DMA_SxCR_DIR_1);

  /******************Memory size**************************/
  // MSize
  //00 - 8 bits, 01 - 16, 10 = 32
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_MSIZE_0);
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_MSIZE_1);
  /******************MEM Increment**********************/
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_MINC_Msk);

  //LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_1);
  //LL_DMA_SetMemoryBurstxfer(DMA1, LL_DMA_STREAM_1, LL_DMA_MBURST_SINGLE);

  /******************MEM Address to buffer**************/
  DMA1_Stream1->M0AR = (uint32_t)&xfr_ptr->BufferADC1;
  /*******************Number of data transfer***********/
  DMA1_Stream1->NDTR = 10;  //0 - 65536
  /***********************DMA Mode**********************/
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_CIRC_Msk); // Circular mode
  //DMA1_Stream1->CR = (DMA1_Stream1->CR | (0x00000100)); //circular mode

  /*******************Flow controller*******************/
  //SET_BIT(DMA1_Stream1->CR, DMA_SxCR_PFCTRL_Msk);//ADC in control
  /******************Periph size************************/
  //00 - 8 bits, 01 - 16, 10 = 32  // 16 bot
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_PSIZE_0);
  CLEAR_BIT(DMA1_Stream1->CR, DMA_SxCR_PSIZE_1);
  /******************Peripheral no Increment*************/
  CLEAR_BIT(DMA1_Stream1->CR, DMA_SxCR_PINCOS_Msk);
  /******************Periph request**********************/
  //SET_BIT(DMAMUX1_Channel1->CCR, DMAMUX_CxCR_DMAREQ_ID_1 | DMAMUX_CxCR_DMAREQ_ID_3);//adc2_dma
  DMAMUX1_Channel1->CCR = (DMAMUX1_Channel1->CCR & ~(DMAMUX_CxCR_DMAREQ_ID_Msk))
                          | (DMAMUX1_CxCR_DMAREQ_ID::ADC1_DMA << DMAMUX_CxCR_DMAREQ_ID_Pos);

  /******************Periph address***********************/
  DMA1_Stream1->PAR = (uint32_t)&ADC2->DR;
  /******************TC IT********************************/
  // transfer complete and error interupt
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_TCIE_Msk | DMA_SxCR_TEIE_Msk);  //TC IT
  //SET_BIT(DMA1_Stream1->CR, DMA_SxCR_HTIE_Msk); //HTIE is 0x0
  //SET_BIT(DMA1_Stream1->CR, DMA_SxCR_TCIE_Msk | DMA_SxCR_TEIE_Msk| DMA_SxCR_DMEIE_Msk | DMA_SxCR_HTIE_Msk ); //TC
  //??? 10 is TCIE... HTIE is 0x0
  DMA1_Stream1->CR = (DMA1_Stream1->CR | (0x00000010));  //Half transfer interrupt
  DMA1->LIFCR = DMA_LIFCR_CTCIF1;                        //Clear IT in LISR Register
  NVIC_SetVector(DMA1_Stream1_IRQn, (uint32_t)&DMA1_Stream1_IRQHandler);
  NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /*******************Enable DMA****************************/
  //Serial.println("DMA_Set");
  //Serial.println(DMA1_Stream1->CR);
  SET_BIT(DMA1_Stream1->CR, DMA_SxCR_EN_Msk);

  //while(1);
}
void DMA1_Stream1_IRQHandler(void) {
  if (DMA1->LISR & DMA_LISR_TCIF1) {
    TComplete += 1;
    DMA1->LIFCR = DMA_LIFCR_CTCIF1;
  }
  if (DMA1->LISR & DMA_LISR_TEIF1) {
    DMA1->LIFCR = DMA_LIFCR_CTEIF1;
  }
}
void ADC_Init(void) {
  /*******************Horloges**************************/
  SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN_Msk);  //SYSCFG clock
  delay(1000);
  SET_BIT(RCC->APB4ENR, RCC_APB4ENR_RTCAPBEN_Msk);  //RTCAPB clock
  delay(1000);
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_ADC12EN_Msk);  //ADC12 clocks
  delay(1000);
  SET_BIT(RCC->AHB4ENR, RCC_AHB4ENR_GPIOAEN_Msk);  //GPIOA clock
  delay(1000);
  /********************Port config************************/
  SET_BIT(GPIOA->MODER, GPIO_MODER_MODE1_0);
  SET_BIT(GPIOA->MODER, GPIO_MODER_MODE1_1);
  CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPD1_0);
  CLEAR_BIT(GPIOA->PUPDR, GPIO_PUPDR_PUPD1_1);
  SET_BIT(SYSCFG->PMCR, SYSCFG_PMCR_PA0SO_Msk);
  delay(1000);  //PA0_C in analog mode
  /********************ADC voltage regulator***************/
  CLEAR_BIT(ADC2->CR, ADC_CR_DEEPPWD_Msk);  //END DEEPPWD
  SET_BIT(ADC2->CR, ADC_CR_ADVREGEN_Msk);   //ENABLE ADC VOLTAGE REG
  delay(1000);                              //WAIT VOLTAGE REG
  /********************ADC calibration*********************/
  CLEAR_BIT(ADC2->CR, ADC_CR_ADCALDIF_Msk);
  SET_BIT(ADC2->CR, ADC_CR_ADCALLIN_Msk);
  SET_BIT(ADC2->CR, ADC_CR_ADCAL_Msk);
  while (ADC_CR_ADCAL & ADC_CR_ADCAL_Msk != 0) {}
  /******************ADC clock*****************************/
  SET_BIT(ADC12_COMMON->CCR, ADC_CCR_CKMODE_0 | ADC_CCR_CKMODE_1);
  /*******************ADC Prescaler************************/
  SET_BIT(ADC12_COMMON->CCR, ADC_CCR_PRESC_0 | ADC_CCR_PRESC_1);
  /*******************Input Mode***************************/
  CLEAR_BIT(ADC2->DIFSEL, ADC_DIFSEL_DIFSEL_0);  //Single Ended
  /*******************ADC Enable***************************/
  SET_BIT(ADC2->ISR, ADC_ISR_ADRDY_Msk);
  SET_BIT(ADC2->CR, ADC_CR_ADEN_Msk);
  while (ADC_ISR_ADRDY & ADC_ISR_ADRDY_Msk != 1) {}
  SET_BIT(ADC2->ISR, ADC_ISR_ADRDY_Msk);
  /********************ADC RES*****************************/
  SET_BIT(ADC2->CFGR, ADC_CFGR_RES_2 | ADC_CFGR_RES_1);
  CLEAR_BIT(ADC2->CFGR, ADC_CFGR_RES_0);
  /********************ADC Data Management*****************/
  SET_BIT(ADC2->CFGR, ADC_CFGR_DMNGT_0 | ADC_CFGR_DMNGT_1);  //DMA Circular mode
  /********************OVRMODE*****************************/
  SET_BIT(ADC2->CFGR, ADC_CFGR_OVRMOD_Msk);  //Erase old data
  /********************CONT/Single/Discont*****************/
  SET_BIT(ADC2->CFGR, ADC_CFGR_DISCEN_Msk);  // discontinuous mode
  CLEAR_BIT(ADC2->CFGR, ADC_CFGR_CONT_Msk);  // | ADC_CFGR_DISCEN_Msk
  /********************Trigger Detection*******************/
  SET_BIT(ADC2->CFGR, ADC_CFGR_EXTEN_0 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_3);  //Trig rising edge TRGO2
  CLEAR_BIT(ADC2->CFGR, ADC_CFGR_EXTEN_1 | ADC_CFGR_EXTSEL_0 | ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_4);
  /********************INput Preselection******************/
  SET_BIT(ADC2->PCSEL, ADC_PCSEL_PCSEL_0);  //Chan 0
  /********************Sample Time reg*********************/
  SET_BIT(ADC2->SMPR1, ADC_SMPR1_SMP0_0);  //2.5 CLCK Cycles
  /********************ADC IT******************************/
  SET_BIT(ADC2->IER, ADC_IER_EOCIE_Msk | ADC_IER_EOSMPIE_Msk);  //| ADC_IER_EOSIE_Msk | ADC_IER_OVRIE_Msk
  NVIC_EnableIRQ(ADC_IRQn);
  NVIC_SetVector(ADC_IRQn, (uint32_t)&ADC_IRQHandler);
}
void ADC_IRQHandler(void) {
  if (ADC2->ISR & 0x02) {
    Eosmp += 1;
    SET_BIT(ADC2->ISR, ADC_IER_EOSMPIE_Msk);
  }
  if (ADC2->ISR & 0x04) {
    Eocie += 1;
    SET_BIT(ADC2->ISR, ADC_IER_EOCIE_Msk);
  }
  //  if (ADC2->ISR & 0x08) {
  //    Eos += 1;
  //    SET_BIT(ADC2->ISR, ADC_IER_EOSIE_Msk);
  //  }
}
void ADC_Start(void) {
  SET_BIT(ADC2->CR, ADC_CR_ADSTART_Msk);  //Start
}
void AC_TIMER1_Init(void) {
  //------------------------------Horloge Timer1----------------------------------------//
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN_Msk);
  //------------------------------Select TRGO source-----------------------------------//
  TIM1->PSC = 8399;  //PSC=1
  //----------------------------------------------------------TIM2 autoreload register--------------------------------------------//
  TIM1->ARR = 10000;
  //---------selectupdatevent-----------------//
  SET_BIT(TIM1->CR2, TIM_CR2_MMS2_1);
  //-----------IT-----------------------//
  //  SET_BIT(TIM1->DIER,TIM_DIER_UIE_Msk);
  //  NVIC_SetPriority(TIM1_UP_IRQn,0);
  //  NVIC_SetVector(TIM1_UP_IRQn, (uint32_t)&TIM1_UP_IRQHandler);
  //  NVIC_EnableIRQ(TIM1_UP_IRQn);
  SET_BIT(TIM1->CR1, TIM_CR1_CEN_Msk);  //CEN=1
}