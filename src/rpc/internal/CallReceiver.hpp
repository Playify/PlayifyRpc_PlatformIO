
struct FunctionCallContext;
struct DataInput;

template<typename T>
struct ReturnType{};

namespace RpcInternal{
	using MethodSignatureTuple=std::tuple<std::vector<String>,String>;
}


struct CallReceiver{
	std::vector<std::function<bool(const FunctionCallContext& ctx,DataInput args)>> callers;
	std::vector<std::function<RpcInternal::MethodSignatureTuple(ProgrammingLanguage lang)>> signatures;

	//Used for lambdas like [](int a){return a+1;}
	template<typename Func,typename... Args>
	CallReceiver& func(Func func,Args... names);

	//Used for lambdas like [](const FunctionCallContext& ctx,int a){ctx.resolve(a+1);}
	template<typename Func,typename... Args,typename Return>
	CallReceiver& add(Func func,ReturnType<Return> returns,Args... names);
	template<typename Func,typename... Args>
	CallReceiver& add(Func func,nullptr_t returnsVoid,Args... names){return add(func,ReturnType<void>(),names...);};
	template<typename Func,typename... Args>
	CallReceiver& add(Func func,std::pair<const char*,const char*> returns,Args... names);


	template<typename T>
	CallReceiver& smartProperty(T& ref,const std::function<void()>& onChange=nullptr);
	template<typename T>
	CallReceiver& getter(T& ref);

	CallReceiver& clear(){
		callers.clear();
		signatures.clear();
		return *this;
	}
};
