#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>

static int my_thread(void *data) {
    while(!kthread_should_stop()) {
        printk("default message...\n");
        printk(KERN_INFO "info moessage...\n");
        printk(KERN_ALERT "alert message...\n");

        ssleep(1);
    }

    printk("stop thread...\n");
    return 0;
}

static int printk_exam_init(void) {
    
    struct task_struct *t;
    t = kthread_run(my_thread, NULL, "my_thread");
    ssleep(30);
    kthread_stop(t);
    return 0;
    
}

static void printk_exam_exit(void) {
    printk(KERN_INFO "module_exit!\n");
}

module_init(printk_exam_init);
module_exit(printk_exam_exit);
MODULE_LICENSE("GPL");