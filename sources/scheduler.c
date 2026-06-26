/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 23:42:17 by jzorreta          #+#    #+#             */
/*   Updated: 2026/06/25 23:09:11 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Returns the scheduling priority for this coder
   FIFO: priority = now (first to arrive wins)
   EDF:  priority = deadline (last_compile_start + time_to_burnout) */
long long	get_priority(t_coder *coder)
{
	if (coder->sim->args.scheduler == FIFO)
		return (get_time_ms());
	return (coder->last_compile_start + coder->sim->args.time_to_burnout);
}

/* Returns 1 if element at index i should be swapped with parent p.
   Lower priority value = higher urgency. Tie-breaks by coder_id. */
int	should_swap(t_heap *heap, int i, int p)
{
	if (heap->data[i].priority < heap->data[p].priority)
		return (1);
	if (heap->data[i].priority == heap->data[p].priority)
		return (heap->data[i].coder_id < heap->data[p].coder_id);
	return (0);
}

/* Sifts element at index i up toward the root until heap order holds. */
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

/* Returns the index (a or b) whose element has higher priority.
   Uses the same ordering as should_swap. */
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

/* Sifts element at index i down until both children have lower priority. */
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
