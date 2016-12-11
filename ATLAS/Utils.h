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

template<class... Targs1, class... Targs2>
struct merge_variant<std::variant<Targs1...>, std::variant<Targs2...>>
{
    using type = std::variant<Targs1..., Targs2...>;
};

template<class... Targs1, class... Targs2, class... Trest>
struct merge_variant<std::variant<Targs1...>, std::variant<Targs2...>, Trest...>
{
    using type = typename merge_variant<std::variant<Targs1..., Targs2...>, Trest...>::type;
};

template<class... T>
using merge_variant_t = typename merge_variant<T...>::type;