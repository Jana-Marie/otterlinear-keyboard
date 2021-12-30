#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"
#include "mapping.h"

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;
DMA_HandleTypeDef hdma_tim1_ch3_up;
PCD_HandleTypeDef hpcd_USB_FS;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
void check_bootloader(void);
void run_bootloader(void);
void append_key(uint8_t key);
void clear_key_list();

struct key_t{
    uint8_t id;
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} key;

struct keylist_t{
  uint8_t keycode[6];
  uint8_t numkeys;
  key_t usb_key
  //void (*append)(uint8_t key);
  //void (*clear)();
} keylist;// = {.append = append_key, .clear = clear_key_list};

uint16_t drive_map[12] = {
  0b0000000000100000,
  0b0000000000010000,
  0b0000000000001000,
  0b0000000000000100,
  0b0000000000000010,
  0b0000000000000001,
  0b1000000000000000,
  0b0000010000000000,
  0b0000001000000000,
  0b0000000100000000,
  0b0000000010000000,
  0b0000000001000000
};

uint16_t receive_map[12] = {};
uint16_t key_map[5] = {};

int main(void)
{
  HAL_Init();
  run_bootloader();
  SystemClock_Config();
  MX_GPIO_Init();
  //MX_DMA_Init();
  //MX_TIM1_Init();
  check_bootloader();
  MX_USB_HID_INIT();

  while (1)
  {
    for(uint8_t i = 0; i < 12; i++){
      GPIOA->ODR = drive_map[i];
      HAL_Delay(1);
      receive_map[i] = GPIOB->IDR >> 4;
      for(int k = 0; k < 5; k++){ // map rotated by 90deg
        key_map[k] = (key_map[k] & ~(1ul << i)) | (((receive_map[i] >> k) & 0x01) << i);
      }
      HAL_Delay(2);
    }

    key.id = 1;
    key.modifier = 0;

    if(key_map[2] == 1) key.modifier |= 0x01;
    if(key_map[1] == 1) key.modifier |= 0x02;
    if(key_map[0] == 1) key.modifier |= 0x01;
    if(key_map[0] >> 3 == 1) key.modifier |= 0x04;
    if(key_map[0] >> 7 == 1) key.modifier |= 0x04;

    uint8_t keycode_cnt = 0;
    for(uint8_t i = 0; i <= 4; i++){
      for(uint8_t k = 0; k <= 12; k++){
        if(((key_map[i] >> k) & 0x01) == 1){
          if(layer_0[i][k] != 0x00) {
            key.keycode[keycode_cnt++] = layer_0[i][k];
          }
        }
      }
    }

    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&key, sizeof(key));
    keycode_cnt = 0;
    memset(&key, 0, sizeof key);
  }
}

// ####### Keyhandling #######
//
void append_key(uint8_t key){
  keylist.keycode[keylist.numkeys] = key;
};

void clear_key_list(){

};

// ####### Bootloader #######

void run_bootloader(void){
  if ( *((unsigned long *)0x200017FC) == 0xDEADBEEF ) {
    void (*SysMemBootJump)(void);
    volatile uint32_t addr = 0x1FFFC400;

    HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError = 0;;
    HAL_FLASH_Unlock();
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = 0x08000000; // start address
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
    {
    	SECTORError = SECTORError + 1;
    }

    SYSCFG->CFGR1 = 0x01;
    SysMemBootJump = (void (*)(void)) (*((uint32_t *)(addr + 4)));
    __set_MSP(*(uint32_t *)addr);

    SysMemBootJump();

    while (1);
  }
}

void check_bootloader(void){
  for(uint8_t i = 0; i < 12; i++){
    GPIOA->ODR = drive_map[i];
    HAL_Delay(1);
    receive_map[i] = GPIOB->IDR >> 4;
    HAL_Delay(1);
  }
  if(receive_map[4] == 3 && receive_map[5] == 3 && receive_map[6] == 3){
    *((unsigned long *)0x200017FC) = 0xDEADBEEF;
    NVIC_SystemReset();
  }
}

// ####### Init #######

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim1);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
  HAL_TIM_OC_Init(&htim1);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
}


static void MX_DMA_Init(void)
{

  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, C6_Pin|C5_Pin|C4_Pin|C3_Pin
                          |C2_Pin|C1_Pin|C12_Pin|C11_Pin
                          |C10_Pin|C9_Pin|C8_Pin|C7_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = C6_Pin|C5_Pin|C4_Pin|C3_Pin
                          |C2_Pin|C1_Pin|C12_Pin|C11_Pin
                          |C10_Pin|C9_Pin|C8_Pin|C7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = R5_Pin|R4_Pin|R3_Pin|R2_Pin
                          |R1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void Error_Handler(void){
  while(1){

  }
}
