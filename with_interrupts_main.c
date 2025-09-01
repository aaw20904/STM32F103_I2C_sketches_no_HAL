//Author Andrii Androsovych
//NOTE: These sketches was developed for well known STM32F103 MCU
///indexes pointers to arrays
volatile uint16_t i2cRxSlaveIndex=0;
volatile uint16_t i2cTxSlaveIndex=0;
//The limit for buffers for safety
volatile  const uint16_t  slaveI2cBufferLimit = 260;
//errors (copy of SR)
volatile uint32_t i2cSlaveErrorFlags;

volatile uint8_t i2cRxSlaveBuffer[260];
volatile uint8_t i2cTxSlaveBuffer[256]= {"Alice was beginning to get very tired of sitting by her sister on the bank,"
		" and of having nothing to do: once or twice she had peeped into the"
		" book her sister was reading, but it had no pictures or conversations in it, "
		"'and what is the use of a book,' thou"};

void i2c_usr_slave_init (uint8_t address) {
	 //disable I2C
	 I2C1->CR1 &= ~I2C_CR1_PE;
  //write own address
	 I2C1->OAR1 = (address << 1);
  //enable acknowledgements
	 I2C1->CR1 |=  I2C_CR1_ACK;
	 //enable event interrupts and bufer interrupts
	 I2C1->CR2 |= I2C_CR2_ITEVTEN|I2C_CR2_ITBUFEN|I2C_CR2_ITERREN;
	  //turn I2C on, enable acknowledge
	 I2C1->CR1 |= I2C_CR1_PE|I2C_CR1_ACK;
}

//==========INTERRUPT SERVICE ROUTINES======
/*there must be enable event and error interrupts*/
void I2C1_EV_IRQHandler(void)
{
	uint8_t tempVar;
  
	if (I2C1->SR1 & I2C_SR1_ADDR) {
      //Address sent (master mode)/matched (slave mode)
       //clear flag
		tempVar = I2C1->SR2;
		if (I2C1->SR2 & I2C_SR2_TRA) {
			///slave transmitter mode
			GPIOC->BSRR = GPIO_BSRR_BR14; //turn led on Tx (optional)
		} else {
			//slave receiver mode
			GPIOC->BSRR = GPIO_BSRR_BR14; //turn on led Rx (optional)
		}
		i2cRxSlaveIndex = 0;
		i2cTxSlaveIndex = 0;

	}
	if (I2C1->SR1 & I2C_SR1_RXNE) {
     //Data register not empty (receivers)
		 //NOTE:clear flag when a user reads data
		//read data
		i2cRxSlaveBuffer[i2cRxSlaveIndex] = I2C1->DR;
		i2cRxSlaveIndex++;
       ///for safety
		if (i2cRxSlaveIndex >  slaveI2cBufferLimit){
			i2cRxSlaveIndex=0;
		}
	}

	if (I2C1->SR1 & I2C_SR1_TXE) {
     ///Data register empty (transmitters)
		//interrupt fires firstly when the address has been sent
		//and affer each byte
		I2C1->DR =i2cTxSlaveBuffer[i2cTxSlaveIndex];
		i2cTxSlaveIndex++;
		//for safety
		if (i2cTxSlaveIndex >  slaveI2cBufferLimit){
			i2cTxSlaveIndex=0;
		}
	}

	if (I2C1->SR1 & I2C_SR1_STOPF) {
     //Stop detection receiver (slave mode)
		tempVar = I2C1->SR2; //read SR1 and SR2 (as in ref manual)
		I2C1->CR1 |= I2C_CR1_PE; //write ANY bit inside CR! (ass in ref manual)
		GPIOB->BSRR = GPIO_BSRR_BS14; //turn LED off
	}
   
}

 
void I2C1_ER_IRQHandler(void) {
  
    if (I2C1->SR1 & I2C_SR1_ADDR) {
		//turn the led ON (optional) 
       GPIOC->BSRR = GPIO_BSRR_BR13;
    }
	
    //save error states inside a global state variable
    	i2cSlaveErrorFlags = I2C1->SR2;
	//clerar error flags
    	i2cSlaveErrorFlags &= (I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_OVR);
    	if (I2C1->SR1 & I2C_SR1_AF) {
    			//end byte  of the slave transmitter (the last byte NACKed)
    			//clear flag
    			I2C1->SR1 &= ~I2C_SR1_AF;
			   //turn the LED off (optional)
    			GPIOC->BSRR = GPIO_BSRR_BS13; 
    	}

    	if (I2C1->SR1 & I2C_SR1_ARLO) {
    		//clear flag arbitration lost
    		I2C1->SR1 &= ~I2C_SR1_ARLO;
    	}

    	if (I2C1->SR1 & I2C_SR1_BERR) {
    		//clear flag bus error
    		I2C1->SR1 &= ~I2C_SR1_BERR;
    	}

    	if (I2C1->SR1 & I2C_SR1_OVR) {
    		//clear flag overrun/underrun
    		I2C1->SR1 &= ~I2C_SR1_OVR;
    	}
   
}
