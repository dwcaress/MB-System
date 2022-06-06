
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef FLAG_UTILS_HPP  // include guard
#define FLAG_UTILS_HPP
#include <iostream>

// flag_var provides a uniform API for flag variables of different sizes
template <typename T>
class flag_var
{
public:
    flag_var();
    flag_var(T t);
    flag_var(const flag_var<T> &t);

    T get();
    void set(T t);
    bool any_set(T mask);
    bool all_set(T mask);
    bool all_clr(T mask);
    bool is_set(T mask);
    bool is_clr(T mask);
    void operator=(T rhs) {set(rhs);}

private:
    T mFlags;
};

template <typename T>
flag_var<T>::flag_var()
{
    mFlags = 0;
}

template <typename T>
flag_var<T>::flag_var(T t)
{
    mFlags = t;
}

template <typename T>
flag_var<T>::flag_var(const flag_var<T> &t)
{
    mFlags = t.mFlags;
}

template <typename T>
T flag_var<T>::get()
{
    return (mFlags);
}

template <typename T>
void flag_var<T>::set(T t)
{
    mFlags = t;
}

template <typename T>
bool flag_var<T>::any_set(T mask)
{
    return ((mFlags&mask)!=0);
}

template <typename T>
bool flag_var<T>::all_set(T mask)
{
    return ((mFlags&mask)==mask);
}

template <typename T>
bool flag_var<T>::is_set(T mask)
{
    return all_set(mask);
}

template <typename T>
bool flag_var<T>::all_clr(T mask)
{
    return ((mFlags&mask)==0);
}

template <typename T>
bool flag_var<T>::is_clr(T mask)
{
    return all_clr(mask);
}

// bitwise operators
template<typename T, typename U>
T operator<<(flag_var<T> lhs, U rhs) {return (lhs.get() << rhs);}
template<typename T, typename U>
T operator>>(flag_var<T> lhs, U rhs) {return (lhs.get() >> rhs);}
template<typename T>
T operator&(flag_var<T> lhs, flag_var<T> rhs) {return (lhs.get() & rhs.get());}
template<typename T, typename U>
T operator&(flag_var<T> lhs, U rhs) {return (lhs.get() & rhs);}
template<typename T>
T operator|(flag_var<T> lhs, flag_var<T> rhs) {return (lhs.get() | rhs.get());}
template<typename T, typename U>
T operator|(flag_var<T> lhs, U rhs) {return (lhs.get() | rhs);}

// comparison operators
template<typename T, typename U>
bool operator==(flag_var<T> lhs, U rhs) { return (lhs.get() == rhs);}
template<typename T>
bool operator==(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() == rhs.get());}
template<typename T, typename U>
bool operator!=(flag_var<T> lhs, U rhs) { return (lhs.get() != rhs);}
template<typename T>
bool operator!=(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() != rhs.get());}
template<typename T, typename U>
bool operator>(flag_var<T> lhs, U rhs) { return (lhs.get() > rhs);}
template<typename T>
bool operator>(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() > rhs.get());}
template<typename T, typename U>
bool operator<(flag_var<T> lhs, U rhs) { return (lhs.get() < rhs);}
template<typename T>
bool operator<(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() < rhs.get());}
template<typename T, typename U>
bool operator>=(flag_var<T> lhs, U rhs) { return (lhs.get() >= rhs);}
template<typename T>
bool operator>=(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() >= rhs.get());}
template<typename T, typename U>
bool operator<=(flag_var<T> lhs, U rhs) { return (lhs.get() <= rhs);}
template<typename T>
bool operator<=(flag_var<T> lhs, flag_var<T> rhs) { return (lhs.get() <= rhs.get());}

// assignment operators
template<typename T, typename U>
flag_var<T> operator<<=(flag_var<T> &lhs, U rhs) { lhs.set((lhs.get() << rhs));return lhs;}
template<typename T, typename U>
flag_var<T> operator>>=(flag_var<T> &lhs, U rhs) { lhs.set(lhs >> rhs);return lhs;}

template<typename T, typename U>
flag_var<T> &operator&=(flag_var<T> &lhs, U rhs) { lhs.set((lhs.get() & rhs)); return lhs;}
template<typename T, typename U>
flag_var<T> &operator|=(flag_var<T> &lhs, U rhs) { lhs.set((lhs.get() | rhs)); return lhs;}
template<typename T>
flag_var<T> & operator&=(flag_var<T> &lhs, flag_var<T> rhs) { lhs.set((lhs.get() & rhs.get())); ; return lhs;}
template<typename T>
flag_var<T> & operator|=(flag_var<T> &lhs, flag_var<T> rhs) { lhs.set((lhs.get() | rhs.get()));; return lhs;}

// ioostream operators
template<class T>
std::ostream& operator<<(std::ostream& out, flag_var<T>& obj)
{
    out << obj.get();
    return out;
}
#endif

