/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 14:43:50 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/11 20:29:22 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_waiter	heap_pop(t_heap *heap)
{
	t_waiter	root;

	if (heap->size <= 0)
		return ((t_waiter){0, 0});
	root = heap->data[0];
	heap->data[0] = heap->data[heap->size - 1];
	heap->size--;
	bubble_down(heap, 0);
	return (root);
}

int	heap_init(t_heap *heap, int capacity)
{
	heap->data = malloc(sizeof(t_waiter) * capacity);
	if (!heap->data)
		return (-1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}

t_waiter	heap_peek(t_heap *heap)
{
	t_waiter	empty;

	if (heap->size <= 0)
	{
		empty.coder_id = -1;
		empty.priority = -1;
		return (empty);
	}
	return (heap->data[0]);
}

void	heap_push(t_heap *heap, int coder_id, long long priority)
{
	heap->data[heap->size].coder_id = coder_id;
	heap->data[heap->size].priority = priority;
	heap->size++;
	bubble_up(heap, heap->size - 1);
}

void	heap_swap(t_heap *heap, int a, int b)
{
	t_waiter	tmp;

	tmp = heap->data[a];
	heap->data[a] = heap->data[b];
	heap->data[b] = tmp;
}
