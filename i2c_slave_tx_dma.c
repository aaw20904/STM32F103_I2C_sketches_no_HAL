
/*

█▀▄▀█ ▄▀█ █ █▄░█ ░ █░█
█░▀░█ █▀█ █ █░▀█ ▄ █▀█
*/

typedef   struct {
	 I2C_TypeDef * hDevice;
	  uint16_t i2cSlaveRxPacketLength;
	  uint16_t i2cSlaveTxPacketLength;
	  uint16_t  slaveI2cBufferLimit;
	  uint16_t i2cRxSlaveIndex;
	  uint16_t i2cTxSlaveIndex;
	  uint32_t i2cSlaveErrorFlags;
	  uint8_t * i2cRxSlaveBuffer;
	  uint8_t * i2cTxSlaveBuffer;
	  uint8_t i2cSlaveAddress;
	  DMA_Channel_TypeDef * dmaRxChanel;  
	  DMA_Channel_TypeDef * dmaTxChanel;

} wrp_i2c_slave_header;

void wrp_i2c_slave_tx_init_DMA(wrp_i2c_slave_header*   header);

/*

█▀▄▀█ ▄▀█ █ █▄░█ ░ █▀▀
█░▀░█ █▀█ █ █░▀█ ▄ █▄▄
*/

volatile uint8_t i2cRxSlaveBuffer[260];
volatile uint8_t i2cTxSlaveBuffer[256]= {"Alice was beginning to get very tired of sitting by her sister on the bank,"
		" and of having nothing to do: once or twice she had peeped into the"
		" book her sister was reading, but it had no pictures or conversations in it, "
		"'and what is the use of a book,' thou"};
volatile uint8_t i2cRxMasterBuffer[260];
volatile uint8_t i2cTxMasterBuffer[256]={
		"So she was considering in her own mind (as well as she could, for the "
		"hot day made her feel very sleepy and stupid), whether the pleasure of "
		"making a daisy-chain would be worth the trouble of getting up and "
		"picking the daisies, when suddenly a White Rabbit"
};


  //1)declare as a global variable - to have access in the interrupts
volatile wrp_i2c_slave_header i2cSlaveHeader;

//2) Implementation of the initializer
void wrp_i2c_slave_tx_init_DMA ( wrp_i2c_slave_header* header) {
     //====DMA====TX channel======
	header->dmaTxChanel->CCR &= ~DMA_CCR_EN;
     	/*DIR=1(read from memory),
     	MSIZE, PSIZE=0 (1 byte),
     	MINC=1, (increment memory),
     	PINS=0 (NO periph. increment),
     	TCIE=1 (transfer complete interrupt)*/
	header->dmaTxChanel->CCR = DMA_CCR_DIR|DMA_CCR_MINC|DMA_CCR_MINC|DMA_CCR_TCIE;
        //packet length
	header->dmaTxChanel->CNDTR = header->i2cSlaveTxPacketLength;
      //addresses of peripherial and memory
	header->dmaTxChanel->CPAR = &header->hDevice->DR;
	header->dmaTxChanel->CMAR = header->i2cTxSlaveBuffer;
     ///----------I2C---initialization
	 //enable own address
	header->hDevice->CR1 &= ~I2C_CR1_PE;
	 //enable DMA
	header->hDevice->CR2 |= I2C_CR2_DMAEN;
	header->hDevice->OAR1 = (header->i2cSlaveAddress << 1);
	header->hDevice->CR1 |=  I2C_CR1_ACK;
	 //enable event interrupts and bufer interrupts
	header->hDevice->CR2 |= I2C_CR2_ITEVTEN|I2C_CR2_ITBUFEN|I2C_CR2_ITERREN;
	  //turn I2C on, enable acknowledge
	header->hDevice->CR1 |= I2C_CR1_PE|I2C_CR1_ACK;
}

void main(){
/*3) Initialize an instance of a structure,
 these values will be used in initialization and inside interrupts
 to restore options in DMA after transactions*/
     i2cSlaveHeader.hDevice = I2C1;  //device
    i2cSlaveHeader.i2cRxSlaveBuffer = 0;  //Rx bufer
    i2cSlaveHeader.i2cTxSlaveBuffer = i2cTxSlaveBuffer; //Tx bufer
    i2cSlaveHeader.i2cSlaveRxPacketLength = 0;
    i2cSlaveHeader.i2cSlaveTxPacketLength = 128;
    i2cSlaveHeader.slaveI2cBufferLimit = 255;  //maximum length (not used)
    i2cSlaveHeader.i2cSlaveAddress = 0x20;
    i2cSlaveHeader.dmaRxChanel = 0;
    i2cSlaveHeader.dmaTxChanel = DMA1_Channel6;

//4) Run LL init functions (enable clocks in AHBENR, set data rate in I2C and timing constants)

//5) Init DMA and slave I2C ind Rx and Tx modes:
   wrp_i2c_slave_tx_init_DMA(&i2cSlaveHeader);
//6)When there is the BUSY I2C bug, clear I2C and init again:
  if (I2C1->SR2 & I2C_SR2_BUSY) {
	  //when busy bug, clean it by reset:
  		I2C1->CR1 |= I2C_CR1_SWRST;
		HAL_Delay(10);
	   I2C1->CR1 |= 0;
	   I2C1->CCR = 0xa0; //ATTENTION! data from LL initializer
       I2C1->TRISE = 0x21; //ATTENTION! data from LL initialyzer
		wrp_i2c_slave_init_DMA(&i2cSlaveHeader);
   }
  //7) Do any actions - all the data will be in DMA RAM buffers 
  
  

}

