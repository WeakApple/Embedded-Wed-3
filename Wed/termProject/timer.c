#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/intterupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>

#define HIGH 1
#define LOW 0

int led[4] = {23, 24, 25, 1}, flag = 0;
int sw[4] = {4, 17, 27, 22};
static int mod = 0;

static struct timer_list timeer;

irqreturn_t irq_handler(int irq, void *dev_id){
    switch (irq)
    {
        case 59:
        break;
        case 60:
        break;
        case 61:
        break;
        case 62:
        break;
    }
    return 0;
}

static int switch_interrupt_init(void){
    int rsw, rled,i;
    printk(KERN_INFO "sw it init\n");
    for(i=0;i<4;i++){
        rsw = gpio_request(sw[i], "SW");
        rsw = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler,IRQF_TRIGGER_RISING, "IRQ", (void *)(irq_handler));
        if(rsw<0){
            printk(KERN_INFO "request_irq failed!\n");
        }
    }
    for(i=0;i<4;i++){
        rled = gpio_request(led[i], "LED");
        if(ret<0){
            printk(KERN_INFO "led module rq failed\n");
        }
    }
    return 0;
}

static void switch_interrupt_exit(void){
    int i;
    printk(KERN_INFO "sw it exti\n");
    for(i=0;i<4;i++){
        free_irq(gpio_to_irq(sw[i]), (void *)(irq_handler));
        gpio_free(sw[i]);
    }
    for(i=0;i<4;i++){
        gpio_free(led[i]);
    }
}

module_init(switch_interrupt_init);
module_exit(switch_interrupt_exit);
MODULE_LICENSE("GPL")