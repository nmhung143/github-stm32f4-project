#include "main.h"
#include<stdbool.h>
#include<stdio.h>
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define RS_Port GPIOB
#define EN_Port GPIOB
#define D4_Port GPIOB
#define D5_Port GPIOB
#define D6_Port GPIOB
#define D7_Port GPIOB 


char DATA[27];


void LCD_Enable()
{
HAL_GPIO_WritePin(EN_Port,EN_Pin,1);
HAL_Delay(1);
HAL_GPIO_WritePin(EN_Port,EN_Pin,0);	
HAL_Delay(1);	
}

void LCD_Send4Bit(unsigned char Data)
{
HAL_GPIO_WritePin(D4_Port,D4_Pin,Data&0x01);
HAL_GPIO_WritePin(D5_Port,D5_Pin,(Data>>1)&0x01);
HAL_GPIO_WritePin(D6_Port,D6_Pin,(Data>>2)&0x01);
HAL_GPIO_WritePin(D7_Port,D7_Pin,(Data>>3)&0x01);	
}

void LCD_SendCommand(unsigned char command)
{
	LCD_Send4Bit(command >>4);
	LCD_Enable();
	LCD_Send4Bit(command);
	LCD_Enable();
}

void LCD_Clear()
{
 	LCD_SendCommand(0x01);  
  HAL_Delay(1);	
}

void LCD_Init()
{
	LCD_Send4Bit(0x00);
  HAL_GPIO_WritePin(RS_Port,RS_Pin,0);
	LCD_Send4Bit(0x03);
	LCD_Enable();
	LCD_Enable();
	LCD_Enable();
	LCD_Send4Bit(0x02);
	LCD_Enable();
	LCD_SendCommand(0x28);
	LCD_SendCommand(0x0C);
	LCD_SendCommand(0x06);
	LCD_SendCommand(0x01);
}

void LCD_Gotoxy(unsigned char x, unsigned char y)
{
unsigned char address;
if(y==0)
address=0x80;
else if(y==1)
{
address=0xc0;
}
else if(y==2)
{
address=0x94;
}
else
address=0xd4;
address+=x;
LCD_SendCommand(address);
}

void LCD_PutChar(unsigned char Data)
{
  HAL_GPIO_WritePin(RS_Port,RS_Pin,1);
 	LCD_SendCommand(Data);
  HAL_GPIO_WritePin(RS_Port,RS_Pin,0);
}

void LCD_Puts(char *s)
{
   	while (*s){
      	LCD_PutChar(*s);
     	s++;
   	}
}

void print( uint8_t i) {
	sprintf(DATA, "%d", i);
	LCD_Puts(DATA);
}

#define UP 7 
#define DOWN 8
#define ENTER 9
#define CANCEL 10
#define HOME 6
#define ALL_MENU 8

char getVolt();
uint8_t getTemp();
void showModule(uint8_t k);
void firstPage();
void setTemp(uint8_t temp);
void setVolt(uint8_t volt);
int keyScan();

enum ids{_BEGIN,_CAI_DAT,_NHIET_DO,_DIEN_AP,_THONG_TIN,_MODULE_1,_MODULE_2,_MODULE_3,_MODULE_4,};

struct TypeMenu {
  uint8_t MenuID;
  uint8_t SubMenu;
  char MenuName[16];
};

int ContainMenu, PAGE = 0, LEVEL[7], LevelPtr = 0, STATE = _BEGIN;
bool AccessENTER = true , AccessCANCEL = true;

struct TypeMenu Menu[ALL_MENU] = {
  {_CAI_DAT,_BEGIN,"Cai dat"},
  {_NHIET_DO,_CAI_DAT,"Nhiet do"},
  {_DIEN_AP,_CAI_DAT,"Dien ap"},
  {_THONG_TIN,_BEGIN,"Thong tin"},
  {_MODULE_1,_THONG_TIN,"Module 1"},
  {_MODULE_2,_THONG_TIN,"Module 2"},
  {_MODULE_3,_THONG_TIN,"Module 3"},
  {_MODULE_4,_THONG_TIN,"Module 4"},
};
struct TypeMenu MenuPoiter[2];

int key;

int amp[4][10];
char ampName[10][10] = {"Acquy1","Acquy2","Acquy3","Acquy4","Acquy5","Acquy6","Acquy7","Acquy8","Acquy9","Acquy10"};

