namespace RpcInternal{
	namespace Helpers{
		namespace FirstType{
			template<typename... Types>
			struct first_type;

			template<typename First, typename... Rest>
			struct first_type<First, Rest...> {
				using type = First;  // Correctly extracts the first type
			};

			template<>
			struct first_type<> {
				using type = void;  // Specialization for an empty parameter pack
			};
		}
		
		namespace MakeFunction{

			template<typename T>
			struct function_traits:function_traits<decltype(&T::operator ())>{
				// For generic types that are functors, delegate to its 'operator()'
			};

			template<typename ClassType,typename ReturnType,typename... Args>
			struct function_traits<ReturnType(ClassType::*)(Args...) const>{
				// for pointers to member function
				typedef std::function<ReturnType(Args...)> type;
				typedef FirstType::first_type<Args...> first;
				typedef ReturnType returns;
				static constexpr int count=sizeof...(Args);
			};

			template<typename ClassType,typename ReturnType,typename... Args>
			struct function_traits<ReturnType(ClassType::*)(Args...)>{
				// for pointers to member function
				typedef std::function<ReturnType(Args...)> type;
				typedef FirstType::first_type<Args...> first;
				typedef ReturnType returns;
				static constexpr int count=sizeof...(Args);
			};

			template<typename ReturnType,typename... Args>
			struct function_traits<ReturnType (*)(Args...)>{
				// for function pointers
				typedef std::function<ReturnType(Args...)> type;
				typedef FirstType::first_type<Args...> first;
				typedef ReturnType returns;
				static constexpr int count=sizeof...(Args);
			};


			template<typename L>
			typename function_traits<L>::type make_function(L l){ return (typename function_traits<L>::type)l; }
			
			template<typename ReturnType,typename... Args,class T>//handles bind & multiple function call operator()'s
			auto make_function(T&& t)->std::function<decltype(ReturnType(t(std::declval<Args>()...)))(Args...)>{ return {std::forward<T>(t)}; }
			
			template<typename ReturnType,typename... Args>//handles explicit overloads
			auto make_function(ReturnType(* p)(Args...))->std::function<ReturnType(Args...)>{ return {p}; }
			
			template<typename ReturnType,typename... Args,typename ClassType>//handles explicit overloads
			auto make_function(ReturnType(ClassType::*p)(Args...))->std::function<ReturnType(Args...)>{ return {p}; }
		}

		namespace ConstRef{
// Remove const and reference qualifiers from a single type
			template<typename T>
			struct remove_const_ref{
				using type=typename std::remove_const<typename std::remove_reference<T>::type>::type;
			};

// Helper template to apply remove_const_ref to a parameter pack
			template<typename... Args>
			struct remove_const_ref_pack;
			template<typename T>
			struct remove_const_ref_pack<T>{
				using type=typename remove_const_ref<T>::type;
			};
			template<typename T,typename... Rest>
			struct remove_const_ref_pack<T,Rest...>{
				using type=typename std::tuple<typename remove_const_ref<T>::type,typename remove_const_ref_pack<Rest...>::type>::type;
			};

			template<typename... Args>
			using remove_const_ref_pack_t=typename remove_const_ref_pack<Args...>::type;


//removes const& parameters
			template<typename ReturnType,typename... Args>
			auto removeConstReferenceParameters(std::function<ReturnType(Args...)> func)
			->std::function<ReturnType(remove_const_ref_pack_t<Args>...)>{
				return {func};
			}
		}


		template<typename Func>
		auto function(Func&& func)->decltype(ConstRef::removeConstReferenceParameters(MakeFunction::make_function(std::forward<Func>(func)))){
			return ConstRef::removeConstReferenceParameters(MakeFunction::make_function(std::forward<Func>(func)));
		}

		namespace Apply{
			template<size_t N>
			struct Apply{
				template<typename F,typename T,typename... A>
				static auto
				apply(F&& f,T&& t,A&& ... a)->decltype(Apply<N-1>::apply(std::forward<F>(f),std::forward<T>(t),
																		 std::get<N-1>(std::forward<T>(t)),
																		 std::forward<A>(a)...)){
					return Apply<N-1>::apply(std::forward<F>(f),std::forward<T>(t),std::get<N-1>(std::forward<T>(t)),
											 std::forward<A>(a)...);
				}
			};

			template<>
			struct Apply<0>{
				template<typename F,typename T,typename... A>
				static auto apply(F&& f,T&&,A&& ... a)->decltype(std::forward<F>(f)(std::forward<A>(a)...)){
					return std::forward<F>(f)(std::forward<A>(a)...);
				}
			};

			template<typename F,typename T>
			auto apply(F&& f,T&& t)->decltype(Apply<std::tuple_size<typename std::decay<T>::type>::value>::apply(
					std::forward<F>(f),std::forward<T>(t))){
				return Apply<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f),
																						  std::forward<T>(t));
			}
		}
	}
}