/*
* Copyright (c) 2015-2022 Sergey Bakhurin
* Digital Signal Processing Library [http://dsplib.org]
*
* This file is part of libdspl-2.0.
*
* is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser  General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* DSPL is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef DSPL_H
#define DSPL_H

#include <math.h>

/* math const definition */
#ifndef M_PI
    #define M_PI        3.1415926535897932384626433832795
#endif

#ifndef M_2PI
    #define M_2PI       6.283185307179586476925286766559
#endif

/* Window functions */
int window(float* w, int n, int win_type, float param);
int win_bartlett        (float *w, int n, int win_type);
int win_bartlett_hann   (float *w, int n, int win_type);
int win_blackman        (float *w, int n, int win_type);
int win_blackman_harris (float *w, int n, int win_type);
int win_blackman_nuttall(float *w, int n, int win_type);
int win_cheby           (float *w, int n, float param);
int win_cos             (float *w, int n, int win_type);
int win_flat_top        (float *w, int n, int win_type);
int win_gaussian        (float *w, int n, int win_type, float sigma);
int win_hamming         (float *w, int n, int win_type);
int win_hann            (float *w, int n, int win_type);
int win_kaiser          (float* w, int n, int win_type, float param);
int win_lanczos	        (float *w, int n, int win_type);
int win_nuttall	        (float *w, int n, int win_type);
int win_rect            (float *w, int n);
int bessel_i0(float* x, int n, float* y);
int cheby_poly1(float* x, int n, int ord, float* y);
int polyval(float* a, int ord, float* x, int n, float* y);
int fir_linphase(int ord, float w0, float w1, int filter_type,int win_type, float win_param, float* h);
int fir_linphase_lpf(int ord, float wp, int win_type,float win_param, float* h);
int linspace(float x0, float x1, int n, int type, float* x);

#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup TYPES_GROUP
\typedef complex_t
\brief Complex data type.

libdspl-2.0 describes complex numbers data type as an array
of two `float` elements.
First element sets real part, second --- imaginary part.

For example:

\code{.cpp}
    complex_t z;
    z[0] =  1.0;
    z[1] = -2.0;
\endcode

Variable `z = 1-2j`, here `j` - imaginary unit.

For the convenience of working with complex numbers implemented
special macros: \ref RE, \ref IM, \ref ABSSQR
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup TYPES_GROUP
\typedef complex_t
\brief Описание комплексного типа данных.

Комплексный тип данных в библиотеке libdspl-2.0 определен как
массив из двух элементов типа `float`.
При этом первый элемент массива определяет реальную часть
комплексного числа, а второй - мнимую.

Например:

\code{.cpp}
    complex_t z;
    z[0] =  1.0;
    z[1] = -2.0;
\endcode

Переменная `z = 1-2j`, где `j` - мнимая единица.

Для удобства работы с комплексными числами реализованы
специальные макросы: \ref RE, \ref IM, \ref ABSSQR
***************************************************************************** */
#endif
typedef float complex_t[2];



#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup DFT_GROUP
\struct fft_t
\brief Fast Fourier Transform Object Data Structure

The structure stores pointers to twiddle factors and arrays of intermediate
data of the fast Fourier transform algorithm.

The libdspl-2.0 library uses an FFT algorithm for composite size.

\param  n
The size of the FFT vector for which memory is allocated
in the structure arrays.  \n
The parameter `n` must be equal to an integer power of two (radix 2). \n \n

\param  w
Pointer to the vector of twiddle factors. \n
The size of the vector is `[n x 1]`. \n
The memory must be allocated and an array of twiddle factors
must be filled with the \ref fft_create function. \n\n

\param  t0
Pointer to the vector of intermediate results of the FFT algorithm. \n
The size of the vector is `[n x 1]`. \n
Memory must be allocated by \ref fft_create function. \n\n

\param  t1
Pointer to the vector of intermediate results. \n
The size of the vector is `[n x 1]`. \n
The memory must be allocated with the \ref fft_create function. \n\n

\param w32
Static twiddle factors vector for 32-points FFT. \n \n

\param w64
Static twiddle factors vector for 32-points FFT. \n \n

\param w128
Static twiddle factors vector for 32-points FFT. \n \n

\param w256
Static twiddle factors vector for 32-points FFT. \n \n

\param w512
Static twiddle factors vector for 32-points FFT. \n \n

\param w1024
Dynamic twiddle factors vector for 32-points FFT. \n \n

\param w2048
Dynamic twiddle factors vector for 32-points FFT. \n \n

\param w4096
Dynamic twiddle factors vector for 32-points FFT. \n \n

The structure is calculated with the \ref fft_create function once
before using the FFT algorithm. \n
A pointer to an object of this structure may be
reused when calling FFT functions. \n
Before exiting the program, dedicated memory for twiddle factors and arrays of
intermediate data must be cleared by the \ref fft_free function.

For example:

\code
fft_t pfft = {0};     // Structure fft_t and clear all fields
int n = 64;           // FFT size

int err;

// Create and fill FFT structure for 64-points FFT
err = fft_create(&pfft, n);

// FFT calculation here
// FFT calculation here one more
// ...

// Clear fft structure
fft_free(&pfft);
\endcode

\note
It is important to note that if the object `fft_t` was created for the FFT size
equal to` n`, it can only be used for FFT of size `n`. \n \n
It’s also worth noting that the FFT functions independently control the size,
and independently allocate the memory of the FFT object, if necessary.
So if you call any function using the `fft_t` structure with filled
data for the FFT length `k` for calculating the FFT of length`n`,
then the structure arrays will be automatically recreated for the length `n`.

\author  Sergey Bakhurin  www.dsplib.org
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup DFT_GROUP
\struct fft_t
\brief Структура данных объекта быстрого преобразования Фурье

Структура хранит указатели на массивы поворотных коэффициентов
и массивы промежуточных данных алгоритма быстрого преобразования Фурье.

Библиотека libdspl-2.0 использует для БПФ алгоритм для составной длины

\param  n
Размер вектора БПФ, для которого выделена память в массивах структуры.  \n
Парметр `n` должен быть равен целой степени двойки. \n \n

