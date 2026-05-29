/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 14:54:02 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/08 15:10:07 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_dongles(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->args.nb_coders)
	{
		if (sim->dongles[i].queue.data)
			free(sim->dongles[i].queue.data);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		i++;
	}
	free(sim->dongles);
}

void	cleanup_sim(t_sim *sim)
{
	int	i;

	if (sim->coders)
	{
		i = 0;
		while (i < sim->args.nb_coders)
		{
			pthread_mutex_destroy(&sim->coders[i].compile_mutex);
			i++;
		}
		free(sim->coders);
		sim->coders = NULL;
	}
	cleanup_dongles(sim);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
}
