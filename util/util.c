/**
 * @file util.c
 * @brief 伪链表数据队列工具
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2023-09-05
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2023-09-05	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#include "util.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"util"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/
/*============================ EXTERNAL IMPLEMENTATION =======================*/
void curve_data_queue_init(curve_data_queue_t *queue)
{
	rt_uint16_t index;
	
	for (index = 0; index < MAX_COUNT; index++)
	{
		queue->queue[index].next = (index + 1) % MAX_COUNT;
	}
	queue->head = queue->tail = queue->read_point = 0;  // 初始化队列为空
}
/*
		struct array_link_element
{
	rt_uint16_t data;		// 迪文屏曲线数据均为2B
	rt_uint8_t next;		// 曲线数据个数不可能超过128个；这与迪文数据帧最大长度有关
};

		struct curve_data_queue
{
	struct array_link_element  queue[MAX_COUNT];
	rt_uint16_t head;		// 队列头指针
	rt_uint16_t tail;		// 队列尾指针
	rt_uint16_t read_point;	// 队列读指针
};


   这里没有考虑到多线程的问题，当在添加数据的时候恰好又在读取数据，那么数据可能会丢失 ， 所以我们在更高层中添加了互斥量 mutex  
*/
void curve_data_queue_add_data(curve_data_queue_t *queue, rt_uint16_t data)
{
	queue->queue[queue->tail].data = data;
	queue->tail = queue->queue[queue->tail].next;
	if (queue->tail == queue->head)   //  队列已满，覆盖最早的数据 
	{
		queue->head = queue->queue[queue->head].next;
	}
}
// 很怪的问题，编译的问题是很难解释的
rt_uint16_t curve_data_queue_get_all_data(curve_data_queue_t *queue, rt_uint16_t *buff)
{
	rt_uint16_t index = 0;
	rt_uint16_t head;
	
	head = queue->head;
	// while (head != queue->tail)
	// {
	// 	buff[index++] = queue->queue[head].data;
	// 	head = queue->queue[head].next;
	// }

	for(head = queue->head; head != queue->tail; head = queue->queue[head].next)
	{
		buff[index++] = queue->queue[head].data;
	}
	
	queue->read_point = queue->tail;
	
	return index;
}

rt_uint16_t curve_data_queue_get_last_data(curve_data_queue_t *queue, rt_uint16_t *buff)
{
	rt_uint16_t index = 0;
	rt_uint16_t head;
	
	head = queue->read_point;
	while (head != queue->tail)
	{
		buff[index++] = queue->queue[head].data;
		head = queue->queue[head].next;
	}
	queue->read_point = queue->tail;
	
	return index;
}
