/**
  ******************************************************************************
  * @file    uSDCard/src/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    30-September-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Portions COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    main.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   This example provides a basic example of how to use the SDIO 
  *          firmware library and an associate driver to perform read/write 
  *          operations on the SD Card memory (SD Card V1.0, V1.1, V2.0 and  
  *          SDHC (High Capacity) protocol)that could be mounted on the board.   
  *          Modified to support the STM32F4DISCOVERY and STM32F4DIS-BB modules. 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery_sdio_sd.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include "stm32f4_discovery_debug.h"

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SDIO_uSDCard
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      100  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

#define SD_OPERATION_ERASE          0
#define SD_OPERATION_BLOCK          1
#define SD_OPERATION_MULTI_BLOCK    2 
#define SD_OPERATION_END            3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Buffer_Block_Tx[BLOCK_SIZE], Buffer_Block_Rx[BLOCK_SIZE];
uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
SD_Error Status = SD_OK;
__IO uint32_t SDCardOperation = SD_OPERATION_BLOCK;

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void SD_EraseTest(void);
void SD_SingleBlockTest(void);
void SD_MultiBlockTest(void);
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);
static void Delay(__IO uint32_t nCount);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Delay
  * @param  None
  * @retval None
  */
static void Delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for (index = (100000 * nCount); index != 0; index--);
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{

  // Open VCCB PB10 HIGH  
  /* GPIOE Peripheral clock enable */
  GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* Configure PB10 in output push-pull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_10); //GPIO HIGH

  

  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */

  /* Initialize LEDs available on STM324xG-EVAL board *************************/
  // STM_EVAL_LEDInit(LED3);
  // STM_EVAL_LEDInit(LED4);
  // STM_EVAL_LEDInit(LED5);
  // STM_EVAL_LEDInit(LED6);
	// STM_EVAL_LEDOff(LED4);  

  /* Interrupt Config */
  NVIC_Configuration();

  STM32f4_Discovery_Debug_Init();
  Delay(500);

  // LED blink test
  // while(1){
  //   STM_EVAL_LEDToggle(LED4);
  //   Delay(0xFFFFFF);
  // }
  

  printf("Init SD card \n\r");  
  /*------------------------------ SD Init ---------------------------------- */
  if ((Status = SD_Init()) != SD_OK) {
    printf("could not open init SD card \n\r");  
  }else{
    printf("init SD card ok!\n\r");
  }
        
  printf("SDCardOperation: %d\n\r", SDCardOperation);     
  while((Status == SD_OK) && (SDCardOperation != SD_OPERATION_END) && (SD_Detect()== SD_PRESENT))
  // while((Status == SD_OK) && (SDCardOperation != SD_OPERATION_END))
  {
    printf("Start test...\n\r");
    switch(SDCardOperation)
    {
      /*-------------------------- SD Erase Test ---------------------------- */
      case (SD_OPERATION_ERASE):
      {
        printf("SD_OPERATION_ERASE \n\r");
        SD_EraseTest();
        SDCardOperation = SD_OPERATION_BLOCK;
        break;
      }
      /*-------------------------- SD Single Block Test --------------------- */
      case (SD_OPERATION_BLOCK):
      {
        printf("SD_OPERATION_BLOCK \n\r");
        SD_SingleBlockTest();
        SDCardOperation = SD_OPERATION_MULTI_BLOCK;
        break;
      }       
      /*-------------------------- SD Multi Blocks Test --------------------- */
      case (SD_OPERATION_MULTI_BLOCK):
      {
        printf("SD_OPERATION_MULTI_BLOCK \n\r");
        SD_MultiBlockTest();
        SDCardOperation = SD_OPERATION_END;
        break;
      }              
    }
  }
  
  /* Infinite loop */
  while (1){
    printf("loop \n\r");
    Delay(500);
  }
}

/**
  * @brief  Configures SDIO IRQ channel.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_Init(&NVIC_InitStructure);  
}

/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
void SD_EraseTest(void)
{  
  /*------------------- Block Erase ------------------------------------------*/
  if (Status == SD_OK) {
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    Status = SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
    printf("SD_Erase: %d\r\n", Status);
  }

  if (Status == SD_OK) {    
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    printf("SD_ReadMultiBlocks: %d\r\n", Status);

    /* Check if the Transfer is finished */    
    Status = SD_WaitReadOperation();
    printf("SD_WaitReadOperation: %d\r\n", Status);

    /* Wait until end of DMA transfer */
    while(SD_GetStatus() != SD_TRANSFER_OK){
      Delay(500);
    }
  }

  /* Check the correctness of erased blocks */
  if (Status == SD_OK) {    
    EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
    printf("eBuffercmp: %d\r\n", EraseStatus);
  }
  
  if (EraseStatus == PASSED) {
    printf("Test PASSED\r\n");
  } else {
    printf("Test FAILED\r\n");    
  }
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
void SD_SingleBlockTest(void)
{
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE, 0x320F);

  if (Status == SD_OK) {
    /* Write block of 512 bytes on address 0 */
    Status = SD_WriteBlock(Buffer_Block_Tx, 0x00, BLOCK_SIZE);
    printf("SD_WriteBlock: %d\r\n",Status );
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();
    printf("SD_WaitWriteOperation: %d\r\n",Status );
    // while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK) {
    /* Read block of 512 bytes from address 0 */
    Status = SD_ReadBlock(Buffer_Block_Rx, 0x00, BLOCK_SIZE);
    printf("SD_WritSD_ReadBlockeBlock: %d\r\n",Status );
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    printf("SD_WaitReadOperation: %d\r\n",Status);
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK) {
    TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE);
    printf("TransferStatus1: %d\r\n",TransferStatus1 );
  }
  
  if (TransferStatus1 == PASSED) {
    STM_EVAL_LEDOn(LED4);
    printf("PASSED!\r\n");
  } else {
    printf("FAILED!\r\n");
    STM_EVAL_LEDOff(LED4);
    STM_EVAL_LEDOn(LED6);    
  }
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
void SD_MultiBlockTest(void)
{
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

  if (Status == SD_OK) {
    /* Write multiple block of many bytes on address 0 */
    Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    printf("SD_WriteMultiBlocks: %d\r\n",Status );
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();
    printf("SD_WaitWriteOperation: %d\r\n",Status );
    // while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK) {
    /* Read block of many bytes from address 0 */
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    printf("SD_ReadMultiBlocks: %d\r\n",Status );
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    printf("SD_WaitReadOperation: %d\r\n",Status );
    // while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK) {
    TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
    printf("TransferStatus2: %d\r\n",TransferStatus2 );
  }
  
  if(TransferStatus2 == PASSED) {
    printf("PASSED!\r\n" );
    STM_EVAL_LEDOn(LED5);
  } else {
    printf("FAILE!\r\n");
    STM_EVAL_LEDOff(LED5);
    STM_EVAL_LEDOn(LED6);    
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer1 identical to pBuffer2
  *         FAILED: pBuffer1 differs from pBuffer2
  */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2) {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}

/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  * @retval None
  */
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++) {
    pBuffer[index] = index + Offset;
  }
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00)) {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/