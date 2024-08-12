/*
 * 05.saraR5PublishMQTT.c
 *
 *  Created on: 11 ene. 2024
 *      Author: anton
 */


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdio.h"
//#include "IPAddress.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

bool saraR5disconnectMQTT(void){
	char buffer[STANDARD_RESPONSE_BUFFER_SIZE] = "";
	uint8_t disconnectResult = saraR5MQTTdisconnect(buffer, sizeof(buffer));

	if (disconnectResult == SARA_R5_ERROR_SUCCESS) {
	        printf("MQTT disconnection successful.\n");
	        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
	        return true;
	    } else {
	        printf("Error disconnecting MQTT...Probably because no profile was active.");
	        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
	        return false;
	    }
	}

bool saraR5configureMQTTClient(void) {
    char buffer[STANDARD_RESPONSE_BUFFER_SIZE] = "";
    const char *clientId = "IulianCellular";
    uint8_t clientResult = saraR5SetMQTTclientId(clientId, buffer, sizeof buffer);
    HAL_Delay(1000);

    if (clientResult == SARA_R5_ERROR_SUCCESS) {
        printf("MQTT client configured successfully.\n");
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
        return true;
    } else {
        printf("Error configuring MQTT client: %d\n", clientResult);
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
        return false;
    }
}

bool saraR5configureMQTTServer(void) {
    char buffer[STANDARD_RESPONSE_BUFFER_SIZE] = "";
    const char *serverName = "test.mosquitto.org";
    int port = 1883;

    uint8_t serverResult = saraR5SetMQTTserver(serverName, port, buffer, sizeof buffer);

    if (serverResult == SARA_R5_ERROR_SUCCESS) {
        printf("MQTT server configured successfully.\n");
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
        return true;
    } else {
        printf("Error configuring MQTT server: %d\n", serverResult);
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE);
        return false;
    }
}

bool saraR5connectMQTT(void) {
    char buffer3[STANDARD_RESPONSE_BUFFER_SIZE] = "";
    uint8_t connectResult = saraR5MQTTconect(buffer3, sizeof buffer3);

    if (connectResult == SARA_R5_ERROR_SUCCESS) {
        printf("MQTT connection successful.\n");
        memset(buffer3, 0, STANDARD_RESPONSE_BUFFER_SIZE); // Clear the buffer
        return true;
    } else {
        printf("Error establishing MQTT connection: %d\n", connectResult);
        memset(buffer3, 0, STANDARD_RESPONSE_BUFFER_SIZE); // Clear the buffer
        return false;
    }
}

bool saraR5publishMQTTTopic(void) {
    char buffer[STANDARD_RESPONSE_BUFFER_SIZE] = "";
    char message[50];// Buffer for the message

    // Generate a random temperature between 15 and 40
    int randomTemperature = rand() % 26 + 15; // 26 is the range (40 - 15 + 1), 15 is the start number

    // Format the message with the random temperature
	sprintf(message, "Temperatura actual: %d", randomTemperature);

    const char *topic = "/uoc/iulian";
    uint8_t topicLength = strlen(topic);
    int QoS = 0; // Quality of Service level
    int retain = 0; // Retain flag
    uint8_t hex_mode = 0; // Hexadecimal mode
    //const uint8_t *message = (const uint8_t *)"Iulian";
    uint8_t messageLength = strlen((const char *)message);

    uint8_t result = saraR5PublishMQTT(topic, topicLength, buffer, sizeof buffer, QoS, retain, hex_mode, (const uint8_t *)message, messageLength);

    if (result == SARA_R5_ERROR_SUCCESS) {
        printf("Publication to MQTT topic successful.\n");
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE); // Clear the buffer
        return true;
    } else {
        printf("Error publishing to MQTT topic: %d\n", result);
        memset(buffer, 0, STANDARD_RESPONSE_BUFFER_SIZE); // Clear the buffer
        return false;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	Ip_adress ip[MAX_OPS];
	myApn apn[MAX_OPS];
	SARA_R5_pdp_type Type;

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
  MX_USART1_UART_Init();

  /* Buffer to store the response from the module. */
  char resposta[STANDARD_RESPONSE_BUFFER_SIZE];

  // 1. Initialize saraR5
  saraR5Init(SARA_RESPONSE_OK, resposta);
  if (saraR5GetAPN(0, apn, ip, &Type) == SARA_R5_ERROR_SUCCESS) {
  	  printf("APN and IP obtained successfully.\n");
        printf("Connected to operator: %s\n", apn[0].apn); // Cambia 'operatorName' al miembro correcto
    } else {
  	  printf("Error obtaining APN and IP! Freezing... !\n");
  	  while(1);
    }
    HAL_Delay(100);

    // 2. Activate PDP Action
    memset(resposta, 0, sizeof(resposta));
    if (saraR5PerformPDPaction(1, SARA_R5_PSD_ACTION_DESACTIVATE, resposta, sizeof(resposta)) != SARA_R5_ERROR_SUCCESS) {
    	printf("Warning: performPDPaction (deactivate profile) failed. Probably because no profile was active.\n");
	}
	HAL_Delay(1000);

	memset(resposta, 0, sizeof(resposta));
	if (saraR5PerformPDPaction(1, SARA_R5_PSD_ACTION_LOAD, resposta, sizeof(resposta)) != SARA_R5_ERROR_SUCCESS) {
		printf("performPDPaction (load from NVM) failed! Freezing...\n");
		while(1);
	}
        HAL_Delay(1000);

	memset(resposta, 0, sizeof(resposta));
	if (saraR5PerformPDPaction(1, SARA_R5_PSD_ACTION_ACTIVATE, resposta, sizeof(resposta)) != SARA_R5_ERROR_SUCCESS) {
		printf("performPDPaction (activate profile) failed! Freezing...\n");
		while(1);
	}
	HAL_Delay(2000);

	// Disconnect MQTT (if necessary)
	if (!saraR5disconnectMQTT()) {
		printf("Initial MQTT disconnection failed. Attempting to continue...\n");
	}

	// Configure MQTT client
	if (!saraR5configureMQTTClient()) {
		printf("Error configuring MQTT client. Terminating program.\n");
		return -1; // Terminate the program if client configuration fails
	}

	if (!saraR5configureMQTTServer()) {
		printf("Error configuring MQTT server. Terminating program.\n");
		return -1;
	}

	if (!saraR5connectMQTT()) {
		printf("Failed to establish MQTT connection. This could be due to network issues. Please check the configuration and network status, and try again.\n");
		return -1;
	}

	uint32_t lastPublishTime = 0;
	int publishCount = 0;
	while (1) {
		if (HAL_GetTick() - lastPublishTime > 20000) { // Check if 20 seconds have passed
			if (!saraR5publishMQTTTopic()) {
				printf("Failed to publish. Trying again in 20 seconds.\n");
			}else{
				publishCount++;
				if (publishCount >= 5) {
					printf("Published 5 times, stopping now.\n");
				    break; // Exit the loop after 5 successful publishes
				}
			}
			lastPublishTime = HAL_GetTick();
		}
	}


  /* USER CODE BEGIN WHILE */

  while (1)
  {

    /* USER CODE BEGIN 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
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
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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