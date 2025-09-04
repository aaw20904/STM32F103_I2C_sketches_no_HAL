//Author: Anrii Androsovych
//NOTE: I2C slave runs in Rx and Tx modes
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

void i2c_usr_slave_init (wrp_i2c_slave_header*   header);

/*

█▀▄▀█ ▄▀█ █ █▄░█ ░ █▀▀
█░▀░█ █▀█ █ █░▀█ ▄ █▄▄
*/
volatile wrp_i2c_slave_header i2cSlaveHeader;

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


void i2c_usr_slave_init (wrp_i2c_slave_header*   header) {
	 //disable I2C
	 header->hDevice->CR1 &= ~I2C_CR1_PE;
  //write own address
	 header->hDevice->OAR1 = (header->i2cSlaveAddress << 1);
  //enable acknowledgements
	 header->hDevice->CR1 |=  I2C_CR1_ACK;
	 //enable event interrupts and bufer interrupts
	 header->hDevice->CR2 |= I2C_CR2_ITEVTEN|I2C_CR2_ITBUFEN|I2C_CR2_ITERREN;
	  //turn I2C on, enable acknowledge
	 header->hDevice->CR1 |= I2C_CR1_PE|I2C_CR1_ACK;
}
  main(){

	  //init structure
	    i2cSlaveHeader.hDevice = I2C1;  //device
      i2cSlaveHeader.i2cRxSlaveBuffer = i2cRxSlaveBuffer;  //Rx bufer
      i2cSlaveHeader.i2cTxSlaveBuffer = i2cTxSlaveBuffer; //Tx bufer
      i2cSlaveHeader.i2cSlaveRxPacketLength = 128;
      i2cSlaveHeader.i2cSlaveTxPacketLength = 128;
      i2cSlaveHeader.slaveI2cBufferLimit = 255;  //maximum length (not used)
      i2cSlaveHeader.i2cSlaveAddress = 0x20;
      i2cSlaveHeader.dmaRxChanel = 0;
      i2cSlaveHeader.dmaTxChanel = 0;
	  
      ///initialization RCC clocks, GPIOs  of I2C  by the LL drivers
	  //5) Init   slave I2C ind Rx and Tx modes:
      i2c_usr_slave_init(&i2cSlaveHeader);
	  
      //6)When there is the BUSY I2C bug, clear I2C and init again:
        if (I2C1->SR2 & I2C_SR2_BUSY) {
      	  //when busy bug, clean it by reset:
        		I2C1->CR1 |= I2C_CR1_SWRST;
      		HAL_Delay(10);
			 I2C1->CR1  = 0;
      		  //NOTE: you can call here the the function  LL_I2C_Init(I2C_TypeDef *I2Cx, LL_I2C_InitTypeDef *I2C_InitStruct);
				//insted of the three next steps - it is inside the MX_I2C2_Init() or MX_I2C1_Init function.
				I2C1->CCR = 0xa0; //ATTENTION! data from LL initializer
				I2C1->TRISE = 0x21; //ATTENTION! data from LL initialyzer
				I2C1->CR2 = 0x20;//ATTENTION! data from LL initialyzer
      		i2c_usr_slave_init(&i2cSlaveHeader);
         }

	  
  }

/*

█ █▄░█ ▀█▀ █▀▀ █▀█ █▀█ █░█ █▀█ ▀█▀ █▀ ░ █▀▀
█ █░▀█ ░█░ ██▄ █▀▄ █▀▄ █▄█ █▀▀ ░█░ ▄█ ▄ █▄▄
*/
void I2C1_EV_IRQHandler(void)
{
	uint8_t tempVar;

		if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ADDR) {
	      //Address sent (master mode)/matched (slave mode)
	       //clear flag
			tempVar = I2C1->SR2;
			if (i2cSlaveHeader.hDevice->SR2 & I2C_SR2_TRA) {
				///slave transmitter mode
				GPIOC->BSRR = GPIO_BSRR_BR14; //turn led on Tx (optional)
			} else {
				//slave receiver mode
				GPIOC->BSRR = GPIO_BSRR_BR14; //turn on led Rx (optional)
			}
			i2cSlaveHeader.i2cRxSlaveIndex = 0;
			i2cSlaveHeader.i2cTxSlaveIndex = 0;

		}
		if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_RXNE) {
	     //Data register not empty (receivers)
			 //NOTE:clear flag when a user reads data
			//read data
			i2cSlaveHeader.i2cRxSlaveBuffer[i2cSlaveHeader.i2cRxSlaveIndex] = I2C1->DR;
			i2cSlaveHeader.i2cRxSlaveIndex++;
	       ///for safety
			if (i2cSlaveHeader.i2cRxSlaveIndex >  i2cSlaveHeader.slaveI2cBufferLimit){
				i2cSlaveHeader.i2cRxSlaveIndex=0;
			}
		}

		if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_TXE) {
	     ///Data register empty (transmitters)
			//interrupt fires firstly when the address has been sent
			//and affer each byte
			i2cSlaveHeader.hDevice->DR =i2cSlaveHeader.i2cTxSlaveBuffer[i2cSlaveHeader.i2cTxSlaveIndex];
			i2cSlaveHeader.i2cTxSlaveIndex++;
			//for safety
			if (i2cSlaveHeader.i2cTxSlaveIndex >  i2cSlaveHeader.slaveI2cBufferLimit){
				i2cSlaveHeader.i2cTxSlaveIndex=0;
			}
		}

		if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_STOPF) {
	     //Stop detection receiver (slave mode)
			tempVar = I2C1->SR2; //read SR1 and SR2 (as in ref manual)
			i2cSlaveHeader.hDevice->CR1 |= I2C_CR1_PE; //write ANY bit inside CR! (ass in ref manual)
			GPIOB->BSRR = GPIO_BSRR_BS14; //turn LED off
		}

}

 
void I2C1_ER_IRQHandler(void)
{
	 if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ADDR) {
			//turn the led ON (optional)
	       GPIOC->BSRR = GPIO_BSRR_BR13;
	    }

	    //save error states inside a global state variable
	 i2cSlaveHeader.i2cSlaveErrorFlags = i2cSlaveHeader.hDevice->SR2;
		//clerar error flags
	 i2cSlaveHeader.i2cSlaveErrorFlags &= (I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_OVR);
	    	if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_AF) {
	    			//end byte  of the slave transmitter (the last byte NACKed)
	    			//clear flag
	    		i2cSlaveHeader.hDevice->SR1 &= ~I2C_SR1_AF;
				   //turn the LED off (optional)
	    			GPIOC->BSRR = GPIO_BSRR_BS13;
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

  
}
