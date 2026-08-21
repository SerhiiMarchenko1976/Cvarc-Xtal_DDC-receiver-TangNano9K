
void action() { //обработка нажатий на кнопки
  if (txrx_mode == RX_MODE) { //на экране приема
    if (lkey == 1) {tp_band=false;if (numband > N_BANDS - 1)numband = 0;show_band = SHOW_VALUE;show_srate = -1;show_conf=-1;}
    if (lkey == 2) {tp_control=false;if (numstep > 4)numstep = 0;}
    if (lkey == 3) {tp_control=false;if (num_filter > 3)num_filter = 0;}
    if (lkey == 4) {freq -= i2s_sample_rate_rx / 4;if (freq < 1000000)freq = 1000000;}
    if (lkey == 8) {freq += i2s_sample_rate_rx / 4;if (freq > 39999999)freq = 39999999;}
    if (lkey == 7) {tun_mode++;show_tun_mode = SHOW_VALUE;if (tun_mode > PAN)tun_mode = TUN;if (tun_mode == TUN)pos_fft = PCH;}
    if (lkey == 6) {tp_control=false;change_rx_rate = true;show_srate = SHOW_VALUE; show_band=-1;show_conf=-1;show_volume=-1;show_tun_mode = -1;}
    if (lkey == 5) {tp_control=false;if (rf_mode > AM)rf_mode = LSB;}
    if (lkey == 9) {agc_coeff+=1;if(agc_coeff>100)agc_coeff=100;show_volume=SHOW_VALUE;show_band=-1;show_srate=-1;show_conf=-1;show_tun_mode = -1;}
    if (lkey ==10) {agc_coeff-=1;if(agc_coeff=0)agc_coeff=0;show_volume=SHOW_VALUE;show_band=-1;show_srate=-1;show_conf=-1;show_tun_mode = -1;}
    if (lkey ==11) {
      flag_write_parameters = true; //кнопка энкодера(сохранение текущих парметров)
      show_conf = SHOW_VALUE;show_band=-1;show_srate=-1; show_volume=-1;show_tun_mode = -1;
    }
    lkey = 0;bkey=0;
  }
  if (txrx_mode == SETUP_MODE) { //на экране настройки кнопок
    if (bkey == 1 && !flag_exit_setup && n_button < 8) {
      flag_write_config = true;
    }
    if ((bkey == 1) && flag_exit_setup) {
      //gfx.fillScreen(BLACK);
      //g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
      //gfx.pushSprite(0,0);
      //gfx.fillScreen(BLACK);
      redraw = true;
      flag_exit_setup = false;
      speak_out = true;
    }
    lkey = 0; bkey = 0;
  }

}

void time1() {
  int static ms1;
  if ((cur_ms < ms1) || ((cur_ms - ms1) > 1000 )) { //1sec
    ms1 = cur_ms;
    if (show_tun_mode == 1)show_tun_mode = 0;show_tun_mode--;
    if (show_tun_mode < 0)show_tun_mode = -1;
    if (show_band == 1)show_band = 0;show_band--;
    if (show_band < 0)show_band = -1;
    if (show_srate == 1)show_srate = 0;show_srate--;
    if (show_srate < 0)show_srate = -1;
    if (show_conf == 1)show_conf = 0;show_conf--;
    if (show_conf < 0)show_conf = -1;
    if (show_volume == 1)show_volume = 0;show_volume--;
    if (show_volume < 0)show_volume = -1;
    t_fps = fps;
    b_heap=ESP.getFreeHeap();
    getLocalTime(&timeinfo,0);
    ////Serial.println(NTAPS_RX);
    //for (int i=0;i<NTAPS_RX;i++){
    ////Serial.print(i);//Serial.print(":");//Serial.println(fir_coeffs[i]);
    ////Serial.print(i);//Serial.print(":");//Serial.println(audio_buffer[i].re);
    //}
    ////Serial.println("-------------");
    ////Serial.println(CONFIG_DSP_OPTIMIZED);
    ////Serial.println(in_right);
    ////Serial.println(freq/1000);
    ////Serial.println((freq/1000)%5);
    ////Serial.println(gpio_get_level((gpio_num_t)ROTARY_ENCODER_BUTTON_PIN));
    ////Serial.println();

  }
}

