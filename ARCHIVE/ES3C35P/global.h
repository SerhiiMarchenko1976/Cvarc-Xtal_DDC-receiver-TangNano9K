 #include "tp.h"

int i_tx,i_rx,a_heap,b_heap,av_level,mount;

//SPI
ESP32DMASPI::Master master;

uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;


#define SPI_SCLK    (gpio_num_t)45
#define SPI_MISO    (gpio_num_t)0
#define SPI_MOSI    (gpio_num_t)46
#define SPI_SS      (gpio_num_t)7

//Encoder pins
#define ROTARY_ENCODER_A_PIN      (gpio_num_t)(43) //tx
#define ROTARY_ENCODER_B_PIN      (gpio_num_t)(44) //rx
#define ROTARY_ENCODER_BUTTON_PIN (gpio_num_t)(-1)

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_7
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12

//pins for FPGA bitstream load in passive serial mode
//#define oDATA          (gpio_num_t)(6)
//#define oDCLK          (gpio_num_t)(7)
//#define oNCONFIG       (gpio_num_t)(15)
//#define iNSTATUS       (gpio_num_t)(16)
#define START_FPGA     (gpio_num_t)(42)

//выход i2s аудио(на динамик через встроенный кодек)
#define STD_MCLK_OUT     (gpio_num_t)(17)
#define STD_WS_OUT       (gpio_num_t)(21) 
#define STD_DIN_OUT      (gpio_num_t)(16)   
#define STD_DOUT_OUT     (gpio_num_t)(15)
#define STD_BCLK_OUT     (gpio_num_t)(18)

//I2S IO Pin define
#define I2S_MCK   17
#define I2S_BCK   18
#define I2S_DINT  16
#define I2S_DOUT  15
#define I2S_WS    21
#define I2S_NUM   I2S_NUM_1

#define AP_ENABLE 1


//mems i2s-микрофон
#define STD_MCLK_IN_TX        (gpio_num_t)(17)//
#define STD_WS_IN_TX          (gpio_num_t)(21)//(6)//
#define STD_DIN_IN_TX         (gpio_num_t)(16)//(7)//
#define STD_DOUT_IN_TX        (gpio_num_t)(15)//
#define STD_BCLK_IN_TX        (gpio_num_t)(18)//(15)//


//пины встроенного тачскрина
#define TOUCH_SDA  (gpio_num_t)38
#define TOUCH_SCL  (gpio_num_t)39
#define TOUCH_INT  (gpio_num_t)47
#define TOUCH_RST  (gpio_num_t)48
//#define TOUCH_WIDTH  480
//#define TOUCH_HEIGHT 320

//LCD
#define LCD_BL  (gpio_num_t)41 //пин подсветки дисплея
#define LCD_CS  (gpio_num_t)10
#define LCD_SCK (gpio_num_t)12
#define LCD_D0  (gpio_num_t)11
#define LCD_D1  (gpio_num_t)13
#define LCD_D2  (gpio_num_t)14
#define LCD_D3  (gpio_num_t)9
#define LCD_RST (gpio_num_t)-1 //GFX_NOT_DEFINED

/*
#define QSPI_PORT SPI2_HOST
#define QSPI_FREQUENCY 80000000
#define QSPI_MODE SPI_MODE0


//pin define for ESP32-S3, include lcd pin
#define LCD_CS  10
#define LCD_BL  41
//below is QSPI pin
#define QSPI_SCLK 12
#define QSPI_D0   11
#define QSPI_D1   13
#define QSPI_D2   14
#define QSPI_D3   9
*/

#define STACK_SIZE 2048

#define RX_DIS_FREQ 63000000 //частота дискретизации приемника в плис Hz
#define TX_DIS_FREQ 63000000 //частота дискретизации передатчика в плис Hz

//Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_CS,LCD_SCK,LCD_D0,LCD_D1,LCD_D2,LCD_D3);
//Arduino_GFX *g = new Arduino_NV3041A(bus, LCD_RST, 0 /* rotation */, true /* IPS */);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite gfx = TFT_eSprite(&tft);
ST77922 g = ST77922();
//TFT_eSprite gfx = TFT_eSprite(&g);

