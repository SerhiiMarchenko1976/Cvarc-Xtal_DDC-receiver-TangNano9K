
#include "include/dspl.h" // www.dsplib.org
#include "dsps_fir.h"     // esp-dsp fir-filter (integrated in arduino-esp32)
#include "coeffs48.h"

#define NTAPS_RX (222)
  
fir_f32_t fir_rx;
fir_f32_t fir_90;
fir_f32_t fir_00;

float fir_coeffs[NTAPS_RX+2];
float delay_state_rx[NTAPS_RX+2];
float delay_state_tx_90[NTAPS_TX+1];
float delay_state_tx_00[NTAPS_TX+1];



static void init_filters (uint8_t num_filter){
  static uint8_t old_filter = 100;
  static int old_s_rate = 0;
  if ((num_filter == old_filter) && (old_s_rate == i2s_sample_rate_rx)) return;
  speak_out=false;
  switch (num_filter){
    case 0:indent=100;bandwidth=3000;  break;//порядок не менять!
    case 1:indent=100;bandwidth=2400;  break;
    case 2:indent=300;bandwidth=700;   break;
    case 3:indent=100;bandwidth=6000;  break;
  }
  hz = (float)i2s_sample_rate_rx / (float)NUM_SAMPLE_BUF;
  bins = bandwidth/(i2s_sample_rate_rx/NUM_SAMPLE_BUF);
  bin_start = indent/(i2s_sample_rate_rx/NUM_SAMPLE_BUF);
  /* Генерация коэффициентов фильтра
   * Параметры:
   * 1.NTAPS_RX порядок фильтра (кол-во коэффициентов). Для данной реализации обязательно четное.
   * 2.indent нижняя граница полосы пропускания гц
   * 3.bandwidth верхняя граница полосы пропускания гц
   * 4.DSPL_FILTER_BPASS тип фильтра (полосовой)
   * 5.DSPL_WIN_* тип оконной функции
   * 6.0 параметр оконной функции(если требуется)
   * 7.fir_coeffs имя массива для коэффициентов (тип float, размер = порядок фильтра+1)
   */
  int ret = fir_linphase(NTAPS_RX,(float)indent/(float)(i2s_sample_rate_rx),\
                                  (float)(indent+bandwidth)/(float)(i2s_sample_rate_rx),\
                                  DSPL_FILTER_BPASS, DSPL_WIN_HANN,0,fir_coeffs);
  dsps_fir_init_f32(&fir_rx,fir_coeffs, delay_state_rx, NTAPS_RX);
  dsps_fir_init_f32(&fir_90, h90, delay_state_tx_90, NTAPS_TX);
  dsps_fir_init_f32(&fir_00, h00, delay_state_tx_00, NTAPS_TX);
  old_filter = num_filter;
  old_s_rate = i2s_sample_rate_rx;
}
