#ifndef BOXEDFUNCTION_INCLUDED
#define BOXEDFUNCTION_INCLUDED

#include "BoxedValue.hpp"
#include "FunctionBinder.hpp"

class BoxedFunction
{
    template<typename T>
    struct FunctionSignature
    {
    };

    template<typename Ret, typename ... Params>
    struct FunctionSignature<Ret (Params...)>
    {
        typedef Ret Return_Type;
        typedef Ret (Signature)(Params...);
    };

    template<typename Ret, typename T, typename ... Params>
    struct FunctionSignature<Ret (T::*)(Params...) const>
    {
        typedef Ret Return_Type;
        typedef Ret (Signature)(Params...);
    };

    template<size_t ... I>
    struct Indexes
    {
    };

    template<size_t S, size_t ... I>
    struct MakeIndexes
    {
        typedef typename MakeIndexes<S-1, I..., sizeof...(I)>::indexes indexes;
    };

    template<size_t ... I>
    struct MakeIndexes<0, I...>
    {
        typedef Indexes<I...> indexes;
    };

    template<typename Ret, typename ... Params>
    static std::vector<Type_Info> buildParameterTypeList(Ret (*)(Params...))
    {
        return { user_type<Ret>(), user_type<Params>()... };
    }

    struct Container
    {
        virtual ~Container() {};

        virtual BoxedValue call(const std::vector<BoxedValue> &params) const = 0;
        virtual bool match(const std::vector<BoxedValue> &params) const = 0;
        virtual const Type_Info &getReturnType() const = 0;
        virtual std::vector<Type_Info> getParameterType() const = 0;
        virtual int getParameterCount() const = 0;
    };

    template<typename Func, typename Callable>
    struct Container_impl : Container
    {
        Container_impl(Callable f) : mObj(std::move(f)), mTypes(buildParameterTypeList(static_cast<Func *>(nullptr))), mParamCount(static_cast<int>(mTypes.size()) - 1)
        {
        }

        virtual ~Container_impl() {};

        BoxedValue call(const std::vector<BoxedValue> &params) const
        {
            return call(params, FunctionSignature<Func>());
        }

        template<typename Ret, typename ... Params>
        BoxedValue call(const std::vector<BoxedValue> &params, const FunctionSignature<Ret (Params...)> & signature) const
        {
            typedef typename MakeIndexes<sizeof...(Params)>::indexes indexes;
            return call_func(signature, mObj, params, indexes());
        }

        template<typename Ret, typename ... Params, size_t ... I>
        BoxedValue call_func(const FunctionSignature<Ret (Params...)> &, const Callable &f,
                             const std::vector<BoxedValue> &params, Indexes<I...>) const
        {
            (void)params;
            return f(BoxedValue::cast<Params>(params[I])...);
        }

        template<typename Ret = void, typename ... Params, size_t ... I>
        BoxedValue call_func(const FunctionSignature<void (Params...)> &, const Callable &f,
                             const std::vector<BoxedValue> &params, Indexes<I...>) const
        {
            (void)params;
            f(BoxedValue::cast<Params>(params[I])...);
            return BoxedValue();
        }

        bool match(const std::vector<BoxedValue> &params) const
        {
            if(mParamCount != params.size())
                return false;

            for(int x = 0; x < params.size(); x++)
            {
                if(!params[x].match(mTypes[x + 1]))
                    return false;
            }

            return true;
        }

        const Type_Info &getReturnType() const
        {
            return mTypes[0];
        }

        std::vector<Type_Info> getParameterType() const
        {
//            std::vector<Type_Info> types;
//            for(int x = 1; x < mTypes.size(); x++)
//                types.push_back(mTypes[x]);
//            return types;
            return {mTypes.begin()+1, mTypes.end()};
        }

        int getParameterCount() const
        {
            return mParamCount;
        }

        Callable mObj;

        std::vector<Type_Info> mTypes;
        int mParamCount;
    };


    template<typename Ret, typename ... Param>
    struct FunctionCaller
    {
        FunctionCaller(Ret( * func)(Param...) ) : mFunction(func) {}

        template<typename ... Inner>
        Ret operator()(Inner&& ... inner) const
        {
            return (mFunction)(std::forward<Inner>(inner)...);
        }

        Ret(*mFunction)(Param...);

    };

    template<typename Ret, typename Class, typename ... Param>
    struct MemberFunctionCaller
    {
        MemberFunctionCaller(Ret (Class::*func)(Param...)) : mFunction(func) {}

        template<typename ... Inner>
        Ret operator()(Class &o, Inner&& ... inner) const
        {
            return (o.*mFunction)(std::forward<Inner>(inner)...);
        }

        Ret (Class::*mFunction)(Param...);
    };

    template<typename Ret, typename ... Param>
    std::shared_ptr<Container> get(Ret (*func)(Param...))
    {
        auto caller = FunctionCaller<Ret, Param...>(func);
        return std::make_shared<Container_impl<Ret (Param...), decltype(caller)>>(caller);
    }

    template<typename Ret, typename Class, typename ... Param>
    std::shared_ptr<Container> get(Ret (Class::*func)(Param...))
    {
        auto caller = MemberFunctionCaller<Ret, Class, Param...>(func);
        return std::make_shared<Container_impl<Ret (Class &, Param...), decltype(caller)>>(caller);
    }

    template<typename T>
    std::shared_ptr<Container> get(const T& t)
    {
        typedef typename FunctionSignature<decltype(&T::operator())>::Signature Signature;
        return std::make_shared<Container_impl<Signature, T>>(t);
    }

    template<typename T, typename Arg, typename ...  Bindeds>
    std::shared_ptr<Container> get(const T& t, const Arg& arg, Bindeds... b)
    {
        return get(bindFirstArgument(t, arg), b...);
    }

public:
    template<typename Callable>
    BoxedFunction(Callable f) : mContainer(get(std::move(f)))
    {
    }

    template<typename Callable, typename ... Bindeds>
    BoxedFunction(Callable f, Bindeds... b) : mContainer(get(std::move(f), std::forward<Bindeds>(b)...))
    {
    }

    BoxedFunction() : mContainer(std::move(get([](){}))) ///TODO this is bad, fix it
    {
    }

    ~BoxedFunction()
    {
    }

    BoxedValue operator()() const
    {
        return operator()({});
    }

    BoxedValue operator()(const std::vector<BoxedValue> &params) const
    {
        if(mContainer->match(params))
            return mContainer->call(params);
        else
        {
            std::string error = "Argument dosent match function signature. (";
            for(const BoxedValue& b : params)
                error += b.type().name() + ", ";

            error.replace(error.size() - 2, 2, ") to (");

            for(const Type_Info& t : getParameterType())
                error += t.name() + ", ";

            error.replace(error.size() - 2, 2, ")");
            throw std::runtime_error(error);
        }
    }

    bool match(const std::vector<BoxedValue> &params) const
    {
        return mContainer->match(params);
    }

    const Type_Info &getReturnType() const
    {
        return mContainer->getReturnType();
    }

    std::vector<Type_Info> getParameterType() const
    {
        return mContainer->getParameterType();
    }

    int getParameterCount() const
    {
        return mContainer->getParameterCount();
    }

protected:
private:
    std::shared_ptr<Container> mContainer;
};




#endif // BOXEDFUNCTION_INCLUDED
