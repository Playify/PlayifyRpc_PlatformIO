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

	std::shared_ptr<FunctionCallContext::Shared> _data;

	constexpr FunctionCallContext():_data(){}
	constexpr FunctionCallContext(nullptr_t):_data(){}

	explicit FunctionCallContext(const std::shared_ptr<FunctionCallContext::Shared>& data):_data(data){}

	[[nodiscard]] bool isFinished() const{
		return !_data||_data->isFinished;
	}

	[[nodiscard]] bool isCancelled() const{
		return !_data||_data->isCancelled;
	}

	void cancelSelf() const{
		if(!_data)return;
		_data->cancel();
	}

	void onCancel(const std::function<void()>& onCancel) const{
		if(!_data)return;
		if(_data->isCancelled)onCancel();
		else _data->onCancel=onCancel;
	}


	template<typename... Args>
	void sendMessage(Args... args) const{
		if(!_data)return;
		if(_data->isFinished)return;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::MessageToExecutor);
		data.writeLength(_data->callId);
		DynamicData::writeDynamicArray(data,args...);
		RpcConnection::send(data);
	}

	void setMessageListener(MessageFunc func) const{
		if(!_data)return;
		_data->setMessageListener(std::move(func));
	}

	template<typename... Args>
	void setMessageListener(std::function<void(Args...)> func) const{
		if(!_data)return;
		_data->setMessageListener(make_messageFunc(func));
	}

	template<typename Func>
	void setMessageListener(Func func) const{
		if(!_data)return;
		_data->setMessageListener(make_messageFunc(func));
	}


	template<typename T>
	void resolve(T result) const{
		if(!_data)return;
		if(_data->isFinished)return;
		_data->isFinished=true;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::FunctionSuccess);
		data.writeLength(_data->callId);
		DynamicData::writeDynamic(data,result);
		RpcConnection::send(data);
	}
	void resolve() const{resolve(nullptr);}

	void reject(const RpcError& error) const{
		if(!_data)return;
		if(_data->isFinished)return;
		_data->isFinished=true;

		DataOutput data;
		data.writeByte(RpcConnection::PacketType::FunctionError);
		data.writeLength(_data->callId);
		data.writeError(error);
		RpcConnection::send(data);
	}
	void reject(const String& msg) const{ reject(RpcError(msg.c_str()));}
	void reject(const char*& msg) const{ reject(RpcError(msg));}

	bool operator <(const FunctionCallContext& other) const{ return _data<other._data; }

	bool operator >(const FunctionCallContext& other) const{ return _data>other._data; }

	bool operator ==(const FunctionCallContext& other) const{ return _data==other._data; }

	explicit operator bool() const{ return bool(_data); }
};