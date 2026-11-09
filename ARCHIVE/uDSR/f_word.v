#include <stdint.h>
#include <math.h>

// Функция для вычисления 32-битного слова частоты
uint32_t calculate_tuning_word(double f_dial_hz) {
    const double f_clk = 63000000.0; // Новая тактовая частота 63 МГц
    const double pow_2_32 = 4294967296.0; // 2^32
    
    // Вычисляем слово частоты с округлением до ближайшего целого
    double tw_exact = (f_dial_hz * pow_2_32) / f_clk;
    uint32_t tuning_word = (uint32_t)round(tw_exact);
    
    // Если частота равна 0, возвращаем 1 (так как 0 — это флаг "без изменений" для FPGA)
    if (tuning_word == 0) {
        tuning_word = 1; 
    }
    
    return tuning_word;
}
/*
Пример: Если вы хотите настроиться на середину диапазона 40 метров — 7 100 000 Гц (7.1 МГц), 
микроконтроллер вычислит и отправит по SPI слово: 7,100,000 * 2^32 / 63,000,000=484032592{hex = 0x1CD6D450)}



*/