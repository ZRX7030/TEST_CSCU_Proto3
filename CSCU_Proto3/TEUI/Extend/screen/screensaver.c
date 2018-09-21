#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

//此进程会检测用户点击屏幕的动作，对应地调亮或调暗背光。
//因此，TEUI进程无需再管理背光。
//除了具备背光管理功能之外，此进程还会向TEUI进程发送两个信号：
//  SIGUSR1 (10) : 表示进入屏保模式
//  SIGUSR2 (12) : 表示退出屏保模式
//TEUI进程捕获这两个信号，执行相应的UI动作即可。


//#define TSC_DEV                               "/dev/input/touchscreen0"  //Touchscreen device
#define TSC_DEV                               "/dev/input/event0"  //Touchscreen device
#define BRIGHTNESS_CONTROLLER                 "/sys/class/backlight/pwm-backlight/brightness"  //Backlight brightness controller
#define BRIGHTNESS_DEFAULT                    "75\n"    //Default backlight brightness (%)
#define BRIGHTNESS_LOW                        "10\n"    //Lower backlight brightness (%)
#define ENTER_SCRN_SAVING_TIME                120       //Wait how long (seconds) to enter scrnsaving mode 

static int g_is_scrnsaving_mode = 1;

//Set backlight brightness
//return 0 if success, else return -1.
static int BL_set_brightness(char *brightness)
{
  int fd;
  int ret;
  
  fd = open(BRIGHTNESS_CONTROLLER, O_RDWR);
  if(fd < 0) {
    printf("%s(): ", __func__);
    perror("open() ");
    return -1;
  }
  
  ret = write(fd, brightness, strlen(brightness));
  if(ret != strlen(brightness)) {
    printf("%s(): ", __func__);
    perror("write() ");
    return -1;
  }
  
  printf("Write brightness = %s", brightness);
  return 0;
}

static void enter_scrnsaving_mode(int sig)
{
  int ret;
    
  if(g_is_scrnsaving_mode) {
    return;
  }
  
  ret = BL_set_brightness(BRIGHTNESS_LOW);
  if(ret)  {
    return;
  }
  
  printf("%s() \n", __func__);
  
  g_is_scrnsaving_mode = 1;
  
  //@TODO: Send a signal to QT APP...
  //kill(-1, SIGUSR1);
  system("killall -10 TEUI");
}

static void exit_scrnsaving_mode(void)
{
  int ret;
    
  if(!g_is_scrnsaving_mode) {
    return;
  }
  
  ret = BL_set_brightness(BRIGHTNESS_DEFAULT);
  if(ret)  {
    return;
  }
  
  printf("%s() \n", __func__);
  
  g_is_scrnsaving_mode = 0;
  
  //@TODO: Send another signal to QT APP...
  //kill(-1, SIGUSR2);
  system("killall -12 TEUI");
}

int main(int argc, char *argv[])
{
  int fd;
  int ret;
  struct input_event event;
  
  g_is_scrnsaving_mode = 1;
  exit_scrnsaving_mode();
  
  //Start the timer
  signal(SIGALRM, enter_scrnsaving_mode);
  alarm(ENTER_SCRN_SAVING_TIME);
  
  fd = open(TSC_DEV, O_RDONLY);
  if(fd <= 0) {
    printf("%s(): ", __func__);
    perror("open() ");
    return fd;
  }
  
  while(1) {
    //Read from the input device
    ret = read(fd, &event, sizeof(event));  //Blocking here
    if(ret != sizeof(event)) {
      printf("%s(): ", __func__);
      perror("read() ");
      continue;
    }
   
	//printf("event.type is ...%d value is %d\n", event.type, event.value);
    //Parse event data
    if(event.type == EV_ABS) {  //Touchscreen is pressed
      exit_scrnsaving_mode();
      alarm(ENTER_SCRN_SAVING_TIME);  //Reload the timer
    }

	if(event.type == EV_KEY && event.value == 0)    //key  action and key up
	{
		system("buzzer-out.sh &");
	}
  }  //while(1)
  
  close(fd);
  return 0;
}
