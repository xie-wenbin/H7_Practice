#ifndef BSP_MEMORY_H_
#define BSP_MEMORY_H_

#include "rtx_lib.h"

#define BSP_USE_MEMORY_MALLOC  1

typedef enum
{
    MEM_ZONE_D1_AXIM = 0,
    MEM_ZONE_D2,
    MEM_ZONE_D3,
    MEM_ZONE_DTCM,
    MEM_ZONE_MAX_NUM
} Memory_Zone_Enum;

void InitMem(void);
void* AllocMemZone(uint8_t memZone, uint32_t size);
void FreeMemZone(uint8_t memZone, void *mem);

void *AllocMemD1(unsigned int size);
void FreeMemD1(void *memblk);
#endif /* BSP_MEMORY_H_ */
