#include <Arduino.h>
#include <WebSocketsClient.h>

#include "secrets.hpp"
/* contents of secrets.hpp:
#define RPC_HOST "192.168.0.1"
#define RPC_TOKEN "????"
*/

#include "rpc/rpc.hpp"



void setup() {
	Serial.begin(115200);
	
	//TODO connect with WiFi
	Rpc::setName("EspTest");
	Rpc::setup(RPC_TOKEN,RPC_HOST);
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
