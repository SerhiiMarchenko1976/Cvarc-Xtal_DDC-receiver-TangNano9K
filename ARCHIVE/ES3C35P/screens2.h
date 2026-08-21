void draw_Grid(int px, int py, int w, int h )
{
    // рисование сетки на спектре
    int div =  (int)(h / 6);
    for (int x = 0; x < w + div; x += div)if((px+x)<xwin+479){
      gfx->drawFastVLine(px+x, py, h, GRAY);
    }
    gfx->drawFastVLine(xwin+479, py, h, GRAY);
    for (int y = 0; y < h + div; y += div){
        if(y>h)break;
        //gfx->drawFastHLine(px, px+w, py+y,GRAY);
        gfx->drawLine(px,py+y, px+w, py+y,GRAY);
      }
}

void draw_spectr(){//спектр
      int x = 0;
      int y = 0;
      int static p_y=0;
      int static p_x=0;
      int16_t col_s;
      int width_b = bandwidth/(i2s_sample_rate_rx/NUM_SAMPLE_BUF);
      int indent_b = indent/(i2s_sample_rate_rx/NUM_SAMPLE_BUF);
      gfx->fillRect(0+xwin,70+ywin,480,108,BLACK);
      for (int i = 0; i<NUM_SAMPLE_BUF/2;i++){
       y=wp_value[i]/20-10;if(y>102)y=102;
       col_s = 0b0000000101001100;
       switch(rf_mode){ //смена цвета спектра в полосе пропускания и рисование "шторки"
        case LSB:
          if((i<=pos_fft-indent_b)&&(i>=pos_fft-width_b-indent_b)){
            gfx->drawFastVLine(x+xwin,74+ywin,102,0b0001100110000111); //"шторка"
            col_s = col_s|0b1000010000010000; //увеличение яркости спектра в полосе пропускания
            }break;
        case USB:
          if((i>=pos_fft+indent_b)&&(i<=pos_fft+width_b+indent_b)){
            gfx->drawFastVLine(x+xwin,74+ywin,102,0b0001100110000111);//"шторка"
            col_s = col_s|0b1000010000010000; //увеличение яркости спектра в полосе пропускания
            }break;
        case  AM:
          if(((i<=pos_fft)&&(i>=pos_fft-width_b/2))||((i>=pos_fft)&&(i<=pos_fft+width_b/2))){
            gfx->drawFastVLine(x+xwin,74+ywin,102,0b0001100110000111);//"шторка"
            col_s = col_s|0b1000010000010000; //увеличение яркости спектра в полосе пропускания
            }break;
       }
      gfx->drawFastVLine(x+xwin,175-y+ywin,y,col_s); //рисование подготовленного спектра
      if(x>0)gfx->drawLine(p_x,p_y,x+xwin,175-y+ywin,GRAY|0b0100001000001000);//линия по верхушкам спектра
      p_x=x+xwin;p_y=175-y+ywin;
      x++;if(x>503)break;
     }
}

void draw_waterfall(){ //отображаем массив буферов fft для водопада
  static uint16_t marker[NUM_SAMPLE_BUF/2-9]={0};
  uint8_t y = 190;
  int x = 0;
  for (int i=WP_LINE;i>0;i--){
    //вывод подготовленных строк водопада в видеобуфер
    gfx->draw16bitRGBBitmap(x+xwin,i+y+ywin,(uint16_t*)&wp[wp_num[i-1]][0],NUM_SAMPLE_BUF/2-9,1);
  }
  gfx->drawFastVLine(x+xwin+pos_fft,y+ywin+1,WP_LINE,YELLOW); //вертикальный маркер текущей частоты
}