\param  w
Указатель на вектор поворотных коэффициентов алгоритма БПФ. \n
Размер вектора `[n x 1]`.  \n
Память должна быть выделена и массив поворотных коэффициентов
должен быть заполнен функцией \ref fft_create.  \n \n

\param  t0
Указатель на вектор промежуточных вычислений алгоритма БПФ. \n
Размер вектора `[n x 1]`. \n
Память должна быть выделена функцией \ref fft_create. \n \n

\param  t1
Указатель на вектор промежуточных вычислений алгоритма БПФ. \n
Размер вектора `[n x 1]`. \n
Память должна быть выделена функцией \ref fft_create. \n \n

\param w32
Статический вектор поворотных коэффициентов 32-точечного БПФ. \n \n

\param w64
Статический вектор поворотных коэффициентов 64-точечного БПФ. \n \n

\param w128
Статический вектор поворотных коэффициентов 128-точечного БПФ. \n \n

\param w256
Статический вектор поворотных коэффициентов 256-точечного БПФ. \n \n

\param w512
Статический вектор поворотных коэффициентов 512-точечного БПФ. \n \n

\param w1024
Статический вектор поворотных коэффициентов 1024-точечного БПФ. \n \n

\param w2048
Статический вектор поворотных коэффициентов 2048-точечного БПФ. \n \n

\param w4096
Статический вектор поворотных коэффициентов 4096-точечного БПФ. \n \n


Структура заполняется функцией \ref fft_create один раз
до использования алгоритма БПФ.  \n
Указатель на объект данной структуры может быть
многократно использован при вызове функций БПФ. \n
Перед выходом из программы выделенную память под поворотные
коэффициенты и массивы промежуточных данных
необходимо очистить функцией \ref fft_free. Например:
\code
fft_t pfft = {0};     // объявляем объект fft_t и обнуляем все поля
int n = 64;           // Размер БПФ
int err;

// создаем объект для 64-точечного БПФ
err = fft_create(&pfft, n);

// Вызов БПФ функции
// Еще раз вызов БПФ функции
// ...

// очистить память объекта БПФ
fft_free(&pfft);
\endcode

\note
Важно отметить, что если объект `fft_t` был создан для размера БПФ равного `n`,
то он может быть использован только для БПФ размера `n`.  \n\n
Также необходимо заметить, что функции БПФ самостоятельно контролируют размер,
и самостоятельно выделяют память объекта БПФ при необходимости.
Так если вызвать любую функцию использующую структуру `fft_t` с заполненными
данными для длины БПФ `k` для расчета БПФ длины `n`,
то массивы структуры будут автоматически пересозданы для длины `n`.

\author
Бахурин Сергей.
www.dsplib.org
***************************************************************************** */
#endif
typedef struct
{
    complex_t*  w;
    complex_t*  t0;
    complex_t*  t1;
   
    /* radix-2 twiddle factors vectors */
    complex_t    w32[ 32];
    complex_t    w64[ 64];
    complex_t   w128[128];
    complex_t   w256[256];
    complex_t   w512[512];
    complex_t*  w1024;
    complex_t*  w2048;
    complex_t*  w4096;
    int         n;
} fft_t;



#define RAND_TYPE_MRG32K3A 0x00000001
#define RAND_TYPE_MT19937  0x00000002
#define RAND_MT19937_NN    312

#ifdef DOXYGEN_ENGLISH

#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup SPEC_MATH_RAND_GEN_GROUP
\struct random_t
\brief Структура параметров датчиков псевдослучайных чисел.

Структура хранит инициализацию и текущие регистры различных датчиков
псевдослучайных чисел. В библиотеке используются следующие датчики:
\li MRG32K3A -- 32 битный датчик разработан Пьером Лекуэром [1].
\li MT19937-64 -- 64-битный датчик
<a href = "https://en.wikipedia.org/wiki/Mersenne_Twister">
Вихрь Мерсенна
</a> [2, 3].

\note
[1] Pierre L'Ecuyer, (1999) Good Parameters and Implementations for Combined
    Multiple Recursive Random Number Generators. Operations Research
    47(1):159-164. https://doi.org/10.1287/opre.47.1.159 \n\n
[2] T. Nishimura, ``Tables of 64-bit Mersenne Twisters // ACM Transactions
    on Modeling and Computer Simulation 10. (2000) 348--357. \n\n
[3] M. Matsumoto and T. Nishimura  Mersenne Twister: a 623-dimensionally
    equidistributed uniform pseudorandom number generator // ACM Transactions
    on Modeling and Computer Simulation 8. (Jan. 1998) 3--30.  \n\n

\param  mrg32k3a_seed
Начальная инициализация датчика MRG32K3A. \n \n

\param  mrg32k3a_x
Первый вектор состояния рекурсивного датчика MRG32K3A. \n \n

\param  mrg32k3a_y
Второй вектор состояния рекурсивного датчика MRG32K3A. \n \n

\param  mt19937_mt
Первый вектор состояния рекурсивного датчика MT19937-64. \n \n

\param  mt19937_mti
Текущий индекс в векторе состояния датчика MT19937-64. \n \n

Параметры данной структуры заполняются автоматически функцией `random_init`
и используются функциями генерации псевдослучайных векторов.

\author Бахурин Сергей. www.dsplib.org
***************************************************************************** */
#endif
typedef struct
{

    float mrg32k3a_seed;
    float mrg32k3a_x[3];
    float mrg32k3a_y[3];

    /* The array for the MT19937 state vector */
    unsigned long long mt19937_mt[RAND_MT19937_NN];
    int                mt19937_mti;

    int type;

}random_t;



#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def RE(x)
\brief Macro sets real part of the complex number.

Example:
\code{.cpp}
    complex_t z;
    RE(z) =  1.0;
    IM(z) = -2.0;
\endcode

Variable `z = 1-2j`, here `j` - imaginary unit.

This macro can be used to return
real part of the complex number:

\code{.cpp}
    complex_t z = {3.0, -4.0};
    float    r;
    r = RE(z);
\endcode
In this example `z = 3-4i`,
but variable `r` will keep 3.
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def RE(x)
\brief Макрос определяющий реальную часть комплексного числа.

