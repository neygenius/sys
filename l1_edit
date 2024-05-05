#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <unordered_set>
#include "check.hpp"

// Глобальные переменные для хранения последнего полученного сигнала,
// последнего предсказанного значения и PID партнера по игре
volatile sig_atomic_t last_sig;
volatile sig_atomic_t last_predicted_value;
volatile sig_atomic_t partner_pid;

// Функция, вызываемая при завершении программы
// Отправляет сигнал SIGCHLD партнеру по игре
void at_exit() {
    check(kill(partner_pid, SIGCHLD));
}

// Функция для генерации и отправки предсказания партнеру
void predict(std::unordered_set<int>& buf) {
    bool flag;
    int answer;
    do {
        // Генерируем случайное число от 0 до 10
        answer = rand() % 11;
        flag = false;
        // Проверяем, было ли это число уже предсказано
        if (buf.find(answer) != buf.end())
            flag = true;
    } while (flag);
    buf.insert(answer);
    std::cout << "Maybe it`s " << answer << "?" << std::endl;
    // Отправляем предсказанное значение партнеру с помощью сигнала SIGRTMIN
    check(sigqueue(partner_pid, SIGRTMIN, sigval_t { answer }));
}

// Обработчик сигналов
void signal_handler(int signo, siginfo_t* si, void* ctx) {
    last_sig = signo;
    if (si->si_code == SI_QUEUE) {
        // Если сигнал был отправлен с помощью sigqueue(), сохраняем переданное значение
        last_predicted_value = si->si_value.sival_int;
    }
}

int main() {
    int max_games;
    std::cin >> max_games;
    int games = 0, rounds = 0, answer;
    std::unordered_set<int> buf {};
    sigset_t set, suspend_set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // Создаем маску сигналов для приостановки процесса
    sigfillset(&suspend_set);
    sigdelset(&suspend_set, SIGRTMIN);
    sigdelset(&suspend_set, SIGUSR1);
    sigdelset(&suspend_set, SIGUSR2);
    sigdelset(&suspend_set, SIGCHLD);
    struct sigaction handler_action {};
    handler_action.sa_sigaction = signal_handler;
    handler_action.sa_flags = SA_SIGINFO;
    handler_action.sa_mask = set;

    // Регистрируем обработчики сигналов
    check(sigaction(SIGRTMIN, &handler_action, NULL));
    check(sigaction(SIGUSR1, &handler_action, NULL));
    check(sigaction(SIGUSR2, &handler_action, NULL));
    check(sigaction(SIGCHLD, &handler_action, NULL));

    atexit(at_exit);
    srand(time(NULL));
    partner_pid = getpid();
    pid_t pid = check(fork());
    if (pid > 0) {
        // Родительский процесс
        partner_pid = pid;
        answer = rand() % 10;
        std::cout << "Guessed: " << answer << std::endl;
        check(kill(partner_pid, SIGUSR2));
    }
    while (true) {
        if (last_sig == SIGRTMIN) {
            // Обработка сигнала SIGRTMIN (предсказание партнера)
            if (last_predicted_value == answer) {
                std::cout << "Correct answer" << std::endl;
                std::cout << "Game lasts " << ++rounds << " rounds\n\n" << std::endl;
                check(kill(partner_pid, SIGUSR1));
                if (++games >= max_games) {
                    exit(EXIT_SUCCESS);
                }
                buf = {};
            }
            else {
                std::cout << "Wrong answer" << std::endl;
                ++rounds;
                check(kill(partner_pid, SIGUSR2));
            }
        }
        else if (last_sig == SIGUSR1) {
            // Обработка сигнала SIGUSR1 (начало новой игры)
            rounds = 0;
            ++games;
            if (games < max_games) {
                answer = rand() % 10;
                std::cout << "Guessed: " << answer << std::endl;
                check(kill(partner_pid, SIGUSR2));
            }
        }
        else if (last_sig == SIGUSR2) {
            // Обработка сигнала SIGUSR2 (запрос на предсказание)
            predict(buf);
        }
        sigsuspend(&suspend_set);
        if (last_sig == SIGCHLD) {
            // Обработка сигнала SIGCHLD (завершение дочернего процесса)
            if (pid==0 && getppid() != partner_pid)
                exit(EXIT_FAILURE);
        }
    }
}

'''
не забудь это поменять 
void signal_handler(int signo, siginfo_t* si, void* ctx) {
    last_sig = signo;
    if (si->si_code == SI_QUEUE) {
        // Если сигнал был отправлен с помощью sigqueue(), сохраняем переданное значение
        last_predicted_value = si->si_value.sival_int;
    }
}
'''
