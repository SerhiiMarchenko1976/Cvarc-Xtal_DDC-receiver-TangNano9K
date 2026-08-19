

xSemaphoreHandle xDSP;
xSemaphoreHandle xIN;
xSemaphoreHandle xOUT;
xSemaphoreHandle xOUTEND;

static TaskHandle_t x_in;
static TaskHandle_t x_dsp;
static TaskHandle_t x_out;

i2s_chan_handle_t                RX_chan_tx;
i2s_std_config_t                 RX_std_cfg_tx;
i2s_chan_handle_t                TX_chan_rx; 
i2s_std_config_t                 TX_std_cfg_rx;

// Структура 40-битного пакета (5 байт) для ПЛИС
union SdrPacket40 {
    struct {
        uint32_t data;    // 32 бита полезных данных (частота или I/Q)
        uint8_t  command; // 8 бит — Адрес регистра (Команда)
    } __attribute__((packed)) reg;
    uint8_t bytes[5];     // Массив из 5 байт для DMA-передачи
};

// Функция для отправки одиночных команд (Частота, АТТ, УВЧ) в ПЛИС
static void sdr_spi_send_register(uint8_t cmd_addr, uint32_t payload_data) {
    union SdrPacket40 packet;
    packet.reg.command = cmd_addr;
    packet.reg.data = payload_data;
    
    // Разворот байт (Endianness) для Verilog ПЛИС
    spi_master_tx_buf[0] = packet.bytes[4]; // Команда летит первой
    spi_master_tx_buf[1] = packet.bytes[3];
    spi_master_tx_buf[2] = packet.bytes[2];
    spi_master_tx_buf[3] = packet.bytes[1];
    spi_master_tx_buf[4] = packet.bytes[0];
    
    // Передаем строго 5 байт
    master.transfer(spi_master_tx_buf, spi_master_rx_buf, 5);
}

void io_fpga(){
    union SdrPacket40 packet;
    static bool ptt_from_fpga = false;
    
    if (txrx_mode == RX_MODE) {
        packet.reg.command = 0x00; // На приеме шлем пустую команду, просто забираем звук из FIFO
        packet.reg.data = 0;
    } else {
        packet.reg.command = 0x03; // На передаче шлем команду 03 и звук микрофона
        // Пакуем 16 бит IMAG и 16 бит REAL в 32-битное поле
        uint32_t audio_word = 0;
        audio_word |= (uint16_t)output_buffer[0].re; // Ваша текущая точка звука
        audio_word |= ((uint32_t)(uint16_t)output_buffer[0].im << 16);
        packet.reg.data = audio_word;
    }

    // Разворот байт для отправки в ПЛИС
    spi_master_tx_buf[0] = packet.bytes[4];
    spi_master_tx_buf[1] = packet.bytes[3];
    spi_master_tx_buf[2] = packet.bytes[2];
    spi_master_tx_buf[3] = packet.bytes[1];
    spi_master_tx_buf[4] = packet.bytes[0];

    // Передаем и принимаем строго 5 байт (40 бит) через DMA
    master.transfer(spi_master_tx_buf, spi_master_rx_buf, 5);

    // Распаковываем принятые 40 бит ответа (Звук приема из FIFO ПЛИС)
    uint64_t rx_raw = 0;
    rx_raw |= ((uint64_t)spi_master_rx_buf[0] << 32);
    rx_raw |= ((uint64_t)spi_master_rx_buf[1] << 24);
    rx_raw |= ((uint64_t)spi_master_rx_buf[2] << 16);
    rx_raw |= ((uint64_t)spi_master_rx_buf[3] << 8);
    rx_raw |= ((uint64_t)spi_master_rx_buf[4]);

    ptt_from_fpga = (bool)((rx_raw >> 39) & 0x01);
    if (ptt_from_fpga) {
              txrx_mode = TX_MODE;
          } else {
              txrx_mode = RX_MODE;
          }

    // Раскладываем по буферам I/Q сэмплы
    int16_t rx_imag_val = (int16_t)(rx_raw & 0xFFFF);        // Младшие 16 бит
    int16_t rx_real_val = (int16_t)((rx_raw >> 16) & 0xFFFF); // Следующие 16 бит

    // Заполняем ваш массив input_buffer для обработки и вывода в I2S
    for (int i = 0; i < NUM_SAMPLE_BUF; i++) {
        input_buffer[i].re = (int32_t)rx_real_val; 
        input_buffer[i].im = (int32_t)rx_imag_val;
    }
}



