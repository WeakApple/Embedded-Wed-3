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

static int mode = 0; // 모드: 0=리셋, 1=전체, 2=개별, 3=수동
static int led[4] = {23, 24, 25, 1};
static int sw[4] = {4, 17, 27, 22};
static struct timer_list led_timer;
static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static int irq[4];

// LED 상태 관리 변수
static unsigned char led_state[4] = {0};

// 타이머 콜백
void led_timer_callback(struct timer_list *t) {
    int i;

    if (mode == 1) { // 전체 모드
        for (i = 0; i < 4; i++) {
            gpio_direction_output(led[i], led_state[0]);
        }
        led_state[0] ^= 1; // 상태 반전
    } else if (mode == 2) { // 개별 모드
        static int current_led = 0;
        for (i = 0; i < 4; i++) {
            gpio_direction_output(led[i], i == current_led ? 1 : 0);
        }
        current_led = (current_led + 1) % 4;
    }

    if (mode == 1 || mode == 2) {
        mod_timer(&led_timer, jiffies + HZ * 2); // 2초 후 재호출
    }
}

// 인터럽트 핸들러
irqreturn_t irq_handler(int irq, void *dev_id) {
    int i;
    int j;

    for (i = 0; i < 4; i++) {
        if (irq == gpio_to_irq(sw[i])) {
            if (mode == 3) { // 수동 모드
                led_state[i] ^= 1; // 상태 반전
                gpio_direction_output(led[i], led_state[i]);
            } else if (i == 0) {
                mode = 1; // 전체 모드
                mod_timer(&led_timer, jiffies + HZ * 2);
            } else if (i == 1) {
                mode = 2; // 개별 모드
                mod_timer(&led_timer, jiffies + HZ * 2);
            } else if (i == 2) {
                mode = 3; // 수동 모드
            } else if (i == 3) {
                mode = 0; // 리셋 모드
                del_timer_sync(&led_timer);
                for (j = 0; j < 4; j++) {
                    gpio_direction_output(led[j], 0);
                }
            }
        }
    }
    return IRQ_HANDLED;
}

// 모듈 초기화
static int __init led_switch_init(void) {
    int i, ret;

    for (i = 0; i < 4; i++) {
        ret = gpio_request(led[i], "LED");
        if (ret < 0) return ret;

        ret = gpio_request(sw[i], "SW");
        if (ret < 0) return ret;

        irq[i] = gpio_to_irq(sw[i]);
        ret = request_irq(irq[i], irq_handler, IRQF_TRIGGER_RISING, "switch_irq", NULL);
        if (ret < 0) return ret;
    }

    timer_setup(&led_timer, led_timer_callback, 0);
    printk(KERN_INFO "LED Switch Driver Initialized\n");
    return 0;
}

// 모듈 종료
static void __exit led_switch_exit(void) {
    int i;

    del_timer_sync(&led_timer);
    for (i = 0; i < 4; i++) {
        free_irq(irq[i], NULL);
        gpio_free(led[i]);
        gpio_free(sw[i]);
    }

    printk(KERN_INFO "LED Switch Driver Removed\n");
}

module_init(led_switch_init);
module_exit(led_switch_exit);
MODULE_LICENSE("GPL");
