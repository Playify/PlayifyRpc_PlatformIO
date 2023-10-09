
#include "WebSocketsClient.h"

#ifndef RPC_TOKEN
#error RPC_TOKEN is not defined
#endif
#ifndef RPC_HOST
#error RPC_HOST is not defined
#endif

namespace RpcConnection{

	enum PacketType{
		FunctionCall=0,
		FunctionSuccess=1,
		FunctionError=2,
		FunctionCancel=3,
		MessageToExecutor=4,
		MessageToCaller=5,
	};
	
	bool connected=false;
	WebSocketsClient webSocket;
	void onEvent(WStype_t type, uint8_t * payload, size_t length);
	void doConnect();
	void doDisconnect();
	
	void setup(){
		webSocket.onEvent(onEvent);
		webSocket.begin(RPC_HOST,80,"/rpc");
		webSocket.setExtraHeaders("Cookie: token=" RPC_TOKEN ";");
		webSocket.setReconnectInterval(5000);


		// start heartbeat (optional)
		// ping server every 15000 ms
		// expect pong from server within 3000 ms
		// consider connection disconnected if pong is not received 2 times
		webSocket.enableHeartbeat(15000, 3000, 2);
	}
	
	void loop(){
		webSocket.loop();
	}

	void receiveRpc(DataInput data);

	void onEvent(WStype_t type, uint8_t * payload, size_t length){
		switch(type){
			case WStype_DISCONNECTED:
				connected=false;
				doDisconnect();
				Serial.println("[WebSocket] Disconnected!");
				break;
			case WStype_CONNECTED:{
				Serial.print("[WebSocket] Connected to url: ");
				Serial.println((char*)payload);
				
				doConnect();
				break;
			}
			case WStype_TEXT:
				Serial.print("[WebSocket] : ");
				Serial.println((char*)payload);
				break;
			case WStype_BIN:{
				Serial.printf("[WebSocket] recv: %u\n",length);
				
				receiveRpc(DataInput(payload,length));
				break;
			}
			default:
				break;
		}
	}

	void send(const DataOutput& data){
		webSocket.sendBIN(data.data(),data.size());
	}
}