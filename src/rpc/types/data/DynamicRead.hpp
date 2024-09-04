
struct FunctionCallContext;

namespace RpcInternal{
	namespace DynamicData{
		template<typename T>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,T& value);

		template<typename T>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::vector<T>& value);
		template<typename... Types>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::tuple<Types...>& value);

		
		template<typename T>
		bool readDynamic(DataInput& data,T& value){
			DataInput backup=data;
			int args=1;
			auto already=std::vector<DataInput>();
			if(read(data,args,already,value)&&(args&-1)==0)
				return true;
			data=backup;
			return false;
		}

		template<>
		bool readDynamic(DataInput& data,DataInput& value){
			value=data;
			return true;
		}


		namespace Apply{
			template<size_t N>
			struct Apply{
				template<typename F,typename T,typename... A>
				static inline auto
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
				static inline auto apply(F&& f,T&&,A&& ... a)->decltype(std::forward<F>(f)(std::forward<A>(a)...)){
					return std::forward<F>(f)(std::forward<A>(a)...);
				}
			};

			template<typename F,typename T>
			inline auto apply(F&& f,T&& t)->decltype(Apply<std::tuple_size<typename std::decay<T>::type>::value>::apply(
					std::forward<F>(f),std::forward<T>(t))){
				return Apply<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f),
																						  std::forward<T>(t));
			}
		}

		namespace ReadAll{
			bool readAll(DataInput&,int& argCount,std::vector<DataInput>&){ return argCount==0; }

			template<typename T>
			bool readAll(DataInput& d,int& argCount,std::vector<DataInput>& already,MultipleArguments<T>& value){
				if(argCount==0)return true;
				value.resize(argCount);
				size_t i=0;
				while(argCount>0)
					if(!read(d,argCount,already,value[i++]))
						return false;
				return true;
			}

			template<typename T,typename... Args>
			bool readAll(DataInput& d,int& argCount,std::vector<DataInput>& already,T& curr,Args& ... rest){
				return read(d,argCount,already,curr)&&readAll(d,argCount,already,rest...);
			}
		}

		template<typename... Args>
		bool readDynamicArray(DataInput& data,Args& ... args){
			DataInput backup=data;
			int32_t argCount=data.readLength();
			auto already=std::vector<DataInput>();
			bool b=ReadAll::readAll(data,argCount,already,args...);
			if(b)return true;
			data=backup;
			return false;
		}

		template<>
		bool readDynamicArray(DataInput& data,DataInput& args){
			args=data;
			return true;
		}


		template<typename... Args>
		bool callDynamicArray(DataInput& data,std::function<void(Args...)> func){
			std::tuple<remove_const_ref_pack_t<Args>...> argsTuple;

			bool b=Apply::apply([&data](remove_const_ref_pack_t<Args>& ... args){
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b) Apply::apply(func,argsTuple);
			return b;
		}

		template<typename Arg0,typename... Args>
		bool callDynamicArray(DataInput& data,std::function<void(Arg0,Args...)> func,const Arg0& ctx){
			std::tuple<Arg0,remove_const_ref_pack_t<Args>...> argsTuple;

			bool b=Apply::apply([&data,&ctx](Arg0& arg0,remove_const_ref_pack_t<Args>& ... args){
				arg0=ctx;
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b) Apply::apply(removeConstReferenceParameters(func),argsTuple);
			return b;
		}

		template<typename Arg>
		bool callDynamicSingle(DataInput& data,std::function<void(Arg)> func){
			Arg arg;

			if(readDynamic(data,arg)){
				func(arg);
				return true;
			}else return false;
		}
	}
}

template<typename Func>
bool DataInput::tryCall(Func func){
	return RpcInternal::DynamicData::callDynamicArray(*this,RpcInternal::removeConstReferenceParameters(
															  RpcInternal::make_function(func)));
}

template<typename Func>
bool DataInput::tryCallSingle(Func func){
	return RpcInternal::DynamicData::callDynamicSingle(*this,RpcInternal::removeConstReferenceParameters(
			RpcInternal::make_function(func)));
}

template<typename T>
bool DataInput::tryGetResult(T& value){
	return RpcInternal::DynamicData::readDynamic(*this,value);
}

template<typename... Args>
bool DataInput::tryGetArgs(Args& ... args){
	return RpcInternal::DynamicData::readDynamicArray(*this,args...);
}