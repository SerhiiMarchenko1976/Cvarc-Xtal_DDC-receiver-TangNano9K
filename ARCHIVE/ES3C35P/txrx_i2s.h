

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


void io_fpga(){
    spi_master_tx_buf = (uint8_t*)&output_buffer[0].re;
    spi_master_rx_buf = (uint8_t*)&input_buffer[0].re;
    master.transfer(spi_master_tx_buf,spi_master_rx_buf, SPI_BUFFER_SIZE);
}

static void txrx_in(void *args)
{
    size_t r_bytes = 0;
    int size_buf = NUM_SAMPLE_BUF*sizeof(COMPLEX_int);
    uint32_t srate = 0b10;
    while (1) {
      //прием из плис след.порции отсчетов в старшую часть рабочего буфера и передача выходного буфера
      io_fpga();
      
        if(change_rx_rate && txrx_mode==RX_MODE){
          i2s_channel_disable(RX_chan_tx);    
          RX_std_cfg_tx.clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(i2s_sample_rate_rx);
          i2s_channel_reconfig_std_clock(RX_chan_tx, &RX_std_cfg_tx.clk_cfg);
          i2s_channel_enable(RX_chan_tx);    
          change_rx_rate=false;
          if(i2s_sample_rate_rx==96000){srate=0b10;}
          if(i2s_sample_rate_rx==48000){srate=0b01;}
          if(i2s_sample_rate_rx==24000){srate=0b00;}
        }
         //перенос ранее принятых отсчетов в младшую часть рабочего буфера с перекрытием 50% (overlap & save)
        for (int i=0;i<NUM_SAMPLE_BUF;i++){
            workbuf_in[i].re = workbuf_tmp[i].re;
            workbuf_in[i].im = workbuf_tmp[i].im;
        }
        //ждем сигнала от dsp-обработчика о готовности dsp-буфера для приема след.партии отсчетов
        
        for (int i=0; i<NUM_SAMPLE_BUF; i++) {
            workbuf_in[i+NUM_SAMPLE_BUF].re = workbuf_tmp[i].re = (float)((input_buffer[i].re&0xffffff)<<2);
            workbuf_in[i+NUM_SAMPLE_BUF].im = workbuf_tmp[i].im = (float)((input_buffer[i].im&0xffffff)<<2);
            fft_in[i].re = (float)((input_buffer[i].re&0xffff));
            fft_in[i].im = (float)((input_buffer[i].im&0xffff));
            //ptt=(input_buffer[i].im&0xf0000000)>>31; //n_ptt из fpga (раскомментировать, если fpga подключен)
            if (txrx_mode==RX_MODE)output_buffer[i].re=rx_tune_phase;//слово частоты приема в fpga;
            if (txrx_mode==RX_MODE)output_buffer[i].im=srate;//слово управления в fpga;
            if (txrx_mode==RX_MODE)output_buffer[i].im &=0x7FFFFFFF;//ptt off

          }
          fft_for_display((float*)&fft_in);//отправляем fft-буфер в обработку для спектра
        xSemaphoreTake(xIN, portMAX_DELAY);
        if(txrx_mode==RX_MODE){//режим приема
          
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
          xSemaphoreGive(xDSP);//разрешаем демодуляцию и фильтрацию рабочего буфера
         }
         if(txrx_mode==TX_MODE){//режим передачи
          //i2s-микрофон
          i2s_channel_read(TX_chan_rx, &input_buffer_m, NUM_SAMPLE_BUF*sizeof(COMPLEX_int), &r_bytes, portMAX_DELAY);
          for (int i=0; i<NUM_SAMPLE_BUF; i++) { //копируем все принятые отсчеты в рабочий буфер
           workbuf_re[i] = workbuf_im[i] = (float)((input_buffer_m[i].re));
           //workbuf_im[i] = (float)(input_buffer_m[i].im);
          }
          xSemaphoreGive(xDSP);//разрешаем dsp-обработку рабочего буфера передачи
       }
      //vTaskDelay(1);
    }
    vTaskDelete(NULL);
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