static void txrx_in( void * args)
{
    size_t r_bytes = 0;
    int size_buf = NUM_SAMPLE_BUF*sizeof(COMPLEX_int);
    //uint32_t srate = 0b10;
    static uint32_t last_rx_freq = 0;
    static uint32_t last_tx_freq = 0;
    static uint8_t  last_peripheral = 0;
  

  while(1) {
    // 1. СТРОГО СИНХРОНИЗИРУЕМСЯ ПО СЕМАФОРУ КОДЕКА (Как в вашем оригинале)
    xSemaphoreTake(xIN, portMAX_DELAY);

    // 2. АВТОМАТИЧЕСКАЯ УВЯЗКА ЧАСТОТЫ ДИСКРЕТИЗАЦИИ S_RATE
    if(change_rx_rate && txrx_mode==RX_MODE){
      i2s_channel_disable(RX_chan_tx);    
      RX_std_cfg_tx.clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(i2s_sample_rate_rx);
      i2s_channel_reconfig_std_clock(RX_chan_tx, &RX_std_cfg_tx.clk_cfg);
      i2s_channel_enable(RX_chan_tx);    
      change_rx_rate=false;
      if(i2s_sample_rate_rx==96000){s_rate=2;}
      if(i2s_sample_rate_rx==48000){s_rate=1;}
      if(i2s_sample_rate_rx==24000){s_rate=0;}
    }

    // 3. ОТПРАВКА ИЗМЕНИВШИХСЯ РЕГИСТРОВ В ПЛИС (Новый 40-битный стандарт)
    if (rx_tune_phase != last_rx_freq) {
        sdr_spi_send_register(0x01, rx_tune_phase);
        last_rx_freq = rx_tune_phase;
    }

    if (tx_tune_phase != last_tx_freq) {
        sdr_spi_send_register(0x04, tx_tune_phase);
        last_tx_freq = tx_tune_phase;
    }

    // Собираем маску периферии (Команда 0x02)
    uint32_t current_peripheral = 0;
    current_peripheral |= (dac_level & 0xFF); 
    if (att_on)    current_peripheral |= (1 << 8);
    if (preamp_on) current_peripheral |= (1 << 9);
    current_peripheral |= ((s_rate & 0x03) << 10);

    if (current_peripheral != last_peripheral) {
        sdr_spi_send_register(0x02, current_peripheral);
        last_peripheral = current_peripheral;
    }

    // 4. ПЕРЕНОС СТАРЫХ ОТСЧЕТОВ (Ваш оригинальный блок overlap & save)
    for (int i=0; i<NUM_SAMPLE_BUF; i++){
        workbuf_in[i].re = workbuf_tmp[i].re;
        workbuf_in[i].im = workbuf_tmp[i].im;
    }

    // 5. СКОРОСТНОЙ ПОТОКОВЫЙ ОБМЕН ЗВУКОМ НА 40 БИТ
    io_fpga();

    // Разбираем флаг тангенты PTT, если он возвращается из io_fpga
    // (Если ptt_from_fpga_local равен true, вы можете переключать режим txrx_mode)

    // 6. НАПРАВЛЯЕМ ЗВУК В ТРАКТЫ ОБРАБОТКИ И БПФ (Ваш чистый оригинальный цикл)
    for (int i=0; i<NUM_SAMPLE_BUF; i++) {
        int16_t sample_re = (int16_t)input_buffer[i].re;
        int16_t sample_im = (int16_t)input_buffer[i].im;

        workbuf_in[i+NUM_SAMPLE_BUF].re = workbuf_tmp[i].re = (float)sample_re;
        workbuf_in[i+NUM_SAMPLE_BUF].im = workbuf_tmp[i].im = (float)sample_im;
        
        fft_in[i].re = (float)sample_re;
        fft_in[i].im = (float)sample_im;
    }

    // Отправляем спектр на водопад дисплея
    fft_for_display((float*)&fft_in);

    // 7. ПЕРЕДАЧА БУФЕРОВ НА ДЕМОДУЛЯЦИЮ (Строго ваш оригинальный unrolled-цикл с шагом i+=8)
    if(txrx_mode==RX_MODE){
      for(int i=0; i<NUM_FFT_BUF; i+=8) {
        workbuf_dsp[i].re = workbuf_in[i].re;
        workbuf_dsp[i].im = workbuf_in[i].im;
        workbuf_dsp[i+1].re = workbuf_in[i+1].re;
        workbuf_dsp[i+1].im = workbuf_in[i+1].im;
        workbuf_dsp[i+2].re = workbuf_in[i+2].re;
        workbuf_dsp[i+2].im = workbuf_in[i+2].im;
        workbuf_dsp[i+3].re = workbuf_in[i+3].re;
        workbuf_dsp[i+3].im = workbuf_in[i+3].im;
        workbuf_dsp[i+4].re = workbuf_in[i+4].re;
        workbuf_dsp[i+4].im = workbuf_in[i+4].im;
        workbuf_dsp[i+5].re = workbuf_in[i+5].re;
        workbuf_dsp[i+5].im = workbuf_in[i+5].im;
        workbuf_dsp[i+6].re = workbuf_in[i+6].re;
        workbuf_dsp[i+6].im = workbuf_in[i+6].im;
        workbuf_dsp[i+7].re = workbuf_in[i+7].re;
        workbuf_dsp[i+7].im = workbuf_in[i+7].im;
      }
      xSemaphoreGive(xDSP);
    }

    if(txrx_mode==TX_MODE){
      i2s_channel_read(TX_chan_rx, &input_buffer_m, NUM_SAMPLE_BUF*sizeof(COMPLEX_int), &r_bytes, portMAX_DELAY);
      for (int i=0; i<NUM_SAMPLE_BUF; i++) { 
       workbuf_re[i] = workbuf_im[i] = (float)((input_buffer_m[i].re));
      }
      xSemaphoreGive(xDSP);
    }
  }
}




