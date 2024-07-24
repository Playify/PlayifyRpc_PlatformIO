namespace RpcInternal{
// For generic types that are functors, delegate to its 'operator()'
	template<typename T>
	struct function_traits: public function_traits<decltype(&T::operator())>{
	};

// for pointers to member function
	template<typename ClassType,typename ReturnType,typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...) const>{
		typedef std::function<ReturnType(Args...)> type;
	};

// for pointers to member function
	template<typename ClassType,typename ReturnType,typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...)>{
		typedef std::function<ReturnType(Args...)> type;
	};

// for function pointers
	template<typename ReturnType,typename... Args>
	struct function_traits<ReturnType (*)(Args...)>{
		typedef std::function<ReturnType(Args...)> type;
	};


	template<typename L>
	static typename function_traits<L>::type make_function(L l){ return (typename function_traits<L>::type)(l); }

//handles bind & multiple function call operator()'s
	template<typename ReturnType,typename... Args,class T>
	auto make_function(T&& t)
	->std::function<decltype(ReturnType(t(std::declval<Args>()...)))(Args...)>{ return {std::forward<T>(t)}; }

//handles explicit overloads
	template<typename ReturnType,typename... Args>
	auto make_function(ReturnType(* p)(Args...))
	->std::function<ReturnType(Args...)>{
		return {p};
	}

//handles explicit overloads
	template<typename ReturnType,typename... Args,typename ClassType>
	auto make_function(ReturnType(ClassType::*p)(Args...))
	->std::function<ReturnType(Args...)>{
		return {p};
	}




//===============


// Remove const and reference qualifiers from a single type
	template<typename T>
	struct remove_const_ref{
		using type=typename std::remove_const<typename std::remove_reference<T>::type>::type;
	};

// Helper template to apply remove_const_ref to a parameter pack
	template<typename... Args>
	struct remove_const_ref_pack;

	template<typename... Args>
	using remove_const_ref_pack_t=typename remove_const_ref_pack<Args...>::type;

	template<typename T,typename... Rest>
	struct remove_const_ref_pack<T,Rest...>{
		using type=typename std::tuple<typename remove_const_ref<T>::type,typename remove_const_ref_pack<Rest...>::type>::type;
	};

	template<typename T>
	struct remove_const_ref_pack<T>{
		using type=typename remove_const_ref<T>::type;
	};


//removes const& parameters
	template<typename ReturnType,typename... Args>
	auto removeConstReferenceParameters(std::function<ReturnType(Args...)> func)
	->std::function<ReturnType(remove_const_ref_pack_t<Args>...)>{
		return {func};
	}
}