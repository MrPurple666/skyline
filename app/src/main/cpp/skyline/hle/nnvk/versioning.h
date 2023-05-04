#pragma once

#include <cstring>
#include <stdexcept>
#include "types.h"

#define NNVK_VERSIONED_STRUCT(type, ...) \
        class type; \
        template<> \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wunused-local-typedef\"") \
        constexpr size_t VersionedSize<type>(ApiVersion version) { \
            using E = internal::ApiVersionAndSize; \
            return internal::MapApiVersionToSize<__VA_ARGS__>(version); \
        } \
        _Pragma("clang diagnostic pop")  \
        static_assert(sizeof(type) <= VersionedSize<type>(ApiVersion(0, 0)), "Versioned struct is too small"); \
        union type##InternalHolder {     \
            type value; \
            u8 storage[VersionedSize<type>(ApiVersion(0, 0))];         \
            template<typename... Args>   \
            type##InternalHolder(Args &&... args) : value(ApiVersion(0, 0), std::forward<Args>(args)...) {}    \
            ~type##InternalHolder() { value.~type(); }                 \
            type *operator->() { return &value; }                         \
            type &operator*() { return value; }                          \
        };



#define NNVK_FILL_VERSIONED_STRUCT(type) \
        std::memset(this + 1, 0, VersionedSize<type>(version) - sizeof(type));

namespace nnvk {
    /**
     * @brief Holds a version of the NVN API
     */
    struct ApiVersion {
        int major;
        int minor;

        constexpr ApiVersion(int major, int minor) : major(major), minor(minor) {}

        constexpr bool operator<(const ApiVersion &rhs) const {
            return major < rhs.major || (major == rhs.major && minor < rhs.minor);
        }

        constexpr bool operator==(const ApiVersion &rhs) const {
            return major == rhs.major && minor == rhs.minor;
        }

        constexpr bool operator<=(const ApiVersion &rhs) const {
            return rhs < *this || *this == rhs;
        }

        constexpr bool operator>(const ApiVersion &rhs) const {
            return !(rhs <= *this);
        }

        constexpr bool operator>=(const ApiVersion &rhs) const {
            return !(rhs < *this);
        }
    };

    /**
     * @brief Allows defining versions as "maj.min"_V
     */
    constexpr ApiVersion operator"" _V(const char *str, size_t size) {
        int major{};
        int minor{};
        int i{};

        for (; i < size && str[i] != '.'; i++) {
            major = major * 10 + (str[i] - '0');
        }

        for (i++; i < size; i++) {
            minor = minor * 10 + (str[i] - '0');
        }

        return ApiVersion{major, minor};
    }

    namespace internal {
        struct ApiVersionAndSize {
            ApiVersion version;
            size_t size;
        };

        /**
         * @brief Maps an input API version to a size using a list of version and size pairs, such that the size of the largest version less than or equal to the input version is returned
         * @tparam verSizes A list of version and size pairs
         */
        template<size_t DefaultVer, ApiVersionAndSize... VersionDescs>
        constexpr size_t MapApiVersionToSize(ApiVersion version) {
            size_t size{DefaultVer};
            ((version >= VersionDescs.version ? size = VersionDescs.size : size), ...);
            return size;
        }
    }

    /**
     * @brief Acts as a sizeof() for versioned structs, 'NNVK_VERSIONED_STRUCT' is used to specialise this
     */
    template<typename T>
    size_t VersionedSize(ApiVersion version) {
        throw std::runtime_error("VersionedSize not implemented for this type");
    }
}