/* ----------------------------------------------------------------------
 * Project: TinyEngine
 * Title:   main.cpp
 *
 * Reference papers:
 *  - MCUNet: Tiny Deep Learning on IoT Device, NeurIPS 2020
 *  - MCUNetV2: Memory-Efficient Patch-based Inference for Tiny Deep Learning, NeurIPS 2021
 *  - MCUNetV3: On-Device Training Under 256KB Memory, NeurIPS 2022
 * Contact authors:
 *  - Wei-Ming Chen, wmchen@mit.edu
 *  - Wei-Chen Wang, wweichen@mit.edu
 *  - Ji Lin, jilin@mit.edu
 *  - Ligeng Zhu, ligeng@mit.edu
 *  - Song Han, songhan@mit.edu
 *
 * Target ISA:  ARMv7E-M
 * -------------------------------------------------------------------- */

#include "main.h"
#include "camera.h"
#include "lcd.h"
#include "profile.h"
#include "stdio.h"
#include "string.h"
#include "testing_data/golden_data.h"
#include "testing_data/images.h"
extern "C" {
#include "genNN.h"
#include "tinyengine_function.h"
}
// #define TESTTENSOR
#define UseCamera 0  // 1 for using Arducam; 0 for not using Arducam.
#define NoCamera_Person 1  // 1 for person; 0 for non-person. (Only applicable when UseCamera == 0)

 #include "stm32746g_discovery.h"

static void SystemClock_Config(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);


/*
TODO: Modify based on your model's input size
*/
#define RES_W 32
#define RES_H 32
#define OUTPUT_CH 2

/*
TODO: Modify if the model changes
- MAX_BOX_PER_CLASS (int) : maximum number of boxes considered per class
- MAX_NUM_CLASSES (int) : number of classes
- THRESHOLD (float) : threshold to filter boxes with low confidence score (the higher the more stringent)
*/
#define MAX_BOX_PER_CLASS 40
#define MAX_NUM_CLASSES 4
#define THRESHOLD 0.35

void SystemClock_Config(void);
void StartDefaultTask(void const *argument);

signed char out_int[OUTPUT_CH];