Например:
\code{.cpp}
    complex_t z;
    RE(z) =  1.0;
    IM(z) = -2.0;
\endcode

Переменная `z = 1-2j`, где `j` --- мнимая единица.

Аналогично, макрос можно использовать для получения
реальной части комплексного числа:

\code{.cpp}
    complex_t z = {3.0, -4.0};
    float    r;
    r = RE(z);
\endcode
В данном примере переменная `z = 3-4i`, а в переменой `r`
будет храниться число 3.
***************************************************************************** */
#endif
#define RE(x) (x[0])



#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def IM(x)
\brief Macro sets imaginary part of the complex number.

Example:
\code{.cpp}
    complex_t z;
    RE(z) =  1.0;
    IM(z) = -2.0;
\endcode

Variable `z = 1-2j`, here `j` - imaginary unit.

This macro can be used to return
imaginary part of the complex number:
\code{.cpp}
    complex_t z = {3.0, -4.0};
    float    r;
    r = IM(z);
\endcode
In this example `z = 3-4i`,
but variable `r` will keep -4.
***************************************************************************** */


#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def IM(x)
\brief Макрос определяющий мнимую часть комплексного числа.

Например:
\code{.cpp}
    complex_t z;
    RE(z) =  1.0;
    IM(z) = -2.0;
\endcode

Переменная `z = 1-2j`, где `j` - мнимая единица.

Аналогично, макрос можно использовать для получения
мнимой части комплексного числа:
\code{.cpp}
    complex_t z = {3.0, -4.0};
    float r;
    r = IM(z);
\endcode
В данном примере переменная `z = 3-4i`,
а в переменой `r` будет храниться число -4.
***************************************************************************** */
#endif
#define IM(x) (x[1])



#define SQR(x) ((x) * (x))


#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def ABSSQR(x)
\brief
The macro returns the square of the modulus of a complex number `x`.

Square of the modulus of a complex number \f$ x = a + j  b \f$ equals:

\f[
    |x|^2 = x x^* = a^2 + b^2.
\f]

Example:
\code{.cpp}
    complex_t z;
    float y;
    RE(z) =  1.0;
    IM(z) = -2.0;
    y = ABSSQR(z);
\endcode

Variable `z = 1-2j`, here `j` - imaginary unit, but variable `y = 5`.
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup TYPES_GROUP
\def ABSSQR(x)
\brief Макрос возвращает квадрат модуля комплексного числа `x`.

Квадрат модуля комплексного числа \f$ x = a + j  b \f$ равен:

\f[
    |x|^2 = x x^* = a^2 + b^2.
\f]

Например:
\code{.cpp}
    complex_t z;
    float y;
    RE(z) =  1.0;
    IM(z) = -2.0;
    y = ABSSQR(z);
\endcode

Переменная `z = 1-2j`, где `j` - мнимая единица, а переменная `y = 5`.
***************************************************************************** */
#endif
#define ABSSQR(x) ((SQR(RE(x))) + (SQR(IM(x))))




#define ABS(x)          sqrt((ABSSQR(x)))


#define ARG(x)          atan2(IM(x), RE(x))


#define CMRE(a,b)       ((RE(a)) * (RE(b)) - (IM(a)) * (IM(b)))


#define CMIM(a,b)       ((RE(a)) * (IM(b)) + (IM(a)) * (RE(b)))


#define CMCONJRE(a, b)  ((RE(a)) * (RE(b)) + (IM(a)) * (IM(b)))


#define CMCONJIM(a, b)  ((IM(a)) * (RE(b)) - (RE(a)) * (IM(b)))



#define RES_OK                                0

/* Error codes                                          */
/* A                                          0x01xxxxxx*/
#define ERROR_ARG_PARAM                       0x01180716
/* B                                          0x02xxxxxx*/
/* C                                          0x03xxxxxx*/
/* D                                          0x04xxxxxx*/
#define ERROR_DAT_TYPE                        0x04012020
#define ERROR_DIV_ZERO                        0x04102226
/* E                                          0x05xxxxxx*/
#define ERROR_ELLIP_MODULE                    0x05121315
/* F                                          0x06xxxxxx*/
#define ERROR_FFT_SIZE                        0x06062021
#define ERROR_FILTER_A0                       0x06090100
#define ERROR_FILTER_APPROX                   0x06090116
#define ERROR_FILTER_FT                       0x06090620
#define ERROR_FILTER_ORD                      0x06091518
#define ERROR_FILTER_ORD_BP                   0x06091519
#define ERROR_FILTER_RP                       0x06091816
#define ERROR_FILTER_RS                       0x06091819
#define ERROR_FILTER_TYPE                     0x06092025
#define ERROR_FILTER_WP                       0x06092316
#define ERROR_FILTER_WS                       0x06092319
#define ERROR_FNAME                           0x06140113
#define ERROR_FOPEN                           0x06151605
#define ERROR_FREAD_SIZE                      0x06180501
#define ERROR_FS                              0x06190000
#define ERROR_FWRITE_SIZE                     0x06231820
/* G                                          0x07xxxxxx*/
#define ERROR_GNUPLOT_CREATE                  0x07161203
#define ERROR_GNUPLOT_FNPNG                   0x07161206
#define ERROR_GNUPLOT_TERM                    0x07161220
/* H                                          0x08xxxxxx*/
/* I                                          0x09xxxxxx*/
#define ERROR_INF                             0x09140600
/* J                                          0x10xxxxxx*/
/* K                                          0x11xxxxxx*/
/* L                                          0x12xxxxxx*/
#define ERROR_LAPACK                          0x12011601
/* M                                          0x13xxxxxx*/
#define ERROR_MALLOC                          0x13011212
#define ERROR_MATRIX_SIZE                     0x13011926
#define ERROR_MIN_MAX                         0x13091413
/* N                                          0x14xxxxxx*/
#define ERROR_NAN                             0x14011400
#define ERROR_NEGATIVE                        0x14050701
/* O                                          0x15xxxxxx*/
#define ERROR_OVERLAP                         0x15220412
/* P                                          0x16xxxxxx*/
#define ERROR_POLY_AN                         0x16150114
#define ERROR_POLY_ORD                        0x16151518
#define ERROR_PTR                             0x16201800
/* Q                                          0x17xxxxxx*/
/* R                                          0x18xxxxxx*/
#define ERROR_RAND_SIGMA                      0x18011909
#define ERROR_RAND_TYPE                       0x18012009
#define ERROR_RESAMPLE_RATIO                  0x18051801
#define ERROR_RESAMPLE_FRAC_DELAY             0x18050604
/* S                                          0x19xxxxxx*/
#define ERROR_SIZE                            0x19092605
#define ERROR_SYM_TYPE                        0x19251320
/* T                                          0x20xxxxxx*/
/* U                                          0x21xxxxxx*/
#define ERROR_UNWRAP                          0x21142318
/* V                                          0x22xxxxxx*/
/* W                                          0x23xxxxxx*/
#define ERROR_WIN_PARAM                       0x23091601
#define ERROR_WIN_SYM                         0x23091925
#define ERROR_WIN_TYPE                        0x23092025
/* X                                          0x24xxxxxx*/
#define ERROR_XCORR_FLAG                      0x24031518
/* Y                                          0x25xxxxxx*/
/* Z                                          0x26xxxxxx*/

