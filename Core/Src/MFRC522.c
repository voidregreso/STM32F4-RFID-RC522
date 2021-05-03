#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "MFRC522.h"
#include "spi.h"

//------------------------------------------------------
/*
 * Function Nameï¼šWrite_MFRC5200
 * Function Description: To a certain MFRC522 register to write a byte of data
 * Input Parametersï¼šaddr - register address; val - the value to be written
 * Return value: None
 */
void Write_MFRC522(uint8_t addr, uint8_t val) {
    //uint32_t rx_bits;
    uint8_t addr_bits = (((addr << 1) & 0x7E));
    //uint8_t rx_bits;
    // set the select line so we can start transferring
    //  MSS_SPI_set_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &addr_bits, 1, 500);
    HAL_SPI_Transmit(&hspi1, &val, 1, 500);
    // clear the select line-- we are done here
    //  MSS_SPI_clear_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
    // burn some time
    // volatile uint32_t ticks;
    // for(ticks=0; ticks < 5000; ++ticks);
}
//-----------------------------------------------
/*
 * Function Name: Read_MFRC522
 * Description: From a certain MFRC522 read a byte of data register
 * Input Parameters: addr - register address
 * Returns: a byte of data read from the
 */
uint8_t Read_MFRC522(uint8_t addr) {
    //uint32_t rx_bits;
    uint8_t rx_bits;
    uint8_t addr_bits = (((addr << 1) & 0x7E) | 0x80);
    // set the select line so we can start transferring
    //  MSS_SPI_set_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &addr_bits, 1, 500);
    HAL_SPI_Receive(&hspi1, &rx_bits, 1, 500);
    // clear the select line-- we are done here
    //  MSS_SPI_clear_slave_select( &g_mss_spi1, MSS_SPI_SLAVE_0 );
    // burn some time
    // volatile uint32_t ticks;
    // for(ticks=0; ticks < 5000; ++ticks);
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
    return (uint8_t) rx_bits; // return the rx bits, casting to an 8 bit int and chopping off the upper 24 bits
}
//--------------------------------------------------------
/*
 * Function Nameï¼šSetBitMask
 * Description: Set RC522 register bit
 * Input parameters: reg - register address; mask - set value
 * Return value: None
 */
void SetBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  // set bit mask
}
//
/*
 * Function Name: ClearBitMask
 * Description: clear RC522 register bit
 * Input parameters: reg - register address; mask - clear bit value
 * Return value: None
*/
void ClearBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
}

//-----------------------------------------------
/*
 * Function Nameï¼šAntennaOn
 * Description: Open antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: None
 * Return value: None
 */
void AntennaOn(void) {
    SetBitMask(TxControlReg, 0x03);
}


/*
  * Function Name: AntennaOff
  * Description: Close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
  * Input: None
  * Return value: None
 */
void AntennaOff(void) {
    ClearBitMask(TxControlReg, 0x03);
}


/*
 * Function Name: ResetMFRC522
 * Description: Reset RC522
 * Input: None
 * Return value: None
 */
