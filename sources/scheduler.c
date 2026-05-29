/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 23:42:17 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/08 14:46:57 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	should_swap(t_heap *heap, int i, int p)
{
	if (heap->data[i].priority < heap->data[p].priority)
		return (1);
	if (heap->data[i].priority == heap->data[p].priority)
		return (heap->data[i].coder_id < heap->data[p].coder_id);
	return (0);
}

void	bubble_up(t_heap *heap, int i)
{
	int	p;

	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!should_swap(heap, i, p))
			break ;
		heap_swap(heap, i, p);
		i = p;
	}
}

int	highest_priority(t_heap *heap, int a, int b)
{
	if (heap->data[a].priority < heap->data[b].priority)
		return (a);
	if (heap->data[a].priority == heap->data[b].priority)
	{
		if (heap->data[a].coder_id < heap->data[b].coder_id)
			return (a);
	}
	return (b);
}

void	bubble_down(t_heap *heap, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size)
			smallest = highest_priority(heap, left, smallest);
		if (right < heap->size)
			smallest = highest_priority(heap, right, smallest);
		if (smallest == i)
			break ;
		heap_swap(heap, i, smallest);
		i = smallest;
	}
}
