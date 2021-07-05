#include <vector>
#include <functional>
#include <type_traits>
#include <concepts>
#include <stdio.h>

/*!
 *  @note   템플릿 특수화 규칙 : 사용자 정의 타입이 아니면 namespace std에 템플릿 특수화는 불법이다. 벡터는 사용자 정의 타입이 아니다.
 *          다음은 템플릿 특수화 규칙에 따라 새로운 namespace를 만들고 hash 함수를 정의한다. 
 *          아마도 C++17 버전부터 빌드 가능
 *
 *          ref : https://stackoverflow.com/questions/47964496/specialization-of-class-for-vectors-of-specific-type-trait
 */
namespace helper {
    template<typename T, typename = void>
    struct hash : std::hash<T> {}; // 템플릿 파라미터를 추가하면서, std::hash<T>의 기능을 그대로 사용한다.
        
    template <class T>
    inline void hash_combine(std::size_t& seed, T const& v)
    {
        seed ^= helper::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

namespace helper {
    template <typename T>
    struct hash<std::vector<T>, std::enable_if_t<std::is_same_v<T, bool> == false>> {
        using argument_type = std::vector<T>;
        using result_type = size_t;

        result_type operator()(argument_type const& in) const
        {
            std::size_t size = in.size();
            std::size_t seed = 0;

            for (std::size_t i = 0; i < size; i++)
                hash_combine(seed, in[i]);

            return seed;
        }
    };
}

/*!
 *  @note   다음은 템플릿 특수화 규칙을 무시하고 namespace std에 템플릿 특수화를 한다.
 *          requires 키워드는 std::hash<bool>에 대한 모호함을 해결한다.
 *          C++17에서는 std::enable_if를 std::hash에 적용할 수 없다.
 *          std::hash는 단 하나의 템플릿 파라미터만 받게 설계되어 있기 때문에, std::enable_if 용 템플릿 파라미터를 추가할 수 없다.
 */
#if __cplusplus > 201703L // C++20 이상
template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
    template<typename T>
    requires (is_same_v<T, bool> == false) // available in C++20
    struct hash<vector<T>>
    {
        using argument_type = vector<T>;
        using result_type = size_t;

        result_type operator()(argument_type const& in) const
        {
            size_t size = in.size();
            size_t seed = 0;

            for (size_t i = 0; i < size; i++)
                hash_combine(seed, in[i]);

            return seed;
        }
    };
}
#endif

int main() {
    std::vector<bool> my_bool_data{true, false, true, false, false, true, true};
    std::vector<char> my_char_data{'a', 'b', 'c'};

    size_t hash_value;
    
    hash_value = helper::hash<std::vector<bool>>{}(my_bool_data);
    printf("from helper hash bool : %#018lx\n", hash_value);

    hash_value = std::hash<std::vector<bool>>{}(my_bool_data);
    printf("from std hash bool    : %#018lx\n", hash_value);

    hash_value = helper::hash<std::vector<char>>{}(my_char_data);
    printf("from helper hash bool : %#018lx\n", hash_value);

    hash_value = std::hash<std::vector<char>>{}(my_char_data);
    printf("from std hash bool    : %#018lx\n", hash_value);

    return 0;
}