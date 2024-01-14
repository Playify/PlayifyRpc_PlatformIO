struct RpcListener{
public:
	MessageFunc _messageFunc;
	std::function<void()> _onDisconnect;
	String _type;
	String _method;
private:
	uint16_t _lastTime;
	uint16_t _timeout;
	PendingCall _call;
	
public:
	explicit RpcListener(
			MessageFunc messageFunc,
			std::function<void()> onDisconnect,
			String type,
			String method="listen"):
			_messageFunc(std::move(messageFunc)),
			_onDisconnect(std::move(onDisconnect)),
			_type(std::move(type)),
			_method(std::move(method)){
		_lastTime=millis();
	}
	

	void loop(){
		auto deltaTime=millis()-_lastTime;
		_lastTime+=deltaTime;
		
		if(!RpcConnection::connected)_timeout=0;
		else if(!_call.state())_timeout=1000;
		else if(_timeout>deltaTime)_timeout-=deltaTime;
		else{
			_timeout+=1000-deltaTime;
			_call=callRemoteFunction(_type,_method);
			_call.setMessageListener(_messageFunc);

			auto onDisconnect=_onDisconnect;
			_call.then(onDisconnect,[onDisconnect](const RpcError&){onDisconnect();});
		}
	}
};