void draw_service(){
    const char* mod;const char* filtr;const char* tun;const char* type_fir;const char* step;
    gfx->drawRect(0+xwin,0+ywin,250,70,YELLOW);gfx->drawRect(249+xwin,0+ywin,256,70,YELLOW);
    gfx->fillRect(250+xwin,1+ywin,254,68,BLACK);
    gfx->drawFastVLine(400+xwin,0+ywin,70,YELLOW);
    gfx->drawFastVLine(479+xwin,0+ywin,70,YELLOW);
    if (rf_mode == LSB) mod = "LSB";
    if (rf_mode == USB) mod = "USB";
    if (rf_mode == AM)  mod = "AM ";
    if (num_filter == 0) filtr = "3000";
    if (num_filter == 1) filtr = "2400";
    if (num_filter == 2) filtr = " 500";
    if (num_filter == 3) filtr = "6000";
    if (tun_mode==0) tun = "TUN";
    if (tun_mode==1) tun = "RIT";
    if (tun_mode==2) tun = "PAN";
    if (numstep==0) step = "  10";
    if (numstep==1) step = "  50";
    if (numstep==2) step = " 100";    
    if (numstep==3) step = " 500";    
    if (numstep==4) step = "1000";
        gfx->setFont(&FreeMonoBold8pt8b);
        gfx->setTextColor(colors[5],BLACK);
        gfx->setCursor(436+xwin,17+ywin);gfx->print(filtr);
        gfx->setCursor(444+xwin,32+ywin);gfx->print(mod);
        gfx->setCursor(436+xwin,47+ywin);gfx->print(step);
        gfx->setCursor(444+xwin,62+ywin);gfx->print(i2s_sample_rate_rx/1000);gfx->print("K");
        gfx->setFont(&CourierCyr6pt8b);gfx->setTextColor(WHITE,BLACK);
        gfx->setCursor(403+xwin,17+ywin);gfx->print("BW:");
        gfx->setCursor(403+xwin,32+ywin);gfx->print("Mode:");
        gfx->setCursor(403+xwin,47+ywin);gfx->print("Step:");
        gfx->setCursor(403+xwin,62+ywin);gfx->print("S/R:");
        if(tun_mode == RIT||tun_mode == PAN)gfx->setTextColor(WHITE,BLACK);else gfx->setTextColor(GRAY|0b0100001000001000,BLACK);
        gfx->setCursor(257+xwin,52+ywin);
        gfx->print("RIT ");gfx->printf("%2d %03d,%02d",  rit_freq/1000000, (rit_freq/1000)%1000, (rit_freq/10)%100 );
        gfx->setCursor(257+xwin,64+ywin);
        if(tun_mode == PAN)gfx->setTextColor(WHITE,BLACK);else gfx->setTextColor(GRAY|0b0100001000001000,BLACK);
        if(txrx_mode==TX_MODE)gfx->setTextColor(RED,BLACK);
        gfx->print("XIT ");gfx->printf("%2d %03d,%02d",  tx_freq/1000000, (tx_freq/1000)%1000, (tx_freq/10)%100 );
        
      int c_freq = freq+hz*PCH; //
      gfx->setFont(&Seven_Segment18pt8b);gfx->setTextSize(1);
      gfx->setTextColor((colors[8]),BLACK);
      gfx->setCursor(255+xwin,30+ywin);
      gfx->printf("%2d %03d,%02d",  c_freq/1000000, (c_freq/1000)%1000, (c_freq/10)%100 );
      gfx->drawFastHLine(249+xwin,38+ywin,152,YELLOW);
        
      gfx->setFont(&Picopixel);
      gfx->setTextColor(GRAY|0b0100001000001000,BLACK);
      gfx->fillRect(xwin,176+ywin,480,15,BLACK);
      for(int i=0;i<NUM_SAMPLE_BUF/2;i+=52){ //шкала видимых частот
        gfx->setCursor(i+xwin,185+ywin);
        c_freq=freq+hz*i;
        gfx->printf("%2d %03d,%02d",  c_freq/1000000, (c_freq/1000)%1000, (c_freq/10)%100 );
       gfx->drawFastVLine(i+xwin,175+ywin,3,GRAY|0b0100001000001000);//маркеры на частотах
      }
   
}

