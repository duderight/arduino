#pragma once

#include "ArduinoHandler.h"

class ArdunioWorker
{
  public:

    // class WorkerInterface
    // {
    //   public:
    //     virtual unsigned long DoWork() = 0;
    // };

    struct WorkerList
    {
      ArduinoHandlerParam<unsigned long &> callback;
      unsigned long delay;
      WorkerList* next;

      template <typename T>
      WorkerList(T* obj, void (T::*method)(unsigned long &), WorkerList* n = nullptr)
        : delay(0)
        , next(n)
      {
        callback.Register(obj, method);
      };
    };

    ArdunioWorker()
      : _list(nullptr)
      , _lastRun(0)
    {
    }

    virtual ~ArdunioWorker()
    {
      while ( _list )
      {
        WorkerList* deleteMe = _list;
        _list = _list->next;
        delete deleteMe;
      }
    }

    template <typename T>
    void AddWorker(T* obj, void (T::*method)(unsigned long &))
    {
      WorkerList* newHead = new WorkerList(obj, method, _list);
      if ( nullptr == newHead )
      {
        return false;
      }
      _list = newHead;
      return true;
    }

    unsigned long RunWorkers()
    {
      unsigned long now = millis();
      unsigned long elapsed = _lastRun ? now - _lastRun : 0;
      _lastRun = now;
      unsigned long next = _maxWait;
      for ( WorkerList* item = _list; nullptr != item; item = item->next )
      {
        if ( elapsed >= item->delay )
        {
          item->delay = _maxWait;
          item->callback.Invoke(/*byref*/ item->delay);
        }
        else
        {
          item->delay -= elapsed;
        }
        if ( item->delay < next )
        {
          next = item->delay;
        }
      }
      return next;
    }

  private:
    static const unsigned long _maxWait = 0xFFFFFFFF;
    WorkerList* _list;
    unsigned long _lastRun;
};

