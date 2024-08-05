#ifndef __led_matrix_HPP_
#define __led_matrix_HPP_

#include "main.hpp"

typedef struct row {
    uint8_t data;
    bool upd_flg;
} row_st;

class led_matrix {
public:
    led_matrix(
        spi_host_device_t spi_host,
        gpio_num_t spi_sck_pin,
        gpio_num_t spi_miso_pin,
        gpio_num_t spi_mosi_pin,
        gpio_num_t spi_cs_pin,
        int spi_speed,
        int brightness
        );

    ~led_matrix(void);
    void Add_Dot(uint8_t x, uint8_t y);
    void Remove_Dot(uint8_t x, uint8_t y);

    void Setting(void);
    void Clear(void);

    void Update_Rows_Task(void);
private:
    void Write_Reg(uint8_t reg, uint8_t value);

    const spi_host_device_t spi_host;
    const gpio_num_t spi_sck_pin;
    const gpio_num_t spi_miso_pin;
    const gpio_num_t spi_mosi_pin;
    const gpio_num_t spi_cs_pin;
    const int spi_speed;
    spi_device_handle_t spi;
    uint8_t brightness;

    row_st * const rows_data;
};


#endif /* __led_matrix_HPP_ */