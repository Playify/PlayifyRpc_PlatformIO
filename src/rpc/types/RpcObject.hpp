
class RpcObject{
public:
	const String type;
	explicit RpcObject(String type):type(std::move(type)){}

	RpcFunction getFunction(String method) const{
		return RpcFunction(type,std::move(method));
	}
	template<typename... Args>
	PendingCall call(String method,Args... args) const{
		return callRemoteFunction(type,method,args...);
	}
};