#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <os_queue.h>
#include <os_task.h>
#include <FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#define Light_Sensor 	6U
/* External Functions */
//extern void adcStartConversion_selChn(adcBASE_t *adc, unsigned channel, unsigned fifo_size, unsigned group);
//extern void adcGetSingleData(adcBASE_t *adc, unsigned group, adcData_t *data);
static TaskHandle_t lightServiceTaskHandle = NULL;
QueueHandle_t xLightQueue = NULL;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void *pvParameters);

uint32 Get_Light_Sensor_data(void) {
    adcData_t adc_data;
    adcData_t *adc_data_ptr = &adc_data;

    /** - Start Group1 ADC Conversion
     *     Select Channel 6 - Light Sensor for Conversion
     */
    adcStartConversion_selChn(adcREG1, Light_Sensor, 1, adcGROUP1);

    /** - Wait for ADC Group1 conversion to complete */
    while (!adcIsConversionComplete(adcREG1, adcGROUP1));

    /** - Read the conversion result
     *     The data contains the Light sensor data
     */
    adcGetSingleData(adcREG1, adcGROUP1, adc_data_ptr);

    /** - Transmit the Conversion data to PC using SCI
     */
    return (adc_data_ptr->value);
}

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    BaseType_t xReturned = pfFAIL;
    xReturned = xTaskCreate(lightServiceTask,
                            LIGHT_SERVICE_NAME,
                            LIGHT_SERVICE_STACK_SIZE,
                            NULL,
                            LIGHT_SERVICE_PRIORITY,
                            &lightServiceTaskHandle);
    xLightQueue = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH,
                                             LIGHT_SERVICE_QUEUE_SIZE);
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void *pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);
    light_event_t queueTask = NULL;
    while (xQueueReceive(xLightQueue, *queueTask, 0) == pdTRUE) {
        if (queueTask == MEASURE_LIGHT)
        {
            uint32 Light_Sensor_Data = Get_Light_Sensor_data();
            int length = snprintf(NULL, 0, "&#37;u", Light_Sensor_Data);
            char *data = malloc(length + 1);
            snprintf(data, "&#37;u", Light_Sensor_Data);
            sciPrintText(scilinREG, data, length + 1);
            free(data);
        }
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    xQueueSend(xLightQueue, event, 0);
    /* USER CODE END */
    return 0;
}