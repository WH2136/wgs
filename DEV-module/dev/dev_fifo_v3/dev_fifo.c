#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include "dev_fifo_head.h"

//ָ�������豸��
#define MAJOR_NUM 250 

//�Լ����ַ��豸
struct mycdev 
{
    int len;
    unsigned char buffer[50];
    struct cdev cdev;
};

MODULE_LICENSE("GPL");

//�豸��
static dev_t dev_num = {0};

//ȫ��gcd 
struct mycdev *gcd;

//�豸��
struct class *cls;

//����û����ݵ����ݣ�������������ע����豸����
static int ndevices = 1;
module_param(ndevices, int, 0644);
MODULE_PARM_DESC(ndevices, "The number of devices for register.\n");


//���豸
static int dev_fifo_open(struct inode *inode, struct file *file)
{
    struct mycdev *cd;
    
    printk("dev_fifo_open success!\n");
    
    //��struct file���ļ�˽������ָ�뱣��struct mycdev�ṹ��ָ��
    cd = container_of(inode->i_cdev,struct mycdev,cdev);
    file->private_data = cd;
    
    return 0;
}

//���豸
static ssize_t dev_fifo_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
    int n;
    int ret;
    char *kbuf;
    struct mycdev *mycd = file->private_data;
    
    printk("read *ppos : %lld\n",*ppos);

    if(*ppos == mycd->len)
        return 0;

    //������С > bufferʣ����ֽ��� :��ȡʵ�ʼǵ��ֽ���
    if(size > mycd->len - *ppos)
        n = mycd->len - *ppos;
    else 
        n = size;
    
    printk("n = %d\n",n);
    //����һ���ļ�λ��ָ���λ�ÿ�ʼ��ȡ����
    kbuf = mycd->buffer + *ppos;

    //�������ݵ��û��ռ�
    ret = copy_to_user(ubuf,kbuf, n);
    if(ret != 0)
        return -EFAULT;
    
    //�����ļ�λ��ָ���ֵ
    *ppos += n;
    
    printk("dev_fifo_read success!\n");

    return n;
}

//д�豸
static ssize_t dev_fifo_write(struct file *file, const char __user *ubuf, size_t size, loff_t *ppos)
{
    int n;
    int ret;
    char *kbuf;
    struct mycdev *mycd = file->private_data;

    printk("write *ppos : %lld\n",*ppos);
    
    //�Ѿ�����bufferβ����
    if(*ppos == sizeof(mycd->buffer))
        return -1;

    //������С > bufferʣ����ֽ���(�ж��ٿռ��д��������)
    if(size > sizeof(mycd->buffer) - *ppos)
        n = sizeof(mycd->buffer) - *ppos;
    else 
        n = size;

    //����һ���ļ�λ��ָ���λ�ÿ�ʼд������
    kbuf = mycd->buffer + *ppos;

    //�������ݵ��ں˿ռ�
    ret = copy_from_user(kbuf, ubuf, n);
    if(ret != 0)
        return -EFAULT;

    //�����ļ�λ��ָ���ֵ
    *ppos += n;
    
    //����dev_fifo.len 
    mycd->len += n;

    printk("dev_fifo_write success!\n");
    return n;
}

//linux �ں���2.6�Ժ��Ѿ�������ioctl����ָ��ṹ��ȡ����֮����unlocked_ioctl
long dev_fifo_unlocked_ioctl(struct file *file, unsigned int cmd,
    unsigned long arg)
{
    int ret = 0;
    struct mycdev *mycd = file->private_data;
    
    switch(cmd)
    {
        case DEV_FIFO_CLEAN:
            printk("CMD:CLEAN\n");
            memset(mycd->buffer, 0, sizeof(mycd->buffer));
            break;

        case DEV_FIFO_SETVALUE:
            printk("CMD:SETVALUE\n");
            mycd->len = arg;
            break;

        case DEV_FIFO_GETVALUE:
            printk("CMD:GETVALUE\n");
            ret = put_user(mycd->len, (int *)arg);
            break;

        default:
            return -EFAULT;
    }

    return ret;
}


//�豸���������ӿ�
static const struct file_operations fifo_operations = {
    .owner = THIS_MODULE,
    .open = dev_fifo_open,
    .read = dev_fifo_read,
    .write = dev_fifo_write,
    .unlocked_ioctl = dev_fifo_unlocked_ioctl,
};


//ģ�����
int __init dev_fifo_init(void)
{
    int i = 0;
    int n = 0;
    int ret;
    struct device *device;
    
    gcd = kzalloc(ndevices * sizeof(struct mycdev), GFP_KERNEL);
    if(!gcd){
        return -ENOMEM;
    }

    //�豸�� : ���豸��(12bit) | ���豸��(20bit)
    dev_num = MKDEV(MAJOR_NUM, 0);

    //��̬ע���豸��
    ret = register_chrdev_region(dev_num,ndevices,"dev_fifo");
    if(ret < 0){

        //��̬ע��ʧ�ܣ����ж�̬ע���豸��
        ret = alloc_chrdev_region(&dev_num,0,ndevices,"dev_fifo");
        if(ret < 0){
            printk("Fail to register_chrdev_region\n");
            goto err_register_chrdev_region;
        }
    }
    
    //�����豸��
    cls = class_create(THIS_MODULE, "dev_fifo");
    if(IS_ERR(cls)){
        ret = PTR_ERR(cls);
        goto err_class_create;
    }
    
    printk("ndevices : %d\n",ndevices);
    
    for(n = 0;n < ndevices;n ++)
    {
        //��ʼ���ַ��豸
        cdev_init(&gcd[n].cdev,&fifo_operations);

        //����豸������ϵͳ
        ret = cdev_add(&gcd[n].cdev,dev_num + n,1);
        if (ret < 0)
        {
            goto err_cdev_add;
        }
        //�����豸��Ϣ���û��ռ�(/sys/class/����/�豸��)
        device = device_create(cls,NULL,dev_num + n,NULL,"dev_fifo%d",n);
        if(IS_ERR(device)){
            ret = PTR_ERR(device);
            printk("Fail to device_create\n");
            goto err_device_create;    
        }
    }
    printk("Register dev_fito to system,ok!\n");

    
    return 0;

err_device_create:
    //���Ѿ��������豸��Ϣ��ȥ
    for(i = 0;i < n;i ++)
    {
        device_destroy(cls,dev_num + i);    
    }

err_cdev_add:
    //���Ѿ���ӵ�ȫ����ȥ
    for(i = 0;i < n;i ++)
    {
        cdev_del(&gcd[i].cdev);
    }

err_class_create:
    unregister_chrdev_region(dev_num, ndevices);

err_register_chrdev_region:
    return ret;

}

void __exit dev_fifo_exit(void)
{
    int i;

    //ɾ��sysfs�ļ�ϵͳ�е��豸
    for(i = 0;i < ndevices;i ++)
    {
        device_destroy(cls,dev_num + i);    
    }

    //ɾ��ϵͳ�е��豸��
    class_destroy(cls);
 
    //��ϵͳ��ɾ����ӵ��ַ��豸
    for(i = 0;i < ndevices;i ++)
    {
        cdev_del(&gcd[i].cdev);
    }
    
    //�ͷ�������豸��
    unregister_chrdev_region(dev_num, ndevices);

    return;
}
module_init(dev_fifo_init);
module_exit(dev_fifo_exit);

