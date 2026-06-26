/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 23:22:24 by jzorreta          #+#    #+#             */
/*   Updated: 2026/06/25 21:48:12 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Returns 1 if coder i has not compiled within time_to_burnout ms.
   Logs the burnout and sets the global stop flag before returning. */
int	check_burnout(t_sim *sim, int i)
{
	long long	last;

	pthread_mutex_lock(&sim->coders[i].compile_mutex);
	last = sim->coders[i].last_compile_start;
	pthread_mutex_unlock(&sim->coders[i].compile_mutex);
	if (get_time_ms() - last >= sim->args.time_to_burnout)
	{
		set_stopped(sim);
		log_burnout(sim, sim->coders[i].id);
		return (1);
	}
	return (0);
}

/* Returns 1 if every coder has reached nb_compiles_required. */
int	check_all_finished(t_sim *sim)
{
	int	i;
	int	count;

	i = 0;
	while (i < sim->args.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].compile_mutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].compile_mutex);
		if (count < sim->args.nb_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

/* Thread-safe read of the stopped flag. */
int	check_stop(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

/* Sets the stopped flag and broadcasts on every dongle's cond so that
   threads blocked in wait_for_dongle wake up and exit cleanly. */
void	set_stopped(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stopped = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->args.nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

/* Monitor thread: polls for burnouts and completion every 0.5 ms. */
void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (!check_stop(sim))
	{
		i = 0;
		while (i < sim->args.nb_coders)
		{
			if (check_burnout(sim, i))
				return (NULL);
			i++;
		}
		if (check_all_finished(sim))
		{
			set_stopped(sim);
			return (NULL);
		}
		usleep(500);
	}
	return (NULL);
}
