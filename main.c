/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <inttypes.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"

static void prvQueueSendTask1( void * pvParameters );
static void prvQueueSendTask2( void * pvParameters );
static void prvQueueReceiveTask( void * pvParameters );

/* The two sender tasks may each enqueue their item before the receiver
 * retrieves one, so two slots are needed to ensure the queue has space to enqueue. */
#define mainQUEUE_LENGTH ( 2 )
#define mainQUEUE_ITEM_SIZE ( sizeof( uint32_t ) )
static uint8_t ucQueueStorageBuffer[ mainQUEUE_LENGTH * mainQUEUE_ITEM_SIZE ];
static StaticQueue_t xQueueBuffer;
static QueueHandle_t xQueue;

#define mainTASK_STACK_SIZE ( configMINIMAL_STACK_SIZE )

static StackType_t uxStackBuffer[ mainTASK_STACK_SIZE ];
static StaticTask_t xTaskBuffer;

/* The receive task has lower priority than the send task, but it will be
 * allowed to run when the send tasks are blocked in xTaskDelayUntil(). */
#define mainQUEUE_RECEIVE_TASK_PRIORITY tskIDLE_PRIORITY+1
#define mainQUEUE_SEND_TASK_PRIORITY tskIDLE_PRIORITY+2

/* Since the FreeRTOS kernel will usually be on single processor,
 * it should be safe to access `uTickCount` across different tasks,
 * just make it volatile to prevent the compiler from caching it.
 * Yes, we could just use xTaskGetTickCount() but that would be too easy. */
static volatile uint32_t uTickCount;

/*-----------------------------------------------------------*/

int main( void )
{
    BaseType_t xResult;

    console_init();

    xQueue = xQueueCreateStatic( mainQUEUE_LENGTH, mainQUEUE_ITEM_SIZE, ucQueueStorageBuffer, &xQueueBuffer );
    configASSERT( xQueue != NULL );

    /* Statically allocate one of the tasks to show that it can be done. */
    xTaskCreateStatic( prvQueueReceiveTask, "Rx", mainTASK_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, uxStackBuffer, &xTaskBuffer );

    xResult = xTaskCreate( prvQueueSendTask1, "Tx", mainTASK_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
    configASSERT( xResult == pdPASS );

    xResult = xTaskCreate( prvQueueSendTask2, "Tx2", mainTASK_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
    configASSERT( xResult == pdPASS );

    vTaskStartScheduler();
    for ( ;; );
    return 0;
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask1( void * pvParameters )
{
    TickType_t xLastWakeTime;
    BaseType_t xResult;
    uint32_t ulCounter = 1;
    TickType_t xPeriod = pdMS_TO_TICKS( 100 );
    ( void ) pvParameters;

    xLastWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        xResult = xQueueSend( xQueue, &ulCounter, 0 );
        configASSERT( xResult == pdPASS );

        xResult = xTaskDelayUntil( &xLastWakeTime, xPeriod );
        configASSERT( xResult == pdTRUE );

        ulCounter += ( uint32_t ) 2;
    }
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask2( void * pvParameters )
{
    TickType_t xLastWakeTime;
    BaseType_t xResult;
    uint32_t ulOut = 2;
    TickType_t xPeriod = pdMS_TO_TICKS( 300 );
    ( void ) pvParameters;

    vTaskDelay( xPeriod );

    xLastWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        xResult = xQueueSend( xQueue, &ulOut, 0 );
        configASSERT( xResult == pdPASS );

        xResult = xTaskDelayUntil( &xLastWakeTime, xPeriod );
        configASSERT( xResult == pdTRUE );
    }
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void * pvParameters )
{
    uint32_t ulReceivedValue;
    ( void ) pvParameters;

    for( ; ; )
    {
        /* in FreeRTOS, portMAX_DELAY indicates an indefinite wait,
         * whereas in SAFERTOS, it indicates the actual finite amount
         * of time specified by that contstant. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        configASSERT( xTaskGetTickCount() == uTickCount );
        /* I'm not sure yet if C99 stuff is allowed in FreeRTOS*/
        console_print( "At tick %"PRIu32" received %"PRIu32"\n", uTickCount, ulReceivedValue );
        /*
        Alternatively:
        console_print( "At tick %ul received %ul\n", uTickCount, ulReceivedValue );
        */
    }
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    uTickCount++;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* FreeRTOS documentation says not to block in idle hook
     * but this blocking is done in the Linux Kernel instead of
     * the embedded FreeRTOS kernel so it's "ok". */
    usleep( 15000 );
}
/*-----------------------------------------------------------*/


void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    vAssertCalled( __FILE__, __LINE__ );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * const pcFileName,
                    unsigned long ulLine )
{
    ( void ) ulLine;
    ( void ) pcFileName;
    console_print( "assertion failed at %s:%d\n", pcFileName, ulLine );
    exit( 1 );
}
/*-----------------------------------------------------------*/
