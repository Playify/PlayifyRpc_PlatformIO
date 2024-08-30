
class RpcObject{
public:
	String type;
	explicit RpcObject(String type):type(std::move(type)){}

	RpcFunction getFunction(String method) const{
		return RpcFunction(type,std::move(method));
	}
	template<typename... Args>
	PendingCall call(String method,Args... args) const{
		return RpcInternal::callRemoteFunction(type,method,args...);
	}
	
	void exists(const Callback<bool>& callback){
		RpcInternal::callRemoteFunction(NULL_STRING,"E",type).then(callback,[callback](const RpcError&){
			callback(false);
		});
	}
	
	void getMethods(const Callback<std::vector<String>>& callback){
		RpcInternal::callRemoteFunction(type,NULL_STRING,"M").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}
};