#include "appl_sdram_test.h"
#include "bsp.h"

#define TEST_ADDRESS	0
#define TEST_BUF_SIZE	256

uint32_t bsp_TestExtSDRAM1(void)
{
    uint32_t i;
    uint32_t *pSRAM;
    uint8_t *pBytes;
    uint32_t err;
    const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

    /* write SRAM */
    pSRAM = (uint32_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < SDRAM_DEVICE01_SIZE / 4; i++)
    {
        *pSRAM++ = i;
    }

    /* read SRAM */
    err = 0;
    pSRAM = (uint32_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < SDRAM_DEVICE01_SIZE / 4; i++)
    {
        if (*pSRAM++ != i)
        {
            err++;
        }
    }

    if (err >  0)
    {
        return  (4 * err);
    }

    pSRAM = (uint32_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < SDRAM_DEVICE01_SIZE / 4; i++)
    {
        *pSRAM = ~*pSRAM;
        pSRAM++;
    }

    err = 0;
    pSRAM = (uint32_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < SDRAM_DEVICE01_SIZE / 4; i++)
    {
        if (*pSRAM++ != (~i))
        {
            err++;
        }
    }

    if (err >  0)
    {
        return (4 * err);
    }

    pBytes = (uint8_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        *pBytes++ = ByteBuf[i];
    }

    err = 0;
    pBytes = (uint8_t *)SDRAM_DEVICE01_ADDR;
    for (i = 0; i < sizeof(ByteBuf); i++)
    {
        if (*pBytes++ != ByteBuf[i])
        {
            err++;
        }
    }
    if (err >  0)
    {
        return err;
    }
    return 0;
}

void WriteSpeedTest(void)
{
	uint32_t start, end, cnt;
	uint32_t i, j;
	uint32_t iTime1, iTime2;
	uint32_t *pBuf;


	/* 设置初始化值并记下开始时间 */
	j = 0;
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	iTime1 = bsp_GetRunTime();	  
	start = DWT_CYCCNT;
	
	/* 以递增的方式写数据到SDRAM所有空间 */
	for (i = 1024*1024/4; i >0 ; i--)
	{
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	

		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;
		*pBuf++ = j++;	
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */
	
    /* 读取写入的是否出错 */
	j = 0;
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	for (i = 0; i < 1024*1024*8; i++)
	{
		if(*pBuf++ != j++)
		{
			printf("写入出错 j=%d\r\n", j);
			break;
		}
	}
		
	/* 打印速度 */
	printf("【32MB数据写耗时】: 方式一:%dms  方式二:%dms, 写速度: %dMB/s\r\n", 
	                  iTime2 - iTime1,  cnt/400000, (SDRAM_DEVICE01_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

void ReadSpeedTest(void)
{	
	uint32_t start, end, cnt;
	uint32_t i;
	uint32_t iTime1, iTime2;
	uint32_t *pBuf;
	__IO  uint32_t ulTemp; /* 设置为__IO类型，防止被MDK优化 */

	/* 设置初始化值并记下开始时间 */
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	iTime1 = bsp_GetRunTime();	
	start = DWT_CYCCNT;
	
	/* 读取SDRAM所有空间数据 */	
	for (i = 1024*1024/4; i >0 ; i--)
	{
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;

		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
		ulTemp = *pBuf++;
	}
	end = DWT_CYCCNT;
	cnt = end - start;
	iTime2 = bsp_GetRunTime();	/* 记下结束时间 */

	/* 打印速度 */
	printf("【32MB数据读耗时】: 方式一:%dms  方式二:%dms, 读速度: %dMB/s\r\n", 
	        iTime2 - iTime1,  cnt/400000, (SDRAM_DEVICE01_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

void ReadWriteTest(void)
{
	uint32_t i;
	uint32_t *pBuf;
	

	/* 写入测试数据0xAAAA5555 */
	pBuf = (uint32_t *)(SDRAM_DEVICE01_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)		
	{
		pBuf[i] = 0xAAAA5555;
	}
	
	printf("物理地址:0x%08X  大小: %d字节  显示: %d字节  数据如下: \r\n", SDRAM_DEVICE01_ADDR + TEST_ADDRESS, SDRAM_DEVICE01_SIZE, TEST_BUF_SIZE*4);
	
	/* 打印数据 */
	pBuf = (uint32_t *)(SDRAM_DEVICE01_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)
	{
		printf(" %04X", pBuf[i]);

		if ((i & 7) == 7)
		{
			printf("\r\n");		/* 每行显示32字节数据 */
		}
		else if ((i & 7) == 3)
		{
			printf(" - ");
		}
	}
}

