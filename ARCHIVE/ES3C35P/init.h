#include "hal/i2s_hal.h"

void spi_init(){

  master.setDataMode(SPI_MODE0);
  master.setFrequency(20000000);
  master.setMaxTransferSize(SPI_BUFFER_SIZE);
  master.setDutyCyclePos(96);              
  master.begin(1, SPI_SCLK, SPI_MISO, SPI_MOSI, SPI_SS);
}

void buf_init(){
  for(int i=0;i<NTAPS_RX+1;i++){
    fir_coeffs[i]=0;
  }
  for(int i=0;i<NTAPS_TX;i++){
    delay_state_tx_90[i]=0;
    delay_state_tx_00[i]=0;
  }
  for(int i=0;i<NTAPS_RX+2;i++){
    delay_state_rx[i]=0;
  }
  for(int i=0;i<NUM_SAMPLE_BUF;i++){
    input_buffer[i].re=input_buffer[i].im=audio_buffer[i].re=audio_buffer[i].im=\
    workbuf_re[i]=workbuf_im[i]=workbuf_out[i].re=workbuf_out[i].im=\
    workbuf_tmp[i].re=workbuf_tmp[i].im=0;
  }
  for(int i=0;i<NUM_FFT_BUF;i++){
    workbuf_in[i].re=workbuf_in[i].im=workbuf_dsp[i].re=workbuf_dsp[i].im=0;
  }
}

void get_conf(){
  EEPROM.begin(sizeof(uint32_t)*32);
  readConfig();
}


pcnt_unit_handle_t pcnt_unit = NULL;
pcnt_unit_config_t unit_config;

void encoder_init(){
    unit_config.high_limit = 2;
    unit_config.low_limit = -2;
    pcnt_new_unit(&unit_config, &pcnt_unit);
    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 1000,};
    pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = ROTARY_ENCODER_B_PIN,
        .level_gpio_num = ROTARY_ENCODER_A_PIN,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a);
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = ROTARY_ENCODER_A_PIN,
        .level_gpio_num = ROTARY_ENCODER_B_PIN,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b);

    pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    int watch_points[] = {-2, -1, 0, 1, 2};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
        pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]);
    }
    pcnt_unit_enable(pcnt_unit);
    pcnt_unit_clear_count(pcnt_unit);
    pcnt_unit_start(pcnt_unit);
    pinMode(ROTARY_ENCODER_BUTTON_PIN,INPUT_PULLUP);
}

static void tp_init(){
    tp.init();
    tp.Set_Rotation(1);
  //tp.begin();
  //tp.setRotation(ROTATION_INVERTED);
}

static void i2s_init()
{
    i2s_chan_config_t RXchan_cfg_tx = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    //i2s_chan_config_t TXchan_cfg_rx = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

   // i2s_new_channel(&TXchan_cfg_rx, &RX_chan_tx, &TX_chan_rx);
    i2s_new_channel(&RXchan_cfg_tx, &RX_chan_tx, &TX_chan_rx);
    
///////////////////////RX////////////////////////////////////////
    //32-Bit, MSB-First, Left-Justified
    //i2s встроенный кодек
    RX_std_cfg_tx.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(i2s_sample_rate_rx);
    RX_std_cfg_tx.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    RX_std_cfg_tx.gpio_cfg.mclk = STD_MCLK_OUT;    // some codecs may require mclk signal, this example does
    RX_std_cfg_tx.gpio_cfg.bclk = STD_BCLK_OUT;
    RX_std_cfg_tx.gpio_cfg.ws   = STD_WS_OUT;
    RX_std_cfg_tx.gpio_cfg.dout = STD_DOUT_OUT;
    RX_std_cfg_tx.gpio_cfg.din  = STD_DIN_OUT;
    RX_std_cfg_tx.gpio_cfg.invert_flags.mclk_inv = false;
    RX_std_cfg_tx.gpio_cfg.invert_flags.bclk_inv = false;
    RX_std_cfg_tx.gpio_cfg.invert_flags.ws_inv   = false;
    RX_std_cfg_tx.slot_cfg.bit_shift=false;
    RX_std_cfg_tx.slot_cfg.left_align=true;
    RX_std_cfg_tx.slot_cfg.big_endian=false;
    RX_std_cfg_tx.slot_cfg.bit_order_lsb=false;
    RX_std_cfg_tx.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
/*
///////////////////////TX////////////////////////////////////////
    //32-Bit, MSB-First, Left-Justified
    //i2s - микрофон 
    TX_std_cfg_rx.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(i2s_sample_rate_tx);
    TX_std_cfg_rx.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    TX_std_cfg_rx.gpio_cfg.mclk = STD_MCLK_IN_TX;    // some codecs may require mclk signal, this example does
    TX_std_cfg_rx.gpio_cfg.bclk = STD_BCLK_IN_TX;
    TX_std_cfg_rx.gpio_cfg.ws   = STD_WS_IN_TX;
    TX_std_cfg_rx.gpio_cfg.dout = STD_DOUT_IN_TX;
    TX_std_cfg_rx.gpio_cfg.din  = STD_DIN_IN_TX;
    TX_std_cfg_rx.gpio_cfg.invert_flags.mclk_inv = false;
    TX_std_cfg_rx.gpio_cfg.invert_flags.bclk_inv = false;
    TX_std_cfg_rx.gpio_cfg.invert_flags.ws_inv   = false;
    TX_std_cfg_rx.slot_cfg.bit_shift=false;
    TX_std_cfg_rx.slot_cfg.left_align=true;
    TX_std_cfg_rx.slot_cfg.big_endian=false;
    TX_std_cfg_rx.slot_cfg.bit_order_lsb=false;
    TX_std_cfg_rx.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

*/
/////////////////////////////////////////////////////////////////

    i2s_channel_init_std_mode(TX_chan_rx, &TX_std_cfg_rx);
    i2s_channel_init_std_mode(RX_chan_tx, &RX_std_cfg_tx);
    i2s_channel_enable(RX_chan_tx);
    i2s_channel_enable(TX_chan_rx);
    change_rx_rate = true;
}
    

