#pragma once

// This can be used to simplify and make more readable the parameters to pass to the Register() method.
#define PASS_OBJECT_METHOD(obj, method)   &obj,&decltype(obj)::method

class ArduinoHandler
{
  public:
    ArduinoHandler()
      : _callback(nullptr)
      , _wrapper(nullptr)
    {
    }

    virtual ~ArduinoHandler()
    {
      Unregister();
    }

    template <typename T_CALLBACK_CLASS>
    void Register(T_CALLBACK_CLASS* obj, void (T_CALLBACK_CLASS::*method)())
    {
      // Dealing with method pointers is tricky.  We can't generically
      // cast it to void* and back like we can with the object pointer.
      // We can't story it in the right type because this class is templated,
      // only this method is.  But we can define the type scoped to this method
      // that is type safe to capture the object and method pointers.  Then
      // store that in a generic member void* pointer as well as the lambda
      // that knows how to convert it back which is also type specific local to
      // this template method. Tricky, I know.  Took a while to come to this design.
      struct CallbackData {
        T_CALLBACK_CLASS* objectPtr;
        void (T_CALLBACK_CLASS::*methodPtr)();
      };

      // If they previously had a handler registered, we need to free it up.
      Unregister();

      // Allocate the callback data structure to hold the object and method pointers.
      _callback = static_cast<void*>(new CallbackData{obj, method});

      // Create the lambda that knows how to convert the void callback data back to
      // the typed data in the structure defined here.
      _wrapper = [](void* voidCallbackData)
      {
          auto* typedCallbackData = static_cast<CallbackData*>(voidCallbackData);
          (typedCallbackData->objectPtr->*typedCallbackData->methodPtr)();
      };
    }

    void Unregister()
    {
      // If they previously had a handler registered, we need to free it up.
      if ( _callback )
      {
        delete _callback;
        _callback = nullptr;
      }
      _wrapper = nullptr;
    }

    bool HasHandler()
    {
      return ( _wrapper && _callback );
    }

    void Invoke()
    {
      if ( HasHandler() )
      {
        _wrapper(_callback);
      }
    }

  private:
    void* _callback;
    void (*_wrapper)(void*);
};

template <typename T_CALLBACK_PARAM>
class ArduinoHandlerParam
{
  public:
    ArduinoHandlerParam()
      : _callback(nullptr)
      , _wrapper(nullptr)
    {
    }

    // template <typename T_CALLBACK_CLASS>
    // ArduinoHandlerParam(T_CALLBACK_CLASS* obj, void (T_CALLBACK_CLASS::*method)(T_CALLBACK_PARAM))
    //   : _callback(nullptr)
    //   , _wrapper(nullptr)
    // {
    //   Register(obj, method);
    // }

    virtual ~ArduinoHandlerParam()
    {
      Unregister();
    }

    template <typename T_CALLBACK_CLASS>
    void Register(T_CALLBACK_CLASS* obj, void (T_CALLBACK_CLASS::*method)(T_CALLBACK_PARAM))
    {
      // Dealing with method pointers is tricky.  We can't generically
      // cast it to void* and back like we can with the object pointer.
      // We can't story it in the right type because this class is templated,
      // only this method is.  But we can define the type scoped to this method
      // that is type safe to capture the object and method pointers.  Then
      // store that in a generic member void* pointer as well as the lambda
      // that knows how to convert it back which is also type specific local to
      // this template method. Tricky, I know.  Took a while to come to this design.
      struct CallbackData {
        T_CALLBACK_CLASS* objectPtr;
        void (T_CALLBACK_CLASS::*methodPtr)(T_CALLBACK_PARAM);
      };

      // If they previously had a handler registered, we need to free it up.
      Unregister();

      // Allocate the callback data structure to hold the object and method pointers.
      _callback = static_cast<void*>(new CallbackData{obj, method});

      // Create the lambda that knows how to convert the void callback data back to
      // the typed data in the structure defined here.
      _wrapper = [](void* voidCallbackData, T_CALLBACK_PARAM param)
      {
          auto* typedCallbackData = static_cast<CallbackData*>(voidCallbackData);
          (typedCallbackData->objectPtr->*typedCallbackData->methodPtr)(param);
      };
    }

    void Unregister()
    {
      // If they previously had a handler registered, we need to free it up.
      if ( _callback )
      {
        delete _callback;
        _callback = nullptr;
      }
      _wrapper = nullptr;
    }

    void Invoke(T_CALLBACK_PARAM param)
    {
      if ( _wrapper && _callback )
      {
        _wrapper(_callback, param);
      }
    }

  private:
    void* _callback;
    void (*_wrapper)(void*, T_CALLBACK_PARAM);
};
