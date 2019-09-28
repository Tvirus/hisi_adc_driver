#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/of_irq.h>

#include "hisi_adc_api.h"



#define VERSION "1.0"



#if 1
#define DEBUG(fmt, arg...)  printk("--HIADC-- " fmt "\n", ##arg)
#else
#define DEBUG(fmt, arg...)
#endif


#define ERROR(fmt, arg...)  printk(KERN_ERR "--HIADC-- " fmt "\n", ##arg)




static int adc_irq = -1;
static struct input_dev *adc_idev = NULL;

static u32 ch0_enable = 0;
static u32 ch1_enable = 0;
static u32 ch0_value = 0;
static u32 ch1_value = 0;
static u32 period = 1000; /* ms */






static ssize_t ch0_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", ch0_enable);
}
static ssize_t ch0_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long en;

    if (kstrtoul(buf, 0, &en))
        return -EINVAL;

    en = !!en;
    hiadc_enable_ch0((u32)en);
    ch0_enable = en;
    return size;
}
static ssize_t ch1_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", ch1_enable);
}
static ssize_t ch1_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long en;

    if (kstrtoul(buf, 0, &en))
        return -EINVAL;

    en = !!en;
    hiadc_enable_ch1((u32)en);
    ch1_enable = en;
    return size;
}
static ssize_t period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", period);
}
static ssize_t period_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long _period;

    if (kstrtoul(buf, 0, &_period))
        return -EINVAL;

    if (hiadc_set_period((u32)_period))
        return -EINVAL;
    period = _period;
    return size;
}
static ssize_t ch0_value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", ch0_value);
}
static ssize_t ch0_value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    return -EPERM;
}
static ssize_t ch1_value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%u\n", ch1_value);
}
static ssize_t ch1_value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    return -EPERM;
}


struct device_attribute adc_attrs[] =
{
    __ATTR(ch0_en,    0660, ch0_enable_show, ch0_enable_store),
    __ATTR(ch1_en,    0660, ch1_enable_show, ch1_enable_store),
    __ATTR(ch0_value, 0660, ch0_value_show,  ch0_value_store),
    __ATTR(ch1_value, 0660, ch1_value_show,  ch1_value_store),
    __ATTR(period_ms, 0660, period_show,     period_store)
};
int adc_attrs_size = sizeof(adc_attrs)/sizeof(adc_attrs[0]);

static int adc_add_sysfs_interfaces(struct device *dev, struct device_attribute *a, int size)
{
    int i;

    for (i = 0; i < size; i++)
        if (device_create_file(dev, a + i))
            goto undo;
    return 0;

undo:
    for (i--; i >= 0; i--)
        device_remove_file(dev, a + i);
    return -ENODEV;
}
static void adc_del_sysfs_interfaces(struct device *dev, struct device_attribute *a, int size)
{
    int i;

    for (i = 0; i < size; i++)
        device_remove_file(dev, a + i);
}



irqreturn_t adc_irq_handler(int irq, void *data)
{
    u32 intr;

    intr = hiadc_get_int();

    if (0 == (intr & (INT_FLAG_IN0 | INT_FLAG_IN1)))
        return IRQ_HANDLED;

    if (intr & INT_FLAG_IN0)
    {
        ch0_value = hiadc_get_ch0();
        input_event(adc_idev, EV_PWR, 0, ch0_value);
    }
    if (intr & INT_FLAG_IN1)
    {
        ch1_value = hiadc_get_ch1();
        input_event(adc_idev, EV_PWR, 1, ch1_value);
    }

    input_sync(adc_idev);

    return IRQ_HANDLED;
}

static int adc_idev_open(struct input_dev *idev)
{
    return 0;
}
static void adc_idev_close(struct input_dev *idev)
{
}





static int adc_probe(struct platform_device *dev)
{
    int ret = 0;


    adc_idev = input_allocate_device();
    if (IS_ERR_OR_NULL(adc_idev))
    {
        ERROR("allocate input device failed !");
        ret = PTR_ERR(adc_idev);
        goto ERR_1;
    }
    set_bit(EV_SYN, adc_idev->evbit);
    set_bit(EV_PWR, adc_idev->evbit);
    adc_idev->name = "hisi_adc";
    adc_idev->open  = adc_idev_open;
    adc_idev->close = adc_idev_close;
    adc_idev->id.bustype = BUS_HOST;
    adc_idev->id.vendor  = 0;
    adc_idev->id.product = 0;
    adc_idev->id.version = 0x1;
    ret = input_register_device(adc_idev);
    if (ret)
    {
        ERROR("register input device failed !");
        input_free_device(adc_idev);
        adc_idev = NULL;
        goto ERR_1;
    }

    adc_irq = platform_get_irq(dev, 0);
    if (0 > adc_irq)
    {
        ERROR("platform get irq failed !");
        goto ERR_2;
    }
    ret = request_threaded_irq(adc_irq, NULL, adc_irq_handler, IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "hisi_adc", NULL);
    if (ret)
    {
        ERROR("request threaded irq failed !");
        goto ERR_2;
    }

    if (hiadc_init())
    {
        ERROR("init adc failed !");
        ret = -ENXIO;
        goto ERR_3;
    }
    hiadc_set_period(period);


    ret = adc_add_sysfs_interfaces(&adc_idev->dev, adc_attrs, adc_attrs_size);
    if (ret)
    {
        ERROR("create sysfs interface failed !");
        goto ERR_4;
    }

    return 0;



ERR_4:
    hiadc_deinit();
ERR_3:
    free_irq(adc_irq, NULL);
ERR_2:
    input_unregister_device(adc_idev);
ERR_1:
    return ret;
}

static int adc_remove(struct platform_device *dev)
{
    adc_del_sysfs_interfaces(&adc_idev->dev, adc_attrs, adc_attrs_size);
    hiadc_deinit();
    free_irq(adc_irq, NULL);
    input_unregister_device(adc_idev);

    return 0;
}

static const struct of_device_id adc_match_table[] =
{
    { .compatible = "hisi_adc", },
    { /* end */ },
};
static struct platform_driver adc_driver =
{
    .probe    = adc_probe,
    .remove   = adc_remove,
    .driver = {
        .name  = "hisi_adc",
        .owner = THIS_MODULE,
        .of_match_table = adc_match_table,
    },
};

static int __init adc_init(void)
{
    DEBUG("Driver Version: %s", VERSION);
    return platform_driver_register(&adc_driver);
}
static void __exit adc_exit(void)
{
    DEBUG("exit");
    platform_driver_unregister(&adc_driver);
}


late_initcall(adc_init);
module_exit(adc_exit);


MODULE_AUTHOR("LLL");
MODULE_DESCRIPTION("hisi_adc driver");
MODULE_LICENSE("GPL");
