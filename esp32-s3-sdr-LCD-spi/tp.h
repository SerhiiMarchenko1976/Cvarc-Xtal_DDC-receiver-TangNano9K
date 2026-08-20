int xwin=0;//начало координат экрана(если экран больше 480х272)
int ywin=0;

//описания экранных кнопок для тачскрина
struct {
  char* b_name =(char*)"BAND";
  int w=130;
  int h=20;
  int x_min=260+xwin;
  int x_max=x_min+w+xwin;
  int y_min=10+ywin;
  int y_max=y_min+h+ywin;
  }tp_nband;


struct {
  char* b_name =(char*)"6000";
  int w=50;
  int h=20;
  int x_min=20+xwin;
  int x_max=x_min+w+xwin;
  int y_min=80+ywin;
  int y_max=y_min+h+ywin;
  }tp_bw6000;

struct {
  char* b_name =(char*)"3000";
  int w=50;
  int h=20;
  int x_min=20+xwin;
  int x_max=x_min+w+xwin;
  int y_min=110+ywin;
  int y_max=y_min+h+ywin;
  }tp_bw3000;

struct {
  char* b_name =(char*)"2400";
  int w=50;
  int h=20;
  int x_min=20+xwin;
  int x_max=x_min+w+xwin;
  int y_min=140+ywin;
  int y_max=y_min+h+ywin;
  }tp_bw2400;

struct {
  char* b_name =(char*)"200";
  int w=50;
  int h=20;
  int x_min=20+xwin;
  int x_max=x_min+w+xwin;
  int y_min=170+ywin;
  int y_max=y_min+h+ywin;
  }tp_bw500;

struct {
  char* b_name =(char*)"1000";
  int w=50;
  int h=20;
  int x_min=130+xwin;
  int x_max=x_min+w+xwin;
  int y_min=80+ywin;
  int y_max=y_min+h+ywin;
  }tp_st1000;

struct {
  char* b_name =(char*)"500";
  int w=50;
  int h=20;
  int x_min=130+xwin;
  int x_max=x_min+w+xwin;
  int y_min=110+ywin;
  int y_max=y_min+h+ywin;
  }tp_st500;

struct {
  char* b_name =(char*)"100";
  int w=50;
  int h=20;
  int x_min=130+xwin;
  int x_max=x_min+w+xwin;
  int y_min=140+ywin;
  int y_max=y_min+h+ywin;
  }tp_st100;

struct {
  char* b_name =(char*)"50";
  int w=50;
  int h=20;
  int x_min=130+xwin;
  int x_max=x_min+w+xwin;
  int y_min=170+ywin;
  int y_max=y_min+h+ywin;
  }tp_st50;

struct {
  char* b_name =(char*)"10";
  int w=50;
  int h=20;
  int x_min=130+xwin;
  int x_max=x_min+w+xwin;
  int y_min=200+ywin;
  int y_max=y_min+h+ywin;
  }tp_st10;

struct {
  char* b_name =(char*)"LSB";
  int w=50;
  int h=20;
  int x_min=245+xwin;
  int x_max=x_min+w+xwin;
  int y_min=80+ywin;
  int y_max=y_min+h+ywin;
  }tp_mode_lsb;

struct {
  char* b_name =(char*)"USB";
  int w=50;
  int h=20;
  int x_min=245+xwin;
  int x_max=x_min+w+xwin;
  int y_min=110+ywin;
  int y_max=y_min+h+ywin;
  }tp_mode_usb;

struct {
  char* b_name =(char*)"AM";
  int w=50;
  int h=20;
  int x_min=245+xwin;
  int x_max=x_min+w+xwin;
  int y_min=140+ywin;
  int y_max=y_min+h+ywin;
  }tp_mode_am;

struct {
  char* b_name =(char*)"24K";
  int w=50;
  int h=20;
  int x_min=360+xwin;
  int x_max=x_min+w+xwin;
  int y_min=80+ywin;
  int y_max=y_min+h+ywin;
  }tp_sr24;

struct {
  char* b_name =(char*)"48K";
  int w=50;
  int h=20;
  int x_min=360+xwin;
  int x_max=x_min+w+xwin;
  int y_min=110+ywin;
  int y_max=y_min+h+ywin;
  }tp_sr48;

struct {
  char* b_name =(char*)"96K";
  int w=50;
  int h=20;
  int x_min=360+xwin;
  int x_max=x_min+w+xwin;
  int y_min=140+ywin;
  int y_max=y_min+h+ywin;
  }tp_sr96;
  
struct {
  char* b_name =(char*)"RIT";
  int w=120;
  int h=40;
  int x_min=260+xwin;
  int x_max=x_min+w+xwin;
  int y_min=50+ywin;
  int y_max=y_min+h+ywin;
  }tp_rit;
