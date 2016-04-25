#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

//ָ�������豸��
#define MAJOR_NUM  250 

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
struct class  *cls;

//���豸
static int dev_fifo_open(struct inode *inode, struct file *file)
{	
	printk("dev_fifo_open success!\n");
		
	return 0;
}

//���豸
static ssize_t dev_fifo_read(struct file *file, char __user *ubuf, size_t size, loff_t *ppos)
{
	int n;
	int ret;
	char *kbuf;
	
	printk("read *ppos : %lld\n",*ppos);

	if(*ppos == gcd->len)
		return 0;

	//������С > bufferʣ����ֽ��� :��ȡʵ�ʼǵ��ֽ���
	if(size > gcd->len - *ppos)
		n = gcd->len  - *ppos;
	else 
		n = size;
	
	printk("n = %d\n",n);
	//����һ���ļ�λ��ָ���λ�ÿ�ʼ��ȡ����
	kbuf = gcd->buffer + *ppos;

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

	printk("write *ppos : %lld\n",*ppos);
	
	//�Ѿ�����bufferβ����
	if(*ppos == sizeof(gcd->buffer))
		return -1;

	//������С > bufferʣ����ֽ���(�ж��ٿռ��д��������)
	if(size > sizeof(gcd->buffer) - *ppos)
		n = sizeof(gcd->buffer) - *ppos;
	else 
		n = size;

	//����һ���ļ�λ��ָ���λ�ÿ�ʼд������
	kbuf = gcd->buffer + *ppos;

	//�������ݵ��ں˿ռ�
	ret = copy_from_user(kbuf, ubuf, n);
	if(ret != 0)
		return -EFAULT;

	//�����ļ�λ��ָ���ֵ
	*ppos += n;
	
	//����dev_fifo.len 
	gcd->len += n;

	printk("dev_fifo_write success!\n");
	return n;
}

//�豸���������ӿ�
static const struct file_operations fifo_operations = {
	.owner = THIS_MODULE,
	.open  = dev_fifo_open,
	.read  = dev_fifo_read,
	.write = dev_fifo_write,
};


//ģ�����
int __init dev_fifo_init(void)
{
	int ret;
	struct device *device;
	
	gcd = kzalloc(sizeof(struct mycdev), GFP_KERNEL);
	if(!gcd){
		return -ENOMEM;
	}

	//�豸�� : ���豸��(12bit) | ���豸��(20bit)
	dev_num = MKDEV(MAJOR_NUM, 0);

	//��̬ע���豸��
	ret = register_chrdev_region(dev_num,1,"dev_fifo");
	if(ret < 0){

		//��̬ע��ʧ�ܣ����ж�̬ע���豸��
		ret = alloc_chrdev_region(&dev_num,0,1,"dev_fifo");
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
	
	
	//��ʼ���ַ��豸
	cdev_init(&gcd->cdev,&fifo_operations);

	//����豸������ϵͳ
	ret = cdev_add(&gcd->cdev,dev_num,1);
	if (ret < 0)
	{
		goto  err_cdev_add;
	}
	//�����豸��Ϣ���û��ռ�(/sys/class/����/�豸��)
	device = device_create(cls,NULL,dev_num,NULL,"dev_fifo%d",0);
	if(IS_ERR(device)){
		ret = PTR_ERR(device);
		printk("Fail to device_create\n");
		goto err_device_create;	
	}
	printk("Register dev_fito to system,ok!\n");

	
	return 0;

err_device_create:
	cdev_del(&gcd->cdev);

err_cdev_add:
	class_destroy(cls);

err_class_create:
	unregister_chrdev_region(dev_num, 1);

err_register_chrdev_region:
	return ret;

}

void __exit dev_fifo_exit(void)
{
	//ɾ��sysfs�ļ�ϵͳ�е��豸
	device_destroy(cls,dev_num );	

	//ɾ��ϵͳ�е��豸��
	class_destroy(cls);
 
	//��ϵͳ��ɾ����ӵ��ַ��豸
	cdev_del(&gcd->cdev);
	
	//�ͷ�������豸��
	unregister_chrdev_region(dev_num, 1);

	return;
}


module_init(dev_fifo_init);
module_exit(dev_fifo_exit);
