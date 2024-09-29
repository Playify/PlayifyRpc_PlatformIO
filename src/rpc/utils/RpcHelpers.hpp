#include <functional>

namespace RpcHelpers{
	void callDelayed(int delayMillis,std::function<void()> callback){
		auto startTime=millis();
		Rpc::addOnLoop([delayMillis,startTime,callback](){
			if(millis()-startTime<delayMillis)return false;
			callback();
			return true;
		});
	}
	
	
	template<typename MessageFunc,typename... Args>
	void autoRecall(MessageFunc onMessage,std::function<void(bool success)> onError,String type,String method,Args... args){
		auto call=RpcInternal::callRemoteFunction(type,method,args...);
		call.setMessageListener(onMessage);
		
		std::function<void()> recall=[=](){
			autoRecall(onMessage,onError,type,method,args...);
		};
		
		call.finally([onError,recall](bool b){
			if(onError)onError(b);
			callDelayed(1000,recall);
		});
	}
	
	template<typename T,typename... Args>
	T& rpcListenValue(T& ref,String type,String method,Args... args){
		autoRecall([&ref](T t){ref=t;},nullptr,type,method,args...);
		return ref;
	}
	
	template<typename T,typename... Args>
	T& rpcListenValueWithDefault(T& ref,T def,String type,String method,Args... args){
		autoRecall([&ref](T t){ref=t;},[&ref,def](bool){ref=def;},type,method,args...);
		return ref;
	}
}