#define DAT_MASK                              0x00000001
#define DAT_float                            0x00000000
#define DAT_COMPLEX                           0x00000001

#define DSPL_MATRIX_BLOCK                     32


#define DSPL_SYMMETRIC                        0x00000000
#define DSPL_PERIODIC                         0x00000001

#define DSPL_FLAG_DIGITAL                     0x00000000
#define DSPL_FLAG_ANALOG                      0x00000001
#define DSPL_FLAG_LOGMAG                      0x00000002
#define DSPL_FLAG_UNWRAP                      0x00000004
#define DSPL_FLAG_FFT_SHIFT                   0x00000008
#define DSPL_FLAG_PSD_TWOSIDED                DSPL_FLAG_FFT_SHIFT




#define DSPL_WIN_SYM_MASK                     0x00000001
#define DSPL_WIN_MASK                         0x00FFFFFE

#define DSPL_WIN_SYMMETRIC                    DSPL_SYMMETRIC
#define DSPL_WIN_PERIODIC                     DSPL_PERIODIC


#define DSPL_WIN_BARTLETT                     0x00000004
#define DSPL_WIN_BARTLETT_HANN                0x00000008
#define DSPL_WIN_BLACKMAN                     0x00000010
#define DSPL_WIN_BLACKMAN_HARRIS              0x00000040
#define DSPL_WIN_BLACKMAN_NUTTALL             0x00000080
#define DSPL_WIN_FLAT_TOP                     0x00000100
#define DSPL_WIN_GAUSSIAN                     0x00000400
#define DSPL_WIN_HAMMING                      0x00000800
#define DSPL_WIN_HANN                         0x00001000
#define DSPL_WIN_LANCZOS                      0x00004000
#define DSPL_WIN_NUTTALL                      0x00008000
#define DSPL_WIN_RECT                         0x00010000
#define DSPL_WIN_COS                          0x00040000
#define DSPL_WIN_CHEBY                        0x00080000
#define DSPL_WIN_KAISER                       0x00100000


#define DSPL_FILTER_TYPE_MASK                 0x000000FF
#define DSPL_FILTER_LPF                       0x00000001
#define DSPL_FILTER_HPF                       0x00000002
#define DSPL_FILTER_BPASS                     0x00000004
#define DSPL_FILTER_BSTOP                     0x00000008

#define DSPL_FILTER_APPROX_MASK               0x0000FF00
#define DSPL_FILTER_BUTTER                    0x00000100
#define DSPL_FILTER_CHEBY1                    0x00000200
#define DSPL_FILTER_CHEBY2                    0x00000400
#define DSPL_FILTER_ELLIP                     0x00000800


#define DSPL_XCORR_NOSCALE                    0x00000000
#define DSPL_XCORR_BIASED                     0x00000001
#define DSPL_XCORR_UNBIASED                   0x00000002



#define ELLIP_ITER                            16
#define ELLIP_MAX_ORD                         24

#define  DSPL_VERIF_FAILED                    1
#define  DSPL_VERIF_SUCCESS                   0

#define PLOT_HOLD                             0x00000001

#define VERIF_STR_BUF         128
#define VERIF_STR_LEN         48
#define VERIF_CHAR_POINT      46
#define VERIF_LEVEL_COMPLEX   1E-11
#define VERIF_LEVEL_float    1E-12



