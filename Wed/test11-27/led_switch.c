#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

#define DEV_NAME "led_switch_driver"
#define DEV_MAJOR_NUMBER 220

// GPIO 설정
static int led[4] = {23, 24, 25, 1};
static int sw[4] = {4, 17, 27, 22};

// 상태 변수
static int mode = 0; // 현재 모드
static struct timer_list led_timer;
static unsigned char led_state[4] = {0};
static int current_led = 0; // 개별 모드에서 현재 LED

// 리셋 모드
void reset_mode(void) {
    int i;
    del_timer_sync(&led_timer);
    mode = 0;
    for (i = 0; i < 4; i++) {
        gpio_direction_output(led[i], 0);
        led_state[i] = 0;
    }
    printk(KERN_INFO "Reset mode activated\n");
}

// 전체 모드
void mode_one(void) {
    mode = 1;
    mod_timer(&led_timer, jiffies + HZ * 2);
    printk(KERN_INFO "All LEDs blink mode activated\n");
}

// 개별 모드
void mode_two(void) {
    mode = 2;
    mod_timer(&led_timer, jiffies + HZ * 2);
    printk(KERN_INFO "Sequential LED blink mode activated\n");
}

// 수동 모드
void mode_three(int idx) {
    led_state[idx] ^= 1;
    gpio_direction_output(led[idx], led_state[idx]);
    printk(KERN_INFO "Manual mode: LED %d toggled\n", idx);
}

// 타이머 콜백 함수
void led_timer_callback(struct timer_list *t) {
    int i;
    if (mode == 1) { // 전체 모드
        for (i = 0; i < 4; i++) {
            gpio_direction_output(led[i], led_state[0]);
        }
        led_state[0] ^= 1;
    } else if (mode == 2) { // 개별 모드
        for (i = 0; i < 4; i++) {
            gpio_direction_output(led[i], i == current_led ? 1 : 0);
        }
        current_led = (current_led + 1) % 4;
    }

    if (mode == 1 || mode == 2) {
        mod_timer(&led_timer, jiffies + HZ * 2);
    }
}

// 사용자로부터 입력받는 write 함수
static ssize_t led_switch_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char user_input;

    if (copy_from_user(&user_input, buf, 1)) {
        return -EFAULT;
    }

    if (mode == 3) {
        if (user_input == '4') {
            reset_mode();
        }else{
            mode_three((int)user_input);
        }
    }else{
        reset_mode();
        switch (user_input) {
        case '1':
            mode_one();
            break;
        case '2':
            mode_two();
            break;
        case '3':
            mode_three(user_input);
            break;
        case '4':
            reset_mode();
            // printk(KERN_INFO "Manual mode activated\n");
            break;
        default:
            printk(KERN_INFO "Invalid input received: %c\n", user_input);
            break;
    }

    }
    return len;
}

// 인터럽트 핸들러
irqreturn_t irq_handler(int irq, void *dev_id) {
    int i;
    
    if (mode != 3){
        reset_mode();
    }

    for (i = 0; i < 4; i++) {
        if (irq == gpio_to_irq(sw[i])) {
            if (mode == 3){
                if (i == 3){
                    reset_mode();
                }else{
                    mode_three(i);
                }
                
            }else{
                switch (i) {
                    case 0: mode_one(); break;
                    case 1: mode_two(); break;
                    case 2: mode = 3; break;
                    case 3: reset_mode(); break;
                }
            }
            
            return IRQ_HANDLED;
        }
    }
    return IRQ_NONE;
}

// 파일 연산 구조체
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = led_switch_write, // write 함수 추가
};

// 디바이스 드라이버 초기화
static int __init led_switch_init(void) {
    int i, ret;

    for (i = 0; i < 4; i++) {
        if ((ret = gpio_request(led[i], "LED")) < 0) return ret;
        if ((ret = gpio_request(sw[i], "SW")) < 0) return ret;

        if ((ret = request_irq(gpio_to_irq(sw[i]), irq_handler, IRQF_TRIGGER_RISING, "switch_irq", NULL)) < 0) return ret;
    }

    timer_setup(&led_timer, led_timer_callback, 0);

    if ((ret = register_chrdev(DEV_MAJOR_NUMBER, DEV_NAME, &fops)) < 0) {
        printk(KERN_ERR "Failed to register char device\n");
        return ret;
    }

    printk(KERN_INFO "LED Switch Driver Initialized\n");
    return 0;
}

// 디바이스 드라이버 종료
static void __exit led_switch_exit(void) {
    int i;
    del_timer_sync(&led_timer);
    for (i = 0; i < 4; i++) {
        free_irq(gpio_to_irq(sw[i]), NULL);
        gpio_free(led[i]);
        gpio_free(sw[i]);
    }
    unregister_chrdev(DEV_MAJOR_NUMBER, DEV_NAME);
    printk(KERN_INFO "LED Switch Driver Removed\n");
}

module_init(led_switch_init);
module_exit(led_switch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("LED Switch Driver");