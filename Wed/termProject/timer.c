#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>

#define HIGH    1
#define LOW     0

int sw[4] = {4, 17, 27, 22};
int led[4] = {23, 24, 25, 1}, flag = 0;

static struct timer_list timer;

static void timer_cb(struct timer_list * timer) {
    int ret, i;
    printk(KERN_INFO "timer callback function\n");

    if (flag == 0) {
        for(i = 0; i < 4; i++) {
            ret = gpio_direction_output(led[i], HIGH);
        }
        flag = 1;
    } else {
        for(i = 0; i < 4; i++) {
            ret = gpio_direction_output(led[i], LOW);
        }
        flag = 0;
    }

    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

}


irqreturn_t irq_handler(int irq, void*dev_id) {
    printk(KERN_INFO"Debug%d\n", irq);

    switch(irq) {
        case 60:
            timer_setup(&timer, timer_cb, 0);
            timer.expires = jiffies + HZ * 2;
            add_timer(&timer);
            break;
        case 61:
            printk(KERN_INFO"sw2 interrupt ocurred!\n");
            break;
        case 62:
            printk(KERN_INFO"sw3 interrupt ocurred!\n");
            break;
        case 63:
            printk(KERN_INFO"sw4 interrupt ocurred!\n");
            break;
    }
    return 0;
}

static int led_switch_init(void) {
    int res, ret, i;
    printk(KERN_INFO"led_switch_init!\n");

    for(i = 0; i < 4; i++) {
        res = gpio_request(sw[i], "sw");
        ret = gpio_request(led[i], "LED");

        res = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler,IRQF_TRIGGER_RISING, "IRQ", (void *)(irq_handler));
        
        if (res < 0)
            printk(KERN_INFO"requset_irq failed!\n");
        if (ret < 0)
            printk(KERN_INFO "led_module gpio_request failed!\n");
    
    }
    return 0;
}

static void led_switch_exit(void) {
    int i;

    printk(KERN_INFO "led_switch_exit\n");

    del_timer(&timer);

    for(i = 0; i < 4; i++) {
        gpio_free(led[i]);
        free_irq(gpio_to_irq(sw[i]), (void *)(irq_handler));
        gpio_free(sw[i]);
    }
}

module_init(led_switch_init);
module_exit(led_switch_exit);
MODULE_LICENSE("GPL");
