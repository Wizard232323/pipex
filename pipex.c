//
//
// EK MINI MIcro SHELLpex 
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void    ft_free(char **tab)
{
    int i;
    int j;

    i = 0;
    j = 0;
    while(tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}

size_t	ft_strlen(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i] != 0)
		i++;
	return (i);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	i;
	size_t	j;
	size_t	k;
	size_t	l;
	char	*s3;

	i = ft_strlen(s1);
	j = ft_strlen(s2);
	k = 0;
	l = 0;
	s3 = (char *)malloc((i + j) + 1);
	if (!s3)
		return (NULL);
	while (s1[k])
	{
		s3[k] = s1[k];
		k++;
	}
	while (s2[l])
	{
		s3[k + l] = s2[l];
		l++;
	}
	s3[k + l] = '\0';
	return (s3);
}



char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	size_t	i;
	size_t	j;
	char	*s2;

	i = 0;
	j = ft_strlen(s);
	if (start >= j)
		start = j;
	if (len >= j - start)
		len = j - start;
	s2 = (char *)malloc(len + 1);
	if (!s2)
		return (NULL);
	while (s[start] && i < len)
	{
		s2[i] = s[start];
		i++;
		start++;
	}
	s2[i] = '\0';
	return (s2);
}

int	nbword(char *s, char c)
{
	int	i;
	int	count;

	i = 1;
	count = 0;
	if (!*s)
		return (0);
	if (s[0] != c)
		count++;
	while (s[i])
	{
		if (s[i - 1] == c && s[i] != c)
			count++;
		i++;
	}
	return (count);
}

char	**ft_split(char const *s, char c)
{
	char	**rep;
	int		word;
	int		i;
	int		j;
	int		k;

    j = 0;
	i = 0;
	k = 0;
	word = nbword((char *)s, c);
	rep = (char **)malloc(sizeof (char *) * (word + 1));
	if (!rep)
		return (NULL);
	rep[word] = NULL;
	while (k < word)
	{
		while (s[i] == c && s[i] != 0)
			i++;
		j = i;
		while (s[i] != c && s[i] != 0)
			i++;
		rep[k] = ft_substr((char *)s, j, i - j);
		k++;
	}
	return (rep);
}

char *find_path(char **envp, char *to_find)
{
    int i; 
    int j;
    
    i = 0;
    while (envp[i])
    {
        j = 0;
        
        while (envp[i][j] == to_find[j])
        {
            j++;
            if (!to_find[j])
                return(&envp[i][j] + 5);
        }
        i++;
    }
    return (0);
}


char *get_valid_path(char *cmd, char **envp)
{

    char *path_line;
    char **array_env;
    char *tmp;
    char *path;
    int i;
    int j;

    i = 0;
    j = 0;
    if (cmd == NULL)
        return (NULL);
    path_line = find_path(envp, "PATH=");
    array_env = ft_split(path_line, ':');

    while(array_env[i])
    {
        tmp = ft_strjoin(array_env[i], "/");
        path = ft_strjoin(tmp, cmd);

        free(tmp);

        if (access(path, X_OK) == 0)
		{
            ft_free(array_env);
		    return (path);
		}
        free(path);
        i++;
    }
    ft_free(array_env);
    return (NULL);
}

void closepipe(int *pipefd)
{   
    close(pipefd[0]);
    close(pipefd[1]);
}

int cmd1(int *pipefd, char *cmd, char **envp)
{   
    char **tab;
    char *valid;

    tab = ft_split(cmd, ' ');
    valid = get_valid_path(tab[0], envp);
    if (valid == NULL)
    {
        write(2, "commande1 inconnue", 18);
        write(2, "\n", 1);
        ft_free(tab);
        exit(1);
    }
    dup2(pipefd[1], 1);
    closepipe(pipefd);
    execve(valid, tab, envp);
    free(valid);
    ft_free(tab);
    exit(1);
}

int cmd2(int *pipefd, char *cmd, char **envp)
{   
    char **tab;
    char *valid;

    tab = ft_split(cmd, ' ');
    valid = get_valid_path(tab[0], envp);
    if (valid == NULL)
    {
        write(2, "commande2 inconnue\n", 18);
        ft_free(tab);
        exit(1);
    }
    dup2(pipefd[0], 0);
    closepipe(pipefd);
    execve(valid, tab, envp);
    ft_free(tab);
    free(valid);
    exit(1);
}


int fail_fork(int *pipefd)
{
    perror("fail to fork");
    closepipe(pipefd);
    exit(1);
}

int main(int argc, char **argv, char **envp)
{
    int pipefd[2];
    int fd1;
    int fd2;
    pid_t pid_tab[2];

    if (argc != 5)
    {
        write(2, "bad number of argument\n" , 24);
        return(1);
    }
    pipe(pipefd);
    // pipefd[0] = output
    // pipefd[1] = input
    fd1 = open(argv[1], O_RDONLY);
    fd2 = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd1 < 0 || fd2 < 0)
    {
        write(2, "Files error\n", 12);
        closepipe(pipefd);
        return (1);
    }
    dup2(fd1, 0);
    dup2(fd2, 1);
    close(fd1);
    close(fd2);
    pid_tab[0] = fork();
    if (pid_tab[0] == 0)
        cmd1(pipefd, argv[2], envp);
    else if (pid_tab[0] < 0)
        fail_fork(pipefd);
    pid_tab[1] = fork();
    if (pid_tab[1] == 0)
        cmd2(pipefd, argv[3], envp);
    else if (pid_tab[1] < 0)
        fail_fork(pipefd);
    closepipe(pipefd);
	waitpid(pid_tab[0], NULL, 0);
	waitpid(pid_tab[1], NULL, 0);
    return 0;
}