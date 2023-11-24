struct RpcListener{
public:
	MessageFunc _messageFunc;
	std::function<void()> _onDisconnect;
	String _type;
	String _method;
private:
	Duration _timeout;
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
	}
	

	void loop(){
		if(!RpcConnection::connected)_timeout=Duration::zero();
		else if(!_call.state())_timeout=1_s;
		else if(TimeHandler::cyclicEvent(_timeout,1_s)){
			_call=callRemoteFunction(_type,_method);
			_call.setMessageListener(_messageFunc);

			auto onDisconnect=_onDisconnect;
			_call.then(onDisconnect,[onDisconnect](const RpcError&){onDisconnect();});
		}
	}
};