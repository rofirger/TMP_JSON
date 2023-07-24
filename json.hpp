/*
* @brief:
* @BUG fix:
*	1. 修复函数未先声明导致编译器无法探寻正确的函数定义BUG
*/
#pragma once
#include <iostream>
#include <sstream>
#include <tuple>
#include <string>
#include <cstring>
#include <list>
#include <vector>
#include <deque>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <utility>
#include <memory>

namespace JSON {
	template<typename _Ty, _Ty _V>
	struct integral_constant {
		static constexpr _Ty value = _V;
		using value_type = _Ty;
		using type = integral_constant;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	template<bool B> using bool_constant = integral_constant<bool, B>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	/* 实现is_same */
	template<typename _First, typename _Second>
	struct is_same : false_type {};
	template<typename _Ty>
	struct is_same<_Ty, _Ty> : true_type {};
	template<typename _First, typename _Second>
	static constexpr bool is_same_v = is_same<_First, _Second>::value;

	/* 实现is_one_of */
	template<typename _Ty, typename ..._Args>
	struct is_one_of : false_type {};
	template<typename _First, typename _Second, typename ..._Args>
	struct is_one_of<_First, _Second, _Args...> : is_one_of<_First, _Args...> {};
	template<typename _First, typename ..._Args>
	struct is_one_of<_First, _First, _Args...> : true_type {};
	template<typename _First, typename ..._Args>
	static constexpr bool is_one_of_v = is_one_of<_First, _Args...>::value;

	/* 实现is_integral，is_floating_point */
	template <typename _Ty>
	using is_integral = is_one_of<_Ty, bool, char, short, int, long, long long>;
	template <typename _Ty>
	static constexpr bool is_integral_v = is_integral<_Ty>::value;
	template <typename _Ty>
	using is_floating_point = is_one_of<_Ty, float, double>;
	template <typename _Ty>
	static constexpr bool is_floating_point_v = is_floating_point<_Ty>::value;

	/****** 实现type_identity ******/
	template<typename _Ty>
	struct type_identity { using type = _Ty; };

	// is_instantiation_of
	template <typename _Ty, template <typename...> typename _Tmp>
	struct is_instantiation_of : false_type {};
	template <template <typename...> typename _Tmp, typename... _Args>
	struct is_instantiation_of<_Tmp<_Args...>, _Tmp> : true_type {};
	// 注意这里不能是template <template <typename...> typename _Tmp1, 
	//	template <typename...> typename _Tmp2， typename... _Args>
	// 因为假如是std::vector<int>, 那这个类型不属于_Tmp1<_Args...>
	template <typename _Ty, template <typename...> typename _Tmp>
	static constexpr bool is_instantiation_of_v = is_instantiation_of<_Ty, _Tmp>::value;

	/* 实现enable_if */
	// 主模板默认_Ty是void
	template<bool _B, typename _Ty = void>
	struct enable_if : type_identity<_Ty> {};
	template<typename _Ty>
	struct enable_if<false, _Ty> {};
	// 注意这里一定不能漏typename，不然会出现奇怪的编译错误。
	template<bool _B, typename _Ty = void>
	using enable_if_t = typename enable_if<_B, _Ty>::type;

	/* 移除 CV */
	template<typename _Ty>
	struct remove_cv : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_cv<const _Ty> : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_cv<volatile _Ty> : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_cv<const volatile _Ty> : type_identity<_Ty> {};
	// for function
	template<typename _Ty, typename ..._Args>
	struct remove_cv<const _Ty(_Args...)> : type_identity<_Ty(_Args...)> {};
	template<typename _Ty, typename ..._Args>
	struct remove_cv<volatile _Ty(_Args...)> : type_identity<_Ty(_Args...)> {};
	template<typename _Ty, typename ..._Args>
	struct remove_cv<const volatile _Ty(_Args...)> : type_identity<_Ty(_Args...)> {};
	template<typename _Ty>
	using remove_cv_t = typename remove_cv<_Ty>::type;

