#pragma once

#include "CoreMinimal.h"

#include <vector>

// Struct to represent grid locations
/*
 *     -x
 *      |
 * -y --+-- +y
 *      |
 *     +x
*/
struct location_t {
    int32 x{ 0 };
    int32 y{ 0 };

    location_t operator+(const location_t& other) const {
        return location_t{ x + other.x, y + other.y };
    }

    location_t operator-(const location_t& other) const {
        return location_t{ x - other.x, y - other.y };
    }

    bool operator==(const location_t& other) const {
        return x == other.x && y == other.y;
    }

    void DebugPrint() const {
        FString f = TEXT("");
        f.AppendInt(x);
        f.Append(" ");
        f.AppendInt(y);
        GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, f);
    }
};

// Define 4 cardinal directions in terms of location offset. 
typedef location_t EDir;
static constexpr EDir E_TOP = { -1, 0 };
static constexpr EDir E_BOTTOM = { 1, 0 };
static constexpr EDir E_LEFT = { 0, -1 };
static constexpr EDir E_RIGHT = { 0, 1 };

struct EDirHash {
    std::size_t operator()(const EDir& dir) const noexcept {
        return std::hash<int32_t>{}(dir.x) ^ (std::hash<int32_t>{}(dir.y) << 1);
    }
};

/**
 * Represent a 2D array.
 * The 2D array is stored in a single array, to improve cache usage.
 */
template <typename T> class Array2D {

public:
    /**
     * Height and width of the 2D array.
     */
    std::size_t height;
    std::size_t width;

    Array2D() {}

    void Init(location_t size, T value) {
        height = size.x;
        width = size.y;
        data.resize(size.x * size.y);
    }

    Array2D(location_t size)
        : height(size.x), width(size.y), data(size.x * size.y) {}

    Array2D(location_t size, T value)
        : height(size.x), width(size.y), data(size.x * size.y, value) {}

    Array2D(location_t size, const std::vector<std::vector<T>> values)
        : height(size.x), width(size.y), data(size.x* size.y) {
        for (size_t i = 0; i < values.size(); i++) for (size_t j = 0; j < values[i].size(); j++)
            get(i, j) = values[i][j];
    }

    void DebugPrint() const {
        for (size_t i = 0; i < height; i++) {
            FString f = TEXT("");
            for (size_t j = 0; j < width; j++)
                f.AppendChar(get(i, j));
            GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, f);
        }
    }

    T& get(location_t location) {
        return get(location.x, location.y);
    }

    // Return defauult value if location is out of bounds.
    T& safe_get(location_t location, T &def) {
        if (location.x < 0 || location.y < 0 || location.x >= height || location.y >= width)
            return def;
        return get(location.x, location.y);
    }

    const T& safe_get(location_t location, const T &def) const {
        if (location.x < 0 || location.y < 0 || location.x >= height || location.y >= width)
            return def;
        return get(location.x, location.y);
    }

    const T& get(location_t location) const {
        return get(location.x, location.y);
    }

    /**
     * The array containing the data of the 2D array.
     */
    std::vector<T> data;

    /**
     * Build a 2D array given its height and width.
     * All the array elements are initialized to default value.
     */
    Array2D(std::size_t height, std::size_t width) noexcept
        : height(height), width(width), data(width* height) {}

    /**
     * Build a 2D array given its height and width.
     * All the array elements are initialized to value.
     */
    Array2D(std::size_t height, std::size_t width, T value) noexcept
        : height(height), width(width), data(width* height, value) {}

    /**
     * Return a const reference to the element in the i-th line and j-th column.
     * i must be lower than height and j lower than width.
     */
    const T& get(std::size_t i, std::size_t j) const noexcept {
        return data[j + i * width];
    }

    /**
     * Return a reference to the element in the i-th line and j-th column.
     * i must be lower than height and j lower than width.
     */
    T& get(std::size_t i, std::size_t j) noexcept {
        check(i >= 0);
        check(j >= 0);
        check(i < height);
        check(j < width);
        return data[j + i * width];
    }

    /**
     * Return the current 2D array reflected along the x axis.
     */
    Array2D<T> reflected() const noexcept {
        Array2D<T> result = Array2D<T>(width, height);
        for (std::size_t y = 0; y < height; y++) {
            for (std::size_t x = 0; x < width; x++) {
                result.get(y, x) = get(y, width - 1 - x);
            }
        }
        return result;
    }

    /**
     * Return the current 2D array rotated 90° anticlockwise
     */
    Array2D<T> rotated() const noexcept {
        Array2D<T> result = Array2D<T>(width, height);
        for (std::size_t y = 0; y < width; y++) {
            for (std::size_t x = 0; x < height; x++) {
                result.get(y, x) = get(x, width - 1 - y);
            }
        }
        return result;
    }

    /**
     * Return the sub 2D array starting from (y,x) and with size (sub_width,
     * sub_height). The current 2D array is considered toric for this operation.
     */
    Array2D<T> get_sub_array(std::size_t y, std::size_t x, std::size_t sub_width,
        std::size_t sub_height) const noexcept {
        Array2D<T> sub_array_2d = Array2D<T>(sub_width, sub_height);
        for (std::size_t ki = 0; ki < sub_height; ki++) {
            for (std::size_t kj = 0; kj < sub_width; kj++) {
                sub_array_2d.get(ki, kj) = get((y + ki) % height, (x + kj) % width);
            }
        }
        return sub_array_2d;
    }

    Array2D<T> center_crop(std::size_t amount) const noexcept {
        Array2D<T> sub_array_2d = Array2D<T>(height - amount*2, width - amount*2);
        for (std::size_t ki = 0; ki < height - amount*2; ki++)
            for (std::size_t kj = 0; kj < width - amount*2; kj++)
                sub_array_2d.get(ki, kj) = get(ki+amount, kj+amount);
        return sub_array_2d;
    }

    /**
     * Check if two 2D arrays are equals.
     */
    bool operator==(const Array2D<T>& a) const noexcept {
        if (height != a.height || width != a.width) {
            return false;
        }

        for (std::size_t i = 0; i < data.size(); i++) {
            if (a.data[i] != data[i]) {
                return false;
            }
        }
        return true;
    }
};

/**
 * Hash function.
 */
namespace std {
    template <typename T> class hash<Array2D<T>> {
    public:
        std::size_t operator()(const Array2D<T>& a) const noexcept {
            std::size_t seed = a.data.size();
            for (const T& i : a.data) {
                seed ^= hash<T>()(i) + (std::size_t)0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
} // namespace std