#ifdef __cplusplus
  extern "C" {
#endif



#ifdef BUILD_LIB
  /* Declare DSPL_API for Windows OS */
  #ifdef WIN_OS
    #define DSPL_API __declspec(dllexport)
  #endif /* WIN_OS */
  /* Declare DSPL_API for LINUX OS */
  #ifdef LINUX_OS
    #define DSPL_API
  #endif /* LINUX_OS */
#endif /* BUILD_DLL */

#define COMMA ,


#ifdef BUILD_LIB
    #define DECLARE_FUNC(type, fn, param)\
                         type DSPL_API fn(param);
#endif

#ifndef BUILD_LIB
    #define DECLARE_FUNC( type, fn, param)\
                          typedef type (*p_##fn)(param);\
                          extern p_##fn   fn;

#endif


#ifdef __cplusplus
  }
#endif


#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup SYS_LOADING_GROUP
\fn void* dspl_load()
\brief Perform dynamic linking and load libdspl-2.0 functions.

This function attempts to link to the library `libdspl.dll` in
Windows system and the `libdspl.so` library on the Linux system.
The library is assumed to be in the same directory as the application.
user, or the path to the library is registered in the operating path variables
system.

Upon successful binding and loading of library functions, the handle is returned
libraries, as well as in the address space of the application appear
pointers to libdspl-2.0 functions.

\note
The returned handle is of type `void *`, which can be cast on Windows
to type `HINSTANCE`. In practice, this is not necessary, because this
the type is cast to `HINSTANCE` automatically if the compiler flag is set,
indicating that the application is being built on Windows.

An example of a simple program that implements dynamic binding with DSPL-2.0.

\code
#include <stdio.h>
#include <stdlib.h>
#include "dspl.h"

int main (int argc, char* argv[])
{
    void *hdspl;          // DSPL handle 
    hdspl = dspl_load (); // Dynamic linking 
    
    // Check the pointer. If `NULL`, then the link failed 
    if (! hdspl)
    {
        printf ("libdspl loading error! \n");
        return -1;
    }
    
    // The link was successful, you can call the functions of DSPL-2.0
    
    //Before correctly terminating the application, you must unlink
    //library and clear memory.
    dspl_free(hdspl);
    
    return 0;
}
\endcode

\author Bakhurin Sergey. www.dsplib.org
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup SYS_LOADING_GROUP
\fn void* dspl_load()
\brief Произвести динамическую линковку и загрузить функции libdspl-2.0.

Данная функция производит попытку связывания с библиотекой `libdspl.dll` в 
системе Windows и с библиотекой `libdspl.so` в системе Linux. 
Предполагается, что библиотека находится в одной директории с приложением
пользователя, или путь к библиотеке прописан в переменных пути операционной
системы.

При удачном связывании и загрузке функций библиотеки возвращается хэндл 
библиотеки, а также в адресном пространстве приложения появляются 
указатели на функции libdspl-2.0. 

\note
Возвращаемый хэндл имеет тип `void*`, который в ОС Windows может быть приведен
к типу `HINSTANCE`. На практике необходимости в этом, нет, потому что данный
тип приводится к `HINSTANCE` автоматически, если выставлен флаг компилятора, 
указывающий, что сборка приложения производится в ОС Windows.

Пример простейшей программы реализующей динамическое связывание с DSPL-2.0.

\code
#include <stdio.h>
#include <stdlib.h>
#include "dspl.h"

int main(int argc, char* argv[])
{
    void* hdspl;           // DSPL хэндл
    hdspl = dspl_load();   // Динамическая линковка
    
    // Проверяем указатель. Если `NULL`, то линковка прошла неудачно
    if(!hdspl)
    {
        printf("libdspl loading error!\n");
        return -1;
    }
    
    // Линковка прошла успешно можно вызывать функции DSPL-2.0
     
    // Перед корректным завершением приложения необходимо разлинковать 
    // библиотеку и очистить память.
    dspl_free(hdspl); 
   
    return 0;
}
\endcode

\author Бахурин Сергей. www.dsplib.org
***************************************************************************** */
#endif
void* dspl_load();





#ifdef DOXYGEN_ENGLISH
/*! ****************************************************************************
\ingroup SYS_LOADING_GROUP
\fn void dspl_free(void* handle)
\brief Cleans up the previously linked DSPL-2.0 dynamic library.

This cross-platform function clears the library `libdspl.dll` in
Windows system and from the library `libdspl.so` on the Linux system.
After cleaning the library, all functions will become unavailable.

\param [in] handle
Handle of the previously linked DSPL-2.0 library. \n
This pointer can be `NULL`, in this case no action
are being produced.

\author Bakhurin Sergey. www.dsplib.org
***************************************************************************** */
#endif
#ifdef DOXYGEN_RUSSIAN
/*! ****************************************************************************
\ingroup SYS_LOADING_GROUP
\fn void dspl_free(void* handle)
\brief Очищает связанную ранее динамическую библиотеку DSPL-2.0.

Данная кроссплатформенная функция производит очистку библиотеки `libdspl.dll` в 
системе Windows и с библиотеки `libdspl.so` в системе Linux. 
После очистки библиотеки все функции станут недоступны.

\param[in] handle
Хэндл прилинкованной ранее библиотеки DSPL-2.0. \n
Данный указатель может быть `NULL`, в этом случае никакие действия не 
производятся.\n\n

\author Бахурин Сергей. www.dsplib.org
**************************************************************************** */
#endif
int window(float* w, int n, int win_type, float param)
{
    switch(win_type & DSPL_WIN_MASK)
    {
        case DSPL_WIN_BARTLETT:
            return win_bartlett(w, n, win_type);
        case DSPL_WIN_BARTLETT_HANN:
            return win_bartlett_hann(w, n, win_type);
        case DSPL_WIN_BLACKMAN:
            return win_blackman(w, n, win_type);
        case DSPL_WIN_BLACKMAN_HARRIS:
            return win_blackman_harris(w, n, win_type);
        case DSPL_WIN_BLACKMAN_NUTTALL:
            return win_blackman_nuttall(w, n, win_type);
        case DSPL_WIN_CHEBY:
            return win_cheby(w, n, param);
        case DSPL_WIN_FLAT_TOP:
            return win_flat_top(w, n, win_type);
        case DSPL_WIN_GAUSSIAN:
            return win_gaussian(w, n, win_type, param);
        case DSPL_WIN_HAMMING:
            return win_hamming(w, n, win_type);
        case DSPL_WIN_HANN:
            return win_hann(w, n, win_type);
        case DSPL_WIN_KAISER:
            return win_kaiser(w, n, win_type, param);
        case DSPL_WIN_LANCZOS:
            return win_lanczos(w, n, win_type);
        case DSPL_WIN_NUTTALL:
            return win_nuttall(w, n, win_type);
        case DSPL_WIN_RECT:
            return win_rect(w, n);
        case DSPL_WIN_COS:
            return win_cos(w, n, win_type);
        default:
            return ERROR_WIN_TYPE;
    }
    return RES_OK;
}



/******************************************************************************
Barlett window function
*******************************************************************************/
int win_bartlett(float *w, int n, int win_type)
{
    float x = 0.0;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = (float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = (float)n;     break;
        default: return ERROR_WIN_SYM;
    }

    for(i = 0; i < n; i++)
    {
        w[i] = 2.0 / x * (x * 0.5-fabs((float)i - x * 0.5));
    }
    return RES_OK;
}





/******************************************************************************
Barlett - Hann    window function
******************************************************************************/
int win_bartlett_hann(float *w, int n, int win_type)
{
    float y;
    float x = 0.0;
    int i;

    if(!w)
    return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = 1.0/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = 1.0/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = 0.62 - 0.48 * fabs(y-0.5)-0.38*cos(M_2PI*y);
        y += x;
    }
    return RES_OK;
}





