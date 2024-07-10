

template<typename T>
auto make_callReceiver(T t) -> typename std::enable_if<!std::is_constructible<CallReceiver,T>::value, CallReceiver>::type{
	auto func=make_function(t);
	return [func](const FunctionCallContext& ctx,DataInput data){
		if(!data.tryCall(func,ctx)){
			ctx.reject(RpcError("Arguments do not meet parameter types"));//TODO RpcDataException
		}
	};
}

template<typename T>
auto make_callReceiver(T t) -> typename std::enable_if<std::is_constructible<CallReceiver,T>::value, CallReceiver>::type{
	return t;
}

template<typename T>
CallReceiver make_smartProperty(T& ref,const std::function<void()>& onChange=nullptr){
	return [&ref,onChange](const FunctionCallContext& ctx,DataInput data){
		T t;
		if(data.tryGetArgs()){}
		else if(data.tryGetArgs(t)){
			ref=t;
			if(onChange)onChange();
		}else return ctx.reject("Error casting args");
		ctx.resolve(ref);
	};
}

template<>
CallReceiver make_smartProperty(bool& ref,const std::function<void()>& onChange){
	return [&ref,onChange](const FunctionCallContext& ctx,DataInput data){
		bool t;
		nullptr_t null;
		if(data.tryGetArgs()){}
		else if(data.tryGetArgs(t)){
			ref=t;
			if(onChange)onChange();
		}else if(data.tryGetArgs(null)){
			ref=!ref;
			if(onChange)onChange();
		}else return ctx.reject("Error casting args");
		ctx.resolve(ref);
	};
}