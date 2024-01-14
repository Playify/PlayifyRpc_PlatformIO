
class RpcFunction{
public:
	String type;
	String method;
	
	explicit RpcFunction(){}
	
	explicit RpcFunction(String type,String method):
		type(std::move(type)),
		method(std::move(method)){}
		
	template<typename... Args>
	PendingCall call(Args... args) const{
		return callRemoteFunction(type,method,args...);
	}
};