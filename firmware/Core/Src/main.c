/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 定義封包結構 (總共 5 Bytes)
// __attribute__((packed)) 是為了讓變數緊密排列，不要有空隙
typedef struct __attribute__((packed)) {
    uint8_t header;   // 固定 0xAA (起始 byte)
    uint8_t cmd;      // 指令類型
    uint8_t len;      // 資料長度
    uint8_t data;     // 資料本體 (例如溫度)
    uint8_t checksum; // 檢查碼 (確保資料沒傳錯)
} BmcPacket;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_byte; // 用來存收到的一個 byte
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 告訴編譯器：下面會有一個叫做 calculate_checksum 的函式
uint8_t calculate_checksum(BmcPacket *pkt);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // 1. 啟動 PWM (控制 LED)
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    // 2. 啟動 ADC (準備讀溫度，雖然這段 Code 還沒用到，先開起來)
    HAL_ADC_Start(&hadc1);

    // ▼▼▼▼▼ 新增這一行：啟動接收中斷 ▼▼▼▼▼
      HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
      // ▲▲▲▲▲ 告訴 MCU：準備收 1 個 byte，收到後存在 rx_byte

    // 測試變數
    uint8_t duty_cycle = 0;
    int8_t step = 1;
    //char msg[] = "BMC Alive\r\n";


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  /* USER CODE BEGIN 3 */
	      // --- 1. 呼吸燈 (維持原樣) ---
	      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty_cycle);
	      duty_cycle += step;
	      if (duty_cycle >= 100 || duty_cycle <= 0) step = -step;

	      // --- 2. 準備封包 (這是新的!) ---
	      BmcPacket tx_packet;
	      tx_packet.header = 0xAA;      // 這是我們約定好的開頭
	      tx_packet.cmd    = 0x01;      // 0x01 代表這是「溫度資料」
	      tx_packet.len    = 0x01;      // 資料長度 1 byte

	      // 這裡我們用 duty_cycle 當作假溫度 (0~100度)
	      // 這樣你會看到溫度隨著燈光亮度一起變！
	      tx_packet.data   = duty_cycle;

	      // 計算檢查碼
	      tx_packet.checksum = calculate_checksum(&tx_packet);

	      // --- 3. 發送二進位封包 ---
	      // 注意轉型 (uint8_t*)，我們要送原始的 byte
	      HAL_UART_Transmit(&huart2, (uint8_t*)&tx_packet, sizeof(BmcPacket), 10);

	      // --- 4. 變慢一點，方便觀察 (改成 500ms 或 1000ms) ---
	      HAL_Delay(100);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// 這是 HAL 庫標準的接收完成回調函式
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) // 確認是來自 USART2
  {
    // 如果收到 'R' (Reset)，就把亮度歸零
    if (rx_byte == 'R' || rx_byte == 'r') {
        // 這裡我們存取全域變數 duty_cycle 需要宣告 extern 或移到上面
        // 為了簡單，我們直接歸零計數器就好，反正 main loop 會用它
        // 注意：正規寫法應該要把 duty_cycle 變成全域變數，這裡暫時用作弊法：
        // 我們直接重設 TIM2 的 Counter，讓它感覺像閃了一下
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }

    // 收到指令後，要「再次啟動」監聽，不然下次就不會理你了
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  }
}
// 計算 Checksum 的函式 (簡單 XOR 演算法)
uint8_t calculate_checksum(BmcPacket *pkt) {
    // 算法：Header ^ Cmd ^ Len ^ Data
    return (pkt->header ^ pkt->cmd ^ pkt->len ^ pkt->data);
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
  while (1)
  {
  }
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
