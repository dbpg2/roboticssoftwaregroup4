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
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "math.h"

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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t colorToGrey(uint16_t pixelValue){
	uint16_t red = ((red & 0xF800) >> 11); //Get the image at red May need that in real life tasks
	uint16_t green = ((green & 0x07E0) >> 5); //Sub the pixel to green
	uint16_t blue = (blue & 0x001F); //Sub the pixel to blue
	int y = 0 ;
	y = (red*0.3) + (green*0.59) + (blue*0.11);
	return (y << 11) + (y << 6) + y;
}

uint32_t timer = 0;

const int iter=3;

typedef enum {
    BOTH_BTN, BTN1_ONLY, BTN2_ONLY, ALL_OFF,
} ButtonState;

typedef enum {
    BUTTON1, BUTTON2,

    //This trick means the NumButtons will equal how many buttons there are (how many enum values)
    //This works as long as the enum values are simple (count up from 0)
    NUM_BUTTON,
} Button;

typedef enum {
    LED1, LED2, LED3, LED4, NUM_LED,
} LED;

/**
 * @brief read the button state
 * return 1 if the button is pressed;
 * return 0 if the button is released;
 */
static inline uint8_t read_button(Button btn) {
    switch (btn) {
    case BUTTON1:
        return !btn_read(BTN1);
    case BUTTON2:
        return !btn_read(BTN2);
    default:
        return 0;
    }
}

void arcStop(){
	gpio_set(In1);
	gpio_set(In2);
	gpio_set(In3);
	gpio_set(In4);

}

void arcForward(){
	gpio_reset(In1);
	gpio_set(In2);
	gpio_reset(In3);
	gpio_set(In4);
	TIM4->CCR3=1600;
	TIM4->CCR4=1600;
}

void arcReverse(){
	gpio_set(In1);
	gpio_reset(In2);
	gpio_set(In3);
	gpio_reset(In4);
	TIM4->CCR3=1600;
	TIM4->CCR4=1600;
}

void arcLeft(angle){
	TIM5->CCR1=99-4*angle

	gpio_reset(In1);
	gpio_set(In2);
	gpio_set(In3);
	gpio_reset(In4);
	TIM4->CCR3=1600;
	TIM4->CCR4=1600;
}

void arcRight(angle){
	TIM5->CCR1=99+4*angle;

	gpio_set(In1);
	gpio_reset(In2);
	gpio_reset(In3);
	gpio_set(In4);
	TIM4->CCR3=1600;
	TIM4->CCR4=1600;
}

static ButtonState btn_state(void) {
    if (read_button(BUTTON1) && read_button(BUTTON2)) {
        return BOTH_BTN;
    } else if (read_button(BUTTON1)) {
        return BTN1_ONLY;
    } else if (read_button(BUTTON2)) {
        return BTN2_ONLY;
    } else {
        return ALL_OFF;
    }
}