uint8_t temp = 30;
uint8_t volt = 20;
uint8_t num=0, num2=0;
uint16_t adc_value[2];

unsigned char SelectFrom(uint8_t from) {
  int i, NumMenu = 0,k=0, i0;                
  for(i=0; i<ALL_MENU; i++) {
    if(Menu[i].SubMenu == STATE) {
      if(NumMenu == 0) i0 = i;
      NumMenu++;
      if(NumMenu>from && k < 2) {
        MenuPoiter[k] = Menu[i];      
        k++;
      }
    if(k == 1) MenuPoiter[1] = Menu[i0]; 
    }
  }                                 
  return NumMenu;
}

int LastMenu() {
  int i;
  for(i=0; i<ALL_MENU; i++){
    if(Menu[i].MenuID == MenuPoiter[0].SubMenu)
    return Menu[i].SubMenu;
  }
  return 0;
}

bool HasSubMenu() {
  char i;
  for(i=0; i<ALL_MENU; i++){
    if(STATE == Menu[i].SubMenu)
      return true;
  }
  return false;
}

void ProcessMenu(){
  if(PAGE > ContainMenu - 1) PAGE = 0;
  
  ContainMenu = SelectFrom(PAGE);
  LCD_Clear();  
  LCD_Gotoxy(0,0);
  LCD_Puts(">");
  LCD_Gotoxy(1,0);
  LCD_Puts(MenuPoiter[0].MenuName);
  
  if(ContainMenu > 1){
    LCD_Gotoxy(1,1);
    LCD_Puts(MenuPoiter[1].MenuName);
    
  }
}

void ProcessAction(uint8_t Key) {
  if(STATE == _BEGIN)    
    AccessCANCEL = false;  
  else  AccessCANCEL = true;                
    switch(Key) {
      case ENTER:
        if(!AccessENTER) break;       
            LEVEL[LevelPtr] = PAGE;    
        LevelPtr++;                     
        STATE = MenuPoiter[0].MenuID; 
        PAGE = 0;                       
        ContainMenu = SelectFrom(0); 
        if(ContainMenu) {
          ProcessMenu();          
        }
        else {
          AccessENTER = false;    
          switch(STATE){
            case _NHIET_DO: 
                setTemp(temp);
                while(num == 0){
                  key = keyScan();
                  switch(key){
                    case ENTER:
                      num++;
                      break;
                    case UP:
                      temp++;
                      setTemp(temp);
                      break;
                    case DOWN:
                      temp--;
                      setTemp(temp);
                      break;
                    case CANCEL:
                      num++;
                      break;
                }       
              }
              num = 0;
              break;
                
                case _DIEN_AP: 
                setVolt(volt);            
                while(num2 == 0){
                  key = keyScan();
                  switch(key){
                    case ENTER:
                      num2++;
                      break;
                    case UP:
                      volt++;
                      setVolt(volt);
                      break;
                    case DOWN:
                      volt--;
                      setVolt(volt);
                      break;
                    case CANCEL:
                      num2++;                     
                      break;
                  }       
                }
                num2 = 0;
                break;
                
                case _MODULE_1: 
									showModule(1);
									break;
                
                case _MODULE_2: 
									showModule(2);
									break;
                
                case _MODULE_3: 
									showModule(3);
									break;
                
                case _MODULE_4: 
									showModule(4);
									break;
          }
          ProcessAction(CANCEL);
        }
      break;
                     
      case UP:                   
        if(!AccessENTER) break;
        if(PAGE == 0) PAGE = ContainMenu;
        PAGE --;                
        ProcessMenu();
        break;
      case DOWN:                 
        if(!AccessENTER) break;
        PAGE ++;                
        ProcessMenu();
        break;
      case CANCEL:                 
        if(AccessCANCEL==false) {
          firstPage();
          break;
        }
        if(LevelPtr) LevelPtr--;   
        PAGE = LEVEL[LevelPtr];    
        if(!HasSubMenu()) {        
          AccessENTER = true;  
          STATE = MenuPoiter[0].SubMenu;
        } 
        else {                   
          STATE = LastMenu();   
        }
        ContainMenu = SelectFrom(0);
        ProcessMenu();          
        break;
      case HOME:
        ContainMenu = SelectFrom(0);
        ProcessMenu();
        break;  
    }
}

