#include <iostream>
#include <utility>
#include <array>
#include <cctype>
#include <random>
#include <chrono>
#include <tuple>
#include <string_view>

template<int N>
using c_int = std::integral_constant<int, N>;

template<int Char>
constexpr bool
is_a_whitespace_char(c_int<Char>)
{
    if constexpr (Char == ' ') return true;
    if constexpr (Char == '\n') return true;
    if constexpr (Char == '\t') return true;
    if constexpr (Char == '\r') return true;
    return false;
}

template<int...Is>
constexpr auto
make_whitespace_flags(std::integer_sequence<int, Is...>)
{
    return std::array<bool, sizeof...(Is)>{
        is_a_whitespace_char(c_int<Is>())...
    };
}

constexpr
std::array<bool, 256>
make_whitespace_flags()
{
    return make_whitespace_flags(std::make_integer_sequence<int, 256>());
}

enum mode
{
    use_table,
    use_branches,
    use_switch,
    use_crt
};

template<mode M>
struct impl_is_white;

template<>
struct impl_is_white<use_table>
{
    bool
    operator()(char c) const
    {
        const static auto tab = make_whitespace_flags();
        return tab[static_cast<unsigned>(c)];
    }
};

template<>
struct impl_is_white<use_branches>
{
    bool
    operator()(char c) const
    {
        return c == ' ' || c == '\n' || c == '\t' || c == '\r';
    }
};

template<>
struct impl_is_white<use_switch>
{
    bool
    operator()(char c) const
    {
        switch (c)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\r':return true;
        }
        return false;
    }
};

template<>
struct impl_is_white<use_crt>
{
    bool
    operator()(char c) const
    {
        return bool(std::isspace(static_cast<unsigned char>(c)));
    }
};

std::string
make_random_chars(std::size_t length = 100'000'000)
{
    std::string result;
    result.reserve(length);

    auto rng = std::random_device();
    auto seq = std::seed_seq({rng(), rng(), rng(), rng(), rng()});
    auto rnd = std::default_random_engine(seq);
    auto candidates = std::string("\n\t\r ,[]\"abcdefghijklmnop:{}");
    auto dist = std::uniform_int_distribution<int>(0, candidates.size() - 1);
    while (length--)
    {
        result += candidates[dist(rnd)];
    }
    return result;
}

std::string make_alternating(std::size_t length = 100'000'000)
{
    std::string result;
    result.reserve(length);

    auto rng = std::random_device();
    auto seq = std::seed_seq({rng(), rng(), rng(), rng(), rng()});
    auto rnd = std::default_random_engine(seq);
    auto candidates = std::string("\n\t\r ");
    auto dist = std::uniform_int_distribution<int>(0, candidates.size() - 1);
    bool flipflop = false;
    while (length--)
    {
        if (length & 1)
            result += candidates[dist(rnd)];
        else
            result += 'a';
    }
    return result;

}

template<mode Mode>
std::tuple<std::chrono::high_resolution_clock::duration, std::size_t>
time_count(std::string_view sample)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto cnt = std::count_if(sample.begin(), sample.end(), impl_is_white<Mode>());
    auto stop = std::chrono::high_resolution_clock::now();
    return std::make_tuple(stop - start, cnt);
}

void run_test(std::string_view title, std::string_view sample)
{
    std::cout << title << '\n';
    std::cout << std::string(title.size(), '=') << '\n';
    auto [t, cnt] = time_count<use_branches>(sample);
    std::cout << "use_branches : " << cnt << " found in " << t.count() << "ns\n";
    std::tie(t, cnt) = time_count<use_table>(sample);
    std::cout << "use_table    : " << cnt << " found in " << t.count() << "ns\n";
    std::tie(t, cnt) = time_count<use_switch>(sample);
    std::cout << "use_switch   : " << cnt << " found in " << t.count() << "ns\n";
    std::tie(t, cnt) = time_count<use_crt>(sample);
    std::cout << "use_crt      : " << cnt << " found in " << t.count() << "ns\n";
}

int
main()
{
    auto sample = make_random_chars();
    run_test("Random Characters", make_random_chars());
    std::cout << '\n';
    run_test("Alternating Whitespace", make_alternating());

    return 0;
}
