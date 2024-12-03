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
int mode = 5; // mode 0 ~ 3번까지 
static int led_state[4] = {0};

static struct timer_list timer;

static void timer_cb_sw0(struct timer_list * timer) {
    static int flag = 0;
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
    
    static int current_led = 0;
    int ret_led;

    timer->expires = jiffies + HZ * 2; 
    add_timer(timer);

    
    ret_led = gpio_direction_output(led[current_led], HIGH);
    printk(KERN_INFO "LED ON %d \n", current_led);

    
    timer->expires = jiffies + HZ * 2; 
    add_timer(timer);
    
    
    ret_led = gpio_direction_output(led[current_led], LOW);
    printk(KERN_INFO "LED OFF %d \n", current_led);

    
    current_led = (current_led + 1) % 4;

}

static void reset_mode(int number) {
    int i;
    mode = number;
    del_timer_sync(&timer);
    for(i = 0; i < 4; i++) {
        gpio_direction_output(led[i], LOW);
    }
    printk(KERN_INFO "RESET_MODE");
}



irqreturn_t irq_handler(int irq, void*dev_id) {
    printk(KERN_INFO"Debug%d\n", irq);
    
    int i;

    switch(irq) {
        case 60:

            if (mode == 0) {
                reset_mode(0);
                break;
            }
            if (mode != 2) {
                timer_setup(&timer, timer_cb_sw0, 0);
                timer.expires = jiffies + HZ * 2;
                add_timer(&timer);
                mode = 0;
                 
            }
            break;

        case 61:

            if (mode == 1) {
                reset_mode(1);
                break;
            }
            if (mode != 2) {
                timer_setup(&timer, timer_cb_sw1, 0);
                timer.expires = jiffies + HZ * 2;
                add_timer(&timer);
                mode = 1;
                
            }
            break;

        case 62:

            if (mode == 2) {
                reset_mode(2);
                break;
            }
            
            if (mode != 2) {

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
                    
                    for (i = 0; i < 4; i++) {
                        led_state[i] = 0;
                        gpio_direction_output(led[i], LOW); 
                    }
                    mode = 3;
                }
                
                

            }

            break;

        case 63:
            if (mode != 3) {
                reset_mode(3);
            }
            
            break;
    }
    return IRQ_HANDLED;
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
