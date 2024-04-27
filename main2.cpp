#include <iostream>
#include <unistd.h>
#include <time.h>
#include <unordered_set>
#include <cstring>
#include <wait.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "check.hpp"

int write_pipe, read_pipe;

void predict(int pipe_fd, std::unordered_set<int>& buf) {
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
    check(write(pipe_fd, &answer, 4));
}

void pipe_handler(int signo) {
    exit(EXIT_SUCCESS);
}

int main() {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGPIPE);
    struct sigaction pipe_action {};
    pipe_action.sa_handler = pipe_handler;
    pipe_action.sa_mask = set;
    check(sigaction(SIGPIPE, &pipe_action, NULL));

    int max_games;
    std::cin >> max_games;
    std::unordered_set<int> buf {};
    int buffer = -2, answer;
    int rounds = 0, game = 0;
    int first_pipe[2];
    int second_pipe[2];
    check(pipe(first_pipe));
    check(pipe(second_pipe));
    srand(time(NULL));
    pid_t pid = check(fork());
    if (pid > 0) {
        prctl(PR_SET_PDEATHSIG, SIGCHLD);
        check(close(first_pipe[0]));
        check(close(second_pipe[1]));
        write_pipe = first_pipe[1];
        read_pipe = second_pipe[0];
        answer = rand() % 10;
        std::cout << "Guessed: " << answer << std::endl;
        check(write(write_pipe, &buffer, 4));
        sleep(1);
    }
    else {
        prctl(PR_SET_PDEATHSIG, SIGCHLD);
        check(close(first_pipe[1]));
        check(close(second_pipe[0]));
        write_pipe = second_pipe[1];
        read_pipe = first_pipe[0];
    }
    while (true) {
        check(read(read_pipe, &buffer, 4));
        if (buffer == -3) {
            exit(EXIT_SUCCESS);
        }
        if (buffer == -1) {
            answer = rand() % 10;
            rounds = 0;
            ++game;
            std::cout << "Guessed " << answer << std::endl;
            buffer = -2;
            check(write(write_pipe, &buffer, 4));
        }
        else if (buffer == -2) {
            predict(write_pipe, buf);
        }
        else {
            if (buffer == answer) {
                std::cout << "Correct answer" << std::endl;
                std::cout << "Game lasts " << ++rounds << " rounds\n\n" << std::endl;
                if (++game >= max_games) {
                    exit(EXIT_SUCCESS);
                }
                buf = {};
                buffer = -1;
                check(write(write_pipe, &buffer, 4));
            } else {
                std::cout << "Wrong answer" << std::endl;
                ++rounds;
                buffer = -2;
                check(write(write_pipe, &buffer, 4));
            }
        }
        sleep(1);
    }
}
