#include <utility>


struct FunctionCallContext{

	struct Shared{
		bool isFinished=false;
		const int32_t callId;
		const String type;
		const String method;

		explicit Shared(int32_t callId,String type,String method):
		callId(callId),
		type(type),
		method(method)
		{}

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

		void cancelSelf(){
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
		_data->cancelSelf();
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

	void getCaller(std::function<void(String)> func) const{
		if(!_data) func(NULL_STRING);
		else callRemoteFunction(NULL_STRING,"c",_data->callId).then(func,[func](const RpcError& err){
			func(NULL_STRING);
		});
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
		data.writeError(error.append(
				(_data->type==NULL_STRING?"<<null>>":_data->type)+"."+
				(_data->method==NULL_STRING?"<<null>>":_data->method)+"(...)"
				));
		RpcConnection::send(data);
	}
	void reject(const String& msg) const{ reject(RpcError(msg.c_str()));}
	void reject(const char*& msg) const{ reject(RpcError(msg));}

	bool operator <(const FunctionCallContext& other) const{ return _data<other._data; }

	bool operator >(const FunctionCallContext& other) const{ return _data>other._data; }

	bool operator ==(const FunctionCallContext& other) const{ return _data==other._data; }

	explicit operator bool() const{ return bool(_data); }
};