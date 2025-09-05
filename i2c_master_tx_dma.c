//Author : Andrii Androsovych
/*
█▀▄▀█ ▄▀█ █ █▄░█ ░ █░█
█░▀░█ █▀█ █ █░▀█ ▄ █▀█
*/

typedef   struct {
	 I2C_TypeDef * hDevice;
	  uint16_t i2cMasterRxPacketLength;
	  uint16_t i2cMasterTxPacketLength;
	  uint16_t  masterI2cBufferLimit;
	  uint16_t i2cRxMasterIndex;
	  uint16_t i2cTxMasterIndex;
	  uint32_t i2cMasterErrorFlags;
	  uint8_t * i2cRxMasterBuffer;
	  uint8_t * i2cTxMasterBuffer;
	  uint8_t i2cMasterAddress;
	  DMA_Channel_TypeDef * dmaRxChanel;
	  DMA_Channel_TypeDef * dmaTxChanel;

} wrp_i2c_master_header;
void i2c_usr_master_tx_init_DMA(wrp_i2c_master_header* header);
/*

█▀▄▀█ ▄▀█ █ █▄░█ ░ █▀▀
█░▀░█ █▀█ █ █░▀█ ▄ █▄▄
*/
volatile wrp_i2c_master_header i2cMasterHeader;

//2) Implementation of the initializer
void i2c_usr_master_init_DMA(wrp_i2c_master_header* header){
	//DMA init
	 header->dmaTxChanel->CCR |= DMA_CCR_MINC|DMA_CCR_DIR|DMA_CCR_TCIE;
	 header->dmaTxChanel->CNDTR = header->i2cMasterTxPacketLength;
	 header->dmaTxChanel->CPAR = &header->hDevice->DR;
	 header->dmaTxChanel->CMAR = header->i2cTxMasterBuffer;
	 //enable own address
	 header->hDevice->CR1 &= ~I2C_CR1_PE;
	  //own address equals zero - master
	 header->hDevice->CR1 |=  I2C_CR1_ACK;
	 //enable event interrupts and bufer interrupts//enable DMA
	 header->hDevice->CR2 |= I2C_CR2_ITEVTEN|I2C_CR2_ITERREN|I2C_CR2_DMAEN;
	  //turn I2C on, enable acknowledge
	 header->hDevice->CR1 |= I2C_CR1_PE|I2C_CR1_ACK;

}

main(){

 //master initialization
      i2cMasterHeader.dmaRxChanel = 0;
      i2cMasterHeader.dmaTxChanel = 0;
      i2cMasterHeader.hDevice = I2C2;
      i2cMasterHeader.i2cMasterAddress = 0x20;
      i2cMasterHeader.i2cMasterRxPacketLength = 0; //Rx Master not realized
      i2cMasterHeader.i2cMasterTxPacketLength = 128;
      i2cMasterHeader.i2cRxMasterBuffer = 0;
      i2cMasterHeader.i2cTxMasterBuffer =  i2cTxMasterBuffer;
      i2cMasterHeader.masterI2cBufferLimit = 255;
      i2cMasterHeader.dmaRxChanel = 0;
      i2cMasterHeader.dmaTxChanel = DMA1_Channel4;
  
        //init a master (RCC, GPIO initialization must be done before)
        i2c_usr_master_tx_init_DMA(&i2cMasterHeader);
        if (I2C2->SR2 & I2C_SR2_BUSY) {
        	//when busy bug, clean it by reset:
        	 I2C2->CR1 |= I2C_CR1_SWRST;
        	 HAL_Delay(10);
        	 I2C2->CR1 = 0;
          //NOTE: you can call here the the function  LL_I2C_Init(I2C_TypeDef *I2Cx, LL_I2C_InitTypeDef *I2C_InitStruct);
          //insted of the three next steps - it is inside the MX_I2C2_Init() or MX_I2C1_Init function.
        	 I2C2->CR2 = 0x20;//ATTENTION! data from LL initialyzer
        	 I2C2->CCR = 0xa0; //ATTENTION! data from LL initialyzer
        	 I2C2->TRISE = 0x21;//ATTENTION! data from LL initialyzer
        	 i2c_usr_master_tx_init_DMA(&i2cMasterHeader);
        }
  
}

