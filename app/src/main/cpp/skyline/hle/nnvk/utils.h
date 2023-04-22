#pragma once

#include <cstddef>
#include <type_traits>
#include <xxhash.h>

namespace nnvk::util {
    /**
     * @return The value aligned up to the next multiple
     * @note The multiple **must** be a power of 2
     */
    template<typename TypeVal>
    requires std::is_unsigned_v<TypeVal>
    constexpr TypeVal AlignUp(TypeVal value, size_t multiple) {
        multiple--;
        return (value + multiple) & ~(multiple);
    }

    /**
     * @return The value aligned up to the next multiple, the multiple is not restricted to being a power of two (NPOT)
     * @note This will round away from zero for negative numbers
     * @note This is costlier to compute than the power of 2 version, it should be preferred over this when possible
     */
    template<typename TypeVal>
    requires std::is_integral_v<TypeVal>
    constexpr TypeVal AlignUpNpot(TypeVal value, ssize_t multiple) {
        return ((value + multiple - 1) / multiple) * multiple;
    }

    /**
     * @return The value aligned down to the previous multiple
     * @note The multiple **must** be a power of 2
     */
    template<typename TypeVal>
    requires std::is_unsigned_v<TypeVal>
    constexpr TypeVal AlignDown(TypeVal value, size_t multiple) {
        return value & ~(multiple - 1);
    }

    /**
     * @return The value of division rounded up to the next integral
     */
    template<typename Type>
    requires std::is_integral_v<Type>
    constexpr Type DivideCeil(Type dividend, Type divisor) {
        return (dividend + divisor - 1) / divisor;
    }

    template<typename TypeVal>
    requires std::is_unsigned_v<TypeVal>
    constexpr bool IsAligned(TypeVal value, size_t multiple) {
        if ((multiple & (multiple - 1)) == 0)
            return !(value & (multiple - 1U));
        else
            return (value % multiple) == 0;
    }

    /**
    * @brief A fast hash for any trivial object that is designed to be utilized with hash-based containers
    */
    template<typename T>
    struct ObjectHash {
        std::size_t operator()(const T &object) const {
            return XXH64(&object, sizeof(object), 0);
        }
    };
}