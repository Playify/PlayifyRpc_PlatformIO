
class RpcFunction{
public:
	const String type;
	const String method;
	explicit RpcFunction(String type,String method):
		type(std::move(type)),
		method(std::move(method)){}
};