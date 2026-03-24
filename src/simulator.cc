#include "simulator.hh"
#include "simulator_impl.hh"
using namespace std;

Simulator::Simulator()
  :
  d_impl(nullptr)
{
}

Simulator::Simulator(std::unique_ptr<Impl> impl)
  :
  d_impl(std::move(impl))
{
}

void Simulator::set_impl(std::unique_ptr<Impl> impl)
{
  d_impl = std::move(impl);
}

void Simulator::start()
{
  d_impl->start();
}

void Simulator::kill()
{
  d_impl->kill();
}

bool Simulator::running() const
{
  return d_impl->running();
}

string Simulator::bms() const
{
  return d_impl->bms();
}

string Simulator::app() const
{
  return d_impl->app();
}

void Simulator::send_cmd(Command cmd, float arg) const
{
  d_impl->send_cmd(cmd, arg);
}

Simulator::~Simulator() = default;