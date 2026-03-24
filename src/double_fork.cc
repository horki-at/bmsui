#include "double_fork.hh"

#ifdef _POSIX

#include <stdexcept>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

void DoubleFork::run(Task child1, Task child2, Task parent)
{
  auto dispatch_child = [](Task task, pid_t &pid) {
    if (pid = ::fork(); pid == -1) // child fork failed.
      throw runtime_error("Forking failed.");
    else if (pid == 0)
    {
      setpgid(0, 0);            // child owns all of its possible children
      task();                   // @important noreturn here
      _exit(1);                 // this shouldn't happen, task must not return
    }
  };
  
  dispatch_child(child1, d_pid1);
  dispatch_child(child2, d_pid2);
  parent();
}

pair<int, int> DoubleFork::wait() const
{
  int status1, status2;
  waitpid(d_pid1, &status1, 0);
  waitpid(d_pid2, &status2, 0);
  return { WEXITSTATUS(status1), WEXITSTATUS(status2) };
}

void DoubleFork::kill() const
{
  if (::kill(-d_pid1, SIGKILL) == -1)
    throw runtime_error("couldn't kill d_pid1.");
  if (::kill(-d_pid2, SIGKILL) == -1)
    throw runtime_error("couldn't kill d_pid2.");
}

#endif