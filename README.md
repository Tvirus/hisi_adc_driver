# hisi_adc_driver
hi3516dv300 芯片adc驱动

**设置采样周期：**  
(单位ms，采样后上报input事件)  
echo 100 > /sys/class/input/inputx/period_ms

**使能通道0：**  
echo 1 > /sys/class/input/inputx/ch0_en

**获取通道0的值：**  
cat /sys/class/input/inputx/ch0_value
