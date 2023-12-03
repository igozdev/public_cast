# public_cast
> Compile time string literal encryptor for C++.

[![license][badge.license]][license]
[![release][badge.release]][release]
[![godbolt][badge.godbolt]][godbolt]

[badge.license]: https://img.shields.io/badge/license-mit-green.svg
[license]: https://github.com/igozdev/public_cast/blob/main/LICENSE
[badge.release]: https://img.shields.io/github/release/igozdev/public_cast.svg
[release]: https://github.com/igozdev/public_cast/releases/latest
[badge.godbolt]: https://img.shields.io/badge/try_it-on_godbolt-indigo.svg
[godbolt]: https://godbolt.org/z/PKnK86Pj7

* [Usage](#usage)
* [Example](#example)

# Usage
## pc::accessor
`pc::accessor` allows for a private member to be accessed via template instantiation, taking the template parameters `<TagType, Member>`.
#### Example, creating an accessor for class `foo`'s private member `bar` with a new tag named `my_tag`:
```c++
template struct pc::accessor<class my_tag, &foo::bar>;
```
> [!WARNING]
> A unique tag should be used for each template instantiation so that in the case of creating accessors for multiple class members of the same type, values are not overwritten.

## pc::public_cast
`pc::public_cast` takes the template parameters `<TagType, MemberType, ObjectType (deduced)>` and 1 argument, the object from which the member is accessed. It returns a reference to the member if it is a member object, or a callable object which calls and returns the result of the member function if it is a member function.
#### Example, accessing object `f` of type `foo`'s private `double` member object `bar` instantiated with tag `foo(bar)`, and private `char(int, float)` member function `baz` instantiated with tag `foo(baz)`:
```c++
int x = pc::public_cast<foo(bar), double>(f);
char y = pc::public_cast<foo(baz), char(int, float)>(f)(10, 5.3f); 
```
> [!IMPORTANT]
> An appropriate template instantiation of `pc::accessor` must be created for each private member that may be accessed by `pc::public_cast` or any other `pc` function.

## pc::publicize & pc::publicize_static
`pc::publicize` takes the template parameters `<TagType, MemberType, ObjectType>` and returns the member pointer of that private member.<br>
`pc::publicize_static` takes the template parameters `<TagType, MemberType, ObjectType>` and returns the pointer to that private static member.
#### Example, accessing the member pointer to `foo`'s private `std::string` member `bar` instantiated with tag `foo(bar)`, and the pointer to `foo`'s private static `float` member `baz` instantiated with tag `foo(baz)`:
```c++
std::string foo::* x = pc::publicize<foo(bar), std::string, foo>();
float* y = pc::publicize_static<foo(baz), float, foo>();
```
## pc::modified & pc::modifier
Certain member functions may have cv or reference qualifiers, and thus their type cannot be described by the general `ReturnType(ArgTypes...)` format. In this case, `pc::modified` is used. `pc::modified` is a type taking the template parameters `<FunctionType, Modifiers>` where `Modifiers` is a value of type `pc::modifier`, and may be used as the member type in `pc::public_cast` or `pc::publicize`.<br>
`pc::modifier` defines the following flags: `pc::modifier::const_`, `pc::modifier::volatile_`, `pc::modifier::lvalue`, and `pc::modifier::rvalue`, of which multiple may be combined using the `|` operator.
#### Example, calling object `f` of type `foo`'s private member function `void bar(int) const &&` instantiated with tag `foo(bar)`:
```c++
pc::public_cast<foo(bar), pc::modified<void(int), pc::modifier::const_ | pc::modifier::rvalue>>(f)(5);`
```

# Example
```c++
#include <public_cast.hpp>
#include <iostream>

class foo // example class
{
private:
    int x = 99;
    inline static int s = 10;

    float bar(int v)
    {
        std::cout << "bar called with " << v << std::endl;
        return static_cast<float>(v) / 2.f;
    }
    static void baz(float v)
    {
        std::cout << "baz called with " << v << std::endl;
    }

    std::string special() const &
    {
        return std::string("special!");
    }
};

namespace tag // instantiate pc::accessor template for private members
{
    template struct pc::accessor<foo(class x), &foo::x>;
    template struct pc::accessor<foo(class s), &foo::s>;
    template struct pc::accessor<foo(class bar), &foo::bar>;
    template struct pc::accessor<foo(class baz), &foo::baz>;
    template struct pc::accessor<foo(class special), &foo::special>;
}

int main()
{
    foo f;

    std::cout << pc::public_cast<foo(tag::x), int>(f) << std::endl; // print `f.x`
    pc::public_cast<foo(tag::x), int>(f) += 5; // add 5 to `f.x`
    std::cout << pc::public_cast<foo(tag::x), int>(f) << std::endl; // print `f.x` again

    float result = pc::public_cast<foo(tag::bar), float(int)>(f)(55); // call `f.bar(55)` and store result
    std::cout << result << std::endl;

    std::string s = pc::public_cast<foo(tag::special), pc::modified<std::string(void), pc::modifier::const_ | pc::modifier::lvalue>>(f)(); // call `f.special()` and store result
    std::cout << s << std::endl;

    std::cout << pc::public_cast<foo(tag::s), int, foo>() << std::endl; // print `foo::s`
    pc::public_cast<foo(tag::baz), void(float), foo>()(3.14f); // call `foo::baz(3.14f)`

    foo new_f;
    auto f_x_ptr = pc::publicize<foo(tag::x), int, foo>();
    std::cout << new_f.*f_x_ptr << std::endl; // print `new_f.x` using member pointer

    std::cout << *pc::publicize_static<foo(tag::s), int, foo>() << std::endl; // print `foo::s` using pointer
}

////////////////// Possible output: //////////////////
// 99
// 104
// bar called with 55
// 27.5
// special!
// 10
// baz called with 3.14
// 99
// 10
```