void firstPage() {
  LCD_Clear();
  LCD_Gotoxy(0,0);
  LCD_Puts("CONG TY ABC");
  LCD_Gotoxy(0,1);
  LCD_Puts("Temp:");
  LCD_Gotoxy(6,1);
  print(getTemp());
  LCD_Gotoxy(8,1);
  LCD_Puts("Volt:");
  LCD_Gotoxy(14,1);
  LCD_Puts("getVolt()");  
}

void setTemp(uint8_t temp) {
  LCD_Gotoxy(0,0);
  LCD_Puts(">Nhiet do: ");
  LCD_Gotoxy(12,0);
  print(temp);
  LCD_Gotoxy(1,1);
  LCD_Puts("Dien ap: ");
  LCD_Gotoxy(10,1);
  print(volt);
}

void setVolt(uint8_t volt) {
  LCD_Gotoxy(0,0);
  LCD_Puts("Nhiet do: ");
  LCD_Gotoxy(12,0);
  print(temp);
  LCD_Gotoxy(1,1);
  LCD_Puts(">Dien ap: ");
  LCD_Gotoxy(10,1);
  print(volt);
}

int keyScan(){
  int i, key;
  
  if(HAL_GPIO_ReadPin(GPIOB, CANCEL_Pin)==1) {
    HAL_Delay(50);
    if(HAL_GPIO_ReadPin(GPIOB, CANCEL_Pin)==1)
    return CANCEL;
  }
	if(HAL_GPIO_ReadPin(GPIOB, ENTER_Pin)==1) {
    HAL_Delay(50);
    if(HAL_GPIO_ReadPin(GPIOB, ENTER_Pin)==1)
    return ENTER;
  }
	if(HAL_GPIO_ReadPin(GPIOB, DOWN_Pin)==1) {
    HAL_Delay(50);
    if(HAL_GPIO_ReadPin(GPIOB, DOWN_Pin)==1)
    return DOWN;
  }
	if(HAL_GPIO_ReadPin(GPIOB, UP_Pin)==1) {
    HAL_Delay(50);
    if(HAL_GPIO_ReadPin(GPIOB, UP_Pin)==1)
    return UP;
  }
  
  return 0;
  HAL_Delay(50);
}

void infor(uint8_t k,uint8_t i) {
  LCD_Clear();
  LCD_Gotoxy(0,0);
  LCD_Puts(ampName[i]);
  LCD_Gotoxy(7,0);
  print(amp[k][i]);
  LCD_Gotoxy(0,1);
  LCD_Puts(ampName[i+1]);
  LCD_Gotoxy(7,1);
  print(amp[k][i+1]);
}





void showModule(uint8_t k) {
  int i = 0;
  infor(k, 0);
  while(key != CANCEL){
    key = keyScan();
    switch(key){
      case DOWN:
        if(i == 8) {
          i = 0;
          infor(k, i);
        }else {
          i+=2;
          infor(k, i);
        }
        break;
      case UP:
        if(i == 0){
          i = 8;
          infor(k, i);         
        }
        else {
          i-=2;
          infor(k, i);
        }
        break;
      case ENTER:
        break;
      case CANCEL:
        ProcessAction(CANCEL);
        break;
    }  
  }
}

uint8_t getTemp() {
	uint8_t temp_realTime = adc_value[0]*(330/4095);
	return temp_realTime;
}

char getVolt() {
	float volt_realTime = (float)adc_value[1]*(3.3/4095.0);
	sprintf(DATA, "%f", volt_realTime);
	return DATA[27];
}

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);


int main(void)
{
  HAL_Init();

  
  SystemClock_Config();

  
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_value, 2);
	
	LCD_Init();
	firstPage();
  
  while (1)
  {
   
  }
  
}


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|RS_Pin|EN_Pin|D4_Pin
                          |D5_Pin|D6_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : DOWN_Pin CANCEL_Pin UP_Pin ENTER_Pin */
  GPIO_InitStruct.Pin = DOWN_Pin|CANCEL_Pin|UP_Pin|ENTER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15
                           PB3 RS_Pin EN_Pin D4_Pin
                           D5_Pin D6_Pin D7_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|RS_Pin|EN_Pin|D4_Pin
                          |D5_Pin|D6_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
