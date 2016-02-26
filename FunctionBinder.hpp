#ifndef CHAISCRIPT_bindFirstArgument_HPP_
#define CHAISCRIPT_bindFirstArgument_HPP_

#include <functional>

class BoxedFunction;

template<typename T>
T* getPointer(T *t)
{
    return t;
}

template<typename T>
T* getPointer(const std::reference_wrapper<T> &t)
{
    return &t.get();
}

template<typename Bind, typename Ret, typename Arg1, typename ... Args>
std::function<Ret (Args...)> bindFirstArgument(Ret (*f)(Arg1, Args...), Bind&& bind)
{
    return std::function<Ret (Args...)>(
               [f, bind](Args...args) -> Ret
    {
        return f(std::forward<Bind>(bind), std::forward<Args>(args)...);
    }
           );
}

template<typename Bind, typename Ret, typename Class, typename ... Args>
std::function<Ret (Args...)> bindFirstArgument(Ret (Class::*f)(Args...), Bind&& bind)
{
    return std::function<Ret (Args...)>(
               [f, bind](Args...args) -> Ret
    {
        return (getPointer(bind)->*f)(std::forward<Args>(args)...);
    }
           );
}

template<typename Bind, typename Ret, typename Class, typename ... Args>
std::function<Ret (Args...)> bindFirstArgument(Ret (Class::*f)(Args...) const, Bind&& bind)
{
    return std::function<Ret (Args...)>(
               [f, bind](Args...args) -> Ret
    {
        return (getPointer(bind)->*f)(std::forward<Args>(args)...);
    }
           );

}

template<typename Bind, typename Ret, typename Arg1, typename ... Args>
std::function<Ret (Args...)> bindFirstArgument(const std::function<Ret (Arg1, Args...)> &f, Bind&& bind)
{
    return std::function<Ret (Args...)>(
               [f, bind](Args...args) -> Ret
    {
        return f(bind, std::forward<Args>(args)...);
    });
}

#endif
