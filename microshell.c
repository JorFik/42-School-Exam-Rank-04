/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: JFikents <Jfikents@student.42Heilbronn.de> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 14:41:02 by JFikents          #+#    #+#             */
/*   Updated: 2024/08/22 11:50:36 by JFikents         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

#define WRONG_ARGS_CD "error: cd: bad arguments\n"
#define CD_ERROR "error: cd: cannot change directory to "
#define ERROR_FATAL "error: fatal\n"
#define ERROR_EXEC "error: cannot execute "

int	print_error(char *str)
{
	while (*str)
		write(2, str++, 1);
	return (1);
}

int	print_error_info(char *str1, char *str2)
{
	print_error(str1);
	print_error(str2);
	return (write(2, "\n", 1));
}

int	cd(char **argv, int delimeter)
{
	if (delimeter != 2)
		return (print_error(WRONG_ARGS_CD));
	if (chdir(argv[1]) == -1)
		return (print_error_info(CD_ERROR, argv[1]));
	return (0);
}

void	set_pipe(bool has_pipe, int *pipe_fd, int end)
{
	if (has_pipe
		&& (dup2(pipe_fd[end], end) == -1
			|| close(pipe_fd[0]) == -1
			|| close(pipe_fd[1]) == -1))
		exit(print_error(ERROR_FATAL));
}

int	exec(char **argv, int delimeter, char **envp)
{
	const bool	has_pipe = argv[delimeter]
		&& !strcmp(argv[delimeter], "|");
	int			pipe_fd[2];
	int			pid;
	int			status;

	if (!has_pipe && !strcmp(*argv, "cd"))
		return (cd(argv, delimeter));
	if (has_pipe && pipe(pipe_fd) == -1)
		exit(print_error(ERROR_FATAL));
	if ((pid = fork()) == -1)
		exit(print_error(ERROR_FATAL));
	if (!pid)
	{
		argv[delimeter] = NULL;
		set_pipe(has_pipe, pipe_fd, STDOUT_FILENO);
		if (!strcmp(*argv, "cd"))
			exit(cd(argv, delimeter));
		execve(*argv, argv, envp);
		exit(print_error_info(ERROR_EXEC, *argv));
	}
	set_pipe(has_pipe, pipe_fd, STDIN_FILENO);
	waitpid(pid, &status, 0);
	return (WIFEXITED(status) && WEXITSTATUS(status));
}

int	get_delimeter_index(char **argv)
{
	int	delimeter;

	delimeter = 0;
	while (argv[delimeter]
		&& strcmp(argv[delimeter], "|")
		&& strcmp(argv[delimeter], ";"))
		delimeter++;
	return (delimeter);
}

int	main(int argc, char **argv, char **envp)
{
	int			delimeter;
	static int	status = 0;

	delimeter = 0;
	while (argc > 1 && argv[delimeter])
	{
		argv += delimeter + 1;
		delimeter = get_delimeter_index(argv);
		if (delimeter)
			status = exec(argv, delimeter, envp);
	}
	return (status);
}