/*
█ █▄░█ ▀█▀ █▀▀ █▀█ █▀█ █░█ █▀█ ▀█▀ █▀ ░ █▀▀
█ █░▀█ ░█░ ██▄ █▀▄ █▀▄ █▄█ █▀▀ ░█░ ▄█ ▄ █▄▄*/

//1) Declare extern variable:
extern volatile wrp_i2c_slave_header i2cSlaveHeader;

void DMA1_Channel6_IRQHandler(void)
{
  //I2C Transmitter DMA channel
	if(DMA1->ISR & DMA_ISR_TCIF6){
       //disable channel Tx
		i2cSlaveHeader.dmaTxChanel->CCR &= ~DMA_CCR_EN;

		//clear all the flags
		DMA1->IFCR = DMA_IFCR_CGIF6;
		//enable event interrupts for I2C
		i2cSlaveHeader.hDevice->CR2 |= I2C_CR2_ITEVTEN;
	}
 
}

 

///--------I2C-----slave-------------handlers----
void I2C1_EV_IRQHandler (void) {
	uint8_t tempVar;
  //SLAVE
	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ADDR) {
		        //Address sent (master mode)/matched (slave mode)
		       //clear flag
			tempVar = i2cSlaveHeader.hDevice->SR2;//clear ADDR flag
			  //disable intrerrupts until DMA transaction has finished
			i2cSlaveHeader.hDevice->CR2 &= ~I2C_CR2_ITEVTEN;
		if (i2cSlaveHeader.hDevice->SR2 & I2C_SR2_TRA) {
				  ///slave transmitter mode
				GPIOB->BSRR = GPIO_BSRR_BS14; //turn led on Tx (ptional)
				  //restore data length
				i2cSlaveHeader.dmaTxChanel->CNDTR = i2cSlaveHeader.i2cSlaveTxPacketLength;
				  //start DMA Tx
				i2cSlaveHeader.dmaTxChanel->CCR |= DMA_CCR_EN;
		} else {
					//slave receiver mode (not implemented here)
					
		}

	}

	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_STOPF) {
     //Stop detection receiver (slave mode)
		tempVar = i2cSlaveHeader.hDevice->SR2; //read SR1 and SR2 (as in ref manual)
		i2cSlaveHeader.hDevice->CR1 |= I2C_CR1_PE; //write ANY bit inside CR! (ass in ref manual)

		GPIOB->BSRR = GPIO_BSRR_BR14; //turn LED off
	}

}

 ///I2C errors handler
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */
    if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ADDR) {
    	//optional LED on
       GPIOB->BSRR = GPIO_BSRR_BS13;
    }
  /* USER CODE END I2C1_ER_IRQn 0 */

  /* USER CODE BEGIN I2C1_ER_IRQn 1 */
    //save error states inside a global state variable
        i2cSlaveHeader.i2cSlaveErrorFlags = i2cSlaveHeader.hDevice->SR2;
        i2cSlaveHeader.i2cSlaveErrorFlags &= (I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_OVR);
    	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_AF) {
    			//end byte  of the slave transmitter (the last byte NACKed)
    			//clear flag
    		i2cSlaveHeader.hDevice->SR1 &= ~I2C_SR1_AF;
    			GPIOB->BSRR = GPIO_BSRR_BR14;
    	}

    	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ARLO) {
    		//clear flag arbitration lost
    		i2cSlaveHeader.hDevice->SR1 &= ~I2C_SR1_ARLO;
    	}

    	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_BERR) {
    		//clear flag bus error
    		i2cSlaveHeader.hDevice->SR1 &= ~I2C_SR1_BERR;
    	}

    	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_OVR) {
    		//clear flag overrun/underrun
    		i2cSlaveHeader.hDevice->SR1 &= ~I2C_SR1_OVR;
    	}
  /* USER CODE END I2C1_ER_IRQn 1 */
}



