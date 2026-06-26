/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 21:48:15 by jzorreta          #+#    #+#             */
/*   Updated: 2026/06/25 22:55:53 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Allocates and initialises all nb_coders dongles */
int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->args.nb_coders);
	if (!sim->dongles)
		return (-1);
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->args.nb_coders);
	i = 0;
	while (i < sim->args.nb_coders)
	{
		sim->dongles[i].id = i;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		if (heap_init(&sim->dongles[i].queue, sim->args.nb_coders) != 0)
			return (-1);
		i++;
	}
	return (0);
}

/* Allocates coders and assigns each one a left dongle (i) and a right
   dongle ((i+1) % nb_coders), creating the circular sharing structure */
int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->args.nb_coders);
	if (!sim->coders)
		return (-1);
	memset(sim->coders, 0, sizeof(t_coder) * sim->args.nb_coders);
	i = 0;
	while (i < sim->args.nb_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].sim = sim;
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1)
			% sim->args.nb_coders];
		pthread_mutex_init(&sim->coders[i].compile_mutex, NULL);
		i++;
	}
	return (0);
}

/* Records the start time, initialises shared mutexes, then sets up
   dongles and coders */
int	init_sim(t_sim *sim)
{
	sim->start_time = get_time_ms();
	sim->stopped = 0;
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	if (init_dongles(sim) != 0)
		return (-1);
	if (init_coders(sim) != 0)
		return (-1);
	return (0);
}

/* Signals stop and joins the first count coder threads.
   Called when pthread_create fails mid-launch. */
static void	abort_threads(t_sim *sim, int count)
{
	set_stopped(sim);
	while (count-- > 0)
		pthread_join(sim->coders[count].thread, NULL);
}

/* Spawns all coder threads and the monitor thread, then joins them all */
int	launch_sim(t_sim *sim)
{
	pthread_t	monitor;
	int			i;

	i = 0;
	while (i < sim->args.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (abort_threads(sim, i), -1);
		i++;
	}
	if (pthread_create(&monitor, NULL, monitor_routine, sim) != 0)
		return (abort_threads(sim, sim->args.nb_coders), -1);
	i = 0;
	while (i < sim->args.nb_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor, NULL);
	return (0);
}
