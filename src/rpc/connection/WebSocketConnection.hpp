#include "WebSocketsClient.h"

namespace RpcInternal{
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

		void onEvent(WStype_t type,uint8_t* payload,size_t length);

		void doConnect();

		void doDisconnect();

		String createUrl(String path);

		String _host;
		uint16_t _port;
		String _path;
		uint32_t lastTime;
		uint32_t _reconnect;
		bool _disconnect;

		void setup(const String& rpcToken,const String& host,uint16_t port,const String& path){
			webSocket.onEvent(onEvent);
			_host=host;
			_port=port;
			_path=path;
			webSocket.setExtraHeaders(("Cookie: RPC_TOKEN="+rpcToken).c_str());


			// start heartbeat (optional)
			// ping server every 15000 ms
			// expect pong from server within 3000 ms
			// consider connection disconnected if pong is not received 2 times
			webSocket.enableHeartbeat(15000,3000,2);

			lastTime=millis();
			_reconnect=1;
		}

		void loop(){
			auto deltaTime=millis()-lastTime;
			lastTime+=deltaTime;

			if(_disconnect){
				webSocket.disconnect();
			}


			if(_reconnect==0)
				webSocket.loop();

			if(!WiFi.isConnected())_reconnect=1;
			else{
				if(_reconnect>deltaTime){
					_reconnect-=deltaTime;
				}else if(_reconnect){
					_reconnect=0;
					webSocket.disconnect();
					webSocket.begin(_host,_port,createUrl(_path));
					webSocket.loop();
				}
			}
		}

		void receiveRpc(DataInput data);

		void onEvent(WStype_t type,uint8_t* payload,size_t length){
			switch(type){
				case WStype_DISCONNECTED:
					connected=false;
					doDisconnect();
					if(!WiFi.isConnected())Serial.println("[Rpc] Error connecting to RPC: Not yet connected to WiFi");
					else Serial.println("[Rpc] Reconnecting to RPC");

					_reconnect=5000;
					break;
				case WStype_CONNECTED:
					Serial.print("[Rpc] Connected to RPC (");
					Serial.print((char*)payload);
					Serial.println(")");

					doConnect();
					break;
				case WStype_TEXT:
					Serial.print("[Rpc] WebSocket message: ");
					Serial.println((char*)payload);
					break;
				case WStype_BIN:
					receiveRpc(DataInput(payload,length,length));
					break;
				//continuation frames are not yet handled correctly
				default:
					break;
			}
		}

		void send(const DataOutput& data){
			webSocket.sendBIN(data.data(),data.size());
		}
	}
}