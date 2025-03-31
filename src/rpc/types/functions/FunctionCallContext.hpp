#include <utility>


struct FunctionCallContext{

	struct Shared{
		bool isFinished=false;
		const int32_t callId;
		const String type;
		const String method;

		explicit Shared(int32_t callId,String type,String method):
		callId(callId),
		type(std::move(type)),
		method(std::move(method))
		{}

		bool isCancelled=false;
		std::function<void()> onCancel;

		RpcInternal::MessageFunc listener=nullptr;

		void setMessageListener(RpcInternal::MessageFunc func){
			listener=std::move(func);
		}
		void doReceive(const DataInput& data) const{
			if(listener==nullptr){
				Serial.println("[Rpc] Error while receiving message: No listener is defined");
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

	std::shared_ptr<Shared> _data;

	constexpr FunctionCallContext()= default;
	// ReSharper disable once CppNonExplicitConvertingConstructor
	constexpr FunctionCallContext(nullptr_t){} // NOLINT(*-explicit-constructor)

	explicit FunctionCallContext(const std::shared_ptr<Shared>& data):_data(data){}

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

	void getCaller(const std::function<void(String)>& func) const{
		if(!_data) func(NULL_STRING);
		else RpcInternal::callRemoteFunction(NULL_STRING,"c",_data->callId).then(func,[func](const RpcError&){
			func(NULL_STRING);
		});
	}


	template<typename T>
	void resolve(T result) const{
		if(!_data)return;
		if(_data->isFinished)return;
		_data->isFinished=true;

		DataOutput data;
		data.writeByte(RpcInternal::RpcConnection::PacketType::FunctionSuccess);
		data.writeLength(_data->callId);
		RpcInternal::DynamicData::TypeDefinition<T>::writeDynamic(data,result);
		RpcInternal::RpcConnection::send(data);
	}
	void resolve() const{resolve(nullptr);}

	void reject(const RpcError& error) const{
		if(!_data)return;
		if(_data->isFinished)return;
		_data->isFinished=true;

		DataOutput data;
		data.writeByte(RpcInternal::RpcConnection::PacketType::FunctionError);
		data.writeLength(_data->callId);
		data.writeError(error.append(
				(_data->type==NULL_STRING?"<<null>>":_data->type)+"."+
				(_data->method==NULL_STRING?"<<null>>":_data->method)+"(...)"
				));
		RpcInternal::RpcConnection::send(data);
	}
	void reject(const String& msg) const{ reject(RpcError(msg.c_str()));}
	void reject(const char*& msg) const{ reject(RpcError(msg));}

	bool operator <(const FunctionCallContext& other) const{ return _data<other._data; }

	bool operator >(const FunctionCallContext& other) const{ return _data>other._data; }

	bool operator ==(const FunctionCallContext& other) const{ return _data==other._data; }

	explicit operator bool() const{ return bool(_data); }
};