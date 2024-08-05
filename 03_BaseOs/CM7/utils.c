#include "utils.h"


//将小写字母转为大写字母,如果是数字,则保持不变.
/**
  * @brief  converts lowercase letters to uppercase letters, 
  *         leaving them unchanged if they are numbers
  * @param  ch letter
  * @retval uint8_t uppercase letter
  */
uint8_t char_upper(uint8_t ch)
{
    if (ch < 'A')
        return ch; // 数字,保持不变.
    if (ch >= 'a')
        return ch - 0x20; // 变为大写.
    else
        return ch; // 大写,保持不变
}


/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the buffer to fill
  * @param  uwBufferLenght: size of the buffer to fill
  * @param  uwOffset: first value to fill on the buffer
  * @retval None
  */
void Fill_Buffer(uint8_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++)
    {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 1: pBuffer identical to pBuffer1
  *         0: pBuffer differs from pBuffer1
  */
uint8_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength)
{
    while (BufferLength--)
    {
        if (*pBuffer1 != *pBuffer2)
        {
            return 1;
        }

        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}
