
namespace RpcInternal{
	namespace RpcConnection{
		std::map<int32_t,std::shared_ptr<PendingCall::Shared>> activeRequests;
		int32_t nextCallId;
	}
	
	template<typename... Args>
	PendingCall callRemoteFunction(String type,String method,Args... args){
		//Intentionally no check for local functions, as this is for microcontrollers, they shouldn't do any shinanigans with calling own functions
		/*auto it=RegisteredTypes::registered.find(type);
		if(it!=RegisteredTypes::registered.end()){}*/

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::FunctionCall);
		const int32_t callId=RpcConnection::nextCallId++;
		data.writeLength(callId);
		data.writeString(type);
		data.writeString(method);
		DynamicData::writeDynamicArray(data,args...);

		const std::shared_ptr<PendingCall::Shared>& shared=std::make_shared<PendingCall::Shared>(callId);


		if(!(RpcConnection::connected||(type==NULL_STRING&&RpcConnection::webSocket.isConnected()))){
			shared->reject(RpcConnectionError("Not connected"));
			return PendingCall(shared);
		}

		RpcConnection::activeRequests[callId]=shared;
		RpcConnection::send(data);

		return PendingCall(shared);
	}
}