void draw_smeter(){

  int x = xwin+125;
  int y = ywin+120;
  int h = 110;
  static int d_x=x;
  static int d_y=y;
  gfx->draw16bitRGBBitmap(35+xwin,1+ywin,(uint16_t*)&s_smeter,184,60);
  float _smeter = -PI*(150-old_smeter)/205;
  d_x=x+cos(_smeter)*h;
  d_y= y+sin(_smeter)*h;
  gfx->drawLine(x, y, d_x, d_y,GREEN);
  gfx->fillRect(50+xwin,60+ywin,180,9,BLACK);
  if(wifi)gfx->draw16bitRGBBitmap(3,5,(uint16_t*)&wifi_on,15,15);
  else gfx->draw16bitRGBBitmap(3,5,(uint16_t*)&wifi_off,15,15);
}

void draw_temp_value(){
  uint16_t s_col = (0b11001010);
  if(show_band>1){
    gfx->setTextColor(0b1000001000000100);
    gfx->setFont(&Seven_Segment18pt8b);
    gfx->setTextSize(2);
    gfx->setCursor(180+xwin,150+ywin);
    gfx->print(bands[numband].name);
    gfx->setTextSize(1);
    return;
  }
  if(show_volume>1){
    gfx->setTextColor(0b1000010000010000);
    gfx->setFont(&FreeMonoBold14pt8b);
    gfx->setTextSize(2);
    gfx->setCursor(100+xwin,120+ywin);
    gfx->print("Volume: ");gfx->print(agc_coeff);
    gfx->setTextSize(1);
    return;
  }
  if(show_srate>1){
    gfx->setTextColor(0b1100011000011000);
    gfx->setFont(&FreeMonoBold8pt8b);
    gfx->setTextSize(2);
    gfx->setCursor(180+xwin,120+ywin);
    gfx->print(i2s_sample_rate_rx/1000);gfx->print(" KS/sec");
    gfx->setTextSize(1);
    return;
  }
  if(show_conf>1){
    gfx->setTextColor(0b1100011000011000);
    gfx->setFont(&FreeMonoBold8pt8b);
    gfx->setTextSize(2);
    gfx->setCursor(80+xwin,120+ywin);
    gfx->print("Parameters saved");
    gfx->setTextSize(1);
    return;
  }
  
}

void drawinfo(){
      gfx->setFont(&FreeMonoBold8pt8b);gfx->setTextColor(WHITE);
      
      //gfx->setCursor(5,250);
      //gfx->print(tp_x);gfx->print(":");gfx->print(tp_y);gfx->print(":");gfx->print(tp_size);
      
      if(draw_info){
        if(t_fps<=0)t_fps=1;
        gfx->setCursor(420,270);
        gfx->print(1000/t_fps);
        gfx->print("Fps");
      }
      int  my_s = timeinfo.tm_sec;
      int  my_m = timeinfo.tm_min;
      int  my_h = timeinfo.tm_hour;
      int  my_d = timeinfo.tm_mday;
      int  my_mm = timeinfo.tm_mon+1;
      int  my_y = timeinfo.tm_year+1900;
      gfx->setCursor(5,270);
      gfx->printf("%02d.%02d.%4d GMT:%02d:%02d:%02d",my_d,my_mm,my_y,my_h,my_m,my_s);
      //gfx->print(" ");gfx->print(mount);
      //gfx->setCursor(5,250);
      gfx->print(" ");
      gfx->print(i_tx,HEX);gfx->print("/");
      //gfx->setCursor(5,270);
      gfx->print(i_rx,HEX);
      //gfx->print(" ");gfx->print(a_heap);gfx->print("/");gfx->print(b_heap);
}
/*
void scroll_wp(){ //сдвиг массива строк "водопада"
      uint8_t tmp = wp_num[WP_LINE-1];
      for (int i=WP_LINE-1;i>0;i--) {wp_num[i] = wp_num[i-1];}
      wp_num[0]=tmp;
}*/