/* USER CODE END PV */
void sobel(uint16_t *cameraDataPtr, uint16_t width, uint16_t height){
	uint16_t vmatrix[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
	uint16_t hmatrix[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	uint16_t copy[160 * 128] = {};
	// int vmatrix[9] = {-10, 15, -10, 0, 0, 0, 10, 15, 10};
	// int hmatrix[9] = {-10, 0, 10, -15, 0, 20, -15, 0, 10};
	uint16_t window[9];
	for (int x=1; x<width-1; x++) {
		for (int y=1; y<height-1; y++) {
			//fill the matrix
			uint16_t k = 0;
			for (int u=x-1; u<=x+1; u++) {
				for (int v=y-1; v<=y+1; v++) {
					window[k++] = *(cameraDataPtr + width * v + u);
				}
			}

			//change inside this
			int horizontal_kernel = 0;
			int vertical_kernel = 0;
			for (int i = 0; i < 9; i++){
				horizontal_kernel += vmatrix[i] * window[i];
				vertical_kernel += hmatrix[i] * window[i];
			}

			copy[x * width + y] = (int)sqrt(horizontal_kernel ^ 2 + vertical_kernel ^ 2);
			//printf("%d", (int)sqrt(horizontal_kernel ^ 2 + vertical_kernel ^ 2));
		}
	}
	for (int x=1; x<width-1; x++) {
		for (int y=1; y<height-1; y++) {
			*(cameraDataPtr + width * y + x) = copy[x * width + y];
		}
	}
}


void sharpen(uint16_t *cameraDataPtr,  uint16_t width, uint16_t height){
	uint16_t matrix[9] = {0, -1, 0, -1, 8, -1, 0, -1, 0};
	uint16_t window[9];
	uint16_t copy[160 * 128] = {};
	for (int x=1; x<width-1; x++) {
		for (int y=1; y<height-1; y++) {
			//fill the matrix
			int k = 0;
			for (int u=x-1; u<=x+1; u++) {
				for (int v=y-1; v<=y+1; v++) {
					window[k++] = *(cameraDataPtr + width * v + u);
				}
			}

			//change inside this
			for (int i = 0; i < 9; i++){
				copy[width * y + x] += matrix[i] * window[i];
			}
		}
	}
	for (int x=1; x<width-1; x++) {
		for (int y=1; y<height-1; y++) {
			*(cameraDataPtr + width * y + x) = copy[width * y + x];
		}
	}
}

void edgextraction(uint16_t *cameraDataPtr,  uint16_t width, uint16_t height) {
	uint16_t window[9];
	    // Loop though entire image array
	    // ignored the outer edge of the image to simplify the code
   const int Gx[3][3] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
   const int Gy[3][3] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
   int edge = 0;


	for (uint16_t x=1; x<width-1; x++) {
		for (uint16_t y=1; y<height-1; y++) {
			// Fill the 3x3 window
		uint16_t k = 0;
		uint16_t j = 0;
		for (int u=x-1; u<=x+1; u++) {
			uint16_t n = 0;
			for (int v=y-1; v<=y+1; v++) {
				window[k++] = *(cameraDataPtr + width * v + u);
				edge += abs(window[k]*Gx[j][n]) + abs(window[k]*Gy[j][n]);
				n++;
			}
			j++;
		}

		*(cameraDataPtr + width * y + x) = edge;
		}
	}
}



void pwm_init(void) {
	TIM5->ARR = 3999;
		//set the timer1 auto-reload counter
		TIM5->PSC = 419;
		TIM5->CCR1 =99 ;


	TIM4->ARR = 839;
		//set the timer1 auto-reload counter
		TIM4->PSC = 9;
		TIM4->CCR3 =700 ;
		TIM4->CCR4 =700 ;
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

int risingedge() {
	static int level = 0;
	if (read_button(BUTTON1) && level == 0) {
		level = 1;
	} else if (!read_button(BUTTON1) && level == 1) {
		level = 0;
		return 1;
	}
	return 0;
}


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
int* getPixel(uint16_t img[120*160],uint16_t x, uint16_t y) {
	int InputArray[120][160];


    for (int x=0; x<120; x++) {
        for(int y=0; y<160; y++) {
            BMP_GetPixelIndex(img[120*160],x,y,&InputArray[x][y]);
        }
    }
	return InputArray;
}
*/
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_CAN1_Init();
	MX_CAN2_Init();
	MX_USART1_UART_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_TIM5_Init();
	MX_TIM4_Init();

	MX_TIM6_Init();
	/* USER CODE BEGIN 2 */
	volatile uint32_t last_ticks = 0;

	// we turn off all the led first
	led_off(LED1);
	led_off(LED2);
	led_off(LED3);
	led_off(LED4);
	tft_init(PIN_ON_TOP, BLACK, WHITE, YELLOW, DARK_GREEN);
	pwm_init();

	// Init camera

	/*
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
	cam_set_colormode(CAM_RGB565);//turn into rgb camera
	*/

	/* USER CODE END 2 */
	uint16_t img[128 * 160] = {0};
	uint16_t printable[128 * 160] = {0};


	while (1) {
		/*
	}
		if (cam_is_frame_ready()) {
			cam_get_rgb565(img);
			for (int i = 0; i < 128*160; i++){
				colorToGrey(img[i]);
			}

			//uint16_t* getPixel(uint16_t img[128*160],uint16_t x, uint16_t y);
			//edgextraction(img, 128, 160);

			sharpen(img, 128, 160);
			sobel(img, 128, 160);

			cam_rgb2printable(img, printable);
			//ColorToGray(printable);
			tft_print_image(printable, 0, 0, cam_sizes[cam_get_framesize()].width,
							cam_sizes[cam_get_framesize()].height);
			tft_update(0);
			led_toggle(LED2);
		}
		*/


//		arcForward();
//		arcLeft();



		tft_prints(0,0,"%d",TIM5->CCR1);
		tft_update(10);

		// led blinky - useful for indicating the condition of the board
		if (HAL_GetTick() - last_ticks >= 1000) {
			led_toggle(LED4);
			last_ticks = HAL_GetTick();
		}
	}
}
	/* USER CODE END 3 */


/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {}
	/* USER CODE END Error_Handler_Debug */

}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint16_t *file, uint32_t line) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
