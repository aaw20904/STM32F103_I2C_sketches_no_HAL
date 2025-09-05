//Author : Andrii Androsovych
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
	  uint8_t WriteFlag;

} wrp_i2c_master_header;

void i2c_usr_master_rx_tx_init(wrp_i2c_master_header* header);

void i2c_usr_slave_rx_tx_init (wrp_i2c_slave_header*   header);
/*
█▀▄▀█ ▄▀█ █ █▄░█ ░ █▀▀
█░▀░█ █▀█ █ █░▀█ ▄ █▄▄
*/

volatile wrp_i2c_slave_header i2cSlaveHeader;
volatile wrp_i2c_master_header i2cMasterHeader;

void i2c_usr_slave_rx_tx_init (wrp_i2c_slave_header*   header) {
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

void i2c_usr_master_rx_tx_init(wrp_i2c_master_header* header){
	 //enable own address
	 header->hDevice->CR1 &= ~I2C_CR1_PE;
	  //own address equals zero - master
	 header->hDevice->CR1 |=  I2C_CR1_ACK;
	 //enable event interrupts and bufer interrupts
	 header->hDevice->CR2 |= I2C_CR2_ITEVTEN|I2C_CR2_ITBUFEN|I2C_CR2_ITERREN;
	  //turn I2C on, enable acknowledge
	 header->hDevice->CR1 |= I2C_CR1_PE|I2C_CR1_ACK;

}

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



main() {
 /*3) Initialize an instance of a structure,
   these values will be used in initialization and inside interrupts
   to restore options in DMA after transactions*/
       i2cSlaveHeader.hDevice = I2C1;  //device
      i2cSlaveHeader.i2cRxSlaveBuffer = i2cRxSlaveBuffer;  //Rx bufer
      i2cSlaveHeader.i2cTxSlaveBuffer = i2cTxSlaveBuffer; //Tx bufer
      i2cSlaveHeader.i2cSlaveRxPacketLength = 4;
      i2cSlaveHeader.i2cSlaveTxPacketLength = 4;
      i2cSlaveHeader.slaveI2cBufferLimit = 255;  //maximum length (not used)
      i2cSlaveHeader.i2cSlaveAddress = 0x20;
      i2cSlaveHeader.dmaRxChanel = DMA1_Channel7;
      i2cSlaveHeader.dmaTxChanel = DMA1_Channel6;
      //master initialization
	  i2cMasterHeader.dmaRxChanel = 0;
	  i2cMasterHeader.dmaTxChanel = 0;
	  i2cMasterHeader.hDevice = I2C2;
	  i2cMasterHeader.i2cMasterAddress = 0x20;
	  i2cMasterHeader.i2cMasterRxPacketLength = 2; //Rx Master not realized
	  i2cMasterHeader.i2cMasterTxPacketLength = 2;
		i2cMasterHeader.i2cRxMasterBuffer = i2cRxMasterBuffer;
		i2cMasterHeader.i2cTxMasterBuffer =  i2cTxMasterBuffer;
		i2cMasterHeader.masterI2cBufferLimit = 255;
		i2cMasterHeader.dmaRxChanel = 0;
		i2cMasterHeader.dmaTxChanel = 0;
		i2cMasterHeader.WriteFlag = 0;



      //5) Init DMA and slave I2C ind Rx and Tx modes:
      i2c_usr_slave_init(&i2cSlaveHeader);
      //6)When there is the BUSY I2C bug, clear I2C and init again:
        if (I2C1->SR2 & I2C_SR2_BUSY) {
      	  //when busy bug, clean it by reset:
           I2C1->CR1 |= I2C_CR1_SWRST;
      	   HAL_Delay(10);
      	   I2C1->CR1  = 0;
      	   	//NOTE: you can call here the  function  LL_I2C_Init(I2C_TypeDef *I2Cx, LL_I2C_InitTypeDef *I2C_InitStruct);
             //insted of the three next steps - it is inside the MX_I2C2_Init() or MX_I2C1_Init function.
      	      I2C1->CCR = 0xa0; //ATTENTION! data from LL initializer
              I2C1->TRISE = 0x21; //ATTENTION! data from LL initialyzer
              I2C1->CR2 = 0x20;//ATTENTION! data from LL initialyzer
      	  	  i2c_usr_slave_init(&i2cSlaveHeader);
         }
  
	//init a master (RCC, GPIO initialization must be done before)
			i2c_usr_master_init(&i2cMasterHeader);
			if (I2C2->SR2 & I2C_SR2_BUSY) {
				//when the busy bug, clean it by reset:
				 I2C2->CR1 |= I2C_CR1_SWRST;
				 HAL_Delay(10);
				 I2C2->CR1 = 0;
				//NOTE: you can call here the  function  LL_I2C_Init(I2C_TypeDef *I2Cx, LL_I2C_InitTypeDef *I2C_InitStruct);
				   //insted of the three next steps - it is inside the MX_I2C2_Init() or MX_I2C1_Init function.
				  I2C2->CCR = 0xa0; //ATTENTION! data from LL initializer
				  I2C2->TRISE = 0x21; //ATTENTION! data from LL initialyzer
				  I2C2->CR2 = 0x20;//ATTENTION! data from LL initialyzer
				 i2c_usr_master_init(&i2cMasterHeader);
			}

  /*
  NOTE: to start transmission, only set START bit in CR1 register
  The type of transaction defined by the WriteFlag member of the master handle-structure.
  This structure also define number data to transmit, address of slave device and other.
  The softwre get info from this structure after START byte.
  */


}

/*
█ █▄░█ ▀█▀ █▀▀ █▀█ █▀█ █░█ █▀█ ▀█▀ █▀ ░ █▀▀
█ █░▀█ ░█░ ██▄ █▀▄ █▀▄ █▄█ █▀▀ ░█░ ▄█ ▄ █▄▄
*/

extern volatile wrp_i2c_master_header i2cMasterHeader;
extern volatile wrp_i2c_slave_header i2cSlaveHeader;

///++++++++++++++++++ The salve event handler:++++++++++++++++++
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

	uint8_t tempVar;

			if (i2cSlaveHeader.hDevice->SR1 & I2C_SR1_ADDR) {
		      //Address sent (master mode)/matched (slave mode)
		       //clear flag
				tempVar = I2C1->SR2;
				if (i2cSlaveHeader.hDevice->SR2 & I2C_SR2_TRA) {
					///slave transmitter mode
					GPIOC->BSRR = GPIO_BSRR_BR13; //turn led on Tx (optional)
				} else {
					//slave receiver mode
					GPIOC->BSRR = GPIO_BSRR_BR13; //turn on led Rx (optional)
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
				GPIOC->BSRR = GPIO_BSRR_BS13; //turn LED off
			}

  /* USER CODE END I2C1_EV_IRQn 0 */

  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}
/// ++++++++++++++++++++++++++++The slave error handler+++++++++++++++++++++++++++++
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */
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
  /* USER CODE END I2C1_ER_IRQn 0 */

  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}
