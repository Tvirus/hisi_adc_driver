/*
*  HI3516DV300 LSADC REG
*/

#ifndef __HI_ADC_API_H__
#define __HI_ADC_API_H__


#include <linux/types.h>




extern u32 hiadc_read_reg(u32 off);
extern void hiadc_write_reg(u32 off, u32 value);
extern u32 hiadc_get_int(void);
extern void hiadc_clean_int(u32 n);
extern u32 hiadc_get_ch0(void);
extern u32 hiadc_get_ch1(void);
extern int hiadc_init(void);
extern void hiadc_deinit(void);
extern void hiadc_enable_ch0(int enable);
extern void hiadc_enable_ch1(int enable);
extern int hiadc_set_period(u32 t);





#define LSADC_REG_BASE             0x120E0000
#define LSADC_REG_CTRL0                0x0000    /* LSADC配置寄存器 */
#define LSADC_REG_CTRL1                0x0004    /* 滤毛刺配置寄存器 */
#define LSADC_REG_CTRL2                0x0008    /* 扫描间隔配置寄存器 */
#define LSADC_REG_CTRL4                0x0010    /* 中断使能寄存器 */
#define LSADC_REG_CTRL5                0x0014    /* 中断状态寄存器 */
#define LSADC_REG_CTRL6                0x0018    /* 中断清除寄存器 */
#define LSADC_REG_CTRL7                0x001C    /* start配置寄存器 */
#define LSADC_REG_CTRL8                0x0020    /* stop配置寄存器 */
#define LSADC_REG_CTRL9                0x0024    /* 转换结果精度寄存器 */
#define LSADC_REG_CTRL10               0x0028    /* LSADC_ZERO寄存器 */
#define LSADC_REG_CTRL11               0x002C    /* 通道0数据寄存器 */
#define LSADC_REG_CTRL12               0x0030    /* 通道0数据寄存器 */



//LSADC_REG_CTRL0
#define LSADC_DATA_DELTA_SHIFT         20
#define LSADC_DATA_DELTA_MASK          0xf         /* LSADC转换结果误差范围 */
#define DEGLITCH_BYPASS               (0x1 << 17) /* 滤毛刺功能bypass */
#define LSADC_RESET                   (0x1 << 15) /* 设置LSADC进入复位状态 */
#define MODEL_SEL                     (0x1 << 13) /* LSADC扫描模式选择 */
#define CH_1_VLD                      (0x1 << 9)  /* LSADC通道1是否有效 */
#define CH_0_VLD                      (0x1 << 8)  /* LSADC通道0是否有效 */


//LSADC_REG_CTRL4
#define INT_ENABLE                    (0x1 << 0)  /* 扫描值有效中断使能位 */

//LSADC_REG_CTRL5
#define LSADC_AUTO_BUSY               (0x1 << 4)  /* 扫描值有效中断使能位 */
#define INT_FLAG_IN1                  (0x1 << 1)  /* 通道1扫描值有效中断标志位 */
#define INT_FLAG_IN0                  (0x1 << 0)  /* 通道0扫描值有效中断标志位 */

//LSADC_REG_CTRL6
#define CLR_INT_FLAG_IN1              (0x1 << 1)  /* 通道1中断清除寄存器 */
#define CLR_INT_FLAG_IN0              (0x1 << 0)  /* 通道0中断清除寄存器 */

//LSADC_REG_CTRL9
#define LSADC_ACTIVE_BIT_SHIFT         0
#define LSADC_ACTIVE_BIT_MASK          0x3ff  /* LSADC转换结果精度 */

//LSADC_REG_CTRL10
#define LSADC_ZERO_SHIFT               0
#define LSADC_ZERO_MASK                0x3ff  /* 键盘无键按下时LSADC的值 */

//LSADC_REG_CTRL11
#define LSADC_DATA_IN0_SHIFT           0
#define LSADC_DATA_IN0_MASK            0x3ff  /* LSADC通道0扫描值 */

//LSADC_REG_CTRL12
#define LSADC_DATA_IN1_SHIFT           0
#define LSADC_DATA_IN1_MASK            0x3ff  /* LSADC通道1扫描值 */




#define DATA_DELTA(x)              (((x) & (LSADC_DATA_DELTA_MASK)) << (LSADC_DATA_DELTA_SHIFT))    /* 获取转换结果误差范围的寄存器位设置 */
#define ACTIVE_BIT(n)              ((((n) <= 10) && ((n) > 0))? ((LSADC_ACTIVE_BIT_MASK) & ((0xffffffff << (10 - (n))))) << (LSADC_ACTIVE_BIT_SHIFT): (ACTIVE_BIT_10))
#define ACTIVE_BIT_1               0x200
#define ACTIVE_BIT_2               0x300
#define ACTIVE_BIT_3               0x380
#define ACTIVE_BIT_4               0x3c0
#define ACTIVE_BIT_5               0x3e0
#define ACTIVE_BIT_6               0x3f0
#define ACTIVE_BIT_7               0x3f8
#define ACTIVE_BIT_8               0x3fc
#define ACTIVE_BIT_9               0x3fe
#define ACTIVE_BIT_10              0x3ff




#endif
