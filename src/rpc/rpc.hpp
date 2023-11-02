#include <utility>

#if ESP32
#include "WiFi.h"
#elif ESP8266
#include "ESP8266WiFi.h"
#endif

#include "vector"
#include "internal/make_function.hpp"

struct PendingCall;

template<typename... Args>
PendingCall callRemoteFunction(String type,String method,Args... args);

namespace Rpc{
	extern String nameOrId;
	extern String id;
}

#include "types/RpcError.hpp"
#include "types/data/DataInput.hpp"
#include "types/data/DataOutput.hpp"
#include "types/data/DynamicWrite.hpp"
#include "types/data/DynamicRead.hpp"
#include "connection/WebSocketConnection.hpp"

#include "types/functions/PendingCall.hpp"
#include "types/functions/FunctionCallContext.hpp"
#include "internal/RegisteredTypes.hpp"
#include "internal/CallFunction.hpp"

#include "types/RpcFunction.hpp"
#include "types/RpcObject.hpp"

#include "connection/receive.hpp"

template<typename T=void>
using Callback=std::function<void(T t)>;


namespace Rpc{
	//Rpc
	String id;
	String nameOrId;

	void setName(const String& name){
		if(!id.length()) id="esp@"+WiFi.macAddress();
		nameOrId=name!=nullptr?name+" ("+id+")":id;
		if(RpcConnection::connected)
			callRemoteFunction(NULL_STRING,"N",nameOrId);
	}

	//Connection
	void setup(const String& rpcToken,String host,uint16_t port=80,String path="/rpc"){
		id="esp@"+WiFi.macAddress();
		RpcConnection::setup(rpcToken,std::move(host),port,std::move(path));
		RegisteredTypes::setup();
	}

	void loop(){
		RpcConnection::loop();
	}

	bool isConnected(){ return RpcConnection::connected; }


	//Functions
	RpcObject createObject(String type){ return RpcObject(std::move(type)); }

	RpcFunction createFunction(String type,String method){ return RpcFunction(std::move(type),std::move(method)); }

	RpcFunction registerFunction(CallReceiver func){
		String method(RegisteredTypes::nextFunctionId++);
		RegisteredTypes::registeredFunctions[method]=std::move(func);
		return RpcFunction("$"+id,method);
	}

	void unregisterFunction(const RpcFunction& func){
		if(func.type!=("$"+id))return;
		RegisteredTypes::registeredFunctions.erase(func.method);
	}

	//callLocal not supported

	template<typename... Args>
	PendingCall callFunction(String type,String method,Args... args){
		return callRemoteFunction(type,method,args...);
	}

	//Types
	std::map<String,CallReceiver>* registerType(const String& type){ return RegisteredTypes::registerType(type); }

	void registerType(const String& type,std::map<String,CallReceiver>* map){ RegisteredTypes::registerType(type,map); }

	void unregisterType(const String& type){ RegisteredTypes::unregisterType(type,true); }


	void checkTypes(const std::vector<String>& types,const Callback<int32_t>& callback){
		callFunction(NULL_STRING,"?",MultipleArguments<String>(types))
				.then([callback](DataInput data){
					if(data.readLength()=='i') callback(data.readInt());
					else callback(-1);
				},[callback](const RpcError&){
					callback(-1);
				});
	}

	void checkType(const String& type,const Callback<bool>& callback){
		checkTypes(std::vector<String>(1,type),Callback<int32_t>([callback](int32_t n){
			callback(n>0);
		}));
	}

	void getAllTypes(const Callback<std::vector<String>>& callback){
		callFunction(NULL_STRING,"T").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}

	void getAllConnections(const Callback<std::vector<String>>& callback){
		callFunction(NULL_STRING,"C").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}
}