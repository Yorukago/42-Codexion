/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jzorreta <jzorreta@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 19:42:52 by jzorreta          #+#    #+#             */
/*   Updated: 2026/07/01 14:09:38 by jzorreta         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# define COLOR_COMPILE	"\033[38;2;173;216;230m"
# define COLOR_DEBUG	"\033[38;2;255;105;180m"
# define COLOR_REFACTOR	"\033[38;2;189;147;249m"
# define COLOR_DONGLE	"\033[38;2;255;255;255m"
# define COLOR_BURNOUT	"\033[38;2;255;0;0m"
# define COLOR_ERROR	"\033[38;2;255;0;0m"
# define COLOR_RESET	"\033[0m"

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include <limits.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}					t_scheduler;

typedef struct s_args
{
	int				nb_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				nb_compiles_required;
	int				dongle_cooldown;
	t_scheduler		scheduler;
}					t_args;

typedef struct s_waiter
{
	int				coder_id;
	long long		priority;
}					t_waiter;

typedef struct s_heap
{
	t_waiter		*data;
	int				size;
	int				capacity;
}					t_heap;

typedef struct s_dongle
{
	int				id;
	int				is_taken;
	long long		released_at;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	t_heap			queue;
}					t_dongle;

typedef struct s_coder
{
	int				id;
	int				compile_count;
	long long		last_compile_start;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t		thread;
	pthread_mutex_t	compile_mutex;
	struct s_sim	*sim;
}					t_coder;

typedef struct s_sim
{
	t_args			args;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	int				stopped;
	long long		start_time;
}					t_sim;

/* cleanup.c */
void				cleanup_dongles(t_sim *sim);
void				cleanup_sim(t_sim *sim);

/* coder.c */
void				*coder_routine(void *arg);

/* dongle.c */
int					take_dongle(t_coder *coder, t_dongle *dongle);
void				release_dongle(t_dongle *dongle);

/* heap.c */
t_waiter			heap_pop(t_heap *heap);
int					heap_init(t_heap *heap, int capacity);
t_waiter			heap_peek(t_heap *heap);
void				heap_swap(t_heap *heap, int a, int b);
void				heap_push(t_heap *heap, int coder_id, long long priority);
void				heap_remove_by_coder_id(t_heap *heap, int coder_id);

/* init.c */
int					init_dongles(t_sim *sim);
int					init_coders(t_sim *sim);
int					init_sim(t_sim *sim);
int					launch_sim(t_sim *sim);

/* logs.c */
long long			get_time_ms(void);
void				log_status(t_coder *coder, const char *msg);
void				log_burnout(t_sim *sim, int coder_id);

/* monitor.c */
int					check_burnout(t_sim *sim, int i);
int					check_all_finished(t_sim *sim);
int					check_stop(t_sim *sim);
void				set_stopped(t_sim *sim);
void				*monitor_routine(void *arg);

/* parser.c */
int					parse_args(int argc, char **argv, t_sim *sim);
int					ft_atoi_strict(const char *str, int *out);

/* scheduler.c */
long long			get_priority(t_coder *coder);
int					should_swap(t_heap *heap, int i, int p);
void				bubble_up(t_heap *heap, int i);
int					highest_priority(t_heap *heap, int a, int b);
void				bubble_down(t_heap *heap, int i);

#endif
