/*
 * -----------------------------------------------------------------------------
 * Project       : ESP32-snake-game
 * File          : main.cpp
 * Author        : nktsb
 * Created on    : 11.07.2024
 * GitHub        : https://github.com/nktsb/ESP32-snake-game
 * -----------------------------------------------------------------------------
 * Copyright (c) 2024 nktsb
 * All rights reserved.
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include "main.hpp"
#include "led_matrix.hpp"
#include "buttons.hpp"
#include "snake.hpp"
#include "buttons.hpp"

extern "C"
{
	void app_main(void);
}


const int DEFAULT_SNAKE_STEP_PERIOD = 250;
const int DEFAULT_SNAKE_FIELD_WIDTH = 8;
const int DEFAULT_SNAKE_FIELD_HEIGHT = 8;

const spi_host_device_t DEFAULT_SPI_HOST = HSPI_HOST;
const gpio_num_t DEFAULT_SPI_SCK_PIN = GPIO_NUM_14;
const gpio_num_t DEFAULT_SPI_MISO_PIN = GPIO_NUM_12;
const gpio_num_t DEFAULT_SPI_MOSI_PIN = GPIO_NUM_13;
const gpio_num_t DEFAULT_SPI_NSS_PIN = GPIO_NUM_15;

QueueHandle_t buttons_queue;

led_matrix *matrix = new led_matrix(DEFAULT_SPI_HOST, DEFAULT_SPI_SCK_PIN,
	DEFAULT_SPI_MISO_PIN, DEFAULT_SPI_MOSI_PIN, DEFAULT_SPI_NSS_PIN, 1000000, 0xF);

static int Get_Current_Time_ms()
{
	return pdTICKS_TO_MS(xTaskGetTickCount());
}

static void Add_Point(int x, int y)
{
	matrix->Add_Dot((uint8_t)x, (uint8_t)y);
}

static void Remove_Point(int x, int y)
{
	matrix->Remove_Dot((uint8_t)x, (uint8_t)y);
}

static void Delay(int time_ms)
{
	vTaskDelay(pdMS_TO_TICKS(time_ms));
}

static void IRAM_ATTR Left_Button_Handler(void *obj)
{
	direction_t dir = LEFT;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(buttons_queue, &dir,&xHigherPriorityTaskWoken);
}

static void IRAM_ATTR Right_Button_Handler(void *obj)
{
	direction_t dir = RIGHT;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(buttons_queue, &dir,&xHigherPriorityTaskWoken);
}

static void IRAM_ATTR Down_Button_Handler(void *obj)
{
	direction_t dir = DOWN;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(buttons_queue, &dir,&xHigherPriorityTaskWoken);
}

static void IRAM_ATTR Up_Button_Handler(void *obj)
{
	direction_t dir = UP;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendFromISR(buttons_queue, &dir,&xHigherPriorityTaskWoken);
}

static void vLedMatrixTask(void* obj)
{
    matrix->Setting();
    for(;;)
    {
        matrix->Update_Rows_Task();
        vTaskDelay(pdMS_TO_TICKS(10));
    };
}

void app_main(void)
{
	buttons_queue = xQueueCreate(16, sizeof(direction_t));

	xTaskCreate(vLedMatrixTask, "led_matrix_task", 1024*4, NULL, 10, NULL);

	snake_game snake(DEFAULT_SNAKE_STEP_PERIOD, 
			Add_Point, Remove_Point, Get_Current_Time_ms, Delay, 
			DEFAULT_SNAKE_FIELD_WIDTH, DEFAULT_SNAKE_FIELD_HEIGHT);

	ESP_ERROR_CHECK(gpio_install_isr_service(0));

	button left_button(Left_Button_Handler, (void*)&snake, GPIO_NUM_27);
	button right_button(Right_Button_Handler, (void*)&snake, GPIO_NUM_26);
	button down_button(Down_Button_Handler, (void*)&snake, GPIO_NUM_25);
	button up_button(Up_Button_Handler, (void*)&snake, GPIO_NUM_10);

	direction_t dir = UP;

	for(;;)
	{
		snake.Snake_Task();
		if(xQueueReceive(buttons_queue, &dir, 0) == pdPASS)
			snake.Set_Direction(dir);
		vTaskDelay(pdMS_TO_TICKS(10));
	};
}