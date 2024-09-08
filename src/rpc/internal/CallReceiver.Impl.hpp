


template<typename T>
CallReceiver& CallReceiver::smartProperty(T& ref,const std::function<void()>& onChange){
	add([&ref](const FunctionCallContext& ctx){
		ctx.resolve(ref);
	},ReturnType<T>());
	add([&ref,onChange](const FunctionCallContext& ctx,T newVal){
		ctx.resolve(ref=newVal);
		if(onChange)onChange();
	},ReturnType<T>(),"value");
	return *this;
}
template<>
CallReceiver& CallReceiver::smartProperty(bool& ref,const std::function<void()>& onChange){
	add([&ref](const FunctionCallContext& ctx){
		ctx.resolve(ref);
	},(bool*)nullptr);
	add([&ref,onChange](const FunctionCallContext& ctx,bool newVal){
		ctx.resolve(ref=newVal);
		if(onChange)onChange();
	},ReturnType<bool>(),"value");
	add([&ref,onChange](const FunctionCallContext& ctx,nullptr_t){
		ctx.resolve(ref=!ref);
		if(onChange)onChange();
	},ReturnType<bool>(),"toggle");
	return *this;
}



template<typename Func>
CallReceiver& CallReceiver::add(Func func){
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function](bool ts)->MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,ts,ts?"unknown":"object?");
	});
	return *this;
}

template<typename Func,typename... Args,typename Return,typename std::enable_if<!std::is_same<Return, const char>::value, int>::type>
CallReceiver& CallReceiver::add(Func func,Return* returns,Args... names){
	static_assert(sizeof...(names)==RpcInternal::function_traits<Func>::count-1,"Invalid amount of parameter names provided");
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array,returns](bool ts)->MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,ts,array,returns);
	});
	return *this;
}
template<typename Func,typename... Args,typename Return>
CallReceiver& CallReceiver::add(Func func,ReturnType<Return>,Args... names){
	static_assert(sizeof...(names)==RpcInternal::function_traits<Func>::count-1,"Invalid amount of parameter names provided");
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array](bool ts)->MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,ts,array,(Return*)nullptr);
	});
	return *this;
}

template<typename Func,typename... Args>
	CallReceiver& CallReceiver::add(Func func,const char* returns,Args... names){
	static_assert(sizeof...(names)==RpcInternal::function_traits<Func>::count-1,"Invalid amount of parameter names provided");
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array,returns](bool ts)->MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,ts,array,returns);
	});
	return *this;
}

template<typename Func,typename... Args>
	CallReceiver& CallReceiver::add(Func func,std::pair<String,String> returns,Args... names){
	static_assert(sizeof...(names)==RpcInternal::function_traits<Func>::count-1,"Invalid amount of parameter names provided");
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array,returns](bool ts)->MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,ts,array,ts?returns.first:returns.second);
	});
	return *this;
}
