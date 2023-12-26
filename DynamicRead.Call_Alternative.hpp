/*

namespace TryRead{
	template<size_t Missing,typename T>
	struct TryRead{
		static inline bool tryRead(DataInput& data,int& argCount,T& value){
			return read(data,argCount,value);
		}
	};

	template<typename T>
	struct TryRead<1,MultipleArguments<T>>{
	static inline bool tryRead(DataInput& data,int& argCount,MultipleArguments<T>& value){
		if(argCount==0)return true;
		value.reserve(argCount);
		while(argCount-->0){
			T entry;
			if(!read(data,argCount,entry))return false;
			value.push_back(entry);
		}
		return true;
	}
};
}

namespace TryCall{
	namespace NthType{
		template<std::size_t N,typename... Types>
		struct NthType;

		template<typename First,typename... Rest>
		struct NthType<0,First,Rest...>{
			using type=First;
		};

		template<std::size_t N,typename First,typename... Rest>
		struct NthType<N,First,Rest...>{
			using type=typename NthType<N-1,Rest...>::type;
		};
	}

	template<size_t Missing>
	struct TryCall{
		template<typename... FuncArgs,typename... Values>
		static inline bool apply(std::function<void(FuncArgs...)> f,DataInput& data,int& argCount,Values&& ... values){
			using CurrArg=typename remove_const_ref<typename NthType::NthType<
					sizeof...(FuncArgs)-Missing,FuncArgs...>::type>::type;

			CurrArg newValue;
			if(!TryRead::TryRead<Missing,CurrArg>::tryRead(data,argCount,newValue))return false;

			return TryCall<Missing-1>::apply(f,data,argCount,std::forward<Values>(values)...,newValue);
		}
	};

	template<>
	struct TryCall<0>{
		template<typename... FuncArgs,typename... Values>
		static inline bool apply(std::function<void(FuncArgs...)> f,DataInput&,int& argCount,Values&& ... a){
			if(argCount!=0)return false;
			f(std::forward<Values>(a)...);
			return true;
		}
	};

	template<typename... FuncArgs>
	inline bool tryCall(DataInput& data,std::function<void(FuncArgs...)> f){
		DataInput backup;
		int argCount=data.readLength();
		bool b=TryCall<sizeof...(FuncArgs)>::apply(f,data,argCount);
		if(b)return true;
		data._data=backup._data;
		data._available=backup._available;
		return false;
	}

	template<typename Arg0,typename... FuncArgs>
	inline bool tryCall(DataInput& data,std::function<void(Arg0,FuncArgs...)> f,Arg0 arg0){
		DataInput backup;
		int argCount=data.readLength();
		bool b=TryCall<sizeof...(FuncArgs)>::apply(f,data,argCount,arg0);
		if(b)return true;
		data._data=backup._data;
		data._available=backup._available;
		return false;
	}

	template<typename Arg0,typename... FuncArgs>
	inline bool tryCall(DataInput& data,std::function<void(const Arg0&,FuncArgs...)> f,Arg0 arg0){
		DataInput backup;
		int argCount=data.readLength();
		bool b=TryCall<sizeof...(FuncArgs)>::apply(f,data,argCount,arg0);
		if(b)return true;
		data._data=backup._data;
		data._available=backup._available;
		return false;
	}
}
*/
