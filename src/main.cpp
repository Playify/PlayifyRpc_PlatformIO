#include <Arduino.h>


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
RegisteredType type;
void setup() {
	Rpc::registerFunction()

	Serial.begin(115200);

	connectWifi();

	WebDebugger::setup();

	Rpc::setName("EspTest");
	Rpc::setup(RPC_TOKEN,RPC_HOST,80);

	Rpc::registerType("EspTest",type);

	type["params"].add([](const FunctionCallContext& ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	},ReturnType<String>(),"args");
	type["arr"].add([](const FunctionCallContext& ctx,const std::vector<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	},ReturnType<String>(),"args");
	type["call"].add([](const FunctionCallContext& ctx,RpcFunction func){

		func.call().then([ctx](DataInput data){
			Serial.println("Forwarding success");
			ctx.resolve(data);
		},[ctx](RpcError e){
			Serial.println("Forwarding error");
			e.printStackTrace();
			ctx.reject(e);
		});
	},ReturnType<nullptr_t>(),"func");

	type["test"].add([](const FunctionCallContext& ctx){
		ctx.resolve();
	},ReturnType<int>());
	type["test"].add([](const FunctionCallContext& ctx){
		ctx.resolve();
	},nullptr);
	type["test2"].smartProperty(x);

	type["wrap"].func([](int i){
		return i+1;
	},"i");
	type["wrap"].func([](int i){
	},"i");

	type["wrap"].func([](){
	});
}

void test(){
	type["func2"].add([](FunctionCallContext ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	},ReturnType<String>(),"args");

	std::vector<String> null;
	DataInput data;
	RpcInternal::DynamicData::readDynamic(data,null);
	data.tryCallSingle([](std::vector<nullptr_t> n){

	});


	Rpc::callFunction("EspTest","func").then([](String z){
	});
	Rpc::callFunction("EspTest","func").then([](){
	});

	std::array<int,4> y={1,2,2};
}




void loop() {
	WebDebugger::loop();
	Rpc::loop();
}