void peak_down(){//плавно снижаем уровни спектра и панорамы
   for (int i=0;i<NUM_SAMPLE_BUF;i++){
        if (fft_inter[i]>limited_fft/2)fft_inter[i]-=limited_fft/20;
        if (fft_inter[i]>limited_fft/5)fft_inter[i]-=limited_fft/100;
        if (fft_inter[i]>limited_fft/10)fft_inter[i]-=limited_fft/200;
        if (fft_inter[i]<=limited_fft/10)fft_inter[i]-=limited_fft/500;
        
   }
}

void screen_control(){
  
  if(tp_control){
    gfx->setFont(&FreeMonoBold8pt8b);
    gfx->setTextColor(WHITE);
    gfx->fillRoundRect(tp_bw6000.x_min,tp_bw6000.y_min,tp_bw6000.w,tp_bw6000.h,3,(num_filter==3)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_bw6000.x_min+5,tp_bw6000.y_min+13);
    gfx->print((char*)tp_bw6000.b_name);

    gfx->fillRoundRect(tp_bw3000.x_min,tp_bw3000.y_min,tp_bw3000.w,tp_bw3000.h,3,(num_filter==0)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_bw3000.x_min+5,tp_bw3000.y_min+13);
    gfx->print((char*)tp_bw3000.b_name);

    gfx->fillRoundRect(tp_bw2400.x_min,tp_bw2400.y_min,tp_bw2400.w,tp_bw2400.h,3,(num_filter==1)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_bw2400.x_min+5,tp_bw2400.y_min+13);
    gfx->print((char*)tp_bw2400.b_name);

    gfx->fillRoundRect(tp_bw500.x_min,tp_bw500.y_min,tp_bw500.w,tp_bw500.h,3,(num_filter==2)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_bw500.x_min+5,tp_bw500.y_min+13);
    gfx->print((char*)tp_bw500.b_name);

////////////////
    gfx->fillRoundRect(tp_st1000.x_min,tp_st1000.y_min,tp_st1000.w,tp_st1000.h,3,(numstep==4)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_st1000.x_min+5,tp_st1000.y_min+13);
    gfx->print((char*)tp_st1000.b_name);

    gfx->fillRoundRect(tp_st500.x_min,tp_st500.y_min,tp_st500.w,tp_st500.h,3,(numstep==3)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_st500.x_min+5,tp_st500.y_min+13);
    gfx->print((char*)tp_st500.b_name);

    gfx->fillRoundRect(tp_st100.x_min,tp_st100.y_min,tp_st100.w,tp_st100.h,3,(numstep==2)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_st100.x_min+5,tp_st100.y_min+13);
    gfx->print((char*)tp_st100.b_name);

    gfx->fillRoundRect(tp_st50.x_min,tp_st50.y_min,tp_st50.w,tp_st50.h,3,(numstep==1)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_st50.x_min+5,tp_st50.y_min+13);
    gfx->print((char*)tp_st50.b_name);
    
    gfx->fillRoundRect(tp_st10.x_min,tp_st10.y_min,tp_st10.w,tp_st10.h,3,(numstep==0)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_st10.x_min+5,tp_st10.y_min+13);
    gfx->print((char*)tp_st10.b_name);

/////////

    gfx->fillRoundRect(tp_mode_lsb.x_min,tp_mode_lsb.y_min,tp_mode_lsb.w,tp_mode_lsb.h,3,(rf_mode==0)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_mode_lsb.x_min+5,tp_mode_lsb.y_min+13);
    gfx->print((char*)tp_mode_lsb.b_name);
    
    gfx->fillRoundRect(tp_mode_usb.x_min,tp_mode_usb.y_min,tp_mode_usb.w,tp_mode_usb.h,3,(rf_mode==1)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_mode_usb.x_min+5,tp_mode_usb.y_min+13);
    gfx->print((char*)tp_mode_usb.b_name);

    gfx->fillRoundRect(tp_mode_am.x_min,tp_mode_am.y_min,tp_mode_am.w,tp_mode_am.h,3,(rf_mode==2)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_mode_am.x_min+5,tp_mode_am.y_min+13);
    gfx->print((char*)tp_mode_am.b_name);

    gfx->fillRoundRect(tp_sr24.x_min,tp_sr24.y_min,tp_sr24.w,tp_sr24.h,3,(i2s_sample_rate_rx==24000)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_sr24.x_min+5,tp_sr24.y_min+13);
    gfx->print((char*)tp_sr24.b_name);

    gfx->fillRoundRect(tp_sr48.x_min,tp_sr48.y_min,tp_sr24.w,tp_sr48.h,3,(i2s_sample_rate_rx==48000)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_sr48.x_min+5,tp_sr48.y_min+13);
    gfx->print((char*)tp_sr48.b_name);

    gfx->fillRoundRect(tp_sr96.x_min,tp_sr96.y_min,tp_sr96.w,tp_sr96.h,3,(i2s_sample_rate_rx==96000)?0b0110000000001100:0b0000010000000000);
    gfx->setCursor(tp_sr96.x_min+5,tp_sr96.y_min+13);
    gfx->print((char*)tp_sr96.b_name);

    gfx->drawRoundRect(tp_sr24.x_min-5,tp_sr24.y_min-5,tp_sr24.w+55,90,5,WHITE);
    gfx->setCursor(tp_sr24.x_min+tp_sr24.w+5,tp_sr24.y_min+40);
    gfx->print("S/R");

    gfx->drawRoundRect(tp_mode_lsb.x_min-5,tp_mode_lsb.y_min-5,tp_mode_lsb.w+55,90,5,WHITE);
    gfx->setCursor(tp_mode_lsb.x_min+tp_mode_lsb.w+5,tp_mode_lsb.y_min+40);
    gfx->print("Mode");

    gfx->drawRoundRect(tp_st1000.x_min-5,tp_st1000.y_min-5,tp_st1000.w+55,150,5,WHITE);
    gfx->setCursor(tp_st1000.x_min+tp_st1000.w+5,tp_st1000.y_min+60);
    gfx->print("Step");

    gfx->drawRoundRect(tp_bw6000.x_min-5,tp_bw6000.y_min-5,tp_bw6000.w+50,120,5,WHITE);
    gfx->setFont(&FreeMonoBold14pt8b);
    gfx->setCursor(tp_bw6000.x_min+tp_bw6000.w+5,tp_bw6000.y_min+60);
    gfx->print("BW");

  }
  if(tp_band){
    int x=20;
    int y=80;
    int c = 0b0000010000000000;
    gfx->setFont(&FreeMonoBold8pt8b);
    gfx->setTextColor(WHITE);
    for(int i=0;i<N_BANDS;i++){
      if(i==numband)c=0b0110000000001100;else c = 0b0000010000000000;
      gfx->fillRoundRect(x,y,50,20,3,c);
      gfx->setCursor(x+5,y+13);gfx->print(bands[i].name);
      x+=60;if(x>400){x=20;y+=40;}
    }
  }
}

void x_display(){
   draw_smeter();    //отобразить s-метр
   draw_spectr();    //отобразить спектр
   draw_service();   //отобразить остальное
   draw_Grid(0+xwin,72+ywin,480,103);//нарисовать сетку на спектре
   draw_temp_value();
   draw_waterfall(); //отобразить панораму
   screen_control();
   drawinfo(); //показать fps
   //if(txrx_mode==RX_MODE)scroll_wp();//сдвинуть панораму на 1 строку вниз
   peak_down();//инкремент показателей спектра и панорамы
}

void screens(uint8_t s){
  switch (s){
    case TX_MODE:x_display();break;
    case RX_MODE:x_display();break;
    case SETUP_MODE:break;
    case 3:break;
 }
}