static void get_shift(struct COMPLEX* input,int pos){//демодуляция SSB в частотной области

          int shift = 0;
          switch (rf_mode){
          case LSB:{ //
            for (int i=0;i<NUM_FFT_BUF/2;i++) {
                input[i].re = input[pos*2-i].re;
                input[i].im = -input[pos*2-i].im;
            }
            break;}
          case USB:{ //
            for (int i=0;i<NUM_FFT_BUF/2;i++) {
                input[i].re = input[pos*2+i].re;
                input[i].im = input[pos*2+i].im;
            }
            break;}
          }
      for (int i=NUM_FFT_BUF/2;i<NUM_FFT_BUF;i++){
        input[i].re = input[i].im = 0;
      }
}

static void get_am(struct COMPLEX* input,int pos){
     float z1= 0.0;
     for(int i=0;i<NUM_FFT_BUF/2;i++){
        float mag = sqrtf((input[i].re*input[i].re) + (input[i].im*input[i].im));
        float z0 = mag+(z1*0.99);
        input[i].re = input[i].im = z0-z1;
        z1=z0;
     }
}

static void txrx_dsp(void * pvParameters){
  size_t t_bytes = 0;
  int size_buf = NUM_SAMPLE_BUF*sizeof(COMPLEX_int);
  while(1){
    xSemaphoreTake(xDSP, portMAX_DELAY);//ждем сигнала о готовности приемного буфера  
     if(txrx_mode==RX_MODE){
          init_filters (num_filter);
          if(rf_mode != AM){
            xtensa_cfft_f32(&cfft,(float*)&workbuf_dsp,0,1);
            get_shift(workbuf_dsp,pos_fft);//сдвиг демодулируемого участка в начало спектра с позиции pos_fft
            xtensa_cfft_f32(&cfft,(float*)&workbuf_dsp,1,1);
          }
          if(rf_mode == AM)get_am(workbuf_dsp,pos_fft);
          //основной фильтр
          dsps_fir_f32(&fir_rx, (float*)&workbuf_dsp, (float*)&workbuf_out, NUM_FFT_BUF);
          xSemaphoreTake(xOUTEND, portMAX_DELAY); //ждем окончания вывода звука предыдущих отсчетов
          xSemaphoreGive(xOUT);//разрешаем вывод звука
          xSemaphoreGive(xIN); //разрешаем прием в dsp буфер следующей порции отсчетов
     }
     if(txrx_mode==TX_MODE){
              switch(rf_mode){//формируем квадратуры (гилберт+фнч)
              case USB:{
                dsps_fir_f32(&fir_90, (float*)&workbuf_re, (float*)&workbuf_re, NUM_SAMPLE_BUF);//Hilbert90
                dsps_fir_f32(&fir_00, (float*)&workbuf_im, (float*)&workbuf_im, NUM_SAMPLE_BUF);//Hilbert00
                break;}
              case LSB:{
                dsps_fir_f32(&fir_00, (float*)&workbuf_re, (float*)&workbuf_re, NUM_SAMPLE_BUF);//Hilbert00
                dsps_fir_f32(&fir_90, (float*)&workbuf_im, (float*)&workbuf_im, NUM_SAMPLE_BUF);//Hilbert90
                break;}
              case AM:{
                dsps_fir_f32(&fir_00, (float*)&workbuf_re, (float*)&workbuf_re, NUM_SAMPLE_BUF);//Hilbert00
                dsps_fir_f32(&fir_00, (float*)&workbuf_im, (float*)&workbuf_im, NUM_SAMPLE_BUF);//Hilbert00
                break;}
              }
              for (int i=0; i<NUM_SAMPLE_BUF; i++) { //переносим обработанный массив в выходной буфер 
                  int16_t tmp_re = ((int)(workbuf_re[i]))>>16;
                  int16_t tmp_im = ((int)(workbuf_im[i]))>>16;
                  output_buffer[i].re = tmp_re;output_buffer[i].re = output_buffer[i].re <<16;
                  output_buffer[i].re |= tmp_im;//квадратуры i/q
                  output_buffer[i].im=tx_tune_phase;  //слово частоты передачи в FPGA
                  output_buffer[i].im |=0x80000000;//ptt on
              }
              xSemaphoreGive(xIN);  //разрешаем прием следующей партии отсчетов и вывод квадратур во фронтенд
          }
       //vTaskDelay(1);
  }
  vTaskDelete(NULL);
}

static void rx_audio_out(void * pvParameters){
  size_t t_bytes = 0;
  int size_buf = NUM_SAMPLE_BUF*sizeof(COMPLEX_int);
  
  while(1){

    xSemaphoreTake(xOUT, portMAX_DELAY);//ждем окончания dsp-обработки
          float agc=(float)agc_coeff;
          for (int i=0; i<NUM_SAMPLE_BUF; i++) { //переносим обработанный массив в выходной буфер с нормализацией в I2S-формат
           if(speak_out){
              audio_buffer[i].im = ((int)(workbuf_out[i].im*agc ));
              audio_buffer[i].re = ((int)(workbuf_out[i].re*agc ));
           }else{
              audio_buffer[i].im = audio_buffer[i].re = 0;
            }
          }
          
          i2s_channel_write(RX_chan_tx,&audio_buffer,size_buf,&t_bytes,portMAX_DELAY);
          xSemaphoreGive(xOUTEND);
   }
  vTaskDelete(NULL);
}
