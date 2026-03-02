
#pragma once

namespace phys
{

class App
{
  public:
    void start();

  protected:
    virtual void tick();

  private:
    void _tick();
    void _render();

    void handleMessages();
};

} // namespace phys
