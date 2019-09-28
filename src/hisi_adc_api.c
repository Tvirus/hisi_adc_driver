#include "hisi_adc_api.h"
#include <asm/io.h>




#define HIADC_WRITE(addr, value)   ((*(volatile u32*)(addr)) = (value))
#define HIADC_READ(addr)           (*((volatile u32*)(addr)))



static void *lsadc_reg_base = NULL;
static void *lsadc_reg_iocfg1_base = NULL;







u32 hiadc_read_reg(u32 off)
{
    return HIADC_READ(lsadc_reg_base + off);
}

void hiadc_write_reg(u32 off, u32 value)
{
    HIADC_WRITE(lsadc_reg_base + off, value);
}

void hiadc_setbits(u32 off, u32 mask, u32 value)
{
    u32 read = 0;

    read = hiadc_read_reg(off);

    read &= ~mask;
    read |= value & mask;
    hiadc_write_reg(off, read);
}

void hiadc_cfg_ch0_io(void)
{
    HIADC_WRITE(lsadc_reg_iocfg1_base + 0x30, 0x430);
}
void hiadc_cfg_ch1_io(void)
{
    HIADC_WRITE(lsadc_reg_iocfg1_base + 0x34, 0x430);
}



/* 清中断，bit0对应通道0，bit1对应通道1 */
void hiadc_clean_int(u32 n)
{
    hiadc_write_reg(LSADC_REG_CTRL6, n & 0x3);
}

/* 查询并清除中断，bit0对应通道0，bit1对应通道1 */
u32 hiadc_get_int(void)
{
    u32 intr;

    intr = hiadc_read_reg(LSADC_REG_CTRL5);
    hiadc_clean_int(intr);
    return intr;
}

/* 开始 */
static void hiadc_start(void)
{
    hiadc_write_reg(LSADC_REG_CTRL7, 1);
}
/* 停止 */
static void hiadc_stop(void)
{
    hiadc_write_reg(LSADC_REG_CTRL8, 1);
}

/* 读通道0 */
u32 hiadc_get_ch0(void)
{
    return hiadc_read_reg(LSADC_REG_CTRL11) & LSADC_DATA_IN0_MASK;
}

/* 读通道1 */
u32 hiadc_get_ch1(void)
{
    return hiadc_read_reg(LSADC_REG_CTRL12) & LSADC_DATA_IN1_MASK;
}

/* 使能通道0 */
void hiadc_enable_ch0(int enable)
{
    if (enable)
    {
        hiadc_cfg_ch0_io();
        hiadc_setbits(LSADC_REG_CTRL0, CH_0_VLD, CH_0_VLD);
        hiadc_start();
    }
    else
    {
        hiadc_setbits(LSADC_REG_CTRL0, CH_0_VLD, 0);
        /* 两个通道都关闭后停止adc */
        if (0 == (hiadc_read_reg(LSADC_REG_CTRL0) & (CH_0_VLD | CH_1_VLD)))
            hiadc_stop();
    }
}
/* 使能通道1 */
void hiadc_enable_ch1(int enable)
{
    if (enable)
    {
        hiadc_cfg_ch1_io();
        hiadc_setbits(LSADC_REG_CTRL0, CH_1_VLD, CH_1_VLD);
        hiadc_start();
    }
    else
    {
        hiadc_setbits(LSADC_REG_CTRL0, CH_1_VLD, 0);
        /* 两个通道都关闭后停止adc */
        if (0 == (hiadc_read_reg(LSADC_REG_CTRL0) & (CH_0_VLD | CH_1_VLD)))
            hiadc_stop();
    }
}

/* 设置周期，单位ms */
int hiadc_set_period(u32 t)
{
    if ((((u32)(-1)) / 3000) <= t)
        return -1;

    hiadc_write_reg(LSADC_REG_CTRL2, t * 3000);
    return 0;
}

int hiadc_init(void)
{
    lsadc_reg_base = ioremap(LSADC_REG_BASE, 0x40);
    if (NULL == lsadc_reg_base)
        return -1;

    lsadc_reg_iocfg1_base = ioremap(0x111f0000, 0x40);
    if (NULL == lsadc_reg_iocfg1_base)
        return -1;

    hiadc_stop();

    /* 默认配置 */
    hiadc_write_reg(LSADC_REG_CTRL0,  DATA_DELTA(30) | DEGLITCH_BYPASS | MODEL_SEL); /* 误差范围，不使能虑毛刺功能，连续扫描 */
    hiadc_write_reg(LSADC_REG_CTRL1,  3000);  //3000对应1ms?
    hiadc_set_period(1000);                               /* 采样周期1s */
    hiadc_write_reg(LSADC_REG_CTRL9,  ACTIVE_BIT(10));    /* 10位精度 */
    hiadc_clean_int(CLR_INT_FLAG_IN0 | CLR_INT_FLAG_IN1); /* 清中断 */
    hiadc_write_reg(LSADC_REG_CTRL4,  INT_ENABLE);        /* 使能中断 */

    return 0;
}

void hiadc_deinit(void)
{
    iounmap(lsadc_reg_base);
}
