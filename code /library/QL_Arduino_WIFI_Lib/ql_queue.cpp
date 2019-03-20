/*****************************************************************************
*
* File Name : ql_queue.cpp
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
#include <string.h>
#include "ql_queue.h"

queue_uart g_RxFrameQUEUE;

void init_queue(queue_uart *queue)
{
    queue -> head = 0;
    queue -> tail = 0;
    queue -> length = 0;
    memset(&queue -> data, 0, MAXSIZE);
}
int is_empty(queue_uart *queue)
{
    return (queue->head == queue->tail);
}
int is_full( queue_uart *queue )
{
    return  ( queue->head == queue->tail + 1) || (queue->tail - queue->head == MAXSIZE - 1 );
}

int get_queue_len(queue_uart *queue)
{
    return queue->length;
}

void en_queue(queue_uart *queue, uint8_t value)
{
    if (is_full(queue))  return;
    else
    {
        queue->data[queue->tail++] = value;
        if(queue->tail == MAXSIZE)
            queue->tail = 0;
        queue->length++;
    }
}
int de_queue(queue_uart *queue, uint8_t *value)
{
    if (is_empty(queue))  return -1;
    *value = queue->data[queue->head++];
    if(queue->head == MAXSIZE)
        queue->head = 0;
    queue->length--;
    return 0;
}
void clear_queue(queue_uart *queue)
{
    queue -> head = 0;
    queue -> tail = 0;
    queue -> length = 0;
}
int get_queue_head(queue_uart *queue, uint8_t *value)
{
    if (is_empty(queue))  return -1;
    *value = queue->data[queue->head];
    return 0;
}
int de_queue_n(queue_uart *queue, int n)
{
    int i = 0;
    if (get_queue_len(queue) < n)  return -1;
    for(i = 0; i < n; i++)
    {
        queue->head++;
        if(queue->head == MAXSIZE)
            queue->head = 0;
        queue->length--;
    }
    return 0;
}