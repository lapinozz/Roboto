#ifndef BOXEDVALUE_H
#define BOXEDVALUE_H

#include <map>
#include <memory>
#include <type_traits>

#include "Type_Info.hpp"

class BoxedValue
{
private:
    struct VoidType {};

    template<typename T>
    struct Container_impl;

    struct Container
    {
        virtual ~Container() {};

        template<typename CastType>
        CastType &cast()
        {
            return ((Container_impl<CastType>*)this)->mObj;
        }
    };

    template<typename T>
    struct Container_impl : Container
    {
        Container_impl(T obj) : mObj(obj) {};
        virtual ~Container_impl() {};
        T mObj;
    };

    template<typename T>
    std::shared_ptr<Container> get(T t)
    {
        auto p = std::make_shared<T>(std::move(t));
        return std::make_shared<Container_impl<decltype(p)>>(p);
    }

    template<typename T>
    std::shared_ptr<Container> get(T* t)
    {
        return get(std::ref(*t));
    }

    template<typename T>
    std::shared_ptr<Container> get(const T* t)
    {
        return get(std::cref(*t));
    }

    template<typename T>
    std::shared_ptr<Container> get(std::reference_wrapper<T> t)
    {
        auto p = &t.get();
        return std::make_shared<Container_impl<decltype(p)>>(p);
    }

public:
    template<typename T,
             typename = typename std::enable_if<!std::is_same<BoxedValue, typename std::decay<T>::type>::value>::type>
    BoxedValue(T &&t) : mTypeInfo(user_type<T>()), mContainer(get(t))
    {
    }

    BoxedValue() : BoxedValue(VoidType())
    {
    }

    const Type_Info& type() const
    {
        return mTypeInfo;
    }

    template<typename T>
    bool match(const T& t) const
    {
        return match(user_type<T>());
    }

    template<typename T>
    bool match() const
    {
        return match(user_type<T>());
    }

    bool match(const BoxedValue& b) const
    {
        return match(b.type());
    }

    bool match(const Type_Info& t) const
    {
        if(t.bare_equal(user_type<void>()))
            return type().bare_equal(user_type<VoidType>());

        return type().bare_equal(t);
    }

    template<typename CastType>
    CastType &cast() const
    {
        if(mTypeInfo.bare_equal(user_type<CastType>()))
            return mContainer->cast<CastType&>();
        else
            throw std::runtime_error("Cast Type dosent match holded type (" + mTypeInfo.name() + " to " + user_type<CastType>().name() + ")");
    }

    template<typename CasteType>
    static CasteType &cast(const BoxedValue& b)
    {
        return b.cast<CasteType>();
    }

#if !defined(_MSC_VER) || _MSC_VER != 1800
    BoxedValue(BoxedValue&&) = default;
    BoxedValue& operator=(BoxedValue&&) = default;
#endif

    BoxedValue(const BoxedValue&) = default;
    BoxedValue& operator=(const BoxedValue&) = default;

private:
    Type_Info mTypeInfo;
    std::shared_ptr<Container> mContainer;
};

#endif // BOXEDVALUE_H
