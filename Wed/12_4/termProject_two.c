#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


#define HIGH    1
#define LOW     0

#define DEVICE_NAME "led_switch"
#define DEV_MAJOR_NUMBER 220


int sw[4] = {4, 17, 27, 22};
int led[4] = {23, 24, 25, 1};
int mode = 5; // mode 0 ~ 3번까지 
static int led_state[4] = {0, 0, 0, 0};
static int current_led = 3;

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
    
    int ret_led;

    ret_led = gpio_direction_output(led[current_led], LOW);
    
    

    current_led = (current_led + 1) % 4;

    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

    ret_led = gpio_direction_output(led[current_led], HIGH);
}

static void reset_mode(int number) {
    int i;
    del_timer_sync(&timer);
    for(i = 0; i < 4; i++) {
        gpio_direction_output(led[i], LOW);
    }
    printk(KERN_INFO "RESET_MODE");
    current_led = 3;
}






static ssize_t led_switch_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char user_input;
    int i;
    int input_i;

    if (copy_from_user(&user_input, buf, 1)) {
        return -EFAULT;
    }

    input_i = user_input - '0';
    


    switch (user_input) {
        case '0' :
            reset_mode(5);
            break;
        
        case '1' :
            reset_mode(0);
            timer_setup(&timer, timer_cb_sw0, 0);
            timer.expires = jiffies + HZ * 2;
            add_timer(&timer);
            

            break;
        
        case '2' :
            reset_mode(1);
            timer_setup(&timer, timer_cb_sw1, 0);
            timer.expires = jiffies + HZ * 2;
            add_timer(&timer);

            break;

        case '3' :

            printk(KERN_INFO "input case %d\n", input_i);

            
            if(input_i >= 0 && input_i < 4) {
                led_state[input_i] ^= 1;
                gpio_direction_output(led[input_i], led_state[input_i] ? HIGH : LOW);
                
                printk(KERN_INFO "input %d\n", input_i);

                if (input_i == 4) {
                    reset_mode(0);
                    printk(KERN_INFO "reset\n");
                    return len;
                }
            }



            
            break;

        case '4' :
            reset_mode(1);
            break;

    }


    return len;

}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = led_switch_write,
};




static int led_switch_init(void) {
    int res_sw, ret_led, i, ret;
    printk(KERN_INFO"led_switch_init!\n");

    for(i = 0; i < 4; i++) {
        res_sw = gpio_request(sw[i], "sw");
        ret_led = gpio_request(led[i], "LED");
        
        if (res_sw < 0)
            printk(KERN_INFO"requset_irq failed!\n");
        if (ret_led < 0)
            printk(KERN_INFO "led_module gpio_request failed!\n");
    
    } 

    ret = register_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME, &fops);

    if (ret < 0) {
        printk(KERN_INFO "Failed to register char device\n");
        return ret;
    }



    return 0;
}

static void led_switch_exit(void) {
    int i;

    printk(KERN_INFO "led_switch_exit\n");

    del_timer_sync(&timer);

    for(i = 0; i < 4; i++) {
        gpio_free(led[i]);
        gpio_free(sw[i]);
    }

    unregister_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME);

}

module_init(led_switch_init);
module_exit(led_switch_exit);
MODULE_LICENSE("GPL");