/*
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite gfx = TFT_eSprite(&tft);
ST77922 g = ST77922();
ST77922_TOUCH tp = ST77922_TOUCH();
void setup(void) 
{
  Serial.begin(115200); //Set the serial port baud rate 115200
  g.Init();
  g.Set_Rotation(0);
  gfx.createSprite(g.Get_Width(), g.Get_Height());
  gfx.setRotation(0);
  gfx.setSwapBytes(1);
  tp.init();
  tp.Set_Rotation(1);
  gfx.fillSprite(BLACK);
  gfx.setTextColor(BLUE);
  gfx.drawString("RST",gfx.width()-36,0,2);
  g.Fill_Colors(0, 0, g.Get_Width(), g.Get_Height(), (uint16_t *)gfx.getPointer()); 
}
*/


//Arduino_GFX *gfx = new Arduino_Canvas(480 /* width */, 320 /* height */, g);

int fps;
int t_fps=99;
bool mark = false;
bool draw_info=true;
bool test;

float sum_fft = 0.0f;
float max_fft = 0.0f;
float min_fft = 10000000.0f;
float s_level = 0.0f;

struct tm timeinfo;
const char *ssid = "Dlink123";          // Change this to your WiFi SSID
const char *password = "Rt4swe123";  // Change this to your WiFi password
const char* ntpServer = "ntp0.ntp-servers.net";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

struct COMPLEX
{
 float re;
 float im;
};

struct COMPLEX_int
{
  int32_t re;
  int32_t im;
};
struct COMPLEX_int16
{
  int16_t re;
  int16_t im;
};
uint32_t i2s_sample_rate_rx = 96000;    //частота дискретизации при приеме
uint32_t i2s_sample_rate_tx = 48000;    //частота дискретизации при передаче
#define NUM_SAMPLE_BUF  (1024)      //размер входного буфера
#define NUM_FFT_BUF     (NUM_SAMPLE_BUF*2)
#define SPI_BUFFER_SIZE NUM_SAMPLE_BUF*8

struct  COMPLEX_int input_buffer[NUM_SAMPLE_BUF]; //входной буфер
struct  COMPLEX_int input_buffer_m[NUM_SAMPLE_BUF]; //входной буфер микрофон
struct  COMPLEX_int output_buffer[NUM_SAMPLE_BUF];//выходной буфер
struct  COMPLEX workbuf_in[NUM_FFT_BUF];          //рабочий буфер приема
struct  COMPLEX workbuf_dsp[NUM_FFT_BUF];          //рабочий буфер dsp
struct  COMPLEX workbuf_out[NUM_FFT_BUF/2];       //промежуточный выходной буфер
struct  COMPLEX workbuf_tmp[NUM_FFT_BUF/2];       //временный буфер
struct  COMPLEX_int audio_buffer[NUM_SAMPLE_BUF];//выходной audio-буфер
float workbuf_re[NUM_SAMPLE_BUF];//рабочий буфер tx re
float workbuf_im[NUM_SAMPLE_BUF];//рабочий буфер tx im

void readConfig();
void writeConfig(uint32_t numpar, uint32_t value);
void write_parameters();

// for touch
bool tp_control = false;
bool tp_band = false;
ST77922_TOUCH tp = ST77922_TOUCH();
//TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);
int tp_x,tp_y,tp_size;

#define WP_LINE 85  //кол-во строк "водопада"
uint16_t wp[WP_LINE][NUM_SAMPLE_BUF/2];//массив строк для водопада[кол-во строк][кол-во точек в строке]
uint16_t wp_value[NUM_SAMPLE_BUF/2];
uint8_t wp_num[WP_LINE];          //массив для хранения порядка номеров строк водопада для вывода на экран
float  fft_r[NUM_SAMPLE_BUF];       //рабочий fft-буфер
COMPLEX fft_in[NUM_SAMPLE_BUF];   //приемный fft-буфер
float  fft_inter[NUM_SAMPLE_BUF]; //отображаемый fft буфер (все элементы постоянно уменьшаются)

#define PCH (NUM_SAMPLE_BUF/4) //середина отображаемомго спектра
int pos_fft = PCH; //позиция указателя на панораме/спектре
float hz=(float)i2s_sample_rate_rx/(float)NUM_SAMPLE_BUF;
int limited_fft = 10000000;
float avg_fft = 0.0f;
bool dec_Ifgain = false;
bool inc_Ifgain = false;
uint32_t agc_coeff = 1;
static float coef_fft = 0.000000001f;

int bandwidth=3000; //текущая полоса пропускания, отображаемая на спектре
int indent = 0;     
int bins = bandwidth/(i2s_sample_rate_rx/NUM_SAMPLE_BUF)/2;
int bin_start = indent/(i2s_sample_rate_rx/NUM_SAMPLE_BUF)/2;
int ptt;