void time_001() {
  int static ms001 = 0;
  int static mut = 0;
  if ((cur_ms < ms001) || ((cur_ms - ms001) > 50 )) {
    ms001 = cur_ms;
    int s_peak = 0;
    int peak_count = 0;
    int sum_peak = 0;
    switch (rf_mode) {
      case USB:
        {
          for (int i = pos_fft + bin_start; i < pos_fft + bin_start + bins; i++) {
            if (s_peak < wp_value[i] / 20 - 10) {
              s_peak = wp_value[i] / 20 - 10;
            }
          }
        } break;
      case LSB:
        {
          for (int i = pos_fft - bins; i < pos_fft - bin_start; i++) {
            if (s_peak < wp_value[i] / 20 - 10) {
              s_peak = wp_value[i] / 20 - 10;
            }
          }
        } break;
    }
    smeter = s_peak;
    if (smeter <= old_smeter && old_smeter - smeter > 2)  {old_smeter -= 2;}
    if (smeter <= old_smeter && old_smeter - smeter >= 1) {old_smeter--;}
    if (smeter > old_smeter && smeter - old_smeter > 10)  {old_smeter += 10;}
    if (smeter > old_smeter && smeter - old_smeter > 5)   {old_smeter += 5;}
    if (smeter > old_smeter && smeter - old_smeter > 2)   {old_smeter += 2;}
    if (smeter > old_smeter && smeter - old_smeter >= 1)  {old_smeter++;}
    if (old_smeter < 0)old_smeter = 0;
    if (old_smeter > 150)old_smeter = 150;
    
    if(!speak_out)mut++;
    if(mut>4){speak_out=true;mut=0;}
    
  }
}

void change_band() {
  static int old_band = 100;
  static bool first = true;
  if (old_band == numband)return;
  if (!first)freq = bands[numband].freq;
  rf_mode  = bands[numband].mode;
  if (!first)num_filter = bands[numband].filter;
  old_band = numband; first = false;
}

void x_encoder(void *args)
{
  while(1){
    int encoderDelta;
    pcnt_unit_get_count(pcnt_unit, &encoderDelta);
    pcnt_unit_clear_count(pcnt_unit);
    if (txrx_mode == RX_MODE) { //реакция на поворот энкодера на 0 экране
     if(show_volume > 1){
      if (encoderDelta > 0){agc_coeff+=1;if(agc_coeff>100)agc_coeff=100;show_volume=SHOW_VALUE;show_band=-1;show_srate=-1;show_conf=-1;}
      if (encoderDelta < 0){agc_coeff-=1;if(agc_coeff >101 )agc_coeff=0;show_volume=SHOW_VALUE;show_band=-1;show_srate=-1;show_conf=-1;}
      }
     else{
      if ((encoderDelta > 0) && (tun_mode == TUN)){freq = freq + step_freq[numstep];if (freq > 29999999)freq = 29999999;bands[numband].freq=freq;}
      if ((encoderDelta < 0) && (tun_mode == TUN)){freq = freq - step_freq[numstep];if (freq < 100000)freq = 100000;bands[numband].freq=freq;}
      if ((encoderDelta > 0) && (tun_mode == RIT || tun_mode == PAN)){pos_fft += 2;if (pos_fft > 479)pos_fft = 479;}
      if ((encoderDelta < 0) && (tun_mode == RIT || tun_mode == PAN)){pos_fft -= 2;if (pos_fft < 1)pos_fft = 1;}
      }
    }
    if (txrx_mode == SETUP_MODE) { //реакция на поворот энкодера на 2 экране
        if (encoderDelta > 0) {n_button++;if (n_button > 8)n_button = 8;redraw = true;}
        if (encoderDelta < 0) {n_button--;if (n_button < 0)n_button = 0;redraw = true;}
     }
     vTaskDelay(1);
    }
}

