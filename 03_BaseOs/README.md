# BaseOs
Based on STM32H747I-DISCO Board

## 调试记录：

1. 当使用SD的DMA传输方式时，其数据buffer需在SDMMC可直接访问到的RAM空间内，否则会报TX/RX_UNDERRUN，因为访问其它域的RAM会因为FIFO传输速度过快，导致FIFO取数据时RAM BUFFER的数据还没传输到FIFO中，造成FIFO空的情况，最终造成UNDERRUN错误。

> ​	**关于DSMMC可直接访问到的空间，详情参考user manual Table 2**

​	在本次appSd.c例程中，将其buffer指定到了AXI RAM区域中。
