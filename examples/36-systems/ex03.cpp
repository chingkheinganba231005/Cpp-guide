#include <array>
#include <cstdio>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

int main() {
    int fds[2];
    if (pipe(fds) != 0) { std::perror("pipe"); return 1; }        // fds[0] read end, fds[1] write end
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa, fds[1], STDOUT_FILENO);  // the child's stdout goes into the pipe
    posix_spawn_file_actions_addclose(&fa, fds[0]);
    char cmd[] = "echo", arg[] = "hello from a child process";
    char* argv[] = {cmd, arg, nullptr};
    pid_t pid;
    if (posix_spawnp(&pid, "echo", &fa, nullptr, argv, environ) != 0) { std::perror("spawn"); return 1; }
    close(fds[1]);
    std::string out;
    std::array<char, 256> buf;
    for (ssize_t n; (n = read(fds[0], buf.data(), buf.size())) > 0;) out.append(buf.data(), static_cast<size_t>(n));
    int status = 0;
    waitpid(pid, &status, 0);
    std::cout << "child " << pid << " said: " << out << "exit status " << WEXITSTATUS(status) << '\n';
    posix_spawn_file_actions_destroy(&fa);
}