#define BTN_CONTROL (tp_x>400&&tp_x<480&&tp_y>0&&tp_y<70)
#define BTN_FREQ_PLUS  (tp_x>250&&tp_x<470&&tp_y>70&&tp_y<150&&!tp_control&&!tp_band)
#define BTN_FREQ_MINUS (tp_x>5&&tp_x<230&&tp_y>70&&tp_y<150&&!tp_control&&!tp_band)
#define BTN_SAVE_CONF  (tp_x>0&&tp_x<220&&tp_y>0&&tp_y<70)
#define BTN_VOL_PLUS   (tp_x>250&&tp_x<480&&tp_y>200&&tp_y<320&&!tp_control&&!tp_band)
#define BTN_VOL_MINUS  (tp_x>0&&tp_x<230&&tp_y>200&&tp_y<320&&!tp_control&&!tp_band)
#define BTN_BW6000 (tp_x>tp_bw6000.x_min&&tp_x<tp_bw6000.x_max&&tp_y>tp_bw6000.y_min&&tp_y<tp_bw6000.y_max&&tp_control)
#define BTN_BW3000 (tp_x>tp_bw3000.x_min&&tp_x<tp_bw3000.x_max&&tp_y>tp_bw3000.y_min&&tp_y<tp_bw3000.y_max&&tp_control)
#define BTN_BW2400 (tp_x>tp_bw2400.x_min&&tp_x<tp_bw2400.x_max&&tp_y>tp_bw2400.y_min&&tp_y<tp_bw2400.y_max&&tp_control)
#define BTN_BW500  (tp_x>tp_bw500.x_min&&tp_x<tp_bw500.x_max&&tp_y>tp_bw500.y_min&&tp_y<tp_bw500.y_max&&tp_control)
#define BTN_ST1000 (tp_x>tp_st1000.x_min&&tp_x<tp_st1000.x_max&&tp_y>tp_st1000.y_min&&tp_y<tp_st1000.y_max&&tp_control)
#define BTN_ST500 (tp_x>tp_st500.x_min&&tp_x<tp_st500.x_max&&tp_y>tp_st500.y_min&&tp_y<tp_st500.y_max&&tp_control)
#define BTN_ST100 (tp_x>tp_st100.x_min&&tp_x<tp_st100.x_max&&tp_y>tp_st100.y_min&&tp_y<tp_st100.y_max&&tp_control)
#define BTN_ST50 (tp_x>tp_st50.x_min&&tp_x<tp_st50.x_max&&tp_y>tp_st50.y_min&&tp_y<tp_st50.y_max&&tp_control)
#define BTN_ST10 (tp_x>tp_st10.x_min&&tp_x<tp_st10.x_max&&tp_y>tp_st10.y_min&&tp_y<tp_st10.y_max&&tp_control)
#define BTN_MODE_LSB (tp_x>tp_mode_lsb.x_min&&tp_x<tp_mode_lsb.x_max&&tp_y>tp_mode_lsb.y_min&&tp_y<tp_mode_lsb.y_max&&tp_control)
#define BTN_MODE_USB (tp_x>tp_mode_usb.x_min&&tp_x<tp_mode_usb.x_max&&tp_y>tp_mode_usb.y_min&&tp_y<tp_mode_usb.y_max&&tp_control)
#define BTN_MODE_AM (tp_x>tp_mode_am.x_min&&tp_x<tp_mode_am.x_max&&tp_y>tp_mode_am.y_min&&tp_y<tp_mode_am.y_max&&tp_control)
#define BTN_SR_24   (tp_x>tp_sr24.x_min&&tp_x<tp_sr24.x_max&&tp_y>tp_sr24.y_min&&tp_y<tp_sr24.y_max&&tp_control)
#define BTN_SR_48   (tp_x>tp_sr48.x_min&&tp_x<tp_sr48.x_max&&tp_y>tp_sr48.y_min&&tp_y<tp_sr48.y_max&&tp_control)
#define BTN_SR_96   (tp_x>tp_sr96.x_min&&tp_x<tp_sr96.x_max&&tp_y>tp_sr96.y_min&&tp_y<tp_sr96.y_max&&tp_control)
#define BTN_RIT   (tp_x>tp_rit.x_min&&tp_x<tp_rit.x_max&&tp_y>tp_rit.y_min&&tp_y<tp_rit.y_max)
#define BTN_BAND   (tp_x>tp_nband.x_min&&tp_x<tp_nband.x_max&&tp_y>tp_nband.y_min&&tp_y<tp_nband.y_max)
#define BTN_SBAND  (tp_x>10&&tp_x<470&&tp_y>70&&tp_y<150&&tp_band)


