/*****************************************************************************
*
* File Name : ql_queue.h
*
* Description: cirular queue to receice uart data
*
* Copyright (c) 2016 http://www.qinglianyun.com
* All rights reserved.
*
* Author : ykgnaw
*
* Date : 2016-06-27
*****************************************************************************/

#ifndef _QL_QUEUE_H_
#define _QL_QUEUE_H_
#include "Arduino.h"

#define MAXSIZE 128

typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t length;
    uint8_t  data[MAXSIZE];
} queue_uart;
/*
* receive queue handler.
*/
extern queue_uart g_RxFrameQUEUE;
/*
*Function: initialize the receive queue.
*parameters:
*        queue - the receive queue
*return:
*/
void init_queue(queue_uart *queue);
/*
*Function: judge the receive queue is empty or not.
*parameters:
*        queue - the receive queue
*return:
*	(1) - the queue is empty.
*	(0) - the queue is not empty.
*/
int  is_empty(queue_uart *queue);
/*
*Function: judge the receive queue is full or not.
*parameters:
*        queue - the receive queue
*return:
*	(1) - the queue is full.
*	(0) - the queue is not full.
*/
int  is_full( queue_uart *queue );
/*
*Function: enqueue the data to the head of the queue.
*parameters:
*        queue - the receive queue.
*        value - the receive data.
*return:
*/
void en_queue(queue_uart *queue, uint8_t value);
/*
*Function: dequeue the data from the tail of the queue.
*parameters:
*        queue - the receive queue.
*        value - data at the tail of the queue.
*return:
*	(-1) - dequeue fail.
*	(0)  - dequeue success.
*/
int  de_queue(queue_uart *queue, uint8_t *value);
/*
*Function: clear the receive queue.
*parameters:
*        queue - the receive queue.
*return:
*/
void clear_queue(queue_uart *queue);
/*
*Function: get the count of data in receive queue.
*parameters:
*        queue - the receive queue.
*return:
*	the count of data in receive queue.
*/
int  get_queue_len(queue_uart *queue);
/*
*Function: get the head data of receive queue but not dequeue.
*parameters:
*        queue - the receive queue.
*        value - the head data of receive queue.
*return:
*	(-1) - get data fail.
*	(0)  - get data success.
*/
int  get_queue_head(queue_uart *queue, uint8_t *value);

#endif
