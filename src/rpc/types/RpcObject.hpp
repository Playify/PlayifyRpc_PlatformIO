
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
	
	void exists(const Callback<bool>& callback){
		callRemoteFunction(NULL_STRING,"E",type).then(callback,[callback](const RpcError&){
			callback(false);
		});
	}
	
	void getMethods(const Callback<std::vector<String>>& callback){
		callRemoteFunction(type,NULL_STRING,"E").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}
};