void t_touched(){ //опрос тачскрина
    static int l_key=0;
    static int b_key=0;
    
    //tp.read();
    if(tp.Get_Touch()){
      tp_x=tp.touch.x[0];tp_y=tp.touch.y[0];tp_size=1;
      if(BTN_FREQ_PLUS)   {l_key=8;t_press=true;}//+ SR/4 kHz
      if(BTN_FREQ_MINUS)  {l_key=4;t_press=true;}//- SR/4 kHz
      if(BTN_SAVE_CONF)   {l_key=11;t_press=true;}//сохранение текущих параметров
      if(BTN_VOL_PLUS)    {l_key=9;t_press=true; }//+volume
      if(BTN_VOL_MINUS)   {l_key=10;t_press=true;}//-volume
      if(BTN_BW6000)      {num_filter=3;l_key=3;t_press=true;}//выбор полосы осн.фильтра
      if(BTN_BW3000)      {num_filter=0;l_key=3;t_press=true;}//
      if(BTN_BW2400)      {num_filter=1;l_key=3;t_press=true;}//
      if(BTN_BW500)       {num_filter=2;l_key=3;t_press=true;}//
      if(BTN_ST1000)      {numstep=4;l_key=2;t_press=true;}//выбор шага перестройки
      if(BTN_ST500)       {numstep=3;l_key=2;t_press=true;}//
      if(BTN_ST100)       {numstep=2;l_key=2;t_press=true;}//
      if(BTN_ST50)        {numstep=1;l_key=2;t_press=true;}//
      if(BTN_ST10)        {numstep=0;l_key=2;t_press=true;}//
      if(BTN_MODE_LSB)    {rf_mode=0;l_key=5;t_press=true;}//режим модуляции
      if(BTN_MODE_USB)    {rf_mode=1;l_key=5;t_press=true;}//
      if(BTN_MODE_AM)     {rf_mode=2;l_key=5;t_press=true;}//
      if(BTN_SR_24)       {i2s_sample_rate_rx=24000;l_key=6;t_press=true;}//частота дискретизации
      if(BTN_SR_48)       {i2s_sample_rate_rx=48000;l_key=6;t_press=true;}//частота дискретизации
      if(BTN_SR_96)       {i2s_sample_rate_rx=96000;l_key=6;t_press=true;}//частота дискретизации
      if(BTN_RIT)         {l_key=7;t_press=true;}//режим отстройки
      if(BTN_SBAND){ //выбор диапазона
        int x=20;int y=80;
        for(int i=0;i<N_BANDS;i++){
          if(tp_x>x&&tp_x<x+50&&tp_y>y&&tp_y<y+20){numband=i; l_key=1;t_press=true; break;}
          x+=60;if(x>400){x=20;y+=40;}
        }
      }
      if(BTN_CONTROL){
        tp_control=true;
        tp_band=false;
        return;
      }
      if(BTN_BAND){
        tp_band=true;
        tp_control=false;
        return;
      }
    }
    else{
      if(t_press&&l_key!=0){lkey=l_key;l_key=0;t_press=false;t_release=true;}
    }
}

void check_ptt(){
    if (ptt && txrx_mode == RX_MODE){
      txrx_mode = TX_MODE;
      speak_out = false;
    }
    if (!ptt && txrx_mode == TX_MODE){
      txrx_mode = RX_MODE;
      speak_out = true;
    }
}

