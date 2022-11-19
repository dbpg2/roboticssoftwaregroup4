/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "camera/camera.h"
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"
// #include "legacy/camera.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define L_MOTOR1_GPIO_Port 	GPIOC
#define L_MOTOR1_Pin 		GPIO_PIN_0
#define L_MOTOR2_GPIO_Port 	GPIOC
#define L_MOTOR2_Pin 		GPIO_PIN_1
#define R_MOTOR1_GPIO_Port 	GPIOC
#define R_MOTOR1_Pin 		GPIO_PIN_14
#define R_MOTOR2_GPIO_Port 	GPIOC
#define R_MOTOR2_Pin 		GPIO_PIN_15
#define HEIGHT 160
#define WIDTH 128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void bNwFilter(uint16_t *img);
void sobel(uint16_t *cameraDataPtr, uint16_t width, uint16_t height);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_CAN1_Init();
	MX_CAN2_Init();
	MX_USART1_UART_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_TIM5_Init();
	MX_TIM6_Init();
	MX_TIM4_Init();
	MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
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

	// motor pwm
	TIM4->ARR = 1000;    //set the timer1 auto-reload counter
	TIM4->PSC = 900;    //set the timer1 prescaler value
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	TIM4->CCR3 = 100; //set the compare value of timer1 channel1
	TIM4->CCR4 = 100; //set the compare value of timer1 channel1

	uint16_t img[128 * 160] = {0};
	uint16_t printable[128 * 160] = {0};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */
		gpio_reset(L_MOTOR1);
		gpio_reset(L_MOTOR2);
		gpio_reset(R_MOTOR1);
		gpio_reset(R_MOTOR2);

		TIM4->CCR3 = 1000; //set the compare value of timer1 channel1
		TIM4->CCR4 = 1000;

    /* USER CODE BEGIN 3 */
		if (cam_is_frame_ready()) {
			cam_get_rgb565(img);
			cam_set_sharpness(10 , 66, 80, 50);
//			bNwFilter(img);
			sobel(img, WIDTH, HEIGHT);

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
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void bNwFilter(uint16_t *img){
	for (int i = 0; i < HEIGHT * WIDTH; i++){
		if (img[i] > 19660){
			img[i] = 65535;
		}
		else{
			img[i] = 0;
		}
	}
}
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
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
