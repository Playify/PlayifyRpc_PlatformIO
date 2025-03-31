
struct FunctionCallContext;

namespace RpcInternal{
	namespace DynamicData{
		template<typename T>
		bool readDynamic(DataInput& data,T& value){
			DataInput backup=data;
			int args=1;
			if(TypeDefinition<T>::read(data,args,value)&&(args&-1)==0)
				return true;
			data=backup;
			return false;
		}

		template<>
		bool readDynamic(DataInput& data,DataInput& value){
			value=data;
			return true;
		}



		namespace ReadAll{
			bool readAll(DataInput&,const int& argCount){ return argCount==0; }

			template<typename T>
			bool readAll(DataInput& d,int& argCount,MultipleArguments<T>& value){
				if(argCount==0)return true;
				value.resize(argCount);
				size_t i=0;
				while(argCount>0)
					if(!TypeDefinition<T>::read(d,argCount,value[i++]))
						return false;
				return true;
			}

			template<typename T,typename... Args>
			bool readAll(DataInput& d,int& argCount,T& curr,Args& ... rest){
				return TypeDefinition<T>::read(d,argCount,curr)&&readAll(d,argCount,rest...);
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


		//CallReceiver without FunctionCallContext argument, returning anything
		template<typename FunctionCallContext,typename Return,typename... Args>
		bool callDynamicArray(DataInput& data,std::function<Return(Args...)> func,const FunctionCallContext& ctx){
			std::tuple<RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>...> argsTuple;

			bool b=RpcInternal::Helpers::Apply::apply([&data](RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>& ... args){
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b) ctx.resolve(RpcInternal::Helpers::Apply::apply(func,argsTuple));
			return b;
		}
		//CallReceiver without FunctionCallContext argument, returning void
		template<typename FunctionCallContext,typename... Args>
		bool callDynamicArray(DataInput& data,std::function<void(Args...)> func,const FunctionCallContext& ctx){
			std::tuple<RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>...> argsTuple;

			bool b=RpcInternal::Helpers::Apply::apply([&data](RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>& ... args){
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b){
				RpcInternal::Helpers::Apply::apply(func,argsTuple);
				ctx.resolve();
			}
			return b;
		}
		//CallReceiver with FunctionCallContext argument
		template<typename FunctionCallContext,typename... Args>
		bool callDynamicArray(DataInput& data,std::function<void(FunctionCallContext,Args...)> func,const FunctionCallContext& ctx){
			std::tuple<FunctionCallContext,RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>...> argsTuple;

			bool b=RpcInternal::Helpers::Apply::apply([&data,&ctx](FunctionCallContext& arg0,RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>& ... args){
				arg0=ctx;
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b) RpcInternal::Helpers::Apply::apply(RpcInternal::Helpers::ConstRef::removeConstReferenceParameters(func),argsTuple);
			return b;
		}

		//MessageFunc
		template<typename... Args>
		bool callDynamicArray(DataInput& data,std::function<void(Args...)> func){
			std::tuple<RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>...> argsTuple;

			const bool b=RpcInternal::Helpers::Apply::apply([&data](RpcInternal::Helpers::ConstRef::remove_const_ref_pack_t<Args>& ... args){
				return readDynamicArray(data,args...);
			},argsTuple);
			if(b) RpcInternal::Helpers::Apply::apply(func,argsTuple);
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
	return RpcInternal::DynamicData::callDynamicArray(*this,RpcInternal::Helpers::function(func));
}

template<typename Func>
bool DataInput::tryCallSingle(Func func){
	return RpcInternal::DynamicData::callDynamicSingle(*this,RpcInternal::Helpers::function(func));
}

template<typename T>
bool DataInput::tryGetResult(T& value){
	return RpcInternal::DynamicData::readDynamic(*this,value);
}

template<typename... Args>
bool DataInput::tryGetArgs(Args& ... args){
	return RpcInternal::DynamicData::readDynamicArray(*this,args...);
}