/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logs.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 21:50:01 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/07 22:16:51 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	log_status(t_coder *coder, const char *msg)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->sim->stop_mutex);
	if (coder->sim->stopped)
	{
		pthread_mutex_unlock(&coder->sim->stop_mutex);
		return ;
	}
	pthread_mutex_unlock(&coder->sim->stop_mutex);
	pthread_mutex_lock(&coder->sim->log_mutex);
	timestamp = get_time_ms() - coder->sim->start_time;
	printf("%lld %d %s\n", timestamp, coder->id, msg);
	pthread_mutex_unlock(&coder->sim->log_mutex);
}

void	log_burnout(t_sim *sim, int coder_id)
{
	long long	timestamp;

	pthread_mutex_lock(&sim->log_mutex);
	timestamp = get_time_ms() - sim->start_time;
	printf("%lld %d burned out\n", timestamp, coder_id);
	pthread_mutex_unlock(&sim->log_mutex);
}
