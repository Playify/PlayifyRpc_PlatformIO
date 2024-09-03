


template<typename T>
CallReceiver& CallReceiver::smartProperty(T& ref,const std::function<void()>& onChange){
	add([&ref](const FunctionCallContext& ctx){
		ctx.resolve(ref);
	},{},(T*)nullptr);
	add([&ref,onChange](const FunctionCallContext& ctx,T newVal){
		ctx.resolve(ref=newVal);
		if(onChange)onChange();
	},{"value"},(T*)nullptr);
	return *this;
}
template<>
CallReceiver& CallReceiver::smartProperty(bool& ref,const std::function<void()>& onChange){
	add([&ref](const FunctionCallContext& ctx){
		ctx.resolve(ref);
	},{},(bool*)nullptr);
	add([&ref,onChange](const FunctionCallContext& ctx,bool newVal){
		ctx.resolve(ref=newVal);
		if(onChange)onChange();
	},{"value"},(bool*)nullptr);
	add([&ref,onChange](const FunctionCallContext& ctx,nullptr_t){
		ctx.resolve(ref=!ref);
		if(onChange)onChange();
	},{"toggle"},(bool*)nullptr);
	return *this;
}



template<typename Func>
CallReceiver& CallReceiver::add(Func func){
	return add(func,"unknown","object");
}

template<typename Func,typename Return>
CallReceiver& CallReceiver::add(Func func,Return* null){
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);});
	signatures.push_back([=](bool ts)->MethodSignatureTuple{return RpcInternal::DynamicData::getMethodSignature(function,ts,null);});
	return *this;
}
template<typename Func>
CallReceiver& CallReceiver::add(Func func,String returnsTs,String returnsCs){
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);});
	signatures.push_back([=](bool ts)->MethodSignatureTuple{return RpcInternal::DynamicData::getMethodSignature(function,ts,ts?returnsTs:returnsCs);});
	return *this;
}


template<typename Func>
CallReceiver& CallReceiver::add(Func func,std::array<String,RpcInternal::function_traits<Func>::count-1> names,String returnsTs,String returnsCs){
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);});
	signatures.push_back([=](bool ts)->MethodSignatureTuple{return RpcInternal::DynamicData::getMethodSignature(function,ts,names,ts?returnsTs:returnsCs);});
	return *this;
}

template<typename Func,typename Return>
CallReceiver& CallReceiver::add(Func func,std::array<String,RpcInternal::function_traits<Func>::count-1> names,Return* null){
	auto function=RpcInternal::removeConstReferenceParameters(RpcInternal::make_function(func));
	callers.push_back([function](const FunctionCallContext& ctx,DataInput args){return RpcInternal::DynamicData::callDynamicArray(args,function,ctx);});
	signatures.push_back([=](bool ts)->MethodSignatureTuple{return RpcInternal::DynamicData::getMethodSignature(function,ts,names,null);});
	return *this;
}