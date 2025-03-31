#pragma once

#include <utility>
#include <list>
#include "vector"

#if ESP32

#include "WiFi.h"

#elif ESP8266
#include "ESP8266WiFi.h"
#endif



enum ProgrammingLanguage{
	CSharp,
	TypeScript,
	JavaScript,
};

#include "internal/helpers.hpp"
#include "internal/CallReceiver.hpp"


struct PendingCall;

namespace RpcInternal{
	template<typename... Args>
	PendingCall callRemoteFunction(String type,String method,Args... args);


	using MessageFunc=std::function<void(DataInput rawData)>;

	template<typename T>
	MessageFunc make_messageFunc(T func);

	std::list<std::function<bool()>> onLoopCallbacks;
}

namespace Rpc{
	extern String name;

	extern String prettyName();

	extern String id;


	//Will be called on loop(), when returning true, the function gets removed
	void addOnLoop(std::function<bool()> func){
		RpcInternal::onLoopCallbacks.push_back(func);
	}
}

template<typename T=void>
using Callback=std::function<void(T t)>;

#include "types/RpcError.hpp"
#include "types/errors/PredefinedErrors.hpp"
#include "types/data/DataInput.hpp"
#include "types/data/DataOutput.hpp"


#include "connection/WebSocketConnection.hpp"
#include "types/functions/PendingCall.hpp"

#include "types/RpcFunction.hpp"
#include "types/RpcObject.hpp"

#include "types/data/MultipleArguments.hpp"
#include "types/data/DynamicData.hpp"
#include "types/data/DynamicWrite.hpp"
#include "types/data/DynamicRead.hpp"
#include "types/data/DynamicTypeStringifier.hpp"


#include "types/functions/FunctionCallContext.hpp"
#include "internal/RegisteredTypes.hpp"
#include "internal/CallFunction.hpp"


#include "connection/receive.hpp"

#include "internal/CallReceiver.Impl.hpp"
#include "utils/RpcHelpers.hpp"

#include "RpcLogger.hpp"

namespace Rpc{
	//Rpc
	String id;

	String prettyName(){ return name!=NULL_STRING?name+" ("+id+")":id; }

	String name=NULL_STRING;

	void setName(const String& n){
		name=n;
		if(RpcInternal::RpcConnection::connected)
			RpcInternal::callRemoteFunction(NULL_STRING,"N",name);
	}

	//Connection
	void setup(const String& rpcToken,const String& host,uint16_t port,const String& path="/rpc"){
		id=String("esp@")+WiFi.getHostname()+"@"+WiFi.macAddress();
		RpcInternal::RpcConnection::setup(rpcToken,host,port,path);
		RpcInternal::RegisteredTypes::setup();
	}

	void loop(){
		RpcInternal::RpcConnection::loop();

		for(auto it=RpcInternal::onLoopCallbacks.begin();it!=RpcInternal::onLoopCallbacks.end();)
			if((*it)()) it=RpcInternal::onLoopCallbacks.erase(it);
			else ++it;
	}

	bool isConnected(){ return RpcInternal::RpcConnection::connected; }

	//Functions
	RpcObject createObject(String type){ return RpcObject(std::move(type)); }

	RpcFunction createFunction(String type,String method){ return RpcFunction(std::move(type),std::move(method)); }

	RpcFunction registerFunction(CallReceiver func){
		String method(RpcInternal::RegisteredTypes::nextFunctionId++);
		RpcInternal::RegisteredTypes::registeredFunctions[method]=std::move(func);
		return RpcFunction("$"+id,method);
	}

	void unregisterFunction(const RpcFunction& func){
		if(func.type!=("$"+id))return;
		RpcInternal::RegisteredTypes::registeredFunctions.erase(func.method);
	}

	//callLocal not supported

	template<typename... Args>
	PendingCall callFunction(String type,String method,Args... args){
		return RpcInternal::callRemoteFunction(type,method,args...);
	}

	//Types
	String generateTypeName(){
		static char possible[65]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
		char uuid[17];
#ifdef ESP32
		esp_fill_random(uuid,16);
#else
		uint32_t *uuid_int = (uint32_t *)uuid;
		uuid_int[0] = RANDOM_REG32;
		uuid_int[1] = RANDOM_REG32;
		uuid_int[2] = RANDOM_REG32;
		uuid_int[3] = RANDOM_REG32;
#endif
		for(uint8_t i=0;i<sizeof(uuid);i++)uuid[i]=possible[uuid[i]&63];
		return "$"+Rpc::id+"$"+String(uuid);
	}

	RegisteredType* registerType(const String& type){ return RpcInternal::RegisteredTypes::registerType(type); }

	void registerType(const String& type,RegisteredType* map){ RpcInternal::RegisteredTypes::registerType(type,map); }

	void registerType(const String& type,RegisteredType& map){ RpcInternal::RegisteredTypes::registerType(type,&map); }

	String registerType(RegisteredType& map){
		String type=Rpc::generateTypeName();
		RpcInternal::RegisteredTypes::registerType(type,&map);
		return type;
	}

	void unregisterType(const String& type){ RpcInternal::RegisteredTypes::unregisterType(type); }


	void checkTypes(const std::vector<String>& types,const Callback<int32_t>& callback){
		callFunction(NULL_STRING,"?",MultipleArguments<String>(types))
				.then(callback,[callback](const RpcError&){
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