	/* 移除 reference */
	template<typename _Ty>
	struct remove_reference : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_reference<_Ty&> : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_reference<_Ty&&> : type_identity<_Ty> {};
	// for function
	template<typename _Ty, typename ..._Args>
	struct remove_reference<_Ty& (_Args...)> : type_identity<_Ty(_Args...)> {};
	template<typename _Ty, typename ..._Args>
	struct remove_reference<_Ty && (_Args...)> : type_identity<_Ty(_Args...)> {};
	template<typename _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	/* 实现 is_array */
	template<typename _Ty>
	struct is_array : false_type {};
	template<typename _Ty>
	struct is_array<_Ty[]> : true_type {};
	template<unsigned _N, typename _Ty>
	struct is_array<_Ty[_N]> : true_type {};
	template<typename _Ty>
	static constexpr bool is_array_v = is_array<_Ty>::value;

	/* 实现remove_extent */
	template<typename _Ty>
	struct remove_extent : type_identity<_Ty> {};
	template<typename _Ty>
	struct remove_extent<_Ty[]> : type_identity<_Ty> {};
	template<unsigned _N, typename _Ty>
	struct remove_extent<_Ty[_N]> : type_identity<_Ty> {};
	template<typename _Ty>
	using remove_extent_t = typename remove_extent<_Ty>::type;

	/* 实现extent */
	template<typename _Ty, unsigned _N = 0>
	struct extent : integral_constant<std::size_t, 0> {};
	template<typename _Ty>
	struct extent<_Ty[], 0> : integral_constant<std::size_t, 0> {};
	template<typename _Ty, unsigned _N>
	struct extent<_Ty[], _N> : extent<_Ty, _N - 1> {};
	template<typename _Ty, std::size_t _S>
	struct extent<_Ty[_S], 0> : integral_constant<std::size_t, _S> {};
	template<typename _Ty, std::size_t _S, unsigned _N>
	struct extent<_Ty[_S], _N> : extent<_Ty, _N - 1> {};

