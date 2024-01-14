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

	void printStackTrace(Print& out=Serial) const{
		out.print(type);
		out.print('(');
		out.print(from);
		out.print(')');

		if(msg.length()&&msg!=NULL_STRING){
			out.print(": ");
			out.print(msg);
		}
		out.println();
		if(stackTrace.length()&&stackTrace!=NULL_STRING){
			out.println(stackTrace);
		}
	}
	String getStackTrace() const{
		String result=type+'('+from+')';

		if(msg.length()&&msg!=NULL_STRING)
			result+=": "+msg;
		result+="\n";
		if(stackTrace.length()&&stackTrace!=NULL_STRING)
			result+=stackTrace;
		
		return result;
	}
};