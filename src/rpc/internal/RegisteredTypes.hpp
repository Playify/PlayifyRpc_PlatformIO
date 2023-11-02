#include <utility>

#include "map"

using CallReceiver=std::function<void(FunctionCallContext ctx,DataInput args)>;



namespace RegisteredTypes{
	std::map<String,std::map<String,CallReceiver>*> registered;
#if ESP32
	uint32_t nextFunctionId=esp_random();
#elif ESP8266
	uint32_t nextFunctionId=os_random();
#endif
	std::map<String,CallReceiver> registeredFunctions;
	
	void registerType(const String& type,std::map<String,CallReceiver>* map){
		if(registered.count(type)){
			auto& pointer=registered[type];
			if(pointer!=map)delete pointer;
			pointer=map;
		}else registered[type]=map;

		if(RpcConnection::connected){
			callRemoteFunction(NULL_STRING,"+",type);
		}
	}
	void registerType(const String& type,std::map<String,CallReceiver> map){
		registerType(type,new std::map<String,CallReceiver>(std::move(map)));
	}
	std::map<String,CallReceiver>* registerType(const String& type){
		auto map=new std::map<String,CallReceiver>();
		registerType(type,map);
		return map;
	}
	
	void unregisterType(const String& type,bool deletePointer){
		if(!registered.count(type))return;

		if(RpcConnection::connected){
			callRemoteFunction(NULL_STRING,"-",type);
		}
		
		if(deletePointer) delete registered[type];
		registered.erase(type);
	}



	void setup(){
		registerType("$"+Rpc::id,&registeredFunctions);
	}
}