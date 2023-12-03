/*
* MIT License
*
* Copyright (c) 2023 igozdev
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <type_traits>

namespace pc
{
	namespace detail
	{
		template <class Tag, class T>
		struct storer
		{
			inline static T value;
		};

		template <class T>
		struct holder
		{
		};
	}

	template <class Tag, auto V>
	struct accessor
	{
		inline static const auto value = detail::storer<Tag, decltype(V)>::value = V;
	};

	enum class modifier
	{
		const_ = 1 << 0,
		volatile_ = 1 << 1,
		lvalue = 1 << 2,
		rvalue = 1 << 3,
	};
	constexpr modifier operator|(modifier lhs, modifier rhs)
	{
		return static_cast<modifier>(static_cast<std::underlying_type_t<modifier>>(lhs) | static_cast<std::underlying_type_t<modifier>>(rhs));
	}
	template <class T, modifier Mods>
	struct modified
	{
		using type = T;
		constexpr static modifier mods = Mods;
	};
	template <class T>
	concept Modified = std::is_same_v<T, modified<typename T::type, T::mods>>;

	template <class Tag, class T, class ClassT>
	auto public_cast(ClassT& t) requires std::is_function_v<T>
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				return
					[&t](TArgs... args) {
						return (t.*detail::storer<Tag, T ClassT::*>::value)(args...);
					};
			}(detail::holder<T>{});
	}
	template <class Tag, class T, class ClassT>
	auto& public_cast(ClassT& t) requires (!std::is_function_v<T> && !Modified<T>)
	{
		return t.*(detail::storer<Tag, T ClassT::*>::value);
	}
	template <class Tag, class T, class ClassT>
	auto& public_cast()
	{
		return *detail::storer<Tag, T*>::value;
	}

	template <class Tag, class T, class ClassT>
	auto publicize() requires (!Modified<T>)
	{
		return detail::storer<Tag, T ClassT::*>::value;
	}
	template <class Tag, class T, class ClassT>
	auto publicize_static()
	{
		return detail::storer<Tag, T*>::value;
	}

	template <class Tag, Modified T, class ClassT>
	auto public_cast(const ClassT& t) requires (T::mods == modifier::const_)
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(volatile ClassT& t) requires (T::mods == modifier::volatile_)
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(ClassT& t) requires (T::mods == modifier::lvalue)
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) &;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(ClassT&& t) requires (T::mods == modifier::rvalue)
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) &&;
				return
					[&t](TArgs... args) {
					return (std::move(t).*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(const volatile ClassT& t) requires (T::mods == (modifier::const_ | modifier::volatile_))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(const ClassT& t) requires (T::mods == (modifier::const_ | modifier::lvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const &;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(const ClassT&& t) requires (T::mods == (modifier::const_ | modifier::rvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const &&;
				return
					[&t](TArgs... args) {
					return (std::move(t).*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(volatile ClassT& t) requires (T::mods == (modifier::volatile_ | modifier::lvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile &;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(volatile ClassT&& t) requires (T::mods == (modifier::volatile_ | modifier::rvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile &&;
				return
					[&t](TArgs... args) {
					return (std::move(t).*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(const volatile ClassT& t) requires (T::mods == (modifier::const_ | modifier::volatile_ | modifier::lvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile &;
				return
					[&t](TArgs... args) {
					return (t.*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto public_cast(const volatile ClassT&& t) requires (T::mods == (modifier::const_ | modifier::volatile_ | modifier::rvalue))
	{
		return
			[&t]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile &&;
				return
					[&t](TArgs... args) {
					return (std::move(t).*detail::storer<Tag, ptr_type>::value)(args...);
			};
		}(detail::holder<typename T::type>{});
	}

	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == modifier::const_)
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == modifier::volatile_)
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == modifier::lvalue)
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) &;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == modifier::rvalue)
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) &&;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::const_ | modifier::volatile_))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::const_ | modifier::lvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const &;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::const_ | modifier::rvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const &&;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::volatile_ | modifier::lvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile &;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::volatile_ | modifier::rvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) volatile &&;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::const_ | modifier::volatile_ | modifier::lvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile &;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
	template <class Tag, Modified T, class ClassT>
	auto publicize() requires (T::mods == (modifier::const_ | modifier::volatile_ | modifier::rvalue))
	{
		return
			[]<class TR, class... TArgs>(detail::holder<TR(TArgs...)>) {
				using ptr_type = TR(ClassT::*)(TArgs...) const volatile &&;
				return detail::storer<Tag, ptr_type>::value;
			}(detail::holder<typename T::type>{});
	}
}