	/* 实现is_function */
	template<typename _Ty>
	struct is_function : false_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)volatile> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const volatile> : true_type {};
	// 注意如何标识函数的返回值约束
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)volatile&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const volatile&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)&&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const&&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)volatile&&> : true_type {};
	template<typename _Ret, typename ..._Args>
	struct is_function<_Ret(_Args...)const volatile&&> : true_type {};
	template<typename _Ty>
	static constexpr bool is_function_v = is_function<_Ty>::value;

	/* 实现add_pointer */
	template <typename _Ty>
	struct add_pointer : type_identity<_Ty*> {};
	// for function
	template <typename _Ret, typename... _Args>
	struct add_pointer<_Ret(_Args...)> : type_identity<_Ret*> {};
	template <typename _Ty>
	using add_pointer_t = typename add_pointer<_Ty>::type;

	/* 实现is_pointer */
	template <typename _Ty>
	struct is_pointer : std::false_type {};
	template <typename _Ty>
	struct is_pointer<_Ty*> : std::true_type {};
	template<typename _Ty>
	static constexpr bool is_pointer_v = is_pointer<_Ty>::value;

	/* 实现conditional */
	template<bool _Bool_con, typename _First, typename _Second>
	struct conditional : type_identity<_First> {};
	template<typename _First, typename _Second>
	struct conditional<false, _First, _Second> : type_identity<_Second> {};
	template<bool _Bool_con, typename _First, typename _Second>
	using conditional_t = typename conditional<_Bool_con, _First, _Second>::type;

	/* 实现decay */
	template<typename _Ty>
	struct decay : type_identity <
		conditional_t<
		is_array_v<_Ty>,
		remove_extent_t<remove_reference_t<remove_cv_t<_Ty>>>,
		conditional_t<is_function_v<_Ty>, add_pointer_t<remove_reference_t<remove_cv_t<_Ty>>>,
		remove_reference_t<remove_cv_t<_Ty>>>
		>>{};
	template<typename _Ty>
	using decay_t = typename decay<_Ty>::type;

	/* 实现判断是否是左值引用 */
	template<typename _Ty>
	struct is_lvalue_reference : false_type {};
	template<typename _Ty>
	struct is_lvalue_reference<_Ty&> : true_type {};
	template<typename _Ty>
	struct is_lvalue_reference<_Ty&&> : false_type {};
	template<typename _Ty>
	static constexpr bool is_lvalue_reference_v = is_lvalue_reference<_Ty>::value;

	/* 实现判断是否是右值引用 */
	template<typename _Ty>
	struct is_rvalue_reference : false_type {};
	template<typename _Ty>
	struct is_rvalue_reference<_Ty&> : false_type {};
	template<typename _Ty>
	struct is_rvalue_reference<_Ty&&> : true_type {};
	template<typename _Ty>
	static constexpr bool is_rvalue_reference_v = is_rvalue_reference<_Ty>::value;

	/* 实现try_add_lvalue_reference */
	template<typename _Ty>
	conditional<is_lvalue_reference_v<_Ty>, _Ty, _Ty&> try_add_lvalue_reference() {};
	/* 实现添加左值引用 */
	template<typename _Ty>
	struct add_lvalue_reference :decltype(try_add_lvalue_reference<_Ty>()) {};
	template<typename _Ty>
	struct add_lvalue_reference<_Ty&&> {};
	template<typename _Ty>
	using add_lvalue_reference_t = typename add_lvalue_reference<_Ty>::type;

	/* 实现try_add_rvalue_reference */
	template<typename _Ty>
	conditional<is_rvalue_reference_v<_Ty>, _Ty, _Ty&&> try_add_rvalue_reference() {};
	/* 实现添加右值引用 */
	template<typename _Ty>
	struct add_rvalue_reference : decltype(try_add_rvalue_reference<_Ty>()){};
	template<typename _Ty>
	struct add_rvalue_reference<_Ty&> : type_identity<_Ty&&> {};
	template<typename _Ty>
	using add_rvalue_reference_t = typename add_rvalue_reference<_Ty>::type;

	/***********************
	* Function declaration *
	***********************/
	template<typename _Ty>
	enable_if_t<is_one_of_v<decay_t<_Ty>,
		int, long, long long, unsigned,
		unsigned long, unsigned long long,
		float, double, long double>, std::string>
		dumps(const _Ty& _value);

	// For std::string, char
	template<typename _Ty>
	enable_if_t<is_one_of_v<decay_t<_Ty>,
		char, std::string>, std::string>
		dumps(const _Ty& _obj);

	std::string dumps(const char* _s);

	template<typename _Ty>
	enable_if_t<is_one_of_v<decay_t<_Ty>,
		void, std::nullptr_t>, std::string >
		dumps(const _Ty& _t);

	template<typename _Ty>
	inline enable_if_t<is_one_of_v<decay_t<_Ty>, bool>, std::string>
		dumps(const _Ty& _b);

	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::vector> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::list> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::deque> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::forward_list> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::set> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::multiset> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_set> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_multiset>,
		std::string>
		dumps(const _Tmp<_Args...>& _obj);

	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::map> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::multimap> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_map> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_multimap>,
		std::string>
		dumps(const _Tmp<_Args...>& _obj);

	template<typename _First, typename _Second>
	std::string
		dumps(const std::pair<_First, _Second>& __obj);

	template<typename _Ty>
	enable_if_t<is_array_v<_Ty>, std::string>
		dumps(const _Ty& __arr);

	template <typename _Ty, std::size_t _N>
	std::string
		dumps(const std::array<_Ty, _N>& __obj);

	template<size_t _N, typename ..._Args>
	enable_if_t<_N == sizeof...(_Args) - 1, std::string>
		dumps(const std::tuple<_Args...>& __obj);

	template<size_t _N, typename ..._Args>
	enable_if_t<_N != 0 && _N != sizeof...(_Args) - 1, std::string>
		dumps(const std::tuple<_Args...>& __obj);

	template<size_t _N = 0, typename ..._Args>
	enable_if_t<_N == 0, std::string>
		dumps(const std::tuple<_Args...>& __obj);

	template<typename _Ty>
	enable_if_t<is_pointer_v<_Ty>, std::string>
		dumps(const _Ty __p);

	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::shared_ptr> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::weak_ptr> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unique_ptr>,
		std::string>
		dumps(const _Tmp<_Args...>& __p);
	/***********************************/

	/** 实现dumps **/
	template<typename _Ty>
	inline enable_if_t<is_one_of_v<decay_t<_Ty>,
		int, long, long long, unsigned,
		unsigned long, unsigned long long,
		float, double, long double>, std::string>
		dumps(const _Ty& _value) { return std::to_string(_value); }
	// For std::string, char
	template<typename _Ty>
	inline enable_if_t<is_one_of_v<decay_t<_Ty>,
		char, std::string>, std::string>
		dumps(const _Ty& _obj) {
		std::stringstream __ss;
		__ss << '"' << _obj << '"';
		return __ss.str();
	}
	// For char*
	inline std::string dumps(const char* _s) { return dumps(std::string(_s)); }
	// For void, nullptr
	template<typename _Ty>
	inline enable_if_t<is_one_of_v<decay_t<_Ty>,
		void, std::nullptr_t>, std::string >
		dumps(const _Ty& _t) { return "null"; }
	// For bool
	template<typename _Ty>
	inline enable_if_t<is_one_of_v<decay_t<_Ty>, bool>, std::string>
		dumps(const _Ty& _b) {
		return _b ? "true" : "false";
	}
	// For vector, list, deque, forward_list, set, multiset, unordered_set, unordered_multiset
	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::vector> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::list> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::deque> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::forward_list> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::set> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::multiset> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_set> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_multiset>,
		std::string>
		dumps(const _Tmp<_Args...>& _obj) {
		std::stringstream __ss;
		__ss << "[";
		for (auto __it = _obj.begin(); __it != _obj.end();) {
			__ss << dumps(*__it);
			if (++__it != _obj.end()) __ss << ", ";
		}
		__ss << "]";
		return __ss.str();
	}

	// For map, multimap, unordered_map, unordered_multimap
	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::map> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::multimap> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_map> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unordered_multimap>,
		std::string>
		dumps(const _Tmp<_Args...>& _obj) {
		std::stringstream __ss;
		__ss << "{";
		for (auto __it = _obj.begin(); __it != _obj.end(); ) {
			__ss << dumps(__it->first);
			__ss << ":";
			__ss << dumps(__it->second);
			if (++__it != _obj.end()) __ss << ", ";
		}
		__ss << "}";
		return __ss.str();
	}
	// For pair
	template<typename _First, typename _Second>
	std::string
		dumps(const std::pair<_First, _Second>& __obj) {
		std::stringstream __ss;
		__ss << "{" << dumps(__obj->first) << ":" << dumps(__obj->second) << "}";
		return __ss.str();
	}
	// For array
	template<typename _Ty>
	enable_if_t<is_array_v<_Ty>, std::string>
		dumps(const _Ty& __arr) {
		std::stringstream __ss;
		__ss << "[";
		for (std::size_t __i = 0; __i < extent<_Ty>::value; ++__i) {
			__ss << dumps(__arr[__i]);
			if (__i != extent<_Ty>::value - 1)	__ss << ", ";
		}
		__ss << "]";
		return __ss.str();
	}
	// For std::array
	template <typename _Ty, std::size_t _N>
	std::string
		dumps(const std::array<_Ty, _N>& __obj) {
		std::stringstream __ss;
		__ss << "[";
		for (auto __it = __obj.begin(); __it != __obj.end();) {
			__ss << dumps(*__it);
			if (++__it != __obj.end())	__ss << ", ";
		}
		__ss << "]";
		return __ss.str();
	}
	// For std::tuple
	template<size_t _N, typename ..._Args>
	enable_if_t<_N == sizeof...(_Args) - 1, std::string>
		dumps(const std::tuple<_Args...>& __obj) {
		std::stringstream __ss;
		__ss << dumps(std::get<_N>(__obj)) << "]";
		return __ss.str();
	}
	template<size_t _N, typename ..._Args>
	enable_if_t<_N != 0 && _N != sizeof...(_Args) - 1, std::string>
		dumps(const std::tuple<_Args...>& __obj) {
		std::stringstream __ss;
		__ss << dumps(std::get<_N>(__obj)) << ", " << dumps<_N + 1, _Args...>(__obj);
		return __ss.str();
	}
	template<size_t _N = 0, typename ..._Args>
	enable_if_t<_N == 0, std::string>
		dumps(const std::tuple<_Args...>& __obj) {
		std::stringstream __ss;
		__ss << "[" << dumps(std::get<_N>(__obj)) << ", " << dumps<_N + 1, _Args...>(__obj);
		return __ss.str();
	}

	// For pointer
	template<typename _Ty>
	enable_if_t<is_pointer_v<_Ty>, std::string>
		dumps(const _Ty __p) {
		return dumps(*__p);
	}
	// For shared_ptr, weak_ptr, unique_ptr
	template<template<typename...> typename _Tmp, typename ..._Args>
	enable_if_t<
		is_instantiation_of_v<_Tmp<_Args...>, std::shared_ptr> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::weak_ptr> ||
		is_instantiation_of_v<_Tmp<_Args...>, std::unique_ptr>,
		std::string>
		dumps(const _Tmp<_Args...>& __p) {
		return dumps(*__p);
	}
}