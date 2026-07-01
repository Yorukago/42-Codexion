/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 21:32:39 by jzorreta          #+#    #+#             */
/*   Updated: 2026/07/01 15:17:59 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
 * ft_atoi_strict:
 *  - str: input string containing digits
 *  - out: output parameter; pointer to an int where the parsed value will
 *         be stored on success. Caller must provide a valid int*
 *  - returns 0 on success (and sets *out),
 *    -1 on error (and leaves *out unspecified)
 */
int	ft_atoi_strict(const char *str, int *out)
{
	int		i;
	long	n;

	i = 0;
	n = 0;
	if (!str || !str[0])
		return (-1);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		n = n * 10 + (str[i] - '0');
		if (n > INT_MAX)
			return (-1);
		i++;
	}
	*out = (int)n;
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
		return (fprintf(stderr,
				COLOR_ERROR "Error: wrong number of arguments\n" COLOR_RESET),
			-1);
	if (parse_positive(argv[1], &sim->args.nb_coders) != 0
		|| parse_positive(argv[2], &sim->args.time_to_burnout) != 0
		|| ft_atoi_strict(argv[3], &sim->args.time_to_compile) != 0
		|| ft_atoi_strict(argv[4], &sim->args.time_to_debug) != 0
		|| ft_atoi_strict(argv[5], &sim->args.time_to_refactor) != 0
		|| parse_positive(argv[6], &sim->args.nb_compiles_required) != 0
		|| ft_atoi_strict(argv[7], &sim->args.dongle_cooldown) != 0)
		return (fprintf(stderr,
				COLOR_ERROR "Error: invalid argument\n" COLOR_RESET), -1);
	if (strcmp(argv[8], "fifo") == 0)
		sim->args.scheduler = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		sim->args.scheduler = EDF;
	else
		return (fprintf(stderr,
				COLOR_ERROR "Error: scheduler must be 'fifo' or 'edf'\n"
				COLOR_RESET), -1);
	return (0);
}
