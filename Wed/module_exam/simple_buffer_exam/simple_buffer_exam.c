#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEV_MAJOR_NUMBER 220
#define DEV_NAME "simple_buffer"

#define BUF_SIZE 10

void * memory_buffer;
ssize_t cur_len;

static int simple_buffer_write(struct file * file, const char * buf, size_t length, loff_t* ofs){
    ssize_t retval;
    printk(KERN_INFO "simple_buffer_write!\n");
    if(length > BUF_SIZE){
        retval = -ENOMEM;
        goto out;
    }
    cur_len = retval = length;
    if(copy_from_user(memory_buffer, buf, retval)){
        retval = -EFAULT;
        goto out;
    }
    printk(KERN_INFO "drv_buffer_write: [%dbyte -> %dbyte]\n", length, retval);
out:
    return retval;
}

static ssize_t simple_buffer_read(struct file * file, char * buf, size_t length, loff_t * ofs){
    ssize_t retval;
    printk(KERN_INFO "simple_buffer_read!\n");
    if(*ofs > 0){
        retval = 0;
        goto out;
    }
    retval = cur_len;
    if(copy_to_user(buf, memory_buffer, retval)){
        retval = -EFAULT;
        goto out;
    }
    printk(KERN_INFO "drv_buffer_read: [%dbyte -> %dbyte]\n", length, retval);
    *ofs += retval;
out:
    return retval;
}

static int simple_buffer_open(struct inode * inode, struct file * file){
    printk(KERN_INFO "simple_buffer_open!\n");
    memory_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if(!memory_buffer){
        printk("error: allocating memory for the buffer\n");
        return -ENOMEM;
    }
    memset(memory_buffer, 0, BUF_SIZE);
    return 0;
}

static int simple_buffer_release(struct inode * inode, struct file * file){
    printk(KERN_INFO "simple_buffer_release!\n");
    if(memory_buffer){
        kfree(memory_buffer);
    }
    return 0;
}

static struct file_operations simple_buffer_fops = {
    .owner = THIS_MODULE,
    .open = simple_buffer_open,
    .release = simple_buffer_release,
    .write = simple_buffer_write,
    .read = simple_buffer_read,
};

static int simple_buffer_init(void){
    printk(KERN_INFO "simple_buffer_init!\n");
    register_chrdev(DEV_MAJOR_NUMBER,DEV_NAME,&simple_buffer_fops);
    return 0;
}

static void simple_buffer_exit(void){
    printk(KERN_INFO "simple_buffer_exit!\n");
    unregister_chrdev(DEV_MAJOR_NUMBER,DEV_NAME);
}

module_init(simple_buffer_init);
module_exit(simple_buffer_exit);
MODULE_LICENSE("GPL");