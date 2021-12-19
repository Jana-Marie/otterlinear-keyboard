
#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"

#define  HID_MEDIA_REPORT  2
#define  HYSTERESIS 200

ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

PCD_HandleTypeDef hpcd_USB_FS;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_USB_PCD_Init(void);

void sendVolDown(void);
void sendVolUp(void);

struct key_t
{
    uint8_t id;
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} key;

uint16_t ADCreg[8];
uint16_t ADCval[8];
uint16_t ADClast[8];

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USB_HID_INIT();

  HAL_ADC_Start_DMA(&hadc, ADCreg, 8);

  while (1)
  {


    for(int i = 0; i < 8; i++){
      ADCval[i] = ADCreg[i];
      if(ADCval[i] - ADClast[i] > 2000 || ADClast[i] - ADCval[i] > 2000 ) ADClast[i] = ADCval[i];
    }

    // Volume Dial 8
    if(ADCval[0] >= ADClast[0] + HYSTERESIS){
      sendVolUp();
      ADClast[0] = ADCval[0];
    } else if(ADCval[0] <= ADClast[0] - HYSTERESIS){
      sendVolDown();
      ADClast[0] = ADCval[0];
    }

    // Dial 1
    if(ADCval[1] >= ADClast[1] + HYSTERESIS){
    } else if(ADCval[1] <= ADClast[1] - HYSTERESIS){
    }

    // SwitchLayer
    if(ADCval[2] >= ADClast[2] + HYSTERESIS){
      sendChar('v');
      ADClast[2] = ADCval[2];
    } else if(ADCval[2] <= ADClast[2] - HYSTERESIS){
      sendChar('v');
      ADClast[2] = ADCval[2];
    }

    // RotatePart Dial 3
    if(ADCval[3] >= ADClast[3] + HYSTERESIS){
      sendChar('R');
      ADClast[3] = ADCval[3];
    } else if(ADCval[3] <= ADClast[3] - HYSTERESIS){
      sendChar('r');
      ADClast[3] = ADCval[3];
    }

    // Grid Dial 4
    if(ADCval[4] >= ADClast[4] + HYSTERESIS){
      sendChar('n');
      ADClast[4] = ADCval[4];
    } else if(ADCval[4] <= ADClast[4] - HYSTERESIS){
      sendChar('N');
      ADClast[4] = ADCval[4];
    }

    // via
    if(ADCval[5] >= ADClast[5] + HYSTERESIS){
    } else if(ADCval[5] <= ADClast[5] - HYSTERESIS){
    }

    // Trackwidth Dial 6
    if(ADCval[6] >= ADClast[6] + HYSTERESIS){
      sendChar('w');
      ADClast[6] = ADCval[6];
    } else if(ADCval[6] <= ADClast[6] - HYSTERESIS){
      sendChar('W');
      ADClast[6] = ADCval[6];
    }

    // zoom
    if(ADCval[7] >= ADClast[7] + HYSTERESIS){
      sendCharWrong(0x3a);
      ADClast[7] = ADCval[7];
    } else if(ADCval[7] <= ADClast[7] - HYSTERESIS){
      sendCharWrong(0x3b);
      ADClast[7] = ADCval[7];
    }

    HAL_Delay(10);
  }
}


