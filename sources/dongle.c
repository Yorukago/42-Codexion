/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 23:32:25 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/08 14:07:36 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long long	get_priority(t_coder *coder)
{
	if (coder->sim->args.scheduler == FIFO)
		return (get_time_ms());
	return (coder->last_compile_start + coder->sim->args.time_to_burnout);
}

static int	can_take(t_coder *coder, t_dongle *dongle)
{
	t_waiter	top;
	long long	now;

	now = get_time_ms();
	top = heap_peek(&dongle->queue);
	if (top.coder_id != coder->id)
		return (0);
	if (dongle->is_taken)
		return (0);
	if (now - dongle->released_at < coder->sim->args.dongle_cooldown)
		return (0);
	return (1);
}

static void	wait_for_dongle(t_dongle *dongle)
{
	struct timespec	ts;
	struct timeval	tv;
	long			nsec;

	gettimeofday(&tv, NULL);
	nsec = tv.tv_usec * 1000L + 500000L;
	ts.tv_sec = tv.tv_sec + nsec / 1000000000L;
	ts.tv_nsec = nsec % 1000000000L;
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
}

int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	long long	priority;

	pthread_mutex_lock(&dongle->mutex);
	priority = get_priority(coder);
	heap_push(&dongle->queue, coder->id, priority);
	while (!can_take(coder, dongle))
	{
		if (check_stop(coder->sim))
		{
			heap_remove_by_coder_id(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return (-1);
		}
		if (coder->sim->args.scheduler == EDF)
		{
			priority = get_priority(coder);
			heap_remove_by_coder_id(&dongle->queue, coder->id);
			heap_push(&dongle->queue, coder->id, priority);
		}
		wait_for_dongle(dongle);
	}
	heap_pop(&dongle->queue);
	dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->mutex);
	log_status(coder, "has taken a dongle");
	return (0);
}

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_taken = 0;
	dongle->released_at = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
