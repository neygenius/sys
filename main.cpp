#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <unordered_set>
#include <sys/prctl.h>
#include "check.hpp"

volatile sig_atomic_t last_sig;
volatile sig_atomic_t last_predicted_value;
volatile sig_atomic_t partner_pid;

void at_exit() {
    check(kill(partner_pid, SIGCHLD));
}

void predict(std::unordered_set<int>& buf) {
    bool flag;
    int answer;
    do {
        answer = rand() % 11;
        flag = false;
        if (buf.find(answer) != buf.end())
            flag = true;
    } while (flag);
    buf.insert(answer);
    std::cout << "Maybe it`s " << answer << "?" << std::endl;
    sleep(1);
    check(sigqueue(partner_pid, SIGRTMIN, sigval_t { answer }));
}

void stop_handler(int signo) {
    _exit(EXIT_SUCCESS);
}

void predict_handler(int signo, siginfo_t* si, void* ctx) {
    last_sig = signo;
    last_predicted_value = si->si_value.sival_int;
}

void signal_handler(int signo) {
    last_sig = signo;
}

int main() {
    int max_games;
    std::cin >> max_games;
    int games = 0, rounds = 0, answer;
    std::unordered_set<int> buf {};
    sigset_t set, suspend_set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);

    sigfillset(&suspend_set);
    sigdelset(&suspend_set, SIGRTMIN);
    sigdelset(&suspend_set, SIGUSR1);
    sigdelset(&suspend_set, SIGUSR2);
    sigdelset(&suspend_set, SIGALRM);
    sigdelset(&suspend_set, SIGCHLD);
    struct sigaction
            predict_action {},
            predicted_action {},
            not_predicted_action {},
            stop_action {},
            alarm_action {};
    predict_action.sa_sigaction = predict_handler;
    predict_action.sa_flags = SA_SIGINFO;
    predict_action.sa_mask = set;
    check(sigaction(SIGRTMIN, &predict_action, NULL));
    predicted_action.sa_handler = signal_handler;
    predicted_action.sa_mask = set;
    check(sigaction(SIGUSR1, &predicted_action, NULL));
    not_predicted_action.sa_handler = signal_handler;
    not_predicted_action.sa_mask = set;
    check(sigaction(SIGUSR2, &not_predicted_action, NULL));
    stop_action.sa_handler = stop_handler;
    stop_action.sa_mask = set;
    check(sigaction(SIGCHLD, &stop_action, NULL));
    alarm_action.sa_handler = signal_handler;
    alarm_action.sa_mask = set;
    check(sigaction(SIGALRM, &alarm_action, NULL));

    atexit(at_exit);
    srand(time(NULL));
    partner_pid = getpid();
    pid_t pid = check(fork());
    if (pid > 0) {
        partner_pid = pid;
        answer = rand() % 10;
        std::cout << "Guessed: " << answer << std::endl;
        sleep(1);
        check(kill(partner_pid, SIGUSR2));
    }
    else {
        prctl(PR_SET_PDEATHSIG, SIGCHLD);
    }
    while (true) {
        if (last_sig == SIGRTMIN) {
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
            rounds = 0;
            ++games;
            answer = rand() % 10;
            std::cout << "Guessed: " << answer << std::endl;
            check(kill(partner_pid, SIGUSR2));
        }
        else if (last_sig == SIGUSR2) {
            predict(buf);
        }
        while (true) {
            alarm(1);
            sigsuspend(&suspend_set);
            if (last_sig == SIGALRM) {
                if (pid==0 && getppid() != partner_pid)
                    exit(EXIT_FAILURE);
            }
            else {
                break;
            }
        }
    }
}