/******************************************************************************
Blackman    window function
******************************************************************************/
int win_blackman(float *w, int n, int win_type)
{
    float y;
    float x = 0.0;
    int i;

    if(!w)
    return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = 0.42 - 0.5* cos(y)+0.08*cos(2.0*y);
        y += x;
    }
    return RES_OK;
}




/******************************************************************************
Blackman - Harris window function
******************************************************************************/
int win_blackman_harris(float *w, int n, int win_type)
{
    float y;
    float x  = 0.0;
    float a0 = 0.35875;
    float a1 = 0.48829;
    float a2 = 0.14128;
    float a3 = 0.01168;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = a0 - a1* cos(y)+a2*cos(2.0*y)-a3*cos(3.0*y);
        y += x;
    }
    return RES_OK;
}




/******************************************************************************
Blackman - Nuttull     window function
******************************************************************************/
int win_blackman_nuttall(float *w, int n, int win_type)
{
    float y;
    float x    = 0.0;
    float a0 = 0.3635819;
    float a1 = 0.4891775;
    float a2 = 0.1365995;
    float a3 = 0.0106411;
    int i;


    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC:        x = M_2PI/(float)(n-1);        break;
        case DSPL_WIN_PERIODIC :        x = M_2PI/(float)n;                break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = a0 - a1* cos(y)+a2*cos(2.0*y)-a3*cos(3.0*y);
        y += x;
    }
    return RES_OK;
}






/******************************************************************************
Chebyshev parametric window function
param sets spectrum sidelobes level in dB
ATTENTION! ONLY SYMMETRIC WINDOW
*******************************************************************************/
int win_cheby(float *w, int n, float param)
{
    int k, i, m;
    float z, dz, sum = 0, wmax=0, r1, x0, chx, chy, in;

    if(!w)
        return ERROR_PTR;

    if(n<2)
        return ERROR_SIZE;

    if(param <= 0.0)
        return ERROR_WIN_PARAM;

    r1 = pow(10, param/20);
    x0 = cosh((1.0/(float)(n-1)) * acosh(r1));

    /* check window length even or odd */
    if(n%2==0)
    {
        dz = 0.5;
        m = n/2-1;
    }
    else
    {
        m = (n-1)/2;
        dz = 0.0;
    }

    for(k = 0; k < m+2; k++)
    {
        z = (float)(k - m) - dz;
        sum = 0;

        for(i = 1; i <= m; i++)
        {
            in = (float)i / (float)n;
            chx = x0 * cos(M_PI * in);
            cheby_poly1(&chx, 1, n-1, &chy);
            sum += chy * cos(2.0 * z * M_PI * in);
        }

        w[k] = r1 + 2.0 * sum;
        w[n-1-k] = w[k];

        /* max value calculation */
        if(w[k]>wmax)
            wmax=w[k];
    }

    /* normalization */
    for(k=0; k < n; k++)
        w[k] /= wmax;

    return RES_OK;
}



/******************************************************************************
Cosine window function
******************************************************************************/
int win_cos(float *w, int n, int win_type)
{
    float y;
    float x = 0.0;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = sin(y);
        y += x;
    }
    return RES_OK;
}






/******************************************************************************
Flat - Top     window function
******************************************************************************/
int win_flat_top(float *w, int n, int win_type)
{
    float y;
    float x  = 0.0;
    float a0 = 1.0;
    float a1 = 1.93;
    float a2 = 1.29;
    float a3 = 0.388;
    float a4 = 0.032;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i<n; i++)
    {
        w[i] = a0 - a1* cos(y)+a2*cos(2.0*y)-a3*cos(3.0*y)+a4*cos(4.0*y);
        y += x;
    }
    return RES_OK;
}






/******************************************************************************
Gaussian window function
******************************************************************************/
int win_gaussian(float *w, int n, int win_type, float alpha)
{
    float a = 0.0;
    float y;
    float sigma;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: a = (float)(n-1)*0.5; break;
        case DSPL_WIN_PERIODIC : a = (float)(n)*0.5; break;
        default: return ERROR_WIN_SYM;
    }


    sigma = 1.0 / (alpha * a);
    for(i = 0; i<n; i++)
    {
        y = ((float)i - a)*sigma;
        w[i] = exp(-0.5*y*y);
    }
        return RES_OK;
}






/******************************************************************************
Hamming window function
******************************************************************************/
int win_hamming(float *w, int n, int win_type)
{
    float x = 0.0;
    float y;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i < n; i++)
    {
        w[i] = 0.54-0.46*cos(y);
        y += x;
    }
    return RES_OK;
}




/******************************************************************************
Hann window function
******************************************************************************/
int win_hann(float *w, int n, int win_type)
{
    float x = 0.0;
    float y;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i < n; i++)
    {
        w[i] = 0.5*(1-cos(y));
        y += x;
    }
    return RES_OK;
}


/******************************************************************************
Kaiser window function
******************************************************************************/
int win_kaiser(float* w, int n, int win_type, float param)
{
    float num, den, x, y, L;
    int i, err;
    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: L = (float)(n-1) / 2.0; break;
        case DSPL_WIN_PERIODIC : L = (float)n / 2.0; break;
        default: return ERROR_WIN_SYM;
    }

    err = bessel_i0(&param, 1, &den);
    if(err != RES_OK)
        return err;
    for(i = 0; i < n; i++)
    { 
        x = 2.0*((float)i - L) / (float)n;
        y = param * sqrt(1.0 - x*x);
        err = bessel_i0(&y, 1, &num);
        if(err != RES_OK)
            return err;
        w[i] = num / den;
    }
    return err;
}



/******************************************************************************
Lanczos window function
******************************************************************************/
int win_lanczos(float *w, int n, int win_type)
{
    float y;
    float x = 0.0;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i < n; i++)
    {
        if((y - M_PI)==0.0)
            w[i] = 1.0;
        else
            w[i] = sin(y - M_PI)/(y - M_PI);
        y += x;
    }
    return RES_OK;

}



