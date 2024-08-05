#include "led_matrix.hpp"

static const uint8_t POWER_ADDR = 0x0C;
static const uint8_t DECODING_ADDR = 0x09;
static const uint8_t BRIGHT_ADDR = 0x0A;
static const uint8_t SCAN_LIM_ADDR = 0x0B;
static const uint8_t TEST_ADDR = 0x0F;

led_matrix::led_matrix(
        spi_host_device_t spi_host = HSPI_HOST,
        gpio_num_t spi_sck_pin = GPIO_NUM_14,
        gpio_num_t spi_miso_pin = GPIO_NUM_12,
        gpio_num_t spi_mosi_pin = GPIO_NUM_13,
        gpio_num_t spi_cs_pin = GPIO_NUM_15,
        int spi_speed = SPI_MASTER_FREQ_8M,
        int brightness = 0xF
        ) : 
        spi_host(spi_host),
        spi_sck_pin(spi_sck_pin),
        spi_miso_pin(spi_miso_pin),
        spi_mosi_pin(spi_mosi_pin),
        spi_cs_pin(spi_cs_pin),
        spi_speed(spi_speed),
        brightness((uint8_t)brightness),
        rows_data(new row_st[8])
{
    memset(&this->rows_data[0], 0, sizeof(row_st) * 8);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = this->spi_mosi_pin;
    buscfg.miso_io_num = this->spi_miso_pin;
    buscfg.sclk_io_num = this->spi_sck_pin;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.flags = SPICOMMON_BUSFLAG_MASTER;

    spi_device_interface_config_t devcfg = {};
    devcfg.command_bits = 0;
    devcfg.address_bits = 0;
    devcfg.mode = 0;
    devcfg.clock_source = SPI_CLK_SRC_DEFAULT;
    devcfg.clock_speed_hz = this->spi_speed;
    devcfg.spics_io_num = this->spi_cs_pin;
    devcfg.queue_size = 1;
    devcfg.pre_cb = NULL;
    devcfg.post_cb = NULL;

    ESP_ERROR_CHECK(spi_bus_initialize(this->spi_host, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(this->spi_host, &devcfg, &this->spi));
}

led_matrix::~led_matrix(void){
    spi_bus_remove_device(this->spi);
    spi_bus_free(this->spi_host);
    delete [] this->rows_data;
}

void led_matrix::Add_Dot(uint8_t x, uint8_t y)
{
    x %= 8;
    y %= 8;

    this->rows_data[y].data |= 1 << x;
    this->rows_data[y].upd_flg = true;
}

void led_matrix::Remove_Dot(uint8_t x, uint8_t y)
{
    x %= 8;
    y %= 8;

    this->rows_data[y].data &= ~(1 << x);
    this->rows_data[y].upd_flg = true;
}

void led_matrix::Update_Rows_Task(void)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        if(this->rows_data[i].upd_flg == true)
        {
            this->Write_Reg((i + 1), this->rows_data[i].data);
            this->rows_data[i].upd_flg = false;
        }
    }
}

void led_matrix::Setting(void)
{
    this->Write_Reg(POWER_ADDR, 0x00);
    this->Write_Reg(DECODING_ADDR, 0x00);
    this->Write_Reg(BRIGHT_ADDR, this->brightness);
    this->Write_Reg(SCAN_LIM_ADDR, 0x07);
    this->Write_Reg(TEST_ADDR, 0x00);
    this->Write_Reg(POWER_ADDR, 0x01);
    this->Clear();
}

void led_matrix::Clear(void)
{
    for(uint8_t i = 1; i <= 8; i++)
        this->Write_Reg(i, 0);
}



void led_matrix::Write_Reg(uint8_t reg, uint8_t value)
{
    spi_transaction_t transmit;
    memset(&transmit, 0, sizeof(spi_transaction_t));

    uint8_t tx_buff[2] = {(uint8_t)reg, value};

    transmit.tx_buffer = tx_buff;
    transmit.length = 8 * sizeof(tx_buff);
    ESP_ERROR_CHECK(spi_device_transmit(this->spi, &transmit));
}

