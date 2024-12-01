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
int led[4] = {23, 24, 25, 1};
int mode = 3; // mode 0 ~ 3번까지 

static struct timer_list timer;

static void timer_cb_sw0(struct timer_list * timer) {
    int flag = 0;
    int ret_led, i;

    if (flag == 0) {
        for(i = 0; i < 4; i++) {
            ret_led = gpio_direction_output(led[i], HIGH);
        }
        flag = 1;
    } else {
        for(i = 0; i < 4; i++) {
            ret_led = gpio_direction_output(led[i], LOW);
        }
        flag = 0;
    }

    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

}

static void timer_cb_sw1(struct timer_list * timer) {
    

    // int ret_led, i;
    
    // for(i = 0; i < 4; i++) {
    //     ret_led = gpio_direction_output(led[i], HIGH);
    //     timer->expires = jiffies + HZ * 2;
    //     ret_led = gpio_direction_output(led[i], LOW);
    //     add_timer(timer);
    // }

    
    static int current_led = 0;
    int ret_led;

    
    ret_led = gpio_direction_output(led[current_led], HIGH);

    
    timer->expires = jiffies + HZ * 2; 
    add_timer(timer);
    
    
    ret_led = gpio_direction_output(led[current_led], LOW);

    
    current_led = (current_led + 1) % 4;

    
    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

}




irqreturn_t irq_handler(int irq, void*dev_id) {
    printk(KERN_INFO"Debug%d\n", irq);
    int led_state[4] = {0};

    switch(irq) {
        case 61:

            if (mode == 0) {
                break;
            }
            if (mode != 3) {
                timer_setup(&timer, timer_cb_sw0, 0);
                timer.expires = jiffies + HZ * 2;
                add_timer(&timer);
                mode = 0;
                 
            }
            break;

        case 62:

            if (mode == 1) {
                break;
            }
            if (mode != 3) {
                timer_setup(&timer, timer_cb_sw1, 0);
                timer.expires = jiffies + HZ * 2;
                add_timer(&timer);
                mode = 1;
                
            }
            break;

        case 63:

            if (mode == 2) {
                break;
            }
            
            if (mode != 3) {

                if (gpio_get_value(sw[0])) {
                    led_state[0] ^= 1; 
                    gpio_direction_output(led[0], led_state[0]);
                }

            
                if (gpio_get_value(sw[1])) {
                    led_state[1] ^= 1; 
                    gpio_direction_output(led[1], led_state[1]);   
                }

                
                if (gpio_get_value(sw[2])) {
                    led_state[2] ^= 1; 
                    gpio_direction_output(led[2], led_state[2]); 
                }

                
                if (gpio_get_value(sw[3])) {
                    for (int i = 0; i < 4; i++) {
                        led_state[i] = 0;
                        gpio_direction_output(led[i], LOW); 
                    }
                    mode = 3;
                }
                
                

            }

            break;

        case 64:

            for (int i = 0; i < 4; i++) {
                gpio_direction_output(led[i], LOW);
            }
            mode = 3;

            break;
    }
    return 0;
}

static int led_switch_init(void) {
    int res_sw, ret_led, i;
    printk(KERN_INFO"led_switch_init!\n");

    for(i = 0; i < 4; i++) {
        res_sw = gpio_request(sw[i], "sw");
        ret_led = gpio_request(led[i], "LED");

        res_sw = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler,IRQF_TRIGGER_RISING, "IRQ", (void *)(irq_handler));
        
        if (res_sw < 0)
            printk(KERN_INFO"requset_irq failed!\n");
        if (ret_led < 0)
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