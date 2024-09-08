
struct FunctionCallContext;
struct DataInput;

template<typename T>
struct ReturnType{};

using MethodSignatureTuple=std::tuple<std::vector<String>,String>;
struct CallReceiver{
	std::vector<std::function<bool(const FunctionCallContext& ctx,DataInput args)>> callers;
	std::vector<std::function<MethodSignatureTuple(bool ts)>> signatures;

	template<typename Func,typename... Args,typename Return,typename std::enable_if<!std::is_same<Return, const char>::value, int>::type = 0>
	CallReceiver& add(Func func,Return* returns,Args... names);
	template<typename Func,typename... Args,typename Return>
	CallReceiver& add(Func func,ReturnType<Return> returns,Args... names);
	template<typename Func,typename... Args>
	CallReceiver& add(Func func,const char* returns,Args... names);
	template<typename Func,typename... Args>
	CallReceiver& add(Func func,std::pair<String,String> returns,Args... names);
	template<typename Func>
	[[deprecated("Use overload with return type and named parameters instead")]]CallReceiver& add(Func func);
	
	template<typename T>
	CallReceiver& smartProperty(T& ref,const std::function<void()>& onChange=nullptr);
	
	CallReceiver& clear(){
		callers.clear();
		signatures.clear();
		return *this;
	}
};

/*


void test(){
	auto x=RpcInternal::make_function([](FunctionCallContext ctx){

	});
	RpcInternal::DynamicData::getMethodSignature(x,true);
	RpcInternal::DynamicData::getMethodSignature(x,true,{},(int*)nullptr);
	RpcInternal::DynamicData::getMethodSignature(x,true,{},String("number"));
	auto y=RpcInternal::make_function([](FunctionCallContext ctx,int x){

	});
	RpcInternal::DynamicData::getMethodSignature(y,true);
	RpcInternal::DynamicData::getMethodSignature(y,true,{"x"},(int*)nullptr);
	RpcInternal::DynamicData::getMethodSignature(y,true,{"x","y"},(int*)nullptr);
}*/