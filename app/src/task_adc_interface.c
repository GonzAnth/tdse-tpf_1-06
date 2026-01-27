
/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_adc_attribute.h"


/********************** macros and definitions *******************************/
#define EVENT_UNDEFINED	(255)
#define MAX_EVENTS		(16)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
struct
{
	uint32_t	head;
	uint32_t	tail;
	uint32_t	count;
	task_adc_ev_t	queue[MAX_EVENTS];
} queue_task_adc;

/********************** external data declaration ****************************/

/********************** external functions definition ************************/
void init_queue_event_task_adc(void)
{
	uint32_t i;

	queue_task_adc.head = 0;
	queue_task_adc.tail = 0;
	queue_task_adc.count = 0;

	for (i = 0; i < MAX_EVENTS; i++)
		queue_task_adc.queue[i] = EVENT_UNDEFINED;
}

void put_event_task_adc(task_adc_ev_t event)
{
	queue_task_adc.count++;
	queue_task_adc.queue[queue_task_adc.head++] = event;

	if (MAX_EVENTS == queue_task_adc.head)
		queue_task_adc.head = 0;
}

task_adc_ev_t get_event_task_adc(void)
{
	task_adc_ev_t event;

	queue_task_adc.count--;
	event = queue_task_adc.queue[queue_task_adc.tail];
	queue_task_adc.queue[queue_task_adc.tail++] = EVENT_UNDEFINED;

	if (MAX_EVENTS == queue_task_adc.tail)
		queue_task_adc.tail = 0;

	return event;
}

bool any_event_task_adc(void)
{
  return (queue_task_adc.head != queue_task_adc.tail);
}

void get_values_task_adc(float *temp, float *bat)
{
    *temp = task_adc_dta.temp_cent;
    *bat  = task_adc_dta.bat_volts;
}

/********************** end of file ******************************************/
