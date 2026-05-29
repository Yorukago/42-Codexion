/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 21:32:39 by jzorreta          #+#    #+#             */
/*   Updated: 2026/05/08 14:36:25 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	ft_atoi_strict(const char *str, int *out)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (-1);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		i++;
	}
	*out = atoi(str);
	if (*out < 0)
		return (-1);
	return (0);
}

static int	parse_positive(const char *str, int *out)
{
	if (ft_atoi_strict(str, out) != 0 || *out <= 0)
		return (-1);
	return (0);
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	if (argc != 9)
		return (fprintf(stderr, "Error: wrong number of arguments\n"), -1);
	if (parse_positive(argv[1], &sim->args.nb_coders) != 0
		|| parse_positive(argv[2], &sim->args.time_to_burnout) != 0
		|| ft_atoi_strict(argv[3], &sim->args.time_to_compile) != 0
		|| ft_atoi_strict(argv[4], &sim->args.time_to_debug) != 0
		|| ft_atoi_strict(argv[5], &sim->args.time_to_refactor) != 0
		|| parse_positive(argv[6], &sim->args.nb_compiles_required) != 0
		|| ft_atoi_strict(argv[7], &sim->args.dongle_cooldown) != 0)
		return (fprintf(stderr, "Error: invalid argument\n"), -1);
	if (strcmp(argv[8], "fifo") == 0)
		sim->args.scheduler = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		sim->args.scheduler = EDF;
	else
		return (fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'\n"),
			-1);
	return (0);
}
