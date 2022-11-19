
#include "main.h"

#include "can.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"


#include "camera.h"
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"

void SystemClock_Config(void);

void sobel(uint16_t *cameraDataPtr, uint16_t width, uint16_t height){
	const uint16_t vmatrix[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
	const uint16_t hmatrix[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	// int vmatrix[9] = {-10, 15, -10, 0, 0, 0, 10, 15, 10};
	// int hmatrix[9] = {-10, 0, 10, -15, 0, 20, -15, 0, 10};
	uint16_t window[9];
	for (int y=1; y<height-1; y++) {
		for (int x=1; x<width-1; x++) {

			//fill the matrix
			int k = 0;
			for (int v=y-1; v<=y+1; v++) {
				for (int u=x-1; u<=x+1; u++) {
					window[k++] = *(cameraDataPtr + width * v + u) >> 11;
				}
			}

			//change inside this
			uint16_t horizontal_kernel = 0;
			uint16_t vertical_kernel = 0;
			for (int i = 0; i < 9; i++){
				horizontal_kernel += vmatrix[i] * window[i];
				vertical_kernel += hmatrix[i] * window[i];
			}

			uint16_t temp = (uint16_t)sqrt(horizontal_kernel * horizontal_kernel + vertical_kernel * vertical_kernel);
			*(cameraDataPtr + width * (y-1) + (x-1)) = ((temp) << 11) + ((temp) << 6) + temp;
		}
	}
}

int main(void) {

	HAL_Init();


	SystemClock_Config();


	MX_GPIO_Init();
	MX_CAN1_Init();
	MX_CAN2_Init();
	MX_USART1_UART_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_TIM5_Init();
	MX_TIM6_Init();

	volatile uint32_t last_ticks = 0;

	// we turn off all the led first
	led_off(LED1);
	led_off(LED2);
	led_off(LED3);
	led_off(LED4);
	tft_init(PIN_ON_TOP, BLACK, WHITE, YELLOW, DARK_GREEN);

	// Init camera
	camera_GPIO_init();
	tft_prints(0, 0, "Initing camera");
	tft_update(0);
	if (camera_init() == CAM_NOT_INITED || camera_init() == CAM_INIT_ERROR) {
		tft_prints(0, 0, "No OV7725 module");
	} else {
		tft_prints(0, 0, "Inited");
		cam_set_state(CAM_CAPTURING);
	}
	tft_update(0);
	cam_set_window(0, 0, QQVGA_120x160);
	cam_set_colormode(CAM_GRAYSCALE);


	uint16_t img[120 * 160] = {0};
	uint16_t printable[120 * 160] = {0};




	while (1) {

		if (cam_is_frame_ready()) {
			cam_get_rgb565(img);
			sobel(img, 120, 160);

			cam_rgb2printable(img, printable);
			tft_print_image(printable, 0, 0, cam_sizes[cam_get_framesize()].width,
							cam_sizes[cam_get_framesize()].height);
			tft_update(0);
			led_toggle(LED2);
		}

		// led blinky - useful for indicating the condition of the board
		if (HAL_GetTick() - last_ticks >= 100) {
			led_toggle(LED4);
			last_ticks = HAL_GetTick();
		}
	}
	/* USER CODE END 3 */
}



/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}


void Error_Handler(void) {

	__disable_irq();
	while (1) {}
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line) {

}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
