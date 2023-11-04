#include <utility>

#define NULL_STRING "\xc3\x28"


struct RpcError{
	String type;
	String from;
	String msg;
	String stackTrace;

	explicit RpcError(const char* msg):
		type("RpcERR"),
		from(Rpc::nameOrId),
		msg(msg),
		stackTrace("<<StackTrace not supported>>"){}

	explicit RpcError(String type,String from,String message,String stackTrace):
		type(std::move(type)),
		from(std::move(from)),
		msg(std::move(message)),
		stackTrace(std::move(stackTrace)){}

	void printStackTrace() const{
		Serial.print(type);
		Serial.print('(');
		Serial.print(from);
		Serial.print(')');

		if(msg.length()&&msg!=NULL_STRING){
			Serial.print(": ");
			Serial.print(msg);
		}
		Serial.println();
		if(stackTrace.length()&&stackTrace!=NULL_STRING){
			Serial.println(stackTrace);
		}
	}
};