uint16_t colors[] = { //
  0b1000000000000000, //0 
  0b1100000000000000, //1
  0b1000010000000000, //2
  0b1100010000000000, //3
  0b1000011000000000, //4
  0b1100011000000000, //5
  0b0000010000000000, //6
  0b0000011000000000, //7
  0b0000011000010000, //8
  0b0000010000010000, //9
  0b0000000000010000, //10
  0b0000000000011000, //11
  0b1000000000010000, //12
  0b1000000000011000, //13
  0b1100000000011000, //14
  0b1000010000011000, //15
  0b1000011000010000, //16
  0b1100010000010000, //17
  0b1000010000010000, //18
  0b1100011000011000  //19
  };

#define GREEN   0b0000011111100000
#define RED     0b1111100000000000
#define BLUE    0b0000000000011111
#define MAGENTA 0b1111100000011111
#define YELLOW  0b1111111111100000
#define CYAN    0b0000011111111111
#define WHITE   0b1111111111111111
#define BLACK   0b0000000000000000

#define GRAY    0b0011100111100111


unsigned long cur_ms   = 0;

uint16_t in_left = 999; //знач.ацп лев.кнопок
bool l_release = false; //признак отпущенной лев.кнопки
bool b_release = false; //признак отпущенной кнопки енкодера
bool t_release = false; //признак отпущенной кнопки на экране
bool l_press = false;   //признак нажатой лев.кнопки
bool b_press = false;   //признак нажатой кнопки енкодера
bool t_press = false;   //признак нажатой кнопки на экране

uint8_t lkey = 0;       //код лев.кнопки для дальн.действия 1..5 (0-нет действия)
uint8_t bkey = 0;       //кнопка енкодера (0-нет действия)

bool redraw = false;
bool redraw_smeter = true;
bool redraw_freq = true;
bool redraw_service = true;

bool fill_fft = true;

bool tuning = true; //
uint8_t rf_mode = 0;//0-LSB, 1-USB, 2-AM
uint8_t tun_mode =2;//0-tuning, 1-rit,2-PAN
uint8_t num_filter = 0; //номер текущего фильтра основной селекции
int smeter = 0;
int old_smeter = 0;
bool change_rx_rate = true;
uint32_t freq = 14200000; //частота настройки (частота левого  края панорамы)
uint32_t rx_tune_phase;
uint32_t tx_tune_phase;
uint32_t rx_freq = freq+hz*pos_fft*2; //частота приема
uint32_t rit_freq = freq+hz*pos_fft*2;//частота расстройки приема
uint32_t tx_freq = freq+hz*pos_fft*2; //частота передачи
uint32_t step_freq[5] = {1,50,100,500,1000}; //шаг перестройки гц
uint32_t numband = 8;//номер текущего диапазона
uint32_t numstep = 2;
bool speak_out = true;
bool mute = false;
bool si = false;
bool wifi = false;

int n_button = 0;
uint16_t value_button[8]={0,1985,2672,3028,3266,3438,3571,3677};
bool flag_exit_setup=false;
uint16_t value_adc = 0;
bool flag_write_config = false;
bool flag_write_parameters = false;
bool flag_start_ok = false;

#define SHOW_VALUE 3  //время показа параметров сек
int show_band = SHOW_VALUE;
int show_srate     = -1;
int show_conf      = -1;
int show_volume    = -1;
int show_tun_mode     = -1;

const int LSB = 0;
const int USB = 1;
const int AM  = 2;
const int TX_MODE = 0;
const int RX_MODE = 1;
const int SETUP_MODE = 2;
const int TUN = 0;
const int RIT = 1;
const int PAN = 2;

uint8_t txrx_mode = RX_MODE;      //1-прием, 0-передача, 2-настройка

struct band {
  uint32_t freq; // frequency in Hz
  char* name;        // name of band
  uint32_t mode;
  uint32_t filter;
};

#define N_BANDS 13
struct band bands[N_BANDS] = {
  1850000,(char *)"160M" ,LSB,0,
  3700000,(char *)"80M " ,LSB,0,
  3995000,(char *)"75M " ,AM, 3,
  4850000,(char *)"60M " ,LSB,0,
  5840000,(char *)"49M " ,AM, 3,
  7100000,(char *)"40M " ,LSB,0,
  9520000,(char *)"31M " ,AM, 3,
  11670000,(char *)"25M ",AM, 3,
  14200000,(char *)"20M ",USB,0,
  17780000,(char *)"16M ",AM, 3,
  21200000,(char *)"15M ",USB,0,
  24920000,(char *)"12M ",AM, 3,
  28350000,(char *)"10M ",USB,0
};
