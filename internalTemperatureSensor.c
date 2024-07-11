#include "main.h"

#define TRIG_PIN GPIO_PIN_0
#define TRIG_PORT GPIOA
#define ECHO_PIN GPIO_PIN_1
#define ECHO_PORT GPIOA

#define I2C_ADDRESS 0x55 

TIM_HandleTypeDef htim1;
I2C_HandleTypeDef hi2c1;

void delay_us(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0); 
    while (__HAL_TIM_GET_COUNTER(&htim1) < us); 
}


void send_trigger_pulse() {
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET); 
    delay_us(10); 
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

uint32_t read_ultrasonic_distance() {
    uint32_t duration, distance;

    send_trigger_pulse();

    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET);

    uint32_t start_time = __HAL_TIM_GET_COUNTER(&htim1);
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET);
    duration = __HAL_TIM_GET_COUNTER(&htim1) - start_time;

    distance = (duration * 0.0343) / 2;

    return distance;
}

void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init(); 
    MX_I2C1_Init(); 

    HAL_TIM_Base_Start(&htim1); 

    while (1) {
        uint32_t distance = read_ultrasonic_distance();

        uint8_t data[2];
        data[0] = (distance >> 8) & 0xFF; 
        data[1] = distance & 0xFF;        

        HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS << 1, data, 2, HAL_MAX_DELAY);

        HAL_Delay(100);
    }
}