void MFRC522_Reset(void) {
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}
//--------------------------------------------------
/*
 * Function Nameï¼šInitMFRC522
 * Description: Initialize RC522
 * Input: None
 * Return value: None
*/
void MFRC522_Init(void) {
    //  MSS_GPIO_set_output( MSS_GPIO_1, 1 );
    MFRC522_Reset();
    // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    Write_MFRC522(TModeReg, 0x80); // 0x8D);      // Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_MFRC522(TPrescalerReg, 0xA9); //0x34); // TModeReg[3..0] + TPrescalerReg
    Write_MFRC522(TReloadRegL, 0x03); //30);
    Write_MFRC522(TReloadRegH, 0xE8); //0);
    Write_MFRC522(TxAutoReg, 0x40);     // force 100% ASK modulation
    Write_MFRC522(ModeReg, 0x3D);       // CRC Initial value 0x6363
    AntennaOn();
}
//------------------------------------------------------------------
/*
 * Function Nameï¼šMFRC522_Request
 * Description: Find cards, read the card type number
 * Input parameters: reqMode - find cards way
 *   TagType - Return Card Type
 *    0x4400 = Mifare_UltraLight
 *    0x0400 = Mifare_One(S50)
 *    0x0200 = Mifare_One(S70)
 *    0x0800 = Mifare_Pro(X)
 *    0x4403 = Mifare_DESFire
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
    uint8_t status;
    uint32_t backBits; // The received data bits
    Write_MFRC522(BitFramingReg, 0x07);   // TxLastBists = BitFramingReg[2..0]
    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if ((status != MI_OK) || (backBits != 0x10)) {
        status = MI_ERR;
    }
    return status;
}

//-----------------------------------------------
/*
 * Function Name: MFRC522_ToCard
 * Description: RC522 and ISO14443 card communication
 * Input Parameters: command - MF522 command word,
 *			 sendData--RC522 sent to the card by the data
 *			 sendLen--Length of data sent
 *			 backData--Received the card returns data,
 *			 backLen--Return data bit length
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint32_t *backLen) {
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    switch (command) {
    case PCD_AUTHENT: {   // Certification cards close
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
    }
    case PCD_TRANSCEIVE: { // Transmit FIFO data
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    }
    default:
        break;
    }
    Write_MFRC522(CommIEnReg, irqEn | 0x80); // Interrupt request
    ClearBitMask(CommIrqReg, 0x80);         // Clear all interrupt request bit
    SetBitMask(FIFOLevelReg, 0x80);         // FlushBuffer=1, FIFO Initialization
    Write_MFRC522(CommandReg, PCD_IDLE);    // NO action; Cancel the current command
    // Writing data to the FIFO
    for (i = 0; i < sendLen; i++) {
        Write_MFRC522(FIFODataReg, sendData[i]);
    }
    // Execute the command
    Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE) {
        SetBitMask(BitFramingReg, 0x80);      // StartSend=1,transmission of data starts
    }
    // Waiting to receive data to complete
    i = 2000;	// i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
    do {
        // CommIrqReg[7..0]
        // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_MFRC522(CommIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));
    ClearBitMask(BitFramingReg, 0x80);      // StartSend=0
    if (i != 0) {
        if(!(Read_MFRC522(ErrorReg) & 0x1B)) { // BufferOvfl Collerr CRCErr ProtecolErr
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;             // ??
            }
            if (command == PCD_TRANSCEIVE) {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits) {
                    *backLen = (n - 1) * 8 + lastBits;
                } else {
                    *backLen = n * 8;
                }
                if (n == 0) {
                    n = 1;
                }
                if (n > MAX_LEN) {
                    n = MAX_LEN;
                }
                // Reading the received data in FIFO
                for (i = 0; i < n; i++) {
                    backData[i] = Read_MFRC522(FIFODataReg);
                }
            }
        } else {
            //printf("~~~ buffer overflow, collerr, crcerr, or protecolerr\r\n");
            status = MI_ERR;
        }
    } else {
        //printf("~~~ request timed out\r\n");
    }
    return status;
}


//---------------------------------------------------------------

/*
 * Function Name: MFRC522_Anticoll
 * Description: Anti-collision detection, reading selected card serial number card
 * Input parameters: serNum - returns 4 bytes card serial number, the first 5 bytes for the checksum byte
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_Anticoll(uint8_t *serNum) {
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint32_t unLen;
    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
    Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    if (status == MI_OK) {
        //Check card serial number
        for (i = 0; i < 4; i++) {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i]) {
            status = MI_ERR;
        }
    }
    //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1
    return status;
}
//---------------------------------------------------



/*
 * Function Name: MFRC522_Read
 * Description: Read block data
 * Input parameters: blockAddr - block address; recvData - read block data
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData) {
    uint8_t status;
    uint32_t unLen;
    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalulateCRC(recvData, 2, &recvData[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    if ((status != MI_OK) || (unLen != 0x90)) {
        status = MI_ERR;
    }
    return status;
}


/*
 * Function Name: MFRC522_Write
 * Description: Write block data
 * Input parameters: blockAddr - block address; writeData - to 16-byte data block write
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData) {
    uint8_t status;
    uint32_t recvBits;
    uint8_t i;
    uint8_t buff[18];
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
    if ((status != MI_OK)) { // || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        status = MI_ERR;
    }
    if (status == MI_OK) {
        for (i = 0; i < 16; i++) {	//Data to the FIFO write 16Byte
            buff[i] = *(writeData + i);
        }
        CalulateCRC(buff, 16, &buff[16]);
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        if ((status != MI_OK)) { // || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
            status = MI_ERR;
        }
    }
    return status;
}

/*
 * Function Name: CalulateCRC
 * Description: CRC calculation with MF522
 * Input parameters: pIndata - To read the CRC data, len - the data length, pOutData - CRC calculation results
 * Return value: None
 */
void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData) {
    uint8_t i, n;
    ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80);			//Clear the FIFO pointer
    //Write_MFRC522(CommandReg, PCD_IDLE);
    //Writing data to the FIFO
    for (i = 0; i < len; i++) {
        Write_MFRC522(FIFODataReg, *(pIndata + i));
    }
    Write_MFRC522(CommandReg, PCD_CALCCRC);
    //Wait CRC calculation is complete
    i = 0xFF;
    do {
        n = Read_MFRC522(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));			//CRCIrq = 1
    //Read CRC calculation result
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}

//--------------------------------------------------------------
/*
 * Function Name: MFRC522_Auth
 * Description: Verify card password
 * Input parameters: authMode - Password Authentication Mode
                 0x60 = A key authentication
                 0x61 = Authentication Key B
             BlockAddr--Block address
             Sectorkey--Sector password
             serNum--Card serial number, 4-byte
 * Return value: the successful return MI_OK
 */
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum) {
    uint8_t status;
    uint32_t recvBits;
    uint8_t i;
    uint8_t buff[12];
    //Verify the command block address + sector + password + card serial number
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i = 0; i < 6; i++) {
        buff[i + 2] = *(Sectorkey + i);
    }
    for (i = 0; i < 4; i++) {
        buff[i + 8] = *(serNum + i);
    }
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))) {
        status = MI_ERR;
    }
    return status;
}

//----------------------------------
/*
 * Function Name: MFRC522_SelectTag
 * Description: election card, read the card memory capacity
 * Input parameters: serNum - Incoming card serial number
 * Return value: the successful return of card capacity
 */
uint8_t MFRC522_SelectTag(uint8_t *serNum) {
    uint8_t i;
    uint8_t status;
    uint8_t size;
    uint32_t recvBits;
    uint8_t buffer[9];
    //ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i = 0; i < 5; i++) {
        buffer[i + 2] = *(serNum + i);
    }
    CalulateCRC(buffer, 7, &buffer[7]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    if ((status == MI_OK) && (recvBits == 0x18)) {
        size = buffer[0];
    } else {
        size = 0;
    }
    return size;
}
//----------------------------------------------------

/*
 * Function Name: MFRC522_Halt
 * Description: Command card into hibernation
 * Input: None
 * Return value: None
 */
void MFRC522_Halt(void) {
    uint32_t unLen;
    uint8_t buff[4];
    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
//--------------------------------------
void MFRC522_StopCrypto1(void) {
    // Clear MFCrypto1On bit
    ClearBitMask(Status2Reg, 0x08); // Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved   MFCrypto1On ModemState[2:0]
}
