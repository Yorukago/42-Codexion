/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 23:11:03 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/13 21:31:53 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	acquire_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle == coder->right_dongle)
		return (0);
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (take_dongle(coder, first) != 0)
		return (0);
	if (take_dongle(coder, second) != 0)
	{
		release_dongle(first);
		return (0);
	}
	return (1);
}

static void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->compile_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->compile_mutex);
	log_status(coder, "is compiling");
	usleep(coder->sim->args.time_to_compile * 1000);
	release_dongle(coder->left_dongle);
	release_dongle(coder->right_dongle);
}

static void	do_debug_refactor(t_coder *coder)
{
	log_status(coder, "is debugging");
	usleep(coder->sim->args.time_to_debug * 1000);
	log_status(coder, "is refactoring");
	usleep(coder->sim->args.time_to_refactor * 1000);
	pthread_mutex_lock(&coder->compile_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->compile_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!check_stop(coder->sim))
	{
		if (!acquire_dongles(coder))
		{
			usleep(500);
			continue ;
		}
		if (check_stop(coder->sim))
			break ;
		do_compile(coder);
		do_debug_refactor(coder);
	}
	return (NULL);
}
