#include <iostream>
#include <unistd.h>
#include <time.h>
#include <unordered_set>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstring>
#include <wait.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include "check.hpp"

mqd_t mq_read, mq_write;


void predict(mqd_t mqd, std::unordered_set<int>& buf) {
    bool flag;
    int tmp;
    do {
        tmp = rand() % 11;
        flag = false;
        if (buf.find(tmp) != buf.end())
            flag = true;
    } while (flag);
    buf.insert(tmp);
    std::cout << "Maybe it`s " << tmp << "?" << std::endl;
    check(mq_send(mqd, reinterpret_cast<const char *>(&tmp), sizeof(tmp), 0));
}

void at_exit_parent() {
    int buffer = -3;
    check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
    check(mq_close(mq_write));
    check(mq_close(mq_read));
    check(mq_unlink("/mq1"));
    check(mq_unlink("/mq2"));
    int stat;
    wait(&stat);
}

void at_exit_child() {
    int buffer = -3;
    check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
    check(mq_close(mq_read));
    check(mq_close(mq_write));
}

void child_handler(int signo) {
    _exit(EXIT_SUCCESS);
}

int main() {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGCHLD);
    struct sigaction child_action {};
    child_action.sa_handler = child_handler;
    child_action.sa_mask = set;
    check(sigaction(SIGCHLD, &child_action, NULL));

    mqd_t mq1, mq2;
    int max_games;
    std::cin >> max_games;
    std::unordered_set<int> buf {};
    int rounds = 0, game = 0;
    int buffer = -2;
    int answer;
    srand(time(NULL));
    mq_attr attr{};
    attr.mq_msgsize = 4;
    attr.mq_maxmsg = 1;
    mq_unlink("/mq1");
    mq_unlink("/mq2");
    mq1 = check(mq_open("/mq1", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr));
    mq2 = check(mq_open("/mq2", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr));
    pid_t pid = check(fork());
    if (pid > 0) {
        mq_read = mq1;
        mq_write = mq2;
        atexit(at_exit_parent);
        answer = rand() % 10;
        std::cout << "Guessed: " << answer << std::endl;
        check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
        sleep(1);
    }
    else {
        prctl(PR_SET_PDEATHSIG, SIGCHLD);
        mq_read = mq2;
        mq_write = mq1;
        atexit(at_exit_child);
    }
    /*
     * -1 - угадал
     * -2 - не угадал
     * -3 - конец игры
     */
    while (true) {
        check(mq_receive(mq_read, reinterpret_cast<char *>(&buffer), 4, nullptr));
        if (buffer == -3) {
            exit(EXIT_SUCCESS);
        }
        if (buffer == -1) {
            answer = rand() % 10;
            rounds = 0;
            ++game;
            buffer = -2;
            std::cout << "Guessed: " << answer << std::endl;
            check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
        } else if (buffer == -2) {
            predict(mq_write, buf);
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
                check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
            } else {
                std::cout << "Wrong answer" << std::endl;
                ++rounds;
                buffer = -2;
                check(mq_send(mq_write, reinterpret_cast<const char *>(&buffer), 4, 0));
            }
        }
        sleep(2);
    }
}
