#include <Arduino.h>
#include <WebSocketsClient.h>


#include "secrets.hpp"
/* contents of secrets.hpp:
#define RPC_HOST "192.168.0.1"
#define RPC_TOKEN "????"
*/

#include "rpc/TimeHandler.hpp"
#include "rpc/rpc.hpp"

void func(const int& x){
	
}


void setup() {
	Serial.begin(115200);

	//TODO connect with WiFi
	Rpc::setName("EspTest");
	Rpc::setup(RPC_TOKEN,RPC_HOST);

	auto test=Rpc::registerType("EspTest");

	(*test)["func"]=Rpc::createCallReceiver([](const FunctionCallContext& ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	});
	(*test)["func2"]=Rpc::createCallReceiver([](FunctionCallContext ctx,const MultipleArguments<String>& args){
		String result;
		for(const auto& s: args)result+=s;

		ctx.resolve(result);
	});

	DataInput data;
	data.tryCall(func);


	Rpc::callFunction("EspTest","func").then([](String z){
	});
	Rpc::callFunction("EspTest","func").then([](){
	});
}

void test(){
	std::vector<String> null;
	DataInput data;
	int i=1;
	DynamicData::read(data,i,null);
	DynamicData::readDynamic(data,null);
	DynamicData::callDynamicSingle(data,make_function([](std::vector<nullptr_t> n){

	}));
}


/*
namespace Rpc{
	String nameOrId;
}
#include "rpc/types/RpcError.hpp"
#include "rpc/types/data/DataInput.hpp"
#include "rpc/types/data/DataOutput.hpp"
#include "rpc/types/data/DynamicWrite.hpp"
#include "rpc/types/data/DynamicRead.hpp"

void test(){
	DataOutput d;
	DynamicData::writeDynamicArray(d,1,"TEST","u","ii","uaa");
	DataInput in(d.data(),d.size());

	DynamicData::callDynamicArray(in,std::function<void(int x,MultipleArguments<String> s)>([](int x,MultipleArguments<String> s){
		Serial.println(x);
		int i=0;
		for(const auto& item: s){
			Serial.print(i++);
			Serial.print(":");
			Serial.println(item);
		}
	}));
}*/




void loop() {
	Serial.println("LOOP");
	Rpc::loop();
}
