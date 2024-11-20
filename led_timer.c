#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/timer.h>

#define HIGH    1
#define LOW     0

int led[4] = {23, 24, 25, 1}, flag = 0;
static struct timer_list timer;

static void timer_cb(struct timer_list *timer) {
    int ret, i;
    printk(KERN_INFO "timer callback function!\n");

    if(flag == 0) {
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

static int timer_led_module_init(void) {

    int ret, i;
    printk(KERN_INFO "timer_led_module_init!\n");

    for(i = 0; i < 4; i++) {
        ret = gpio_request(led[i], "LED");
        if(ret < 0)
            printk(KERN_INFO "led_module gpio_request failed!\n");
    }


    timer_setup(&timer, timer_cb, 0);
    timer.expires = jiffies + HZ * 2;
    add_timer(&timer);
    return 0;
}

static void timer_led_module_exit(void) {
    int i;
    printk(KERN_INFO "timer_led_module_exit\n");
    del_timer(&timer);
    for(i = 0; i < 4; i++)
        gpio_free(led[i]);
}

module_init(timer_led_module_init);
module_exit(timer_led_module_exit);
MODULE_LICENSE("GPL");