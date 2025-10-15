#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// функция при завершении процесса
void on_exit_h(void) {
    printf("Процесс с PID=%d завершает работу (через atexit)\n", getpid());
}

// обработчик сигнала SIGINT 
void sigint_h(int signum) {
    printf("\nПроцесс %d получил сигнал SIGINT (%d)\n", getpid(), signum);
}

//  обработчик сигнала SIGTERM
void sigterm_h(int signum, siginfo_t *info, void *context) {
    (void)context; 
    printf("\nПроцесс %d получил сигнал SIGTERM (%d) от PID=%d\n",
           getpid(), signum, info->si_pid);
}

int main(void) {
    pid_t pid, wpid;
    int status;

    if (atexit(on_exit_h) != 0) {
        perror("Ошибка при регистрации atexit");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_h) == SIG_ERR) {
        perror("Ошибка при установке обработчика SIGINT");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_sigaction = sigterm_h;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Ошибка при установке обработчика SIGTERM");
        exit(EXIT_FAILURE);
    }

    printf("Главный процесс: PID=%d, PPID=%d\n", getpid(), getppid());

    //создание дочернего процесса
    pid = fork();

    if (pid < 0) {
        perror("Ошибка fork()");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        //дочерний процесс
        printf("Дочерний процесс: PID=%d, PPID=%d\n", getpid(), getppid());
        sleep(3);  // ждём немного, чтобы родитель мог послать сигнал
        printf("Дочерний процесс завершает работу со статусом 5\n");
        exit(5);
    } 
    else {
        // родительский процесс
        printf("Родительский процесс: PID=%d создал дочерний PID=%d\n", getpid(), pid);

        wpid = waitpid(pid, &status, 0);
        if (wpid == -1) {
            perror("Ошибка waitpid()");
            exit(EXIT_FAILURE);
        }

        // Проверка кода завершения
        if (WIFEXITED(status)) {
            printf("Дочерний процесс %d завершился с кодом %d\n",
                   wpid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс %d завершился сигналом %d\n",
                   wpid, WTERMSIG(status));
        }
    }

    printf("Главный процесс завершается.\n");
    return 0;
}