/******************************************************************************
Nuttall window function
******************************************************************************/
int win_nuttall(float *w, int n, int win_type)
{
    float y;
    float x  = 0.0;
    float a0 = 0.355768;
    float a1 = 0.487396;
    float a2 = 0.144232;
    float a3 = 0.012604;
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    switch(win_type & DSPL_WIN_SYM_MASK)
    {
        case DSPL_WIN_SYMMETRIC: x = M_2PI/(float)(n-1); break;
        case DSPL_WIN_PERIODIC : x = M_2PI/(float)n; break;
        default: return ERROR_WIN_SYM;
    }

    y = 0.0;
    for(i = 0; i < n; i++)
    {
        w[i] = a0 - a1* cos(y)+a2*cos(2.0*y)-a3*cos(3.0*y);
        y += x;
    }
    return RES_OK;
}





/******************************************************************************
Rectangle window function
******************************************************************************/
int win_rect(float *w, int n)
{
    int i;

    if(!w)
        return ERROR_PTR;
    if(n<2)
        return ERROR_SIZE;

    for(i = 0; i < n; i++)
        w[i] = 1.0;
    return RES_OK;
}

int bessel_i0(float* x, int n, float* y)
{
    float P16[17] = { 1.0000000000000000000000801e+00,
                       2.4999999999999999999629693e-01,
                       2.7777777777777777805664954e-02,
                       1.7361111111111110294015271e-03,
                       6.9444444444444568581891535e-05,
                       1.9290123456788994104574754e-06,
                       3.9367598891475388547279760e-08,
                       6.1511873265092916275099070e-10,
                       7.5940584360755226536109511e-12,
                       7.5940582595094190098755663e-14,
                       6.2760839879536225394314453e-16,
                       4.3583591008893599099577755e-18,
                       2.5791926805873898803749321e-20,
                       1.3141332422663039834197910e-22,
                       5.9203280572170548134753422e-25,
                       2.0732014503197852176921968e-27,
                       1.1497640034400735733456400e-29};

    float P22[23] = { 3.9894228040143265335649948e-01,
                       4.9867785050353992900698488e-02,
                       2.8050628884163787533196746e-02,
                       2.9219501690198775910219311e-02,
                       4.4718622769244715693031735e-02,
                       9.4085204199017869159183831e-02,
                      -1.0699095472110916094973951e-01,
                       2.2725199603010833194037016e+01,
                      -1.0026890180180668595066918e+03,
                       3.1275740782277570164423916e+04,
                      -5.9355022509673600842060002e+05,
                       2.6092888649549172879282592e+06,
                       2.3518420447411254516178388e+08,
                      -8.9270060370015930749184222e+09,
                       1.8592340458074104721496236e+11,
                      -2.6632742974569782078420204e+12,
                       2.7752144774934763122129261e+13,
                      -2.1323049786724612220362154e+14,
                       1.1989242681178569338129044e+15,
                      -4.8049082153027457378879746e+15,
                       1.3012646806421079076251950e+16,
                      -2.1363029690365351606041265e+16,
                       1.6069467093441596329340754e+16};

    float x2;
    int k;

    if(!x || !y)
        return ERROR_PTR;
    if(n < 1)
        return ERROR_SIZE;

    for(k =0; k < n; k++)
    {
        if(x[k] < 0.0)
            return ERROR_NEGATIVE;

        if(x[k] < 7.75)
        {
            x2 = x[k] * x[k] * 0.25;
            polyval(P16, 16, &x2, 1, y+k);
            y[k] = x2 * y[k] + 1.0;
        }
        else
        {
            x2 = 1.0 / x[k];
            polyval(P22, 22, &x2, 1, y+k);
            y[k] *= exp(x[k]) / sqrt(x[k]);
        }
    }
    return RES_OK;
}

int cheby_poly1(float* x, int n, int ord, float* y)
{
    int k, m;
    float t[2];

    if(!x || !y)
        return ERROR_PTR;
    if(n < 1)
        return ERROR_SIZE;
    if(ord<0)
        return ERROR_POLY_ORD;
    if(ord==0)
    {
        for(k = 0; k < n; k++)
        {
            y[k] = 1.0;
        }
        return RES_OK;
    }

    if(ord==1)
    {
        memcpy(y, x, n*sizeof(float));
        return RES_OK;
    }

    for(k = 0; k < n; k++)
    {
        m = 2;
        t[1] = x[k];
        t[0] = 1.0;
        while(m <= ord)
        {
            y[k] = 2.0 * x[k] *t[1] - t[0];
            t[0] = t[1];
            t[1] = y[k];
            m++;
        }
    }
    return RES_OK;
}

int polyval(float* a, int ord, float* x, int n, float* y)
{
    int k, m;

    if(!a || !x || !y)
        return ERROR_PTR;
    if(ord<0)
        return ERROR_POLY_ORD;
    if(n<1)
        return ERROR_SIZE;

    for(k = 0; k < n; k++)
    {
        y[k] = a[ord];
        for(m = ord-1; m>-1; m--)
            y[k] = y[k]*x[k] + a[m];
    }
    return RES_OK;
}

int linspace(float x0, float x1, int n, int type, float* x)
{
    float dx;
    int k;
    if(n < 2)
        return ERROR_SIZE;
    if(!x)
    return ERROR_PTR;
    switch (type)
    {
        case DSPL_SYMMETRIC:
            dx = (x1 - x0)/(float)(n-1);
            x[0] = x0;
            for(k = 1; k < n; k++)
                x[k] = x[k-1] + dx;
            break;
        case DSPL_PERIODIC:
            dx = (x1 - x0)/(float)n;
            x[0] = x0;
            for(k = 1; k < n; k++)
                x[k] = x[k-1] + dx;
            break;
        default:
            return ERROR_SYM_TYPE;
    }
    return RES_OK;
}

int sinc(float* x, int n, float a, float* y)
{
    int k;
    if(!x || !y)
        return ERROR_PTR;
    if(n<1)
        return ERROR_SIZE;
    for(k = 0; k < n; k++)
        y[k] = (x[k]==0.0) ? 1.0 : sin(a*x[k])/(a*x[k]);

    return RES_OK;
}


