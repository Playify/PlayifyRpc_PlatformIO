#include <utility>

#define NULL_STRING "\xc3\x28"


struct RpcError{
	String type;
	String from;
	String message;
	String stackTrace;
	String jsonData;
public:
	explicit RpcError(const char* msg):
			type("RpcError"),
			from(Rpc::prettyName()),
			message(msg),
			stackTrace("<<StackTrace not supported>>"),
			jsonData(NULL_STRING){}

	explicit RpcError(String type,String from,String message,String stackTrace,String jsonData=NULL_STRING):
			type(std::move(type)),
			from(from==NULL_STRING?Rpc::prettyName():std::move(from)),
			message(std::move(message)),
			stackTrace(std::move(stackTrace)),
			jsonData(std::move(jsonData)){}

	void printStackTrace(Print& out=Serial) const{
		out.print(type);
		out.print('(');
		out.print(from);
		out.print(')');

		if(message.length()&&message!=NULL_STRING){
			out.print(": ");
			out.print(message);
		}
		out.println();
		if(stackTrace.length()&&stackTrace!=NULL_STRING){
			out.println(stackTrace);
		}
	}
	String getStackTrace() const{
		String result=type+'('+from+')';

		if(message.length()&&message!=NULL_STRING)
			result+=": "+message;
		result+="\n";
		if(stackTrace.length()&&stackTrace!=NULL_STRING)
			result+=stackTrace;
		
		return result;
	}
	
	RpcError append(String s) const{
		auto index=stackTrace.indexOf("\ncaused by:");
		
		String trace;
		if(index==-1) trace=stackTrace+"\n\trpc "+s;
		else trace=stackTrace.substring(0,index)+"\n\trpc "+s+stackTrace.substring(index);
		if(!trace.isEmpty()&&trace[0]=='\n')trace=trace.substring(1);

		return RpcError(
				type,
				from,
				message,
				trace,
				jsonData
		);
	}
};