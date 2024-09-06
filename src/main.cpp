#include <Arduino.h>
#include <WebSocketsClient.h>


#include "secrets.hpp"
/* contents of secrets.hpp:
#define RPC_HOST "192.168.0.1"
#define RPC_TOKEN "????"
void connectWifi(){
	...
}
*/

#include "rpc/WebDebugger.hpp"
#include "rpc/rpc.hpp"

void func(const int&){}


int x;
void setup() {
	Serial.begin(115200);
	
	connectWifi();

	WebDebugger::setup();
	
	Rpc::setName("EspTest");
	Rpc::setup(RPC_TOKEN,RPC_HOST,80);

	auto type=Rpc::registerType("EspTest");

	(*type)["params"].add([](const FunctionCallContext& ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	});
	(*type)["arr"].add([](const FunctionCallContext& ctx,const std::vector<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	});
	(*type)["call"].add([](const FunctionCallContext& ctx,RpcFunction func){
		
		func.call().then([ctx](DataInput data){
			Serial.println("Forwarding success");
			ctx.resolve(data);
		},[ctx](RpcError e){
			Serial.println("Forwarding error");
			e.printStackTrace();
			ctx.reject(e);
		});
	});

	(*type)["test"].add([](const FunctionCallContext& ctx){
		ctx.resolve();
	},(int*)nullptr);
	(*type)["test2"].smartProperty(x);
}

void test(){
	RegisteredType  type;
	type["func2"].add([](FunctionCallContext ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	});
	
	std::vector<String> null;
	DataInput data;
	RpcInternal::DynamicData::readDynamic(data,null);
	data.tryCallSingle([](std::vector<nullptr_t> n){

	});

	data.tryCall(func);

	Rpc::callFunction("EspTest","func").then([](String z){
	});
	Rpc::callFunction("EspTest","func").then([](){
	});
	
	

	RpcInternal::DynamicData::getMethodSignature(RpcInternal::removeConstReferenceParameters(RpcInternal::make_function([](const FunctionCallContext& ctx,MultipleArguments<bool> multi){

	})),true);
	RpcInternal::DynamicData::getMethodSignature(RpcInternal::removeConstReferenceParameters(RpcInternal::make_function([](const FunctionCallContext& ctx,std::vector<bool> multi){

	})),true);/*
	RpcInternal::DynamicData::getMethodSignature(RpcInternal::removeConstReferenceParameters(RpcInternal::make_function([](const FunctionCallContext& ctx,std::exception multi){

	})),true);*/
	RpcInternal::DynamicData::getMethodSignature(RpcInternal::removeConstReferenceParameters(RpcInternal::make_function([](const FunctionCallContext& ctx,int multi){

	})),true);
}




void loop() {
	WebDebugger::loop();
	Rpc::loop();
}