/******************************************************************************
 * Linear phase lowpass filter
 ******************************************************************************/
int fir_linphase_lpf(int ord, float wp, int win_type,
                     float win_param, float* h)
{
    int n, err = RES_OK;
    float *w = NULL;
    w = (float*)malloc((ord+1)*sizeof(float));
    err = linspace(-(float)ord*0.5, (float)ord*0.5, ord+1, DSPL_SYMMETRIC, w);
    if(err!=RES_OK)
        goto error_proc;
    err = sinc(w, ord+1, M_PI*wp, h);
    if(err!=RES_OK)
        goto error_proc;
    err = window(w, ord+1, win_type | DSPL_SYMMETRIC, win_param);
    if(err!=RES_OK)
        goto error_proc;
    for(n = 0; n < ord+1; n++)
        h[n] *= w[n] * wp;
error_proc:
    if(w)
        free(w);
    return err;
}
/*-------------------------------------------------------------------------
Значение  win_type           |  Описание
-----------------------------|-------------------------------------------
 DSPL_WIN_BARTLETT           | Непараметрическое окно Бартлетта
-----------------------------|-------------------------------------------
 DSPL_WIN_BARTLETT_HANN      | Непараметрическое окно Бартлетта-Ханна
-----------------------------|-------------------------------------------
 DSPL_WIN_BLACKMAN           | Непараметрическое окно Блэкмана 
-----------------------------|-------------------------------------------
 DSPL_WIN_BLACKMAN_HARRIS    | Непараметрическое окно Блэкмана-Харриса
-----------------------------|-------------------------------------------
 DSPL_WIN_BLACKMAN_NUTTALL   | Непараметрическое окно Блэкмана-Натталла
-----------------------------|-------------------------------------------
 DSPL_WIN_CHEBY              | Параметрическое окно Дольф-Чебышева.
                             | Параметр  win_param  задает уровень
                             | боковых лепестков в дБ.
-----------------------------|-------------------------------------------
 DSPL_WIN_COS                | Непараметрическое косинус-окно
-----------------------------|-------------------------------------------
 DSPL_WIN_FLAT_TOP           | Непараметрическое окно с максимально 
                             | плоской вершиной
-----------------------------|-------------------------------------------
 DSPL_WIN_GAUSSIAN           | Параметрическое окно Гаусса
-----------------------------|-------------------------------------------
 DSPL_WIN_HAMMING            | Непараметрическое окно Хемминга
-----------------------------|-------------------------------------------
 DSPL_WIN_HANN               | Непараметрическое окно Ханна
-----------------------------|-------------------------------------------
 DSPL_WIN_KAISER             | Параметрическое окно Кайзера
-----------------------------|-------------------------------------------
 DSPL_WIN_LANCZOS            | Непараметрическое окно Ланкзоса
-----------------------------|-------------------------------------------
 DSPL_WIN_NUTTALL            | Непараметрическое окно Натталла
-----------------------------|-------------------------------------------
 DSPL_WIN_RECT               | Непараметрическое прямоугольное окно
-------------------------------------------------------------------------
*/

//w0-нижняя частота, w1-верхняя частота, filter_type=DSPL_FILTER_BPASS, win_type=см.выше,
//win_param=0,h = *ссылка на массив коэффициентов длиной ord+1

int fir_linphase(int ord, float w0, float w1, int filter_type,
                          int win_type, float win_param, float* h)
{
    int n, err;
    float wc, b, del;
    if(ord<1)
        return ERROR_FILTER_ORD;
    if(w0 <= 0.0)
        return ERROR_FILTER_WP;
    if(!h)
        return ERROR_PTR;
    switch(filter_type)
    {
        /* Lowpass FIR coefficients calculation */
        case DSPL_FILTER_LPF:
            err = fir_linphase_lpf(ord, w0, win_type, win_param, h);
            break;
        /* Highpass FIR coefficients calculation */
        case DSPL_FILTER_HPF:
            err = fir_linphase_lpf(ord, 1.0-w0, win_type, win_param, h);
            if(err == RES_OK)
            {
                /* LPF filter frequency inversion */
                for(n = 0; n < ord+1; n+=2)
                    h[n] = -h[n];
            }
            break;
        /* Bandpass FIR coefficients calculation */
        case DSPL_FILTER_BPASS:
            if(w1 < w0)
            {
                err = ERROR_FILTER_WS;
                break;
            }
            wc = (w0 + w1) * 0.5; /* central frequency */
            b  =  w1 - w0;        /* bandwidth */
            err = fir_linphase_lpf(ord, b*0.5, win_type, win_param, h);
            if(err == RES_OK)
            {
                /* LPF frequency shifting to the central frequency */
                del = 0.5 * (float)ord;
                for(n = 0; n < ord+1; n++)
                    h[n] *= 2.0 * cos(M_PI * ((float)n - del) * wc);
            }
            break;
        /* BandStop FIR coefficients calculation */
        /* ATTENTION! Bandstop filter must be even order only! */
        case DSPL_FILTER_BSTOP:
        {
            float *h0 = NULL;
            /* check filter order. Return error if order is odd. */
            if(ord%2)
                return ERROR_FILTER_ORD;
            /* check frequency (w1 must be higher than w0) */
            if(w1 < w0)
            {
                err = ERROR_FILTER_WS;
                break;
            }
            /* temp coeff vector */
            h0 = (float*)malloc((ord+1) * sizeof(float));
            /* calculate LPF */
            err = fir_linphase(ord, w0, 0.0, DSPL_FILTER_LPF,
                               win_type, win_param, h0);
            if(err!=RES_OK)
            {
                free(h0);
                return err;
            }
            /* calculate HPF */
            err = fir_linphase(ord, w1, 0.0, DSPL_FILTER_HPF,
                               win_type, win_param, h);
            if(err==RES_OK)
            {
                /* Bandstop filter is sum of lowpass and highpass filters */
                for(n = 0; n < ord+1; n++)
                    h[n] += h0[n];
            }
            free(h0);
            break;
        }
        default:
            err = ERROR_FILTER_FT;
    }
    return err;
}



void  dspl_free(void* handle);



#endif /* DSPL_H */

