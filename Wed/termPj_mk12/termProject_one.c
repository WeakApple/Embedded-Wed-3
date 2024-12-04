#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
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

// 전체 모드
static void timer_cb_sw0(struct timer_list * timer) {
    int ret_led, i;
    for(i = 0; i < 4; i++) {
        ret_led = gpio_direction_output(led[i], led_state[0]);
    }
    led_state[0] ^= 1;
    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

}
// 개별 모드
static void timer_cb_sw1(struct timer_list * timer) {
    
    int ret_led;
    ret_led = gpio_direction_output(led[current_led], LOW);
    printk(KERN_INFO "LED OFF %d", current_led);

    current_led = (current_led + 1) % 4;
    timer->expires = jiffies + HZ * 2;
    add_timer(timer);
    ret_led = gpio_direction_output(led[current_led], HIGH);
    printk(KERN_INFO "LED ON %d", current_led);
}




// 초기화
static void reset_mode(int number) {
    int i;
    mode = number;
    del_timer_sync(&timer);
    for(i = 0; i < 4; i++) {
        gpio_direction_output(led[i], LOW);
        led_state[i] = LOW;
    }
    printk(KERN_INFO "RESET_MODE");
    current_led = 3;
}

// 수동모드 내부 로직
static void sudong_mode(int sw_num) {
    led_state[sw_num] ^= 1;
    gpio_direction_output(led[sw_num], led_state[sw_num] ? HIGH : LOW);
    printk(KERN_INFO "press\n");
}

// 인터럽트 핸들러(스위치)
irqreturn_t irq_handler(int irq, void*dev_id) {
    printk(KERN_INFO"Debug%d\n", irq);
    
    int i;

    switch(irq) {
        case 60:
            //led가 켜진 상태에서 시작하기 위한 초기설정
            if (mode == 2) {          
                sudong_mode(0);
                break;
            }
            reset_mode(0);
            led_state[0] = HIGH;
            timer_setup(&timer, timer_cb_sw0, 0);
            timer.expires = jiffies;
            add_timer(&timer);
            break;

        case 61:
            if (mode == 2) {          
                sudong_mode(1);
                break;
            }
            reset_mode(1);
            timer_setup(&timer, timer_cb_sw1, 0);
            timer.expires = jiffies;
            add_timer(&timer);
            break;

        case 62:
            if (mode != 2) {
                reset_mode(2);
                break;
            }
            sudong_mode(2);
            break;

        case 63:
            if (mode != 3) {
                reset_mode(3);
            }
            break;
    }
    return IRQ_HANDLED;
}

// 맵핑 write
static ssize_t led_switch_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char user_input;
    int i;

    if (copy_from_user(&user_input, buf, 1)) {
        return -EFAULT;
    }

    


    switch (user_input) {
        case '1' :
            if (mode == 2) {          
                sudong_mode(0);
                break;
            }
            reset_mode(0);
            led_state[0] = HIGH;
            timer_setup(&timer, timer_cb_sw0, 0);
            timer.expires = jiffies;
            add_timer(&timer);
            break;
        
        case '2' :
            if (mode == 2) {          
                sudong_mode(1);
                break;
            }
            reset_mode(1);
            timer_setup(&timer, timer_cb_sw1, 0);
            timer.expires = jiffies;
            add_timer(&timer);
            break;

        case '3' :
            if (mode != 2) {
                reset_mode(2);
                break;
            }
            sudong_mode(2);
            break;

        case '4' :
            if (mode != 3) {
                reset_mode(3);
            }
            break;

    }


    return len;

}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = led_switch_write,
};


// 모듈 초기화
static int led_switch_init(void) {
    int ret, res_sw, ret_led, i;
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

    ret = register_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME, &fops);

    if (ret < 0) {
        printk(KERN_INFO "Failed to register char device\n");
        return ret;
    }
    return 0;
}
// 모듈 삭제
static void led_switch_exit(void) {
    int i;

    printk(KERN_INFO "led_switch_exit\n");

    del_timer(&timer);

    for(i = 0; i < 4; i++) {
        gpio_free(led[i]);
        free_irq(gpio_to_irq(sw[i]), (void *)(irq_handler));
        gpio_free(sw[i]);
    }

     unregister_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME);
}

module_init(led_switch_init);
module_exit(led_switch_exit);
MODULE_LICENSE("GPL");