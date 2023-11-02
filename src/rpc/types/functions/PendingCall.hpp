

using MessageFunc=std::function<void(DataInput rawData)>;

template<typename... Args>
MessageFunc createMessageFunc(std::function<void(Args...)> func){
	return MessageFunc([func](DataInput data){
		if(!DynamicData::callDynamicArray(data,func))
			Serial.println("Discarding Message, unable to dynamically read for listener");
	});
}


struct PendingCall{
	enum PendingCallState{
		Pending=0,
		Success=1,
		Error=2,
	};

	struct Shared{
		const int32_t callId;
		MessageFunc listener=nullptr;

		explicit Shared(int32_t callId):callId(callId){}

		bool isCancelled=false;

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
		std::vector<uint8_t> _success;
		RpcError _error{""};
		std::function<void(DataInput result)> onSuccess;
		std::function<void(RpcError)> onError;
		PendingCallState state=Pending;

		void resolve(DataInput data){
			if(state)return;
			state=Success;
			uint32_t available=data.available();
			_success.resize(available);
			data.readFully(_success.data(),available);

			if(!onSuccess)return;
			onSuccess(DataInput(_success.data(),_success.size()));
			onSuccess=nullptr;//no longer used, can be freed
		}
		void reject(const RpcError& error){
			if(state)return;
			state=Error;
			_error=error;
			if(!onError)return;
			onError(error);
			onError=nullptr;//no longer used, can be freed
		}
	};




	const std::shared_ptr<PendingCall::Shared> _data;

	explicit PendingCall(const std::shared_ptr<PendingCall::Shared>& data):_data(data){}

	bool isCancelled()const{
		return _data->isCancelled;
	}

	void cancel()const{
		if(_data->isCancelled)return;
		_data->isCancelled=true;
		if(!_data->state){
			DataOutput data;
			data.writeByte(RpcConnection::PacketType::FunctionCancel);
			data.writeLength(_data->callId);
			RpcConnection::send(data);
		}
	}


	template<typename... Args>
	void sendMessage(Args... args)const{
		if(_data->state)return;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::MessageToExecutor);
		data.writeLength(_data->callId);
		DynamicData::writeDynamicArray(data,args...);
		RpcConnection::send(data);
	}

	void setMessageListener(MessageFunc func)const{_data->setMessageListener(std::move(func));}

	template<typename... Args>
	void setMessageListener(std::function<void(Args...)> func)const{_data->setMessageListener(createMessageFunc(func));}
	

	PendingCallState state()const{
		return _data->state;
	}

	void then(const std::function<void(DataInput result)>& onSuccess,const std::function<void(RpcError)>& onError)const{
		switch(_data->state){
			case Pending:
				_data->onSuccess=onSuccess;
				_data->onError=onError;
				break;
			case Success:
				if(onSuccess!=nullptr)
					onSuccess(DataInput(_data->_success.data(),_data->_success.size()));
				break;
			case Error:
				if(onError!=nullptr)
					onError(_data->_error);
				break;
		}
	}

	//TODO use make_function
	template<typename T>
	void then(const std::function<void(T result)>& onSuccess,const std::function<void(RpcError)>& onError)const{
		then(std::function<void(DataInput result)>([onSuccess,onError](DataInput data){
				if(!DynamicData::callDynamic(data,onSuccess)&&onError!=nullptr)
					onError(RpcError("Error casting result"));
			}),onError);
	}
};