// chain of resposibility handler
#pragma once
#include "Context.h"
#include <memory>

class Handler
{
  protected:
    std::shared_ptr<Handler> nextHandler;

  public:
    virtual ~Handler() = default;

    void setNext(std::shared_ptr<Handler> handler)
    {
        nextHandler = handler;
    }

    virtual void handle(Context &context)
    {
        if (nextHandler)
        {
            nextHandler->handle(context);
        }
    }
};
