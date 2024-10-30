#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int module_exam_init(void) {
    printk(KERN_INFO "module_init!\n");
    return 0;
}

static void module_exam_exit(void) {
    
    printk(KERN_INFO "module_exit\n");
}

module_init(module_exam_init);
module_exit(module_exam_exit);
MODULE_LICENSE("GPL");
