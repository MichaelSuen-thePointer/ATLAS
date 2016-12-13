#pragma once
#include <iostream>
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

template<class... T, class = std::enable_if_t<sizeof...(T)>>
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

template<class... T, class >
std::ostream& operator<<(std::ostream& os, const std::variant<T...>& v)
{
    std::visit([&os](const auto& x) { os << x; }, v);
    return os;
}

struct shift_walk_
{
    static shift_walk_ shift_walk;
    auto operator()(const variable& t, ptrdiff_t offset, ptrdiff_t curr) const
    {
        if (t.index >= curr)
        {
            std::cout << t.index + offset << "\n";
            return make_variable(t.index + offset);
        }
        std::cout << t.index << "\n";
        return make_variable(t.index);
    }
    template<class Tterm>
    auto operator()(const abstraction<Tterm>& t, ptrdiff_t offset, ptrdiff_t curr) const
    {
        auto& r = make_abstraction(t.param_name, shift_walk(t.body, offset, curr + 1));
        std::cout << r << "\n";
        return r;
    }
    template<class Tterm1, class Tterm2>
    auto operator()(const application<Tterm1, Tterm2>& t, ptrdiff_t offset, ptrdiff_t curr) const
    {
        auto& r = make_application(shift_walk(t.left, offset, curr), shift_walk(t.right, offset, curr));
        std::cout << r << "\n";
        return r;
    }
    template<class... T>
    auto operator()(const std::variant<T...>& t, ptrdiff_t offset, ptrdiff_t curr)
    {
        auto& r = std::visit([=](auto& x) { return std::variant<T...>{shift_walk(x, offset, curr)}; }, t);
        std::cout << r << "\n";
        return r;
    }
};
const shift_walk_& shift_walk = shift_walk_::shift_walk;

template<class T>
auto shift_term(const T& t, ptrdiff_t offset)
{
    return shift_walk(t, offset, 0);
}

struct substitute_walk_
{
    static substitute_walk_ substitute_walk;

    template<class Tterm>
    auto operator()(const variable& t, ptrdiff_t from, const Tterm& to) const
    {
        using result_t = merge_variant_t<variable, Tterm>;
        if (t.index == from)
        {
            std::cout << to << "\n";
            return result_t{to};
        }
        std::cout << t << "\n";
        return result_t{t};
    }
    template<class Tbody, class Tterm>
    auto operator()(const abstraction<Tbody>& t, ptrdiff_t from, const Tterm& to) const
    {
        auto& r = make_abstraction(t.param_name, substitute_walk(t.body, from + 1, shift_term(to, 1)));
        std::cout << r << "\n";
        return r;
    }
    template<class Tterm1, class Tterm2, class Tterm>
    auto operator()(const application<Tterm1, Tterm2>& t, ptrdiff_t from, const Tterm& to) const
    {
        auto& r = make_application(substitute_walk(t.left, from, to), substitute_walk(t.right, from, to));
        std::cout << r << "\n";
        return r;
    }

    template<class Tterm, class... T>
    auto operator()(const std::variant<T...>& t, ptrdiff_t from, const Tterm& to) const
    {
        using result_t = merge_variant_t<decltype(substitute_walk(std::declval<T>(), from, to))...>;
        auto& r = std::visit([&](const auto& x)
        {
            return std::visit([](const auto& r) { return result_t{r}; }, make_variant_t<decltype(substitute_walk(x, from, to))>{substitute_walk(x, from, to)});
        }, t);
        std::cout << r << "\n";
        return r;
    }
};
const substitute_walk_& substitute_walk = substitute_walk_::substitute_walk;

template<class T, class Tterm>
auto substitute_term(const T& t, ptrdiff_t from, const Tterm& to)
{
    return substitute_walk(t, from, to);
}

template<class T, class Tterm>
auto substitute_top_term(const Tterm& t, const T& to)
{
    return shift_term(substitute_term(t, 0, shift_term(to, 1)), -1);
}

template<class T>
struct is_val : std::false_type
{};

template<class T>
struct is_val<abstraction<T>> : std::true_type
{};

struct abstraction_checker
{
    template<class T>
    bool operator()(const abstraction<T>&)
    {
        return true;
    }
    template<class T>
    bool operator()(const T&)
    {
        return false;
    }
};

template<class T>
bool is_val_dynamic(const std::variant<T>& term)
{
    return std::visit(abstraction_checker{}, term);
}

template<class T>
bool is_val_dynamic(const T& term)
{
    return false;
}

template<class T>
struct is_value_abstraction : std::false_type
{};

template<class T>
struct is_value_abstraction<abstraction<T>> : std::conditional_t<is_val<T>::value, std::true_type, std::false_type>
{};

