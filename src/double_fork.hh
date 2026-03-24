#ifndef INCLUDED_DOUBLE_FORK_
#define INCLUDED_DOUBLE_FORK_

#ifdef _POSIX

#include "utils.hh"
#include <utility>
#include <functional>
#include <sys/types.h>          // for pid_t

class DoubleFork final
{
  pid_t d_pid1 = -1, d_pid2 = -1;

public:
  using Task = std::function<void()>;
  
  DoubleFork() = default;
  DISABLE_CPY_MV(DoubleFork);
  ~DoubleFork();

  // Runs the parent task and two children tasks
  void run(Task child1, Task child2, Task parent);

  // Forcefully kills both the children. Even if they do not run. Be careful.
  void kill() const;

  std::pair<pid_t, pid_t> pid() const; 

  // Wait for both the children's completion.
  std::pair<int, int> wait() const; 
};

inline DoubleFork::~DoubleFork()
{
  wait();
}

inline std::pair<pid_t, pid_t> DoubleFork::pid() const
{
	return {d_pid1, d_pid2};
}

#endif  // _POSIX
#endif  // INCLUDED_DOUBLE_FORK_