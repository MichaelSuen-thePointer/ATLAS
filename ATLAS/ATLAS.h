#pragma once
#include <memory>
#include <string>
#include <variant>
#include "Utils.h"
namespace ATLAS
{

struct variable
{
    ptrdiff_t index;

    variable(ptrdiff_t index)
        : index(index)
    {
    }

    variable(const variable& other)
        : index(other.index)
    {
    }

    variable& operator=(const variable& other)
    {
        if (this == &other)
            return *this;
        index = other.index;
        return *this;
    }
};

const auto make_variable = [](ptrdiff_t index)
{
    return variable{index};
};

template<class Tterm>
struct abstraction
{
    std::string param_name;
    Tterm body;
    abstraction(const std::string& paramName, const Tterm& body)
        : param_name(paramName),
        body(body)
    {
    }
};

const auto make_abstraction = [](const std::string& pname, auto&& term)
{
    return abstraction<std::decay_t<decltype(term)>>{pname, term};
};

template<class Tterm1, class Tterm2>
struct application
{
    Tterm1 left;
    Tterm2 right;

    application(const Tterm1& appliee, const Tterm2& applier)
        : left(appliee),
        right(applier)
    {
    }
};

const auto make_application = [](auto&& t1, auto&& t2)
{
    return application<std::decay_t<decltype(t1)>, std::decay_t<decltype(t2)>>{t1, t2};
};

inline
std::ostream& operator<<(std::ostream& os, const variable& t)
{
    return os << t.index;
}

template<class... T>
std::ostream& operator<<(std::ostream& os, const std::variant<T...>& v);

template<class Tterm1, class Tterm2>
std::ostream& operator<<(std::ostream& os, const application<Tterm1, Tterm2>& t);

template<class Tterm>
std::ostream& operator<<(std::ostream& os, const abstraction<Tterm>& t)
{
    return os << "(\\." << t.body << ")";
}

template<class Tterm1, class Tterm2>
std::ostream& operator<<(std::ostream& os, const application<Tterm1, Tterm2>& t)
{
    return os << "(" << t.left << " " << t.right << ")";
}

template<class... T, class = std::enable_if_t<(sizeof...(T) > 0)>>
std::ostream& operator<<(std::ostream& os, const std::variant<T...>& v)
{
    std::visit([&os](const auto& x) { os << x; }, v);
    return os;
}

template<class T, class = std::enable_if_t<std::is_same<T, variable>::value>>
auto shift_walk(T& t, ptrdiff_t offset, ptrdiff_t curr)
{
    if (t.index >= curr)
    {
        return make_variable(t.index + offset, t.contex_length + offset);
    }
    return make_variable(t.index, t.contex_length + offset);
}

template<class Tterm>
auto shift_walk(abstraction<Tterm>& t, ptrdiff_t offset, ptrdiff_t curr)
-> decltype(make_abstraction(t.param_name, shift_walk(t.body, offset, curr + 1)));

template<class Tterm1, class Tterm2>
auto shift_walk(application<Tterm1, Tterm2>& t, ptrdiff_t offset, ptrdiff_t curr)
->decltype(make_application(shift_walk(curr, t.left), shift_walk(curr, t.right)));

template<class... T>
auto shift_walk(std::variant<T...>& t, ptrdiff_t offset, ptrdiff_t curr)
->std::variant<T...>;

template<class Tterm>
auto shift_walk(abstraction<Tterm>& t, ptrdiff_t offset, ptrdiff_t curr)
-> decltype(make_abstraction(t.param_name, shift_walk(t.body, offset, curr + 1)))
{
    return make_abstraction(t.param_name, shift_walk(t.body, offset, curr + 1));
}

template<class Tterm1, class Tterm2>
auto shift_walk(application<Tterm1, Tterm2>& t, ptrdiff_t offset, ptrdiff_t curr)
->decltype(make_application(shift_walk(t.left, offset, curr), shift_walk(t.right, offset, curr)))
{
    return make_application(shift_walk(t.left, offset, curr), shift_walk(t.right, offset, curr));
}

template<class... T>
auto shift_walk(std::variant<T...>& t, ptrdiff_t offset, ptrdiff_t curr)
->std::variant<T...>
{
    return std::visit([=](auto& x) { return std::variant<T...>{shift_walk(x, offset, curr)}; }, t);
}

template<class T>
auto shift_term(T& t, ptrdiff_t offset)
{
    return shift_walk(t, offset, 0);
}

template<class Tterm>
auto substitute_walk(variable& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->std::conditional_t<std::is_same<variable, Tterm>::value,
    variable,
    std::variant<variable, decltype(shift_term(to, curr))>>
{
    if (t.index == from + curr)
    {
        return shift_term(to, curr);
    }
    return t;
}

template<class Tbody, class Tterm>
auto substitute_walk(abstraction<Tbody>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->decltype(make_abstraction(t.param_name, substitute_walk(t.body, from, to, curr + 1)));
template<class Tterm1, class Tterm2, class Tterm>
auto substitute_walk(application<Tterm1, Tterm2>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->decltype(make_application(substitute_walk(t.left, from, to, curr), substitute_walk(t.left, from, to, curr)));
template<class Tterm, class... T>
auto substitute_walk(std::variant<T...>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->merge_variant_t<decltype(substitute_walk(std::declval<T>(), from, to, curr))...>;

template<class Tbody, class Tterm>
auto substitute_walk(abstraction<Tbody>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->decltype(make_abstraction(t.param_name, substitute_walk(t.body, from, to, curr + 1)))
{
    return make_abstraction(t.param_name, substitute_walk(t.body, from, to, curr + 1));
}

template<class Tterm1, class Tterm2, class Tterm>
auto substitute_walk(application<Tterm1, Tterm2>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->decltype(make_application(substitute_walk(t.left, from, to, curr), substitute_walk(t.left, from, to, curr)))
{
    return make_application(substitute_walk(t.left, from, to, curr), substitute_walk(t.left, from, to, curr));
}

template<class Tterm, class... T>
auto substitute_walk(std::variant<T...>& t, ptrdiff_t from, Tterm& to, ptrdiff_t curr)
->merge_variant_t<decltype(substitute_walk(std::declval<T>(), from, to, curr))...>
{
    using result_t = merge_variant_t<decltype(substitute_walk(std::declval<T>(), from, to, curr))...>;
    return std::visit([&](auto& x) { return result_t{substitute_walk(x, from, to, curr)}; }, t);
}

template<class T, class Tterm>
auto substitute_term(T& t, ptrdiff_t from, Tterm& to)
{
    return substitute_walk(t, from, to, 0);
}

template<class T, class Tterm>
auto substitute_top_term(Tterm& t, T& to)
{
    return shift_term(substitute_term(t, 0, shift_term(to, 1)), -1);
}

template<class T>
struct is_val : std::false_type
{};

template<class T>
struct is_val<abstraction<T>> : std::true_type
{};

template<>
struct is_val<variable> : std::true_type
{};

template<class Tterm12, class Ttermv2, class = std::enable_if_t<is_val<Ttermv2>::value>>
auto eval1(application<abstraction<Tterm12>, Ttermv2>& t)
{
    return substitute_top_term(t.left.body, t.right);
}

template<class Tvalue1, class Tterm2, class = std::enable_if_t<is_val<Tvalue1>::value>>
auto eval1(application<Tvalue1, Tterm2>& t)
{
    return make_application(t.left, eval1(t.right));
}

template<class Tterm1, class Tterm2>
auto eval1(application<Tterm1, Tterm2>& t, std::enable_if_t<!is_val<Tterm1>::value, int>* = nullptr)
{
    return make_application(eval1(t.left), t.right);
}

}