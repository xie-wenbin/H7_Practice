# BaseOs
Based on STM32H747I-DISCO Board

## 芯片RAM分布：
Ram Memmap

• Up to 864 Kbytes of System SRAM
• 128 Kbytes of data TCM RAM 
• 64 Kbytes of instruction TCM RAM
• 4 Kbytes of backup SRAM

| RAM Region           | address                   | size(Byte) | access core                                      |
| -------------------- | ------------------------- | ---------- | ------------------------------------------------ |
| ITCM                 | 0x0 - 0xFFFF              | 64K        | M7 only                                          |
| Internal Flash bank1 | 0x0800_0000 - 0x0807_FFFF | 512K       | [M7]  [M4 - D2toD1 AHB  -  ART]                  |
| Internal Flash bank2 | 0x0810_0000 - 0x0817_FFFF | 512K       | [M7]  [M4 - D2toD1 AHB -  ART]                   |
| SRAM1                | 0x1000_0000 - 0x1001_FFFF | 128K       | [M4 S/D/I bus]  该地址为M4核内映射地址           |
|                      | 0x3000_0000 - 0x3001_FFFF | ~          | [M7 - AXIM - D1toD2 AHB]  该地址为M7核内映射地址 |
| SRAM2                | 0x1002_0000 - 0x1003_FFFF | 128K       | [M4 S/D/I bus]  该地址为M4核内映射地址           |
|                      | 0x3002_0000 - 0x3003_FFFF | ~          | [M7 - AXIM - D1toD2 AHB]  该地址为M7核内映射地址 |
| SRAM3                | 0x1004_0000 - 0x1004_7FFF | 32K        | [M4 S/D/I bus]  该地址为M4核内映射地址           |
|                      | 0x3004_0000 - 0x3004_7FFF | ~          | [M7 - AXIM - D1toD2 AHB]  该地址为M7核内映射地址 |
| SRAM4                | 0x3800_0000 - 0x3800_FFFF | 64K        | [M7 - AXIM - D1toD3 AHB]                         |
| DTCM                 | 0x2000_0000 - 0x2001_FFFF | 128K       | M7 only                                          |
| AXI SRAM             | 0x2400_0000 - 0x2407_FFFF | 512K       | [M7]  [M4 - D2toD1 AHB -  ART]                   |
| Backup SRAM          | 0x3880_0000 - 0x3880_0FFF | 4K         | [M7 - AXIM - D1toD3 AHB]                         |
|                      |                           |            |                                                  |



## 调试记录：

1. 当使用SD的DMA传输方式时，其数据buffer需在SDMMC可直接访问到的RAM空间内，否则会报TX/RX_UNDERRUN，因为访问其它域的RAM会因为FIFO传输速度过快，导致FIFO取数据时RAM BUFFER的数据还没传输到FIFO中，造成FIFO空的情况，最终造成UNDERRUN错误。

> ​	**关于DSMMC可直接访问到的空间，详情参考user manual Table 2**

​	在本次appSd.c例程中，将其buffer指定到了AXI RAM区域中。