template<class... Tterm1, class... Tterm2>
auto eval1(const application<std::variant<Tterm1...>, std::variant<Tterm2...>>& t)
{
    using result_t1 = merge_variant_t<decltype(substitute_top_term(std::declval<Tterm1>(), t.right))...>;
    using result_t2 = merge_variant_t<result_t1, decltype(make_application(t.left, eval1(t.right)))>;
    using result_t = merge_variant_t<result_t2, decltype(make_application(eval1(t.left), t.right))>;
    if (is_val_dynamic(t.left))
    {
        if (is_val_dynamic(t.right))
        {
            std::cout << "var(abs), var(abs)\n";
            return std::visit([&](const auto x)
            {
                return result_t{substitute_top_term(x.body, t.right)};
            }, t.left);
        }
        else
        {
            std::cout << "var(abs), var\n";
            return result_t{make_application(t.left, eval1(t.right))};
        }
    }
    std::cout << "var, var\n";
    return result_t{make_application(eval1(t.left), t.right)};
}

template<class Tterm1, class... Tterm2>
auto eval1(const application<abstraction<Tterm1>, std::variant<Tterm2...>>& t)
{
    using result_t = merge_variant_t<
        decltype(substitute_top_term(t.left.body, t.right)),
        decltype(make_application(t.left, eval1(t.right)))>;
    if (is_val_dynamic(t.right))
    {
        std::cout << "abs, var(abs)\n";
        return result_t{substitute_top_term(t.left.body, t.right)};
    }
    std::cout << "abs, var\n";
    return result_t{make_application(t.left, eval1(t.right))};
}

template<class... Tterm1, class Tterm2>
auto eval1(const application<std::variant<Tterm1...>, abstraction<Tterm2>>& t)
{
    using result_t1 = merge_variant_t<decltype(substitute_top_term(std::declval<Tterm1>(), t.right))...>;
    using result_t = merge_variant_t<result_t1, decltype(make_application(eval1(t.left), t.right))>;
    if (is_val_dynamic(t.left))
    {
        std::cout << "var(abs), abs\n";
        return result_t{std::visit([&](const auto x) { return substitute_top_term(x.body, t.right); }, t.left)};
    }
    std::cout << "var, abs\n";
    return result_t{make_application(eval1(t.left), t.right)};
}

template<class... Tterm1, class Tterm2>
auto eval1(const application<std::variant<Tterm1...>, Tterm2>& t)
{
    using result_t1 = merge_variant_t<decltype(substitute_top_term(std::declval<Tterm1>(), t.right))...>;
    using result_t = merge_variant_t<result_t1, decltype(make_application(eval1(t.left), t.right))>;
    if (is_val_dynamic(t.left))
    {
        std::cout << "var(abs), T\n";
        return result_t{std::visit([&](const auto x) { return substitute_top_term(x.body, t.right); }, t.left)};
    }
    std::cout << "var, T\n";
    return result_t{make_application(eval1(t.left), t.right)};
}

struct evaluator
{
    auto operator()(const variable& t)
    {
        return t;
    }
    template<class Tterm1, class Tterm2>
    auto operator()(const application<Tterm1, Tterm2>& t)
    {
        return eval(t.left, t.right);
    }
    template<class Tterm>
    auto operator()(const abstraction<Tterm>& t)
    {
        return (*this)(t.body);
    }
    template<class... Tterm>
    auto operator()(const std::variant<Tterm...>& t)
    {
        return std::visit(*this, t);
    }
};

template<class Tterm1, class Tterm2>
auto eval1(const abstraction<Tterm1>& l, const abstraction<Tterm2>& r)
{
    return substitute_top_term(l.body, r);
}

template<class Tterm1, class Tterm2>
auto eval1(const abstraction<Tterm1>& l, const Tterm2& r)
{
    return make_application(l, eval1(r));
}

template<class Tterm1, class Tterm2>
auto eval1(const Tterm1& l, const Tterm2& r)
{
    return make_application(eval1(l), r);
}

template<class Tterm>
auto eval1(const abstraction<Tterm>& t)
{
    std::cout << "abs\n";
    return make_abstraction(t.param_name, eval1(t.body));
}


template<class... T>
auto eval1(const std::variant<T...>& term)
{
    std::cout << "eval " << term << " variant\n";
    using result_t = merge_variant_t<decltype(eval1(std::declval<T>()))...>;
    return std::visit([](const auto& x)
    {
        return std::visit([](const auto& r) { return result_t{r}; }, make_variant_t<decltype(eval1(x))>{eval1(x)});
    }, term);
}

template<class T>
auto eval1(const T& t)
{
    std::cout << "eval " << t << " plain\n";
    return t;
}


}