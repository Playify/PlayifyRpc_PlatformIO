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
	extern String name;
	extern String prettyName();
	extern String id;
}

template<typename T=void>
using Callback=std::function<void(T t)>;

#include "types/RpcError.hpp"
#include "types/Errors/PredefinedErrors.hpp"
#include "types/data/DataInput.hpp"
#include "types/data/DataOutput.hpp"
#include "types/data/DynamicWrite.hpp"
#include "types/data/DynamicRead.hpp"
#include "connection/WebSocketConnection.hpp"

#include "types/functions/PendingCall.hpp"
#include "types/functions/FunctionCallContext.hpp"
#include "internal/RegisteredTypes.hpp"
#include "internal/CallFunction.hpp"
#include "internal/helperFunctionsForUser.hpp"

#include "types/RpcFunction.hpp"
#include "types/RpcObject.hpp"

#include "connection/receive.hpp"

#include "utils/RpcListener.hpp"

#include "types/data/DynamicWrite.Impl.hpp"
#include "types/data/DynamicRead.Impl.hpp"


namespace Rpc{
	//Rpc
	String id;
	String prettyName(){return name!=NULL_STRING?name+" ("+id+")":id;}
	String name=NULL_STRING;

	void setName(const String& n){
		name=n;
		if(RpcConnection::connected)
			callRemoteFunction(NULL_STRING,"N",name);
	}

	//Connection
	void setup(const String& rpcToken,const String& host,uint16_t port,const String& path="/rpc"){
		id=String("esp@")+WiFi.getHostname()+"@"+WiFi.macAddress();
		RpcConnection::setup(rpcToken,host,port,path);
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
	template<typename CallReceiver>
	RpcFunction registerFunction(CallReceiver func){
		String method(RegisteredTypes::nextFunctionId++);
		RegisteredTypes::registeredFunctions[method]=CallReceiver(func);
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
	String generateTypeName(){
		static char possible[65]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
		char uuid[16];
		esp_fill_random(uuid,sizeof(uuid));
		for(uint8_t i=0;i<sizeof(uuid);i++)uuid[i]=possible[uuid[i]&63];
		return "$"+Rpc::id+"$"+String(uuid,16);
	}
	
	RegisteredType* registerType(const String& type){ return RegisteredTypes::registerType(type); }

	void registerType(const String& type,RegisteredType* map){ RegisteredTypes::registerType(type,map); }
	void registerType(const String& type,RegisteredType& map){ RegisteredTypes::registerType(type,&map); }
	String registerType(RegisteredType& map){
		String type=Rpc::generateTypeName();
		RegisteredTypes::registerType(type,&map);
		return type;
	}

	void unregisterType(const String& type){ RegisteredTypes::unregisterType(type); }
	

	template<typename T>
	CallReceiver createCallReceiver(T t){return make_callReceiver(t);}//Helper for registering functions inside a type
	template<typename T>
	CallReceiver createSmartProperty(T t){return make_smartProperty(t);}//Helper for registering property accessors
	template<typename T>
	MessageFunc createMessageFunc(T t){return make_messageFunc(t);}//Helper for registering functions as message receiver


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
		callFunction("Rpc","getAllTypes").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}

	void getAllConnections(const Callback<std::vector<String>>& callback){
		callFunction("Rpc","getAllConnections").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}
	void getRegistrations(const Callback<std::vector<String>>& callback){
		callFunction("Rpc","getRegistrations").then(callback,[callback](const RpcError&){
			callback(std::vector<String>());
		});
	}
	void evalString(String expression,const Callback<String>& callback){
		callFunction("Rpc","evalString",expression).then(callback,[callback](const RpcError& e){
			callback(e.getStackTrace());
		});
	}
	template<typename T>
	void evalObject(String expression,const Callback<T>& callback,const Callback<RpcError> onError){
		callFunction("Rpc","evalObject",expression).then(callback,onError);
	}
	
	//Rpc.listenCalls() is not supported in C++
}