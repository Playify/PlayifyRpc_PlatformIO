#include <utility>



struct FunctionCallContext{

	struct Shared{
		bool isFinished=false;
		const int32_t callId;

		explicit Shared(int32_t callId):callId(callId){}

		bool isCancelled=false;
		std::function<void()> onCancel;

		MessageFunc listener=nullptr;
		void setMessageListener(MessageFunc func){
			listener=std::move(func);
		}

		void doReceive(DataInput data) const{
			if(listener==nullptr){
				Serial.println("Discarding Message, as no listener was defined");
				return;
			}
			listener(data);
		}
		void cancel(){
			if(isFinished||isCancelled)return;
			isCancelled=true;
			if(onCancel!=nullptr){
				onCancel();
				onCancel=nullptr;
			}
		}
	};

	const std::shared_ptr<FunctionCallContext::Shared> _data;

	explicit FunctionCallContext(const std::shared_ptr<FunctionCallContext::Shared>& data):_data(data){}

	bool isFinished(){
		return _data->isFinished;
	}
	bool isCancelled(){
		return _data->isCancelled;
	}

	void cancel(){
		_data->cancel();
	}
	void onCancel(const std::function<void()>& onCancel){
		if(_data->isCancelled)onCancel();
		else _data->onCancel=onCancel;
	}



	template<typename... Args>
	void sendMessage(Args... args){
		if(_data->isFinished)return;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::MessageToExecutor);
		data.writeLength(_data->callId);
		DynamicData::writeDynamicArray(data,args...);
		RpcConnection::send(data);
	}

	void setMessageListener(MessageFunc func){_data->setMessageListener(std::move(func));}
	template<typename... Args>
	void setMessageListener(std::function<void(Args...)> func){_data->setMessageListener(createMessageFunc(func));}
	
	


	template<typename T>
	void resolve(T result){
		if(_data->isFinished)return;
		_data->isFinished=true;
		
		DataOutput data;
		data.writeByte(RpcConnection::PacketType::FunctionSuccess);
		data.writeLength(_data->callId);
		DynamicData::writeDynamic(data,result);
		RpcConnection::send(data);
	}
	void reject(const RpcError& error){
		if(_data->isFinished)return;
		_data->isFinished=true;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::FunctionError);
		data.writeLength(_data->callId);
		data.writeError(error);
		RpcConnection::send(data);
	}
};