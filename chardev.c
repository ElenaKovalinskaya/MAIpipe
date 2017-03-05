#include <linux/kernel.h> /* Для printk() и т.д. */
#include <linux/module.h> /* Эта частичка древней магии, которая оживляет модули */
#include <linux/init.h> /* Определения макросов */
#include <linux/fs.h>
#include <asm/uaccess.h> /* put_user */

#define BUF_LEN 80

/* 
 * Ответ устройства на запрос
 */
static char Message[BUF_LEN];

/* 
 * Позиция в буфере.
 * Используется в том случае, если сообщение оказывется длиннее
 * чем размер буфера. 
 */
static char *Message_Ptr;


// Ниже мы задаём информацию о модуле, которую можно будет увидеть с помощью Modinfo
MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "programming paralympics" );
MODULE_DESCRIPTION( "My new usless module" );
MODULE_SUPPORTED_DEVICE( "test" ); /* /dev/testdevice */

#define SUCCESS 0
#define DEVICE_NAME "test" /* Имя нашего устройства */

// Поддерживаемые нашим устройством операции
static int device_open( struct inode *, struct file * );
static int device_release( struct inode *, struct file * );
static ssize_t  device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset);
static ssize_t device_write(struct file *, const char __user *, size_t length, loff_t *);

// Глобальные переменные, объявлены как static, воизбежание конфликтов имен.
static int major_number; /* Старший номер устройства нашего драйвера */
static int is_device_open = 0; /* Используется ли девайс ? */
static char text[ 5 ] = "test\n"; /* Текст, который мы будет отдавать при обращении к нашему устройству */
static char* text_ptr = text; /* Указатель на текущую позицию в тексте */

// Прописываем обработчики операций на устройством
static struct file_operations fops =
 {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
 };

// Функция загрузки модуля. Входная точка. Можем считать что это наш main()
static int __init test_init( void )
{
 printk( KERN_ALERT "TEST driver loaded!\n" );

 // Регистрируем устройсво и получаем старший номер устройства
 major_number = register_chrdev( 0, DEVICE_NAME, &fops );

 if ( major_number < 0 )
 {
  printk( "Registering the character device failed with %d\n", major_number );
  return major_number;
 }

 // Сообщаем присвоенный нам старший номер устройства
 printk( "Test module is loaded!\n" );

 printk( "Please, create a dev file with 'mknod /dev/test c %d 0'.\n", major_number );

 return SUCCESS;
}

// Функция выгрузки модуля
static void __exit test_exit( void )
{
 // Освобождаем устройство
 unregister_chrdev( major_number, DEVICE_NAME );

 printk( KERN_ALERT "Test module is unloaded!\n" );
}

// Указываем наши функции загрузки и выгрузки
module_init( test_init );
module_exit( test_exit );

static int device_open( struct inode *inode, struct file *file )
{

  printk("device_open\n");
 text_ptr = Message;

 if ( is_device_open )
  return -EBUSY;

 is_device_open++;

 return SUCCESS;
}

static int device_release( struct inode *inode, struct file *file )
{
 printk("device_release\n");
 is_device_open--;
 return SUCCESS;
}


static ssize_t device_write(struct file *file, const char  __user * buffer, size_t length, loff_t * offset)
{
  int i;

  printk("device_write\n");

  for (i = 0; i < length && i < BUF_LEN; i++)
    get_user(Message[i], buffer + i);

  printk("!*%s\n", Message);

  Message_Ptr = Message;

  /* 
   * Вернуть количество принятых байт
   */
  return i;
}


static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset)
{
  /* 
   * Количество байт, фактически записанных в буфер
   */

  printk("device_read\n");

  int byte_read = 0;

 if ( *text_ptr == 0 )
  return 0;

 while ( length && *text_ptr )
 {
  put_user( *( text_ptr++ ), buffer++ );
  length--;
  byte_read++;
 }


 return byte_read;

}