void lcd_init(int l){
    g.Init();
    g.Set_Rotation(1);
    gfx.createSprite(g.Get_Width(), g.Get_Height());
    gfx.fillSprite(BLACK);
    g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
    //gfx.pushSprite(0,0);
    gfx.setSwapBytes(1);
  //gfx.begin(60000000);
  gfx.setTextWrap(false,false);
  //gfx.fillRect(0,0,480,320,BLACK);
  g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
  //gfx.pushSprite(0,0);gfx.pushSprite(0,0);
  ledcAttachChannel(LCD_BL, 2000, 8, 0);
  ledcWrite(LCD_BL, 125);
  //pinMode(LCD_BL, OUTPUT);
  //digitalWrite(LCD_BL, l);
}

/*
int prepareLoading()  
{
  gpio_set_level (oNCONFIG, 0) ; 
  gpio_set_level (oDATA, 0) ;  
  gpio_set_level (oDCLK, 0) ;  
  delay(1);
  gpio_set_level (oNCONFIG, 1) ; 

  int count = 0;
  while (gpio_get_level(iNSTATUS) == 0) {
    count++;
    delay(10);
    if (count >= 255) {
      return -1;
    }
  }

  return 0;
}

void programByte( uint8_t one_byte )
{
  uint8_t bit = 0;
  uint8_t i = 0;
  // write from LSb to MSb 
  for ( i = 0; i < 8; i++ )
  {
    bit = one_byte >> i;
    if (bit & 0x1) gpio_set_level(oDATA, 1);else gpio_set_level(oDATA, 0);
    gpio_set_level(oDCLK, 1);
    gpio_set_level(oDCLK, 0);
  }
}

void fw_loader(){
  pinMode(LCD_BL, OUTPUT);
  if (!SD.begin()){mount=0;return;}
  else mount=1;
  File file = SD.open("/radio.rbf");
  if(!file)return;
  pinMode (iNSTATUS, INPUT_PULLUP);
  pinMode (oNCONFIG, OUTPUT);
  pinMode (oDATA, OUTPUT);
  pinMode (oDCLK, OUTPUT);
  if(prepareLoading()==-1){mount=-1;return;}
  uint8_t buf[1];
  int count=0;
  int blink=0;
  if (file) {
    int len = file.size();
    while (len) {
      file.read(buf,1);
      programByte(buf[0]);
      len--;count++;
      if(count>1000){gpio_set_level(LCD_BL,1);}
      if(count>1100){gpio_set_level(LCD_BL,0);count=0;}
    }
  }
  file.close();
  if (gpio_get_level(iNSTATUS) == 0){mount=-2;return;}
  delay(1000);
  gpio_set_level(oDCLK, 1);
  gpio_set_level(oDCLK, 0);
  gpio_set_level(oDCLK, 1);
  gpio_set_level(oDCLK, 0);
  SD.end();
  pinMode (oNCONFIG, INPUT);
  pinMode (oDATA, INPUT);
  pinMode (oDCLK, INPUT);
}
*/

void start_ok(){

 //WiFi.begin(ssid, password);
 int i_time=0;
 pinMode(ROTARY_ENCODER_BUTTON_PIN,INPUT_PULLUP);
 gfx.fillScreen(BLACK);
 //gfx.setFreeFont(&FreeMonoBold14pt8b);
 gfx.setTextColor(0b1100011000011000);
    gfx.setFreeFont(&FreeMonoBold8pt8b);
    gfx.setTextSize(6);
    gfx.setCursor(150+xwin,170+ywin);
    gfx.print("SDR");
    gfx.setTextSize(1);
  
 /*
 for(int x = 70;x<300;x+=3){ //заставка
   gfx.pushImage(70+xwin,100+ywin,344,72,myBitmap);
   gfx.fillRect(80+xwin,170+ywin,x,5,colors[5]);
   gfx.setCursor(70+xwin,230+ywin);gfx.setTextColor(WHITE,BLACK);//gfx.print("Click encoder to SETUP");
   //if(gpio_get_level((gpio_num_t)ROTARY_ENCODER_BUTTON_PIN)==LOW){txrx_mode=SETUP_MODE;readConfig();speak_out = false;break;}
   g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
   //gfx.pushSprite(0,0);
    //if(WiFi.status() == WL_CONNECTED && i_time == 0){
    //    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //    getLocalTime(&timeinfo,5000);
    //    i_time++;
    //    wifi=true;
    //} 
    
 }
 */
 //WiFi.disconnect(true);
 //WiFi.mode(WIFI_OFF);
 speak_out = true;
 //gfx.fillScreen(BLACK);
 g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
 //gfx.pushSprite(0,0);
 gfx.fillScreen(BLACK);
}
