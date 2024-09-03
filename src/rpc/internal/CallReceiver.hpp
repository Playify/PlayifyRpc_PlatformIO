
struct FunctionCallContext;
struct DataInput;

using MethodSignatureTuple=std::tuple<std::vector<String>,String>;
struct CallReceiver{
	std::vector<std::function<bool(const FunctionCallContext& ctx,DataInput args)>> callers;
	std::vector<std::function<MethodSignatureTuple(bool ts)>> signatures;

	template<typename Func,typename Return>
	CallReceiver& add(Func func,std::array<String,RpcInternal::function_traits<Func>::count-1> names,Return* null);
	template<typename Func>
	CallReceiver& add(Func func,std::array<String,RpcInternal::function_traits<Func>::count-1> names,String returnsTs,String returnsCs);
	template<typename Func,typename Return>
	CallReceiver& add(Func func,Return* null);
	template<typename Func>
	CallReceiver& add(Func func,String returnsTs,String returnsCs);
	template<typename Func>
	CallReceiver& add(Func func);
	
	template<typename T>
	CallReceiver& smartProperty(T& ref,const std::function<void()>& onChange=nullptr);
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