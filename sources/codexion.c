/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 20:34:23 by jzorreta          #+#    #+#             */
/*   Updated: 2026/06/25 22:53:33 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int ac, char **av)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(t_sim));
	if (parse_args(ac, av, &sim) != 0)
		return (1);
	if (init_sim(&sim) != 0)
		return (1);
	if (launch_sim(&sim) != 0)
		return (1);
	cleanup_sim(&sim);
	return (0);
}