//++++++++++++++++++++++++The master event handler+++++++++++++
void I2C2_EV_IRQHandler(void)
{
	/* USER CODE BEGIN I2C2_EV_IRQn 0 */


			uint16_t tempVar, remainder;
		  /* USER CODE BEGIN I2C2_EV_IRQn 0 */
			if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_SB) {
				//START event has been ended
				//1)Write slave address

				 i2cMasterHeader.i2cRxMasterIndex = 0;
				 i2cMasterHeader.i2cTxMasterIndex = 0;

				 //2)Enable acknowledge
				 i2cMasterHeader.hDevice->CR1 |= I2C_CR1_ACK;
				 //3)write address - (b1-b7 address, b0 R/W)
				tempVar = (i2cMasterHeader.i2cMasterAddress << 1);

				 if (i2cMasterHeader.WriteFlag == 0) {
				  //4)master receiver mode - when WriteFlag = 0
					  tempVar |= 1;
				 }
				 //5)write temporary variable to dataRegister and start address transmission
	                 i2cMasterHeader.hDevice->DR = tempVar;
			}

			if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_ADDR) {
			      //Address has been  sent to the slave device
		            //init pointers-indexes
					i2cMasterHeader.i2cRxMasterIndex = 0;
					i2cMasterHeader.i2cTxMasterIndex = 0;
	                  //what is the I2C action type? (R/W)
					if (i2cMasterHeader.hDevice->SR2 & I2C_SR2_TRA) {
						/// ***master T R A N S M I T T E R mode***
						 //2)clear ADDR flag by reading SR2 reg
							tempVar = I2C2->SR2;
						//turn led on Tx (optional)
						GPIOC->BSRR = GPIO_BSRR_BR13;
					} else {
						//***master R E C E I V E R mode***
						//turn on led Rx (optional)
						 GPIOC->BSRR = GPIO_BSRR_BR13;
						if (i2cMasterHeader.i2cMasterRxPacketLength == 1) {
							  ///-----PACKET LENGTH = 1--------
							  //when one byte reception - clear ACK bit
							i2cMasterHeader.hDevice->CR1 &= ~I2C_CR1_ACK;
							    //clear address flag by reading SR1 and SR2:
							  tempVar = I2C2->SR2;
							    //and prepare stop
						    i2cMasterHeader.hDevice->CR1 |= I2C_CR1_STOP;
						} else if (i2cMasterHeader.i2cMasterRxPacketLength == 2){
							  ///------PACKET LENGTH = 2--------
							  //clear address flag by reading SR1 and SR2:
								tempVar = I2C2->SR2;
							//disable ACK
							 i2cMasterHeader.hDevice->CR1 &= ~I2C_CR1_ACK;
							 //enable POS
							 i2cMasterHeader.hDevice->CR1 |= I2C_CR1_POS;
							 //2)clear ADDR flag by reading SR2 reg

						} else if (i2cMasterHeader.i2cMasterRxPacketLength >= 3){
							  ///------PACKET LENGTH >= 3--------
							//2)clear ADDR flag by reading SR2 reg
							tempVar = I2C2->SR2;
						}
					}
			}

			if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_TXE) {
				tempVar = I2C2->SR2;
			     ///Data register empty (transmitters)
					//interrupt also fires firstly when the address has been sent
					//and affer each byte
				if (i2cMasterHeader.i2cTxMasterIndex >= i2cMasterHeader.i2cMasterTxPacketLength) {
					//when the last byte of a packet
					//generate STOP condition
					i2cMasterHeader.hDevice->CR1 |= I2C_CR1_STOP;
					i2cMasterHeader.hDevice->DR = 0xff; //clear flag
				} else {
		           //write new data into data register
					i2cMasterHeader.hDevice->DR =i2cMasterHeader.i2cTxMasterBuffer[i2cMasterHeader.i2cTxMasterIndex];
					i2cMasterHeader.i2cTxMasterIndex++;
				}

			}

			if (i2cMasterHeader.hDevice->SR1 & I2C_SR1_RXNE) {
				//calculate a remainder
				remainder = i2cMasterHeader.i2cMasterRxPacketLength - i2cMasterHeader.i2cRxMasterIndex;

				if (i2cMasterHeader.i2cMasterRxPacketLength == 1) {
					///-----PACKET LENGTH = 1--------
					//just read data, STOP had been set in ADDR event handler
					 i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;

				} else if ((i2cMasterHeader.i2cMasterRxPacketLength == 2) && (i2cMasterHeader.hDevice->SR1 & I2C_SR1_BTF)) {
					///-----PACKET LENGTH = 2--------
					//1)programming the STOP
					  i2cMasterHeader.hDevice->CR1 |= I2C_CR1_STOP;
					//2)read DR twice in sequence:
					 i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
					 i2cMasterHeader.i2cRxMasterIndex++;
					 i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
					//3)disable POS
					 i2cMasterHeader.hDevice->CR1 &= ~I2C_CR1_POS;
				} else if ((remainder > 3) && (i2cMasterHeader.i2cMasterRxPacketLength >=3 )){
					///-----PACKET LENGTH >= 3
					///1) Read data from 0 t0 PacketLength-3
					  i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
					  i2cMasterHeader.i2cRxMasterIndex++;
				} else if ( (i2cMasterHeader.hDevice->SR1 & I2C_SR1_BTF) && (i2cMasterHeader.i2cMasterRxPacketLength >=3) ){
					//------PACKET LENGTH >= 3-----read data PacketLen-2,PacketLen-1-----
                     //1) NACK
					   i2cMasterHeader.hDevice->CR1 &= ~I2C_CR1_ACK;
					 //2)Read PacketLen-2 data
						 i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
						 i2cMasterHeader.i2cRxMasterIndex++;
					//3)Send STOP
						 i2cMasterHeader.hDevice->CR1 |= I2C_CR1_STOP;
					 //2)Read PacketLen-1 data
						 i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
						 i2cMasterHeader.i2cRxMasterIndex++;
				} else if ((remainder == 1) && (i2cMasterHeader.i2cMasterRxPacketLength >=3 )){
					 ////------PACKET LENGTH >= 3-----read the last byte PacketLegth-0
						i2cMasterHeader.i2cRxMasterBuffer[i2cMasterHeader.i2cRxMasterIndex] = i2cMasterHeader.hDevice->DR;
						i2cMasterHeader.i2cRxMasterIndex++;
				}



			}

	  /* USER CODE END I2C2_EV_IRQn 0 */

	  /* USER CODE BEGIN I2C2_EV_IRQn 1 */

	  /* USER CODE END I2C2_EV_IRQn 1 */
	}

///+++++++++++++++++++++++++++++++++++the master error handler++++++++++++++++++++++++++++++++

void I2C2_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_ER_IRQn 0 */
	//save flags
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
