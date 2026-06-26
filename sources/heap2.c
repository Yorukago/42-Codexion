/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap2.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 20:29:04 by jzorreta          #+#    #+#             */
/*   Updated: 2026/06/25 22:55:40 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Removes the element at idx by replacing it with the last element,
   then re-heapifies both up and down to restore order */
static void	heap_remove_at(t_heap *heap, int idx)
{
	if (idx < 0 || idx >= heap->size)
		return ;
	heap->data[idx] = heap->data[heap->size - 1];
	heap->size--;
	if (idx < heap->size)
	{
		bubble_up(heap, idx);
		bubble_down(heap, idx);
	}
}

/* Finds the first entry with the given coder_id and removes it
   Exits after the first match since each coder appears at most once */
void	heap_remove_by_coder_id(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->data[i].coder_id == coder_id)
		{
			heap_remove_at(heap, i);
			return ;
		}
		i++;
	}
}