/*
█ █▄░█ ▀█▀ █▀▀ █▀█ █▀█ █░█ █▀█ ▀█▀ █▀ ░ █▀▀
█ █░▀█ ░█░ ██▄ █▀▄ █▀▄ █▄█ █▀▀ ░█░ ▄█ ▄ █▄▄
*/

void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */
	 //I2C Transmitter DMA channel
		if(DMA1->ISR & DMA_ISR_TCIF4){
	       //disable channel Tx
			i2cMasterHeader.dmaTxChanel->CCR &= ~DMA_CCR_EN;

			//clear all the flags
			DMA1->IFCR = DMA_IFCR_CGIF4;
			//enable event interrupts for I2C
			i2cMasterHeader.hDevice->CR2 |= I2C_CR2_ITEVTEN;
		}
  /* USER CODE END DMA1_Channel4_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

void I2C2_EV_IRQHandler (void) {
  /* USER CODE BEGIN I2C2_EV_IRQn 0 */
	uint16_t tempVar;
  /* USER CODE BEGIN I2C2_EV_IRQn 0 */
	if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_SB) {
		//START event has been ended
		  ///  turn LED on (optional)
				GPIOC->BSRR = GPIO_BSRR_BR13;
		//1)Write slave address
		 i2cMasterHeader.hDevice->DR = (i2cMasterHeader.i2cMasterAddress << 1);
	}

	if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_ADDR) {
	          //Address has been  sent to slave device
	          //2)clear ADDR flag by reading SR2 reg
			tempVar = i2cMasterHeader.hDevice->SR2;
			 //disable intrerrupts until DMA transaction has finished
			      i2cMasterHeader.hDevice->CR2 &= ~I2C_CR2_ITEVTEN;
			  //restore data length
			i2cMasterHeader.dmaTxChanel->CNDTR = i2cMasterHeader.i2cMasterTxPacketLength;
			  //start DMA Tx
			i2cMasterHeader.dmaTxChanel->CCR |= DMA_CCR_EN;

	}

	if (i2cMasterHeader.hDevice->SR1 & (I2C_SR1_TXE)) {
	    	tempVar = i2cMasterHeader.hDevice->SR2; //clear flag
				if ((i2cMasterHeader.dmaTxChanel->CNDTR == 0) && (i2cMasterHeader.hDevice->SR1 &I2C_SR1_BTF )) {
						i2cMasterHeader.hDevice->CR1 |= I2C_CR1_PE;       //write ANY bit inside CR! (ass in ref manual)
						i2cMasterHeader.hDevice->CR1 |= I2C_CR1_STOP;
						                 GPIOC->BSRR  = GPIO_BSRR_BS13;   //turn LED off
				}
	}


  /* USER CODE END I2C2_EV_IRQn 0 */

  /* USER CODE BEGIN I2C2_EV_IRQn 1 */

  /* USER CODE END I2C2_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C2 error interrupt.
  */
void I2C2_ER_IRQHandler (void) {
  /* USER CODE BEGIN I2C2_ER_IRQn 0 */
	i2cMasterHeader.i2cMasterErrorFlags = i2cMasterHeader.hDevice->SR2;
	i2cMasterHeader.i2cMasterErrorFlags &= (I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_OVR|I2C_SR1_AF);
		if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_AF) {
			//end byte  of the slave transmitter (the last byte NACKed)
			//clear flag
			i2cMasterHeader.hDevice->SR1 &= ~I2C_SR1_AF;
			//GPIOB->BSRR = GPIO_BSRR_BR14;
		}

		if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_ARLO){
			//clear flag arbitration lost
			i2cMasterHeader.hDevice->SR1 &= ~I2C_SR1_ARLO;
		}

		if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_BERR){
			//clear flag bus error
			I2C2->SR1 &= ~I2C_SR1_BERR;
		}

		if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_OVR){
			//clear flag overrun/underrun
			I2C2->SR1 &= ~I2C_SR1_OVR;
		}
  /* USER CODE END I2C2_ER_IRQn 0 */

  /* USER CODE BEGIN I2C2_ER_IRQn 1 */

  /* USER CODE END I2C2_ER_IRQn 1 */
}