const uint8_t _asciimap[128] =
{
    0x00,             // NUL
    0x00,             // SOH
    0x00,             // STX
    0x00,             // ETX
    0x00,             // EOT
    0x00,             // ENQ
    0x00,             // ACK
    0x00,             // BEL
    0x2a,           // BS   Backspace
    0x2b,           // TAB  Tab
    0x28,           // LF   Enter
    0x00,             // VT
    0x00,             // FF
    0x00,             // CR
    0x00,             // SO
    0x00,             // SI
    0x00,             // DEL
    0x00,             // DC1
    0x00,             // DC2
    0x00,             // DC3
    0x00,             // DC4
    0x00,             // NAK
    0x00,             // SYN
    0x00,             // ETB
    0x00,             // CAN
    0x00,             // EM
    0x00,             // SUB
    0x00,             // ESC
    0x00,             // FS
    0x00,             // GS
    0x00,             // RS
    0x00,             // US

    0x2c,          //  ' '
    0x1e|0x80,    // !
    0x34|0x80,    // "
    0x20|0x80,    // #
    0x21|0x80,    // $
    0x22|0x80,    // %
    0x24|0x80,    // &
    0x34,          // '
    0x26|0x80,    // (
    0x27|0x80,    // )
    0x25|0x80,    // *
    0x2e|0x80,    // +
    0x36,          // ,
    0x2d,          // -
    0x37,          // .
    0x38,          // /
    0x27,          // 0
    0x1e,          // 1
    0x1f,          // 2
    0x20,          // 3
    0x21,          // 4
    0x22,          // 5
    0x23,          // 6
    0x24,          // 7
    0x25,          // 8
    0x26,          // 9
    0x33|0x80,      // :
    0x33,          // ;
    0x36|0x80,      // <
    0x2e,          // =
    0x37|0x80,      // >
    0x38|0x80,      // ?
    0x1f|0x80,      // @
    0x04|0x80,      // A
    0x05|0x80,      // B
    0x06|0x80,      // C
    0x07|0x80,      // D
    0x08|0x80,      // E
    0x09|0x80,      // F
    0x0a|0x80,      // G
    0x0b|0x80,      // H
    0x0c|0x80,      // I
    0x0d|0x80,      // J
    0x0e|0x80,      // K
    0x0f|0x80,      // L
    0x10|0x80,      // M
    0x11|0x80,      // N
    0x12|0x80,      // O
    0x13|0x80,      // P
    0x14|0x80,      // Q
    0x15|0x80,      // R
    0x16|0x80,      // S
    0x17|0x80,      // T
    0x18|0x80,      // U
    0x19|0x80,      // V
    0x1a|0x80,      // W
    0x1b|0x80,      // X
    0x1c|0x80,      // Y
    0x1d|0x80,      // Z
    0x2f,          // [
    0x31,          // bslash
    0x30,          // ]
    0x23|0x80,    // ^
    0x2d|0x80,    // _
    0x35,          // `
    0x04,          // a
    0x05,          // b
    0x06,          // c
    0x07,          // d
    0x08,          // e
    0x09,          // f
    0x0a,          // g
    0x0b,          // h
    0x0c,          // i
    0x0d,          // j
    0x0e,          // k
    0x0f,          // l
    0x10,          // m
    0x11,          // n
    0x12,          // o
    0x13,          // p
    0x14,          // q
    0x15,          // r
    0x16,          // s
    0x17,          // t
    0x18,          // u
    0x19,          // v
    0x1a,          // w
    0x1b,          // x
    0x1c,          // y
    0x1d,          // z
    0x2f|0x80,    // {
    0x31|0x80,    // |
    0x30|0x80,    // }
    0x35|0x80,    // ~
    0               // DEL
};


void sendChar(uint8_t ch){
    if( ch > 128 ) ch -=128;

    key.id = 1;
    key.keycode[0]=_asciimap[ch]&0x7F;
    key.keycode[1]=0;

    if ( _asciimap[ch] & 0x80) key.modifier |= 0x02;

    for(int i=0; i< 4;i++){
        USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&key, sizeof(key));
        HAL_Delay(10);
    }
    memset(key.keycode, 0 , sizeof(key.keycode));
    key.modifier = 0;
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&key, sizeof(key));
    HAL_Delay(10);
}

void sendCharWrong(uint8_t ch){
    key.id = 1;
    key.keycode[0]=ch&0x7F;
    key.keycode[1]=0;

    if ( _asciimap[ch] & 0x80) key.modifier |= 0x02;

    for(int i=0; i< 4;i++){
        USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&key, sizeof(key));
        HAL_Delay(10);
    }
    memset(key.keycode, 0 , sizeof(key.keycode));
    key.modifier = 0;
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&key, sizeof(key));
    HAL_Delay(10);
}

void sendVolUp(){
  uint8_t report[3];
  report[0]= HID_MEDIA_REPORT;
  report[1]= 0xE9;
  report[2]= 0x00;
  USBD_HID_SendReport(&hUsbDeviceFS, report, 3);
  HAL_Delay(10);

  report[0]= HID_MEDIA_REPORT;
  report[1]= 0x00;
  report[2]= 0x00;
  USBD_HID_SendReport(&hUsbDeviceFS, report, 3);
}

void sendVolDown(){
  uint8_t report[3];
  report[0]= HID_MEDIA_REPORT;
  report[1]= 0xEA;
  report[2]= 0x00;
  USBD_HID_SendReport(&hUsbDeviceFS, report, 3);
  HAL_Delay(10);

  report[0]= HID_MEDIA_REPORT;
  report[1]= 0x00;
  report[2]= 0x00;
  USBD_HID_SendReport(&hUsbDeviceFS, report, 3);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
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

static void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  HAL_ADC_Init(&hadc);

  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  sConfig.Channel = ADC_CHANNEL_1;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_2;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_3;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_4;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_5;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_6;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_7;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_8;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);
}

static void MX_USB_PCD_Init(void)
{

  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  HAL_PCD_Init(&hpcd_USB_FS);
}


static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  while(1){
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,1);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,0);
    HAL_Delay(100);
  }
}
