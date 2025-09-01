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

void i2c_usr_slave_init(uint8_t address){
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
