


namespace RpcInternal{
	namespace DynamicData{
		template<typename... Args>
		void writeDynamicArray(DataOutput& data,Args... args);
	}
}


struct PendingCall{
	enum PendingCallState{
		Pending=0,
		Success=1,
		Error=2,
	};

	struct Shared{
		const int32_t callId;
		RpcInternal::MessageFunc listener=nullptr;

		explicit Shared(int32_t callId):callId(callId){}

		bool isCancelled=false;

		void setMessageListener(RpcInternal::MessageFunc func){
			listener=std::move(func);
		}

		void doReceive(DataInput data) const{
			if(listener==nullptr){
				Serial.println("[Rpc] Error while receiving message: No listener is defined");
				return;
			}
			listener(data);
		}

		std::vector<uint8_t> _success;
		uint32_t _initialLength;
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
			_initialLength=data._initialLength;

			if(!onSuccess)return;
			onSuccess(DataInput(_success.data(),_success.size(),data._initialLength));
			onSuccess=nullptr;//no longer used, can be freed
			onError=nullptr;//no longer used, can be freed
		}

		void reject(const RpcError& error){
			if(state)return;
			state=Error;
			_error=error;
			if(!onError)return;
			onError(error);
			onSuccess=nullptr;//no longer used, can be freed
			onError=nullptr;//no longer used, can be freed
		}
	};


	std::shared_ptr<PendingCall::Shared> _data;

	explicit PendingCall():_data(){}
	explicit PendingCall(const std::shared_ptr<PendingCall::Shared>& data):_data(data){}

	[[nodiscard]] bool isCancelled() const{
		return _data&&_data->isCancelled;
	}

	void cancel() const{
		if(!_data)return;
		if(_data->isCancelled)return;
		_data->isCancelled=true;
		if(!_data->state){
			DataOutput data;
			data.writeByte(RpcInternal::RpcConnection::PacketType::FunctionCancel);
			data.writeLength(_data->callId);
			RpcInternal::RpcConnection::send(data);
		}
	}


	template<typename... Args>
	void sendMessage(Args... args) const{
		if(!_data)return;
		if(_data->state)return;

		DataOutput data;
		data.writeByte(RpcInternal::RpcConnection::PacketType::MessageToExecutor);
		data.writeLength(_data->callId);
		RpcInternal::DynamicData::writeDynamicArray(data,args...);
		RpcInternal::RpcConnection::send(data);
	}

	template<typename Func>
	void setMessageListener(Func func) const{
		if(!_data)return;
		_data->setMessageListener(RpcInternal::make_messageFunc(func));
	}


	[[nodiscard]] PendingCallState state() const{
		if(!_data)return PendingCallState::Error;
		return _data->state;
	}

	const PendingCall& then(const std::function<void(DataInput result)>& onSuccess) const{
		if(!_data) return *this;
		switch(_data->state){
			case Pending:
				_data->onSuccess=onSuccess;
				break;
			case Success:
				if(onSuccess!=nullptr)
					onSuccess(DataInput(_data->_success.data(),_data->_success.size(),_data->_initialLength));
				break;
			case Error:
				break;
		}
		return *this;
	}

	const PendingCall& onError(const std::function<void(RpcError error)>& onError) const{
		if(!_data){
			if(onError!=nullptr)
				onError(RpcError("PendingCall not initialized"));
			return *this;
		}
		switch(_data->state){
			case Pending:
				_data->onError=onError;
				break;
			case Success:
				break;
			case Error:
				if(onError!=nullptr)
					onError(_data->_error);
				break;
		}
		return *this;
	}

	const PendingCall& then(const std::function<void(DataInput result)>& onSuccess,const std::function<void(RpcError error)>& onError) const{
		if(!_data){
			if(onError!=nullptr)
				onError(RpcError("PendingCall not initialized"));
			return *this;
		}
		switch(_data->state){
			case Pending:
				_data->onSuccess=onSuccess;
				_data->onError=onError;
				break;
			case Success:
				_data->onError=onError;//Set error handler while running onSuccess, in case of casting error
				if(onSuccess!=nullptr)
					onSuccess(DataInput(_data->_success.data(),_data->_success.size(),_data->_initialLength));
				_data->onError=nullptr;
				break;
			case Error:
				if(onError!=nullptr)
					onError(_data->_error);
				break;
		}
		return *this;
	}


private:

	template<typename T>
	const PendingCall&
	_then(const std::function<void(T result)>& onSuccess,const std::function<void(RpcError error)>& onError) const{
		auto sharedData=_data;
		return then(std::function<void(DataInput result)>([onSuccess,sharedData](DataInput data){

			if(!data.tryCallSingle(onSuccess)&&sharedData&&sharedData->onError!=nullptr)
				sharedData->onError(RpcDataError("Error casting result"));
		}),onError);
	}

	const PendingCall&
	_then(const std::function<void()>& onSuccess,const std::function<void(RpcError error)>& onError) const{
		return then(
			{[onSuccess](DataInput data){ onSuccess(); }},
			onError);
	}

	template<typename T>
	const PendingCall& _then(const std::function<void(T result)>& onSuccess) const{
		auto sharedData=_data;
		return then(std::function<void(DataInput result)>([onSuccess,sharedData](DataInput data){
			if(!data.tryCallSingle(onSuccess)&&sharedData&&sharedData->onError!=nullptr)
				sharedData->onError(RpcDataError("Error casting result"));
		}));
	}

	const PendingCall& _then(const std::function<void()>& onSuccess) const{
		return then(std::function<void(DataInput)>([onSuccess](DataInput){
			onSuccess();
		}));
	}

public:
	template<typename T>
	const PendingCall& then(const std::function<void(T result)>& onSuccess) const{ return _then(onSuccess); }

	const PendingCall& then(const std::function<void()>& onSuccess) const{ return _then(onSuccess); }

	template<typename SuccessFunc>
	const PendingCall& then(const SuccessFunc& onSuccess) const{
		return _then(RpcInternal::Helpers::function(onSuccess));
	}

	template<typename T>
	const PendingCall& then(const std::function<void(T result)>& onSuccess,const std::function<void(RpcError error)>& onError) const{
		return _then(onSuccess,onError);
	}

	const PendingCall&
	then(const std::function<void()>& onSuccess,const std::function<void(RpcError error)>& onError) const{
		return _then(onSuccess,onError);
	}

	template<typename SuccessFunc>
	const PendingCall& then(const SuccessFunc& onSuccess,const std::function<void(RpcError error)>& onError) const{
		return _then(RpcInternal::Helpers::function(onSuccess),onError);
	}
	const PendingCall& finally(const std::function<void(bool success)>& callback) const{
		return then(RpcInternal::Helpers::MakeFunction::make_function([callback](DataInput){callback(true);}),[callback](RpcError){callback(false);});
	}



	bool operator <(const PendingCall& other) const{ return _data<other._data; }

	bool operator >(const PendingCall& other) const{ return _data>other._data; }

	bool operator ==(const PendingCall& other) const{ return _data==other._data; }

	explicit operator bool() const{ return bool(_data); }
};