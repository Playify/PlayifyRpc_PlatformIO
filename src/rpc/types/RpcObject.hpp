
class RpcObject{
public:
	const String type;
	explicit RpcObject(String type):type(std::move(type)){}

	RpcFunction get(String method) const{
		return RpcFunction(type,std::move(method));
	}
};