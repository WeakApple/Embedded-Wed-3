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

#define DEVICE_NAME "embedded_device"
#define DEV_MAJOR_NUMBER 220

#define IRQ_SW0 60
#define IRQ_SW1 61
#define IRQ_SW2 62
#define IRQ_SW3 63

int sw[4] = {4, 17, 27, 22};
int led[4] = {23, 24, 25, 1};
int mode = 5; // mode 1, 2 => 1: 일반모드, 2: 수동모드
static int led_state[4] = {0, 0, 0, 0}; // led 상태 관리를 위한 배열 (0: LOW, 1: HIGH 두가지 상태로 관리)
static int led_off_idx = 3; // 꺼져야 할 led idx를 나타내는 변수

static struct timer_list timer;


// 전체 모드
static void timer_cb_sw0(struct timer_list * timer) {
    int i;
    // led를 일괄적으로 키고 끄는 부분
    for(i = 0; i < 4; i++) {
        gpio_direction_output(led[i], led_state[0]);
    }
    // led 상태 반전을 위해 0번째 상태를 반대로
    led_state[0] ^= 1;
    // 타이머 재설정
    timer->expires = jiffies + HZ * 2;
    add_timer(timer);

}
// 개별 모드
static void timer_cb_sw1(struct timer_list * timer) {
    // 이전에 켜져있던 led 끄기
    gpio_direction_output(led[led_off_idx], LOW);
    // 다음에 꺼져야할 led idx 계산 (= 이번에 켜져야 하는 led idx)
    led_off_idx = (led_off_idx + 1) % 4;
    // led 켜기
    gpio_direction_output(led[led_off_idx], HIGH);
    // 타이머 설정
    timer->expires = jiffies + HZ * 2;
    add_timer(timer);
    
}

// 초기 timer를 설정하는 부분을 사용자화
static void custom_set_timer(struct timer_list *timer, void (*callback)(struct timer_list *)) {
    // 타이머에 콜백함수 맵핑
    timer_setup(timer, callback, 0);
    // 첫 실행은 바로 실행되야 함 (= expires에 지연을 주지 않음)
    timer->expires = jiffies;
    add_timer(timer);   
}


// 초기화
static void reset_mode(int number) {
    int i;
    mode = number;  // 모드 변경
    del_timer_sync(&timer); // 기존의 timer를 제거
    // led를 꺼짐 상태로 초기화
    for(i = 0; i < 4; i++) {
        gpio_direction_output(led[i], LOW);
        led_state[i] = LOW;
    }
    // 최초로 꺼져야하는 idx 설정
    led_off_idx = 3;
}

// 수동모드 내부 로직
static void manual_mode(int sw_num) {
    // 입력 받은 sw 번호와 맵핑되는 led의 상태를 반전
    led_state[sw_num] ^= 1;
    gpio_direction_output(led[sw_num], led_state[sw_num]); //? HIGH : LOW)

}

// 인터럽트 핸들러(스위치)
irqreturn_t irq_handler(int irq, void*dev_id) {
    switch(irq) {
        case IRQ_SW0:
            //수동모드 시 동작 (아래도 동일)
            if (mode == 2) {          
                manual_mode(0);
                break;
            }
            //기존 동작
            reset_mode(0);
            led_state[0] = HIGH;
            custom_set_timer(&timer, timer_cb_sw0);
            break;

        case IRQ_SW1:
            if (mode == 2) {          
                manual_mode(1);
                break;
            }
            reset_mode(1);
            custom_set_timer(&timer, timer_cb_sw1);
            break;

        case IRQ_SW2:
            // 다른 모드에서 수동모드로 집입 시 동작
            if (mode != 2) {
                reset_mode(2);
                break;
            }
            // 수동모드 내부 동작
            manual_mode(2);
            break;

        case IRQ_SW3:
            // 모드 상관 없이 초기화 동작
            reset_mode(3);
            break;
    }
    return IRQ_HANDLED;
}

// wirte()에 맵핑될 함수
static ssize_t embedded_device_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char user_input;
    // 사용자 입력을 메모리에 복제 (버퍼 사용 X)
    if (copy_from_user(&user_input, buf, 1)) {
        return -EFAULT;
    }
    // 사용자 입력을 기반으로 분기
    switch (user_input) {
        // 전체 모드
        case '1' :
            if (mode == 2) {          
                manual_mode(0);
                break;
            }
            reset_mode(0);
            led_state[0] = HIGH;
            custom_set_timer(&timer, timer_cb_sw0);
            break;
        // 개별 모드
        case '2' :
            if (mode == 2) {          
                manual_mode(1);
                break;
            }
            reset_mode(1);
            custom_set_timer(&timer, timer_cb_sw1);
            break;
        // 수동 모드
        case '3' :
            if (mode != 2) {
                reset_mode(2);
                break;
            }
            manual_mode(2);
            break;
        // 모드 초기화
        case '4' :
            reset_mode(3);
            break;

    }


    return len;

}

// 맵핑 테이블
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = embedded_device_write,
};


// 모듈 초기화
static int embedded_device_init(void) {
    int ret, ret_sw, ret_led, ret_sw_irq, i;
    printk(KERN_INFO"embedded_device_init!\n");

    for(i = 0; i < 4; i++) {
        // sw, led gpio 핀번호 가져오는 부분
        ret_sw = gpio_request(sw[i], "sw");
        ret_led = gpio_request(led[i], "LED");
        
        // sw irq 핸들러 설정
        ret_sw_irq = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler,IRQF_TRIGGER_RISING, "IRQ", (void *)(irq_handler));
        
        // 결과 확인 로그
        if (ret_sw_irq < 0)
            printk(KERN_INFO "requset_irq failed!\n");
        if (ret_led < 0 && ret_sw < 0)
            printk(KERN_INFO "led and switch module gpio_request failed!\n");
    
    } 
    // 디바이스 드라이버 등록
    ret = register_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME, &fops);
    // 디바이스 등록 결과
    if (ret < 0) {
        printk(KERN_INFO "Failed to register char device\n");
        return ret;
    }
    return 0;
}
// 모듈 삭제
static void embedded_device_exit(void) {
    int i;
    // 그냥 메시지
    printk(KERN_INFO "embedded_device_exit\n");
    // 동작하고 있는 타이머 제거
    del_timer(&timer);

    for(i = 0; i < 4; i++) {
        gpio_free(led[i]);  //led gpio 핀 점유 해제
        free_irq(gpio_to_irq(sw[i]), (void *)(irq_handler));    // irq 핸들러 해제
        gpio_free(sw[i]); // sw gpio 핀 점유 해제
    }

     unregister_chrdev(DEV_MAJOR_NUMBER, DEVICE_NAME); // 디바이스 드라이버 제거
}

module_init(embedded_device_init);
module_exit(embedded_device_exit);
MODULE_LICENSE("GPL");