#include "rtx_lib.h"
#include "bsp_memory.h"

#if(1 == BSP_USE_MEMORY_MALLOC)
/* DTCM, 仅CPU可以访问, 128KB , 其中32K用作内存池 */
mem_head_t *DTCMUsed;
uint64_t AppMallocDTCM[32*1024/8]__attribute__((section (".RAM_DTCM")));

/* D1域, AXI SRAM, 512KB , 其中384K用作内存池 */
mem_head_t *AXISRAMUsed;
uint64_t AppMallocAXISRAM[384*1024/8]__attribute__((section (".RAM_D1")));

/* D2域, 128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)  */
mem_head_t *SRAM1Used; 
uint64_t AppMallocSRAM1[288*1024/8]__attribute__((section (".RAM_D2")));

/* D3域, SRAM4, 64KB */
mem_head_t *SRAM4Used;
uint64_t AppMallocSRAM4[64*1024/8]__attribute__((section (".RAM_D3")));
#endif

void InitMem(void)
{
#if(1 == BSP_USE_MEMORY_MALLOC)
    /* 初始化动态内存空间 */
    osRtxMemoryInit(AppMallocDTCM,    sizeof(AppMallocDTCM));
    osRtxMemoryInit(AppMallocAXISRAM, sizeof(AppMallocAXISRAM));
    osRtxMemoryInit(AppMallocSRAM1,   sizeof(AppMallocSRAM1));
    osRtxMemoryInit(AppMallocSRAM4,   sizeof(AppMallocSRAM4));
#endif
}


void* AllocMemZone(uint8_t memZone, uint32_t size)
{
    uint8_t *ptr = NULL;

#if(1 == BSP_USE_MEMORY_MALLOC)

    if (MEM_ZONE_D1_AXIM == memZone)
    {
        ptr = osRtxMemoryAlloc(AppMallocAXISRAM, size, 0);
        AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
    }
    else if (MEM_ZONE_D2 == memZone)
    {
        ptr = osRtxMemoryAlloc(AppMallocSRAM1, size, 0);
        SRAM1Used = MemHeadPtr(AppMallocSRAM1);
    }
    else if (MEM_ZONE_D3 == memZone)
    {
        ptr = osRtxMemoryAlloc(AppMallocSRAM4, size, 0);
        SRAM4Used = MemHeadPtr(AppMallocSRAM4);
    }
    else if (MEM_ZONE_DTCM == memZone)
    {
        ptr = osRtxMemoryAlloc(AppMallocDTCM, size, 0);
        DTCMUsed = MemHeadPtr(AppMallocDTCM);
    }
#endif

    return ptr;
}

void FreeMemZone(uint8_t memZone, void *mem)
{
#if(1 == BSP_USE_MEMORY_MALLOC)
    if (MEM_ZONE_D1_AXIM == memZone)
    {
        osRtxMemoryFree(AppMallocAXISRAM, mem);
        AXISRAMUsed = MemHeadPtr(AppMallocAXISRAM);
    }
    else if (MEM_ZONE_D2 == memZone)
    {
        osRtxMemoryFree(AppMallocSRAM1, mem);
        SRAM1Used = MemHeadPtr(AppMallocSRAM1);
    }
    else if (MEM_ZONE_D3 == memZone)
    {
        osRtxMemoryFree(AppMallocSRAM4, mem);
        SRAM4Used = MemHeadPtr(AppMallocSRAM4);
    }
    else if (MEM_ZONE_DTCM == memZone)
    {
        osRtxMemoryFree(AppMallocDTCM, mem);
        DTCMUsed = MemHeadPtr(AppMallocDTCM);
    }
#endif
}

void *AllocMemD1(unsigned int size)
{
    return (void*)AllocMemZone(MEM_ZONE_D1_AXIM, size);
}

void FreeMemD1(void *memblk)
{
    FreeMemZone(MEM_ZONE_D1_AXIM, memblk);
}
