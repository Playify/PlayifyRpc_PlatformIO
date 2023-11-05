

namespace RpcConnection{
	std::map<int32_t,std::shared_ptr<FunctionCallContext::Shared>> currentlyExecuting;

	void onError(const RpcError& e){
		Serial.println("Error connecting to Server");
		e.printStackTrace();
		webSocket.disconnect();
	}

	void doConnect(){
		callRemoteFunction(NULL_STRING,"N",Rpc::nameOrId).then([](){
			MultipleArguments<String> args;
			for(const auto& pair: RegisteredTypes::registered) args.push_back(pair.first);
			callRemoteFunction(NULL_STRING,"+",args).then([](){
				connected=true;
				Serial.println("Connected to Server");
			},onError);
		},onError);
	}

	void doDisconnect(){
		static const RpcError error("Connection closed");

		for(const auto& item: activeRequests)item.second->reject(error);
		activeRequests.clear();

		for(const auto& item: currentlyExecuting)item.second->cancel();
	}

	void receiveRpc(DataInput data){
		auto packetType=PacketType(data.readByte());

		switch(packetType){
			case FunctionCall:{
				int32_t callId=data.readLength();

				String type=data.readString();
				String method=data.readString();

				auto shared=std::make_shared<FunctionCallContext::Shared>(callId);
				auto fcc=FunctionCallContext(shared);

				const auto& typeIterator=RegisteredTypes::registered.find(type);
				if(typeIterator==RegisteredTypes::registered.end()){
					fcc.reject(RpcError("Type is not registered on this device"));
					break;
				}

				const auto& map=*typeIterator->second.first;
				const auto& methodIterator=map.find(method);
				if(methodIterator==map.end()){
					fcc.reject(RpcError("Method is not defined on this type"));
					break;
				}

				methodIterator->second(fcc,data);
				break;
			}
			case FunctionSuccess:{
				int32_t callId=data.readLength();
				const auto& it=activeRequests.find(callId);
				if(it==activeRequests.end())break;
				it->second->resolve(data);
				activeRequests.erase(it);
				break;
			}
			case FunctionError:{
				int32_t callId=data.readLength();
				const auto& it=activeRequests.find(callId);
				if(it==activeRequests.end())break;
				it->second->reject(data.readError());
				activeRequests.erase(it);
				break;
			}
			case FunctionCancel:{
				int32_t callId=data.readLength();
				const auto& it=currentlyExecuting.find(callId);
				if(it==currentlyExecuting.end())break;
				it->second->cancel();
				break;
			}
			case MessageToExecutor:{
				int32_t callId=data.readLength();
				const auto& it=currentlyExecuting.find(callId);
				if(it==currentlyExecuting.end())break;
				it->second->doReceive(data);
				break;
			}
			case MessageToCaller:{
				int32_t callId=data.readLength();
				const auto& it=activeRequests.find(callId);
				if(it==activeRequests.end())break;
				it->second->doReceive(data);
				break;
			}
		}
	}
}