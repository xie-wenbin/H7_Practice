#ifndef UTILS_H_
#define UTILS_H_

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdint.h>
#include  <math.h>


uint8_t char_upper(uint8_t ch);
void Fill_Buffer(uint8_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset);
uint8_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength);

#endif /* UTILS_H_ */
