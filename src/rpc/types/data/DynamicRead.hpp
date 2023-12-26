
struct FunctionCallContext;

namespace DynamicData{
	template<typename T>
	bool read(DataInput& data,int& argCount,T& value);
	template<typename... T>
	bool read(DataInput& data,int& argCount,std::vector<T...>& value);

	template<typename T>
	bool readDynamic(DataInput& data,T& value){
		DataInput backup=data;
		int args=1;
		if(read(data,args,value)&&(args&-1)==0)
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
		bool readAll(DataInput&,int& argCount){ return argCount==0; }

		template<typename T>
		bool readAll(DataInput& d,int& argCount,MultipleArguments<T>& value){
			if(argCount==0)return true;
			value.reserve(argCount);
			while(argCount-->0){
				T entry;
				if(!read(d,argCount,entry))return false;
				value.push_back(entry);
			}
			return true;
		}

		template<typename T,typename... Args>
		bool readAll(DataInput& d,int& argCount,T& curr,Args& ... rest){
			return read(d,argCount,curr)&&readAll(d,argCount,rest...);
		}
	}

	template<typename... Args>
	bool readDynamicArray(DataInput& data,Args& ... args){
		DataInput backup=data;
		int32_t argCount=data.readLength();
		bool b=ReadAll::readAll(data,argCount,args...);
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

namespace DynamicData{
	template<typename T>
	bool read(DataInput& data,int& argCount,T& value){
		static_assert(std::is_same<T, void>::value,"DynamicData.read is not implemented for this type");
		return false;
	}
	template<typename... T>
	bool read(DataInput& data,int& argCount,std::vector<T...>& value){
		if(argCount==0)return false;
		int32_t type=data.readLength();
		if(type<0&&((-type)%4)==3){
			int32_t count=-type/4;
			value.resize(count);
			for(int i=0;i<count;++i){
				int args=1;
				if(!read(data,args,value)||(args&-1)!=0)
					return false;
			}
			argCount--;
			return true;
		}
		return false;
	}

	template<>
	bool read(DataInput& data,int& argCount,nullptr_t&){
		if(argCount==0)return false;
		if(data.readLength()=='n'){
			argCount--;
			return true;
		}
		return false;
	}

	template<>
	bool read(DataInput& data,int& argCount,bool& value){
		if(argCount==0)return false;
		switch(data.readLength()){
			case 't':
				value=true;
				argCount--;
				return true;
			case 'f':
				value=false;
				argCount--;
				return true;
			default:
				return false;
		}
	}

	template<>
	bool read(DataInput& data,int& argCount,int32_t& value){
		if(argCount==0)return false;
		switch(data.readLength()){
			case 'i':
				value=data.readInt();
				argCount--;
				return true;
			case 'd':
				value=int32_t(data.readDouble());
				argCount--;
				return true;
			case 'l':
				value=int32_t(data.readLong());
				argCount--;
				return true;
			default:
				return false;
		}
	}

	template<>
	bool read(DataInput& data,int& argCount,uint16_t& value){
		if(argCount==0)return false;
		switch(data.readLength()){
			case 'i':
				value=data.readInt();
				argCount--;
				return true;
			case 'd':
				value=int32_t(data.readDouble());
				argCount--;
				return true;
			case 'l':
				value=int32_t(data.readLong());
				argCount--;
				return true;
			default:
				return false;
		}
	}

	template<>
	bool read(DataInput& data,int& argCount,String& value){
		if(argCount==0)return false;
		int32_t type=data.readLength();
		if(type=='n'){
			value=NULL_STRING;
			argCount--;
			return true;
		}
		if(type<0&&((-type)%4)==1){
			int32_t count=-type/4;
			uint8_t chars[count+1];
			data.readFully(chars,count);
			chars[count]=0;
			value=String((char*)chars);
			argCount--;
			return true;
		}
		return false;
	}
	
}



template<typename Func>
bool DataInput::tryCall(Func func){
	return DynamicData::callDynamicArray(*this,removeConstReferenceParameters(make_function(func)));
}
template<typename Func,typename Arg0>
bool DataInput::tryCall(Func func,const Arg0& arg0){
	return DynamicData::callDynamicArray(*this,removeConstReferenceParameters(make_function(func)),arg0);
}
template<typename Func>
bool DataInput::tryCallSingle(Func func){
	return DynamicData::callDynamicSingle(*this,removeConstReferenceParameters(make_function(func)));
}
template<typename T>
bool DataInput::tryGetResult(T value){
	return DynamicData::readDynamic(*this,value);
}
template<typename... Args>
bool DataInput::tryGetArgs(Args&... args){
	return DynamicData::readDynamicArray(*this,args...);
}