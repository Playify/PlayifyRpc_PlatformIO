


template<typename T>
CallReceiver& CallReceiver::smartProperty(T& ref,const std::function<void()>& onChange){
	func([&ref]{return ref;});
	func([&ref,onChange](T value){
		ref=value;
		if(onChange)onChange();
		return ref;
	},"value");
	return *this;
}

template<>
CallReceiver& CallReceiver::smartProperty(bool& ref,const std::function<void()>& onChange){
	func([&ref]{return ref;});
	func([&ref,onChange](const bool value){
		ref=value;
		if(onChange)onChange();
		return ref;
	},"value");
	func([&ref,onChange](nullptr_t){
		ref=!ref;
		if(onChange)onChange();
		return ref;
	},"toggle");
	return *this;
}

template<typename T>
CallReceiver& CallReceiver::getter(T& ref){
	return func([&ref]{return ref;});
}



template<typename Func,typename... Args,typename Return>
CallReceiver& CallReceiver::add(Func func,ReturnType<Return> returns,Args... names){
	static_assert(sizeof...(names)==RpcInternal::Helpers::MakeFunction::function_traits<Func>::count-1,"Invalid amount of parameter names provided");

	auto function=RpcInternal::Helpers::function(func);
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array,returns](ProgrammingLanguage lang)->RpcInternal::MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,returns,array,lang);
	});
	return *this;
}


template<typename Func,typename... Args>
CallReceiver& CallReceiver::add(Func func,std::pair<const char*,const char*> returns,Args... names){
	static_assert(sizeof...(names)==RpcInternal::Helpers::MakeFunction::function_traits<Func>::count-1,"Invalid amount of parameter names provided");

	auto function=RpcInternal::Helpers::function(func);
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array,returns](ProgrammingLanguage lang)->RpcInternal::MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,lang!=ProgrammingLanguage::CSharp?returns.first:returns.second,array,lang);
	});
	return *this;
}



template<typename Func,typename... Args>
CallReceiver& CallReceiver::func(Func func,Args ...names){
	static_assert(sizeof...(names)==RpcInternal::Helpers::MakeFunction::function_traits<Func>::count,"Invalid amount of parameter names provided");

	auto function=RpcInternal::Helpers::function(func);
	std::array<String,sizeof...(names)> array={std::forward<Args>(names)...};
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){
		return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);
	});
	signatures.push_back([function,array](ProgrammingLanguage lang)->RpcInternal::MethodSignatureTuple{
		return RpcInternal::DynamicData::getMethodSignature(function,array,lang);
	});
	return *this;
}


template<typename T>
RpcInternal::MessageFunc RpcInternal::make_messageFunc(T func){
	auto function=Helpers::function(func);
	return [function](DataInput data){
		if(!DynamicData::callDynamicArray(data,function))
			Serial.println("[Rpc] Error while receiving message: Arguments do not match the receiver");
	};
}