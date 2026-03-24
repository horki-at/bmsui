#ifndef INCLUDED_PIPE_
#define INCLUDED_PIPE_

#ifdef _POSIX

#include "utils.hh"
#include <unistd.h>
#include <stdexcept>

class Pipe
{
  int d_pipe[2];

public:
  Pipe() = default;
  DISABLE_CPY_MV(Pipe);
  ~Pipe();

  void open();
  void close();
  void close_read();
  void close_write();

  int readend() const;
  int writeend() const;
};

inline void Pipe::open()
{
  if (pipe(d_pipe) == -1) throw std::runtime_error("Couldn't open the pipe.");
}

inline Pipe::~Pipe() { close(); }
inline void Pipe::close_read() { ::close(d_pipe[0]); }
inline void Pipe::close_write() { ::close(d_pipe[1]); }
inline void Pipe::close() { close_read(); close_write(); }
inline int Pipe::readend() const { return d_pipe[0]; }
inline int Pipe::writeend() const { return d_pipe[1]; }

#endif // _POSIX
#endif // INCLUDED_PIPE_
