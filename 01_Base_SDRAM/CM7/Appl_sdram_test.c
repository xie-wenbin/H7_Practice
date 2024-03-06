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


	/* ���ó�ʼ��ֵ�����¿�ʼʱ�� */
	j = 0;
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	iTime1 = bsp_GetRunTime();	  
	start = DWT_CYCCNT;
	
	/* �Ե����ķ�ʽд���ݵ�SDRAM���пռ� */
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
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	
    /* ��ȡд����Ƿ���� */
	j = 0;
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	for (i = 0; i < 1024*1024*8; i++)
	{
		if(*pBuf++ != j++)
		{
			printf("д����� j=%d\r\n", j);
			break;
		}
	}
		
	/* ��ӡ�ٶ� */
	printf("��32MB����д��ʱ��: ��ʽһ:%dms  ��ʽ��:%dms, д�ٶ�: %dMB/s\r\n", 
	                  iTime2 - iTime1,  cnt/400000, (SDRAM_DEVICE01_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

void ReadSpeedTest(void)
{	
	uint32_t start, end, cnt;
	uint32_t i;
	uint32_t iTime1, iTime2;
	uint32_t *pBuf;
	__IO  uint32_t ulTemp; /* ����Ϊ__IO���ͣ���ֹ��MDK�Ż� */

	/* ���ó�ʼ��ֵ�����¿�ʼʱ�� */
	pBuf = (uint32_t *)SDRAM_DEVICE01_ADDR;
	iTime1 = bsp_GetRunTime();	
	start = DWT_CYCCNT;
	
	/* ��ȡSDRAM���пռ����� */	
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
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ�ٶ� */
	printf("��32MB���ݶ���ʱ��: ��ʽһ:%dms  ��ʽ��:%dms, ���ٶ�: %dMB/s\r\n", 
	        iTime2 - iTime1,  cnt/400000, (SDRAM_DEVICE01_SIZE / 1024 /1024 * 1000) / (iTime2 - iTime1));
}

void ReadWriteTest(void)
{
	uint32_t i;
	uint32_t *pBuf;
	

	/* д���������0xAAAA5555 */
	pBuf = (uint32_t *)(SDRAM_DEVICE01_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)		
	{
		pBuf[i] = 0xAAAA5555;
	}
	
	printf("�����ַ:0x%08X  ��С: %d�ֽ�  ��ʾ: %d�ֽ�  ��������: \r\n", SDRAM_DEVICE01_ADDR + TEST_ADDRESS, SDRAM_DEVICE01_SIZE, TEST_BUF_SIZE*4);
	
	/* ��ӡ���� */
	pBuf = (uint32_t *)(SDRAM_DEVICE01_ADDR + TEST_ADDRESS);
	for (i = 0; i < TEST_BUF_SIZE; i++)
	{
		printf(" %04X", pBuf[i]);

		if ((i & 7) == 7)
		{
			printf("\r\n");		/* ÿ����ʾ32�ֽ����� */
		}
		else if ((i & 7) == 3)
		{
			printf(" - ");
		}
	}
}

