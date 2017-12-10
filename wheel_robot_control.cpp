#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// #include <curses.h>
#include <linux/input.h>
#include <fcntl.h>
#include <termio.h>
#include <memory.h>

class Arduino
{
public:
    // 打开串口
    int open_port()
    {
        struct termios myterio;
        if(((m_Socket_fd = ::open("/dev/ttyACM0", O_RDWR|O_SYNC)) < 0) && ((m_Socket_fd = ::open("/dev/ttyACM1", O_RDWR|O_SYNC)) < 0))  //O_NOCTTY|O_NONBLOCK
        {
        	fprintf(stderr,"open port error\n");
            printf("pen port error\n");
            return -1;
        }
        memset(&myterio, 0, sizeof(myterio));
        myterio.c_cflag      = B9600|CS8|CLOCAL|CREAD;
        myterio.c_iflag      = IGNPAR;
        myterio.c_oflag      = 0;
        myterio.c_lflag      = 0;
        myterio.c_cc[VTIME]  = 0;
        myterio.c_cc[VMIN]   = 0;
        tcsetattr(m_Socket_fd, TCSANOW, &myterio);
        return m_Socket_fd;
    }

    //关闭串口
    void close_port()
    {
        ::close(m_Socket_fd);
        m_Socket_fd=-1;
    }

    //写数据
    unsigned char write_port(char* str)
    {
    	close_port();
    	open_port();
        for(int i = 0; str[i] != 0; i++)
        {
            write(m_Socket_fd, &str[i], 1);
            usleep(10*1000);
        }
        return 0;
    }

    //构造小车控制通信字符串
    void make_msg(int a, int b, int c)
    {
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "%d&%d&%d&", a, b, c);
        printf("vx=%d, vy=%d, w=%d, message=%s length=%d\n", a, b, c, msg, strlen(msg));
        // printw("vx=%d, vy=%d, w=%d, message=%s length=%d\n", a, b, c, msg, strlen(msg));
    }

    //------------------------------------------------
public:
    char msg[100];
    int m_Socket_fd;
};


void limit_value(int &value)
{
    if(value > 62) value = 62;
    if(value < 0) value = 0;
}

int main()
{
  int vx = 0;
  int vy = 0;
  int w = 0;
  int speed[63], index = 31;
  speed[30] = -50;
  speed[31] = 0;
  speed[32] = 50;
  for(int i = 29; i >= 0; --i)
  {
  	speed[i] = speed[i + 1] - 5;
  }
  for(int i = 33; i <= 62; ++i)
  {
  	speed[i] = speed[i - 1] + 5;
  }
  Arduino arduino;


  int keys_fd = open("/dev/input/event2",O_RDONLY);
  if(keys_fd <= 0)
  {
      printf("open event error\n");
      return -1;
  }

  // initscr();
  // cbreak();
  // // noecho();
  // keypad(stdscr, TRUE);
  // clear();
  // mvprintw(5, 5, "WhellRobot key control demo!");
  // refresh();

  arduino.open_port();
  arduino.make_msg(0, 0, 20);
  arduino.write_port(arduino.msg);
  usleep(2000*1000);
  vx = vy = w = 0;
  arduino.make_msg(vx, vy, w);
  arduino.write_port(arduino.msg);
  struct input_event t;
  while(1) {
    // move(7, 5);
    //     clrtoeol();
        read(keys_fd,&t,sizeof(struct input_event));
        if(t.type == 1 && t.value == 1)
        {
            if(t.code == 17)
            {
            	if(index > 31)
            	{
            		index = 30;
            	}
            	--index;
            	limit_value(index);
            	vx = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg); 
        	}
            if(t.code == 31)
            {
            	if(index < 31)
            	{
            		index = 32;
            	}
            	++index;
            	limit_value(index);
            	vx = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg);	 
            }
            if(t.code == 30)
            {
            	if(index < 31)
            	{
            		index = 32;
            	}
            	++index;
            	limit_value(index);
            	vy = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg);
            }
            if(t.code == 32)
            {
            	if(index > 31)
            	{
            		index = 30;
            	}
            	--index;
            	limit_value(index);
            	vy = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg); 
            }
            if(t.code == 16)
            {
            	if(index > 31)
            	{
            		index = 30;
            	}
            	--index;
            	limit_value(index);
            	w = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg); 
            }
            if(t.code == 18)
            {
            	if(index < 31)
            	{
            		index = 32;
            	}
            	++index;
            	limit_value(index);
            	w = speed[index];
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg); 
            }
            if(t.code == 57)
            {
            	vx = vy = w = 0;
            	arduino.make_msg(vx, vy, w);
            	arduino.write_port(arduino.msg); 
            }
            // refresh();
        }
  }
  // close(keys_fd);
  arduino.close_port();

  // endwin();
  exit(0);
}

// sudo apt-get install libncurses5-dev
// sudo g++ wheel_robot_control.cpp -o wheel_robot_control -lcurses