float labels[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int box_cnt[MAX_NUM_CLASSES] = {0};
det_box results[MAX_NUM_CLASSES][MAX_BOX_PER_CLASS];
det_box* results_ptrs[MAX_NUM_CLASSES];

// Execute model inference and post-processing
void invoke_new_weights_givenimg(signed char *out_int8) {
  invoke(labels);

  // Initialize array for detection results
  for (int i = 0; i < MAX_NUM_CLASSES; i++) {
    for (int j = 0; j < MAX_BOX_PER_CLASS; j++) {
      results[i][j].x0 = 0;
      results[i][j].y0 = 0;
      results[i][j].x1 = 0;
      results[i][j].y1 = 0;
      results[i][j].score = 0.0f;
    }
    results_ptrs[i] = results[i];
  }

  // Post-processing to extract bounding boxes from model output
  det_post_procesing(box_cnt, results_ptrs, THRESHOLD);

}

 #define BUTTON1_Pin GPIO_PIN_0
 #define BUTTON1_GPIO_Port GPIOA
 #define BUTTON2_Pin GPIO_PIN_10
 #define BUTTON2_GPIO_Port GPIOF

 uint16_t *RGBbuf;

//extern "C" int main(void);

extern "C" {
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
}

// Declare globally
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

extern "C" int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


extern "C" int main(void) {
  // variable declaration
  char buf[150];
  char showbuf[150];
  char score[150];
  char bbox[150];
  uint8_t buffer[1];


  CPU_CACHE_Enable();
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  lcdsetup();

#ifdef TESTTENSOR
  {
    uint32_t start, end;
    setRGBTestImage();
    start = HAL_GetTick();
    invoke(labels);
    end = HAL_GetTick();
    uint8_t *output = (uint8_t *)getOutput();
    int i;
    for (i = 0; i < 100; i++) {
      sprintf(buf, "%d,", output[i]);
      printLog(buf);
    }
    printLog("\r\n");
    return 0;
  }
#endif

#if UseCamera
  int camErr = initCamera();

  uint32_t start, end;
  StartCapture();
  volatile signed char *input = getInput();
  RGBbuf = (uint16_t *)&input[80 * 80 * 4];
  int t_mode = 0;

  while (1) {
    start = HAL_GetTick();
    ReadCapture();
    StartCapture();
    DecodeandProcessAndRGB(RES_W, RES_H, input, RGBbuf, 1);
    for (int i = 0; i < RES_W; i++) {
      for (int j = 0; j < RES_W; j++) {
        uint8_t red = (int32_t)input[(80 * i + j) * 3] + 128;
        uint8_t green = (int32_t)input[(80 * i + j) * 3 + 1] + 128;
        uint8_t blue = (int32_t)input[(80 * i + j) * 3 + 2] + 128;

        uint16_t b = (blue >> 3) & 0x1f;
        uint16_t g = ((green >> 2) & 0x3f) << 5;
        uint16_t r = ((red >> 3) & 0x1f) << 11;

        RGBbuf[j + RES_W * i] = (uint16_t)(r | g | b);
      }
    }
    loadRGB565LCD(10, 10, RES_W, RES_W, RGBbuf, 3);

  	invoke_new_weights_givenimg(out_int);
  	int person = 0;
  	if (out_int[0] > out_int[1]) {
  	  person = 0;
  	}
  	else {
  	  person = 1;
  	}
  	end = HAL_GetTick();
  	sprintf(showbuf, " Inference ");
  	displaystring(showbuf, 273, 10);
  	detectResponse(person, end - start, t_mode, 0, 0);

  	while (1);
  }

#else
  uint32_t start, end;
  volatile signed char *input = getInput();

  /*
  TODO: Modify if the model changes,
  - input_scale (float) : scale for input tensor of the first layer of the model
  - input_offset (int32_t) : zero point for input tensor of the first layer of the model
  - image (unsigned char*) : input image (imported from images.h)
  - image_scale (int) : scaling factor when displaying the input image and bboxes on LCD (to fit the LCD size)
  */
  float input_scale = 0.01865844801068306;
  int32_t input_offset = -14;
  const unsigned char *image = blob2;
  int image_scale = 8;

  // Pre-process the input image and convert to int8_t
  for (int i = 0; i < RES_W * RES_H * 3; i++) {
    input[i] = (int8_t) (((float) image[i] / (255 * input_scale)) + input_offset);
  }

   RGBbuf = (uint16_t *)&input[RES_H * RES_W * 4];
   int t_mode = 0;

   start = HAL_GetTick();

  // Display the input image on LCD
  for (int i = 0; i < RES_W; i++) {
    for (int j = 0; j < RES_W; j++) {
       uint8_t red = (int32_t)image[(RES_W * i + j) * 3];
       uint8_t green = (int32_t)image[(RES_W * i + j) * 3 + 1];
       uint8_t blue = (int32_t)image[(RES_W * i + j) * 3 + 2];

      uint16_t b = (blue >> 3) & 0x1f;
      uint16_t g = ((green >> 2) & 0x3f) << 5;
      uint16_t r = ((red >> 3) & 0x1f) << 11;

      RGBbuf[j + RES_W * i] = (uint16_t)(r | g | b);
    }
  }
  loadRGB565LCD(10, 10, RES_W, RES_H, RGBbuf, image_scale);

  // Model Inference
  invoke_new_weights_givenimg(out_int);

  // Process the detection results
  for (int i = 0; i < MAX_NUM_CLASSES; i++) {
    for (int j = 0; j < MAX_BOX_PER_CLASS; j++) {

      // Draw red bounding box if score > 0
      if (results_ptrs[i][j].score > 0) {
        drawRedBox(10, 10, results_ptrs[i][j].x0, results_ptrs[i][j].x1,
                      results_ptrs[i][j].y0, results_ptrs[i][j].y1, image_scale);
      }
    }
  }
  
	while (1) {

	}

  end = HAL_GetTick();
#endif
}

 void SystemClock_Config(void) {
   RCC_ClkInitTypeDef RCC_ClkInitStruct;
   RCC_OscInitTypeDef RCC_OscInitStruct;
   HAL_StatusTypeDef ret = HAL_OK;

   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM = 25;
   RCC_OscInitStruct.PLL.PLLN = 432;
   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ = 9;

   ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
   if (ret != HAL_OK) {
     while (1) {
       ;
     }
   }

   ret = HAL_PWREx_EnableOverDrive();
   if (ret != HAL_OK) {
     while (1) {
       ;
     }
   }

   RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

   ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
   if (ret != HAL_OK) {
     while (1) {
       ;
     }
   }
 }
 static void MX_USART1_UART_Init(void)
  {

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
    huart1.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
      Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */

  }

 static void MX_USART3_UART_Init(void)
 {
     huart3.Instance = USART3;
     huart3.Init.BaudRate = 115200;
     huart3.Init.WordLength = UART_WORDLENGTH_8B;
     huart3.Init.StopBits = UART_STOPBITS_1;
     huart3.Init.Parity = UART_PARITY_NONE;
     huart3.Init.Mode = UART_MODE_TX_RX;
     huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     huart3.Init.OverSampling = UART_OVERSAMPLING_16;

     if (HAL_UART_Init(&huart3) != HAL_OK)
     {
         Error_Handler();
     }
 }

 static void Error_Handler(void) {

   BSP_LED_On(LED1);
   while (1) {
   }
 }

 static void CPU_CACHE_Enable(void) {

   SCB_EnableICache();

   SCB_EnableDCache();
 }

 static void MX_GPIO_Init(void) {
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOJ_CLK_ENABLE();
   __HAL_RCC_GPIOI_CLK_ENABLE();
   __HAL_RCC_GPIOK_CLK_ENABLE();
   __HAL_RCC_GPIOF_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();

   HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin,
                     GPIO_PIN_SET);

   HAL_GPIO_WritePin(GPIOI, ARDUINO_D7_Pin | ARDUINO_D8_Pin, GPIO_PIN_RESET);

   HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

   HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

   HAL_GPIO_WritePin(DCMI_PWR_EN_GPIO_Port, DCMI_PWR_EN_Pin, GPIO_PIN_RESET);

   HAL_GPIO_WritePin(GPIOG, ARDUINO_D4_Pin | ARDUINO_D2_Pin | EXT_RST_Pin,
                     GPIO_PIN_RESET);

   GPIO_InitStruct.Pin = OTG_HS_OverCurrent_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(OTG_HS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = RMII_TXD1_Pin | RMII_TXD0_Pin | RMII_TX_EN_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ULPI_D7_Pin | ULPI_D6_Pin | ULPI_D5_Pin | ULPI_D3_Pin |
                         ULPI_D2_Pin | ULPI_D1_Pin | ULPI_D4_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = SPDIF_RX0_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
   HAL_GPIO_Init(SPDIF_RX0_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = OTG_FS_VBUS_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(OTG_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = Audio_INT_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(Audio_INT_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = OTG_FS_P_Pin | OTG_FS_N_Pin | OTG_FS_ID_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ARDUINO_D7_Pin | ARDUINO_D8_Pin | LCD_DISP_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = uSD_Detect_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = TP3_Pin | NC2_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ARDUINO_SCK_D13_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
   HAL_GPIO_Init(ARDUINO_SCK_D13_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = DCMI_PWR_EN_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(DCMI_PWR_EN_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = GPIO_PIN_11;
   GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = LCD_INT_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ULPI_NXT_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
   HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ARDUINO_D4_Pin | ARDUINO_D2_Pin | EXT_RST_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ULPI_STP_Pin | ULPI_DIR_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = RMII_RXER_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(RMII_RXER_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ULPI_CLK_Pin | ULPI_D0_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = ARDUINO_MISO_D12_Pin | ARDUINO_MOSI_PWM_D11_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = BUTTON1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(BUTTON1_GPIO_Port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = BUTTON2_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(BUTTON2_GPIO_Port, &GPIO_InitStruct);
 }

 #ifdef USE_FULL_ASSERT
 void assert_failed(uint8_t *file, uint32_t line) {

   while (1) {
   }
 }
 #endif
