//#include <WiFi.h>
//#include "FS.h"
//#include "SD.h"
#include <SPI.h>
#include <ESP32DMASPIMaster.h>  //
#include "time.h"
#include "driver/i2s_std.h"
#include "driver/pulse_cnt.h"
#include <EEPROM.h>
//#include "src/tamc_gt911/TAMC_GT911.h" //https://github.com/TAMCTec/gt911-arduino
//#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX
#include <TFT_eSPI.h>
#include "ST77922.h"
#include "ST77922_Touch.h"
#include "include/FreeMonoBold8.h"
#include "include/Picopixel.h"
#include "include/CourierCyr6.h"
#include "include/FreeMonoBold14.h"
#include "src/dsp_lib/xtensa_math.h"// https://github.com/whyengineer/esp32-lin/tree/master/components/dsp_lib
#include "global.h"
#include "filters.h"
#include "fft.h"
#include "txrx_i2s.h"
#include "include/s7.h"          //7-segments font
#include "include/images.h"
#include "screens.h"
#include "init.h"
#include "tools.h"
#include "es8311.h"


void setup()
{
  pinMode(START_FPGA,OUTPUT);
  gpio_set_level(START_FPGA,0);
  pinMode(AP_ENABLE, OUTPUT);
  digitalWrite(AP_ENABLE, LOW);
   
  //fw_loader(); //загрузка битстрима в плис из sd
  lcd_init(1);
  get_conf();//восстанавливаем параметры
  buf_init();
  encoder_init();
  tp_init();
  start_ok(); //заставка, вкл wifi, получение времени,проверка на переход в настройки,выкл wifi
  spi_init();  
  fft_init();
  i2s_init();
  // Разбудить кодек ES8311 после настройки I2S каналов
  es8311_codec_init();
  init_filters (num_filter);
  vSemaphoreCreateBinary(xIN);
  vSemaphoreCreateBinary(xDSP);
  vSemaphoreCreateBinary(xOUT);
  vSemaphoreCreateBinary(xOUTEND);
  xTaskCreatePinnedToCore(txrx_in,"txrxin",STACK_SIZE,NULL,14,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(txrx_dsp,"txrxdsp",STACK_SIZE,NULL,12,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(rx_audio_out,"rxout",STACK_SIZE,NULL,10,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(x_encoder,"xencoder",STACK_SIZE/2, NULL, 13, NULL, 1);
  a_heap=ESP.getFreeHeap();
  gpio_set_level(START_FPGA,1);
}

void loop(void){
    cur_ms=millis();
    screens(txrx_mode);
    //gfx->flush();
    //gfx.pushSprite(0,0);
    g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(),(uint16_t*)gfx.getPointer());
    time_001();
    time1();
    control();
    fps=millis()-cur_ms;
}
