
#include "src/dsp_lib/xtensa_const_structs.h"

xtensa_cfft_instance_f32 cfft;
xtensa_cfft_instance_f32 rfft;

float wind[NUM_SAMPLE_BUF];

void window_init_blackman(float *window, int len) 
{
    const float a0 = 0.42;
    const float a1 = 0.5;
    const float a2 = 0.08;

    float len_mult = 1/(float)(len-1);
    for (int i = 0; i < len; i++) {
        window[i] = a0 - a1 * cosf(i * 2 * M_PI * len_mult) + a2 * cosf(i * 4 * M_PI * len_mult);
    }

}

void window_init_blackman_harris(float *window, int len)
{
    const float a0 = 0.35875;
    const float a1 = 0.48829;
    const float a2 = 0.14128;
    const float a3 = 0.01168;

    float len_mult = 1 / (float)(len - 1);
    for (int i = 0; i < len; i++) {
        window[i] = a0
                    - a1 * cosf(i * 2 * M_PI * len_mult)
                    + a2 * cosf(i * 4 * M_PI * len_mult)
                    - a3 * cosf(i * 6 * M_PI * len_mult);
    }
}

void window_init_hann(float *window, int len)
{
    float len_mult = 1 / (float)(len - 1);
    for (int i = 0; i < len; i++) {
        window[i] = 0.5 * (1 - cosf(i * 2 * M_PI * len_mult));
    }
}

void window_init_nuttall(float *window, int len)
{
    const float a0 = 0.355768;
    const float a1 = 0.487396;
    const float a2 = 0.144232;
    const float a3 = 0.012604;

    float len_mult = 1 / (float)(len - 1);
    for (int i = 0; i < len; i++) {
        window[i] = a0
                    - a1 * cosf(i * 2 * M_PI * len_mult)
                    + a2 * cosf(i * 4 * M_PI * len_mult)
                    - a3 * cosf(i * 6 * M_PI * len_mult);
    }
}

void window_init_flat_top(float *window, int len)
{
    const float a0 = 0.21557895;
    const float a1 = 0.41663158;
    const float a2 = 0.277263158;
    const float a3 = 0.083578947;
    const float a4 = 0.006947368;

    float len_mult = 1 / (float)(len - 1);
    for (int i = 0; i < len; i++) {
        window[i] = a0
                    - a1 * cosf(i * 2 * M_PI * len_mult)
                    + a2 * cosf(i * 4 * M_PI * len_mult)
                    - a3 * cosf(i * 6 * M_PI * len_mult)
                    + a4 * cosf(i * 8 * M_PI * len_mult);
    }
}

void fft_init(){
      cfft = xtensa_cfft_sR_f32_len2048;
      rfft = xtensa_cfft_sR_f32_len1024;
      window_init_flat_top(wind,NUM_SAMPLE_BUF);//init window
      for(int i=WP_LINE;i>0;i--){wp_num[i-1]=i-1;} //нумеруем массив номеров строк "водопада"
}

uint16_t hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
    h = (uint32_t)((uint32_t)h * 255) / 360; //color      [0..359]
    s = (uint16_t)((uint16_t)s * 255) / 100; //saturation [0..100]
    v = (uint16_t)((uint16_t)v * 255) / 100; //value      [0..100]
    uint8_t r, g, b;
    uint8_t region, remainder, p, q, t;
    if(s == 0) {
        return (uint16_t)((v >> 3) | ((v >> 2) << 5) | ((v >> 3) << 11));
    }
    region    = h / 43;
    remainder = (h - (region * 43)) * 6;
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch(region) {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }
    uint16_t result = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
    return result;
}

uint16_t select_color(int val,int max){
  
  uint16_t h = map(val,0,max,20,101);
  uint8_t  s = 100;
  uint8_t  v = map(h,0,100,20,100);
  return hsv_to_rgb(h,s,v);  
}

// перенос в fft scroll_wp();
void scroll_wp(){ //сдвиг массива строк "водопада"
      uint8_t tmp = wp_num[WP_LINE-1];
      for (int i=WP_LINE-1;i>0;i--) {wp_num[i] = wp_num[i-1];}
      wp_num[0]=tmp;
}

void fft_for_display(float* input){
    // перенос в fft scroll_wp();
    scroll_wp();
    //static float input_tmp[NUM_SAMPLE_BUF];
    for (int i = 0 ; i < NUM_SAMPLE_BUF/2; i++) {
      //input[i*2] = input[i*2] * wind[i];
      //input[i*2+1] = input[i*2+1] * wind[i];
      }
    xtensa_cfft_f32(&rfft,input,0,1);
    xtensa_cmplx_mag_f32(input,fft_r,NUM_SAMPLE_BUF/2);// получение реальных значений спектральных составляющих
    sum_fft = 0.0f;
    max_fft = 0.0f;
    min_fft = 1000000000.0f;
    for (int i = 0 ; i < NUM_SAMPLE_BUF/2; i++) {
      if(fft_r[i]>=limited_fft)fft_r[i]=limited_fft;
      if (max_fft < fft_r[i])max_fft=fft_r[i];
      if (min_fft > fft_r[i])min_fft=fft_r[i];
      //копирование магнитуд в отображаемый буфер,элементы которого постоянно уменьшаются
      if(fft_r[i]<=fft_inter[i]) fft_r[i]=fft_inter[i];
      if(fft_r[i]>fft_inter[i])fft_inter[i] = fft_r[i];
      //заполняем верхнюю строку массива для отображения "водопада"
      wp_value[i]=wp[wp_num[0]][i]=select_color((int)fft_r[i],limited_fft);
      sum_fft = sum_fft+fft_r[i];
    }
    
    avg_fft = (sum_fft-max_fft)/(NUM_SAMPLE_BUF/2);
    if(max_fft>500  && !dec_Ifgain) dec_Ifgain = true;
    if(avg_fft < 15 && !inc_Ifgain) inc_Ifgain = true;
    if(avg_fft > 20 && !dec_Ifgain) dec_Ifgain = true;   
}