//вычисляем фазовый сдвиг для кордика в ПЛИС
uint32_t calc_tune_phase(uint32_t frequency, uint32_t xtal){

  // M2 = 2^57 / discretization frequency
  // M3 = 2^24, used to round the result
  uint64_t M2=((uint64_t)1<<57)/xtal;
  uint64_t M3=(uint64_t)1<<24;
  uint64_t ratio = frequency * M2 + M3;
  uint32_t tune_phase = (ratio>>25)&0xffffffff;
  return tune_phase;
}

void control() {
  rx_tune_phase=calc_tune_phase(freq,RX_DIS_FREQ);
  //tx_tune_phase=calc_tune_phase(tx_freq,TX_DIS_FREQ);
  tx_tune_phase=calc_tune_phase(tx_freq,96000000);
  check_ptt();
  if (txrx_mode == RX_MODE) {
    if (tun_mode == RIT)tx_freq = freq + PCH*2*hz; else tx_freq = freq + pos_fft *2* hz;
    rit_freq = freq + pos_fft * 2*hz;
    rx_freq = rit_freq;
    change_band();
    t_touched();//опрос тачскина
    if (t_release) {t_release= false; action();}//если отпущена кнопка на экране вызов обработчика
    write_parameters();
    //if(rf_mode == AM){tun_mode = TUN;pos_fft = PCH;}
  }
}

void readConfig() {
  /*номер     параметр
     0        значение ацп для кнопки №1
     1        значение ацп для кнопки №2
     2        значение ацп для кнопки №3
     3        значение ацп для кнопки №4
     4        значение ацп для кнопки №5
     5        значение ацп для кнопки №6
     6        значение ацп для кнопки №7
     7        значение ацп для кнопки №8
     8        номер диапазона
     9        частота настройки
     10       шаг перестройки
     11       номер основного фильтра
     12       sample rate
     13       громкость
  */
  uint32_t i = 0;
  for (i = 0; i < 8; i++) {
    value_button[i] = EEPROM.readUInt(i * sizeof(uint32_t));
    if (value_button[i] > 4095)value_button[i] = 9999;
  }
  numband = EEPROM.readUInt(i * sizeof(uint32_t));    if (numband > N_BANDS - 1 || numband < 0)numband = 0; i++;
  freq = EEPROM.readUInt(i * sizeof(uint32_t));       if (freq > 30000000 || freq < 100000)freq = bands[numband].freq; i++;
  numstep = EEPROM.readUInt(i * sizeof(uint32_t));    if (numstep > 5 || numstep < 0)numstep = 3; i++;
  num_filter = EEPROM.readUInt(i * sizeof(uint32_t)); if (num_filter > 3 || num_filter < 0)num_filter = 0; i++;
  i2s_sample_rate_rx = EEPROM.readUInt(i * sizeof(uint32_t)); if (i2s_sample_rate_rx < 24000 || i2s_sample_rate_rx > 96000) i2s_sample_rate_rx = 96000;i++;
  agc_coeff = EEPROM.readUInt(i * sizeof(uint32_t));  if (agc_coeff==0||agc_coeff>100)agc_coeff=1;i++;
}

void writeConfig(uint32_t numpar, uint32_t value) {
  value_button[numpar] = value;
  EEPROM.writeUInt(numpar * sizeof(uint32_t), value);
  EEPROM.commit();
}

void write_parameters() {
  /*номер     параметр
     0..7     значения АЦП для кнопок
     8        номер диапазона
     9        частота настройки
     10       шаг перестройки
     11       номер основного фильтра
     12       sample rate (24000/48000/96000)
     13       громкость
  */
  if (flag_write_parameters) {
    EEPROM.writeUInt(8 * sizeof(uint32_t), (uint32_t)numband);
    EEPROM.writeUInt(9 * sizeof(uint32_t), (uint32_t)freq);
    EEPROM.writeUInt(10 * sizeof(uint32_t), (uint32_t)numstep);
    EEPROM.writeUInt(11 * sizeof(uint32_t), (uint32_t)num_filter);
    EEPROM.writeUInt(12 * sizeof(uint32_t), (uint32_t)i2s_sample_rate_rx);
    EEPROM.writeUInt(13 * sizeof(uint32_t), (uint32_t)agc_coeff);
    EEPROM.commit();
    flag_write_parameters = false;
  }
}
