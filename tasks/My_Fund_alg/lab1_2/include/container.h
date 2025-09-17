#pragma once

#include <vector>
#include <iostream>




#include <iostream>

template <typename T>
class Container {
public:
    Container() = default;
    Container(const Container& other) = default;
    virtual Container& operator=(const Container& other) = 0;

    virtual ~Container() = default;

//    virtual T* begin() = 0;
//    virtual const T* cbegin() const = 0;
//    virtual T* end() = 0;
//    virtual const T* cend() const = 0;

    virtual bool operator==(const Container& other) const = 0;
    virtual bool operator!=(const Container& other) const = 0;

    virtual std::size_t size() const = 0;
    virtual std::size_t max_size() const = 0;
    virtual bool empty() const = 0;
};
