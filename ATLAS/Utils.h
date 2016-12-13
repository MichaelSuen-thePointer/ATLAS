#pragma once
#include <string>

inline void next_available(std::string& name)
{
    auto it = name.begin();
    while(it != name.end())
    {
        if (*it == 'z')
        {
            ++it;
        }
        else
        {
            ++(*it);
            return;
        }
    }
    name.assign(name.size() + 1, 'a');
}

template<class... T>
struct merge_variant;

template<class... T>
struct merge_variant<std::variant<T...>>
{
    using type = std::variant<T...>;
};

template<class T1, class T2>
struct merge_two
{
    using type = std::variant<T1, T2>;
};

template<class... T1s, class T2>
struct merge_two<std::variant<T1s...>, T2>
{
    using type = std::variant<T1s..., T2>;
};

template<class T1, class... T2s>
struct merge_two<T1, T2s...>
{
    using type = std::variant<T1, T2s...>;
};

template<class... Targs1, class... Targs2>
struct merge_two<std::variant<Targs1...>, std::variant<Targs2...>>
{
    using type = std::variant<Targs1..., Targs2...>;
};

template<class T>
struct merge_variant<T>
{
    using type = T;
};

template<class T1, class T2, class... Trest>
struct merge_variant<T1, T2, Trest...>
{
    using type = typename merge_variant<typename merge_two<T1, T2>::type, Trest...>::type;
};

template<class... T>
using merge_variant_t = typename merge_variant<T...>::type;

template<class T>
struct make_variant
{
    using type = std::variant<T>;
};

template<class... Ts>
struct make_variant<std::variant<Ts...>>
{
    using type = std::variant<Ts...>;
};

template<class T>
using make_variant_t = typename make_variant<T>::type;