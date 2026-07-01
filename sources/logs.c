/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logs.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 21:50:01 by jzorreta          #+#    #+#             */
/*   Updated: 2026/07/01 13:59:15 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Returns the current time in milliseconds */
long long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/* Picks the ANSI color matching a status message */
static const char	*color_for(const char *msg)
{
	if (strcmp(msg, "is compiling") == 0)
		return (COLOR_COMPILE);
	if (strcmp(msg, "is debugging") == 0)
		return (COLOR_DEBUG);
	if (strcmp(msg, "is refactoring") == 0)
		return (COLOR_REFACTOR);
	if (strcmp(msg, "has taken a dongle") == 0)
		return (COLOR_DONGLE);
	return ("");
}

/* Prints "timestamp coder_id msg" to stdout, serialised by log_mutex
   Skips if the simulation has already stopped */
void	log_status(t_coder *coder, const char *msg)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->sim->log_mutex);
	if (!check_stop(coder->sim))
	{
		timestamp = get_time_ms() - coder->sim->start_time;
		printf("%s%lld %d %s" COLOR_RESET "\n",
			color_for(msg), timestamp, coder->id, msg);
	}
	pthread_mutex_unlock(&coder->sim->log_mutex);
}

/* Prints the burnout line for a coder, always printed (no stopped check)
   so the event is never silently swallowed. */
void	log_burnout(t_sim *sim, int coder_id)
{
	long long	timestamp;

	pthread_mutex_lock(&sim->log_mutex);
	timestamp = get_time_ms() - sim->start_time;
	printf(COLOR_BURNOUT "%lld %d burned out" COLOR_RESET "\n",
		timestamp, coder_id);
	pthread_mutex_unlock(&sim->log_mutex);
}
