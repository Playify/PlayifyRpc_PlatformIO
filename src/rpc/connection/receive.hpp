
namespace RpcConnection{
	std::map<int32_t,std::shared_ptr<FunctionCallContext::Shared>> currentlyExecuting;
	std::vector<String> reportedTypes;
	String reportedName;

	bool isUrlEncoded(char c){
		return ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~');
	}

	const char* hex="0123456789ABCDEF";

	String urlEncode(String toEncode){
		char* output=(char*)((malloc(3*toEncode.length()+1)));

		size_t j=0;
		for(const auto& c:toEncode){
			if(isUrlEncoded(c))output[j++]=c;
			else{
				output[j++]='%';
				output[j++]=hex[(c >> 4)&0xF];
				output[j++]=hex[c&0xF];
			}
		}
		output[j++]=0;

		String result(output);
		free(output);
		return result;
	}

	String createUrl(String path){
		reportedName=Rpc::nameOrId;
		path+="?name="+urlEncode(reportedName);
		reportedTypes.resize(RegisteredTypes::registered.size());
		size_t i=0;
		for(const auto& pair:RegisteredTypes::registered){
			path+="&type="+urlEncode(pair.first);
			reportedTypes[i++]=pair.first;
		}

		return path;
	}

	void doConnect(){
		std::vector<String> toRegister(RegisteredTypes::registered.size());
		size_t i=0;
		for(const auto& pair:RegisteredTypes::registered)toRegister[i++]=pair.first;

		std::vector<String> toDelete(reportedTypes);

		for(int registerIndex=toRegister.size()-1;registerIndex>=0;registerIndex--){
			auto reg=toRegister[registerIndex];
			for(size_t deleteIndex=0;deleteIndex<toDelete.size();++deleteIndex){
				auto del=toDelete[deleteIndex];
				if(reg==del){
					toRegister.erase(toRegister.begin()+registerIndex);
					toDelete.erase(toDelete.begin()+deleteIndex);
					registerIndex++;
					deleteIndex--;
				}
			}
		}


		PendingCall call;
		if(!toRegister.empty()||!toDelete.empty())
			if(Rpc::nameOrId!=reportedName)
				call=callRemoteFunction(NULL_STRING,"H",Rpc::nameOrId,toRegister,toDelete);
			else
				call=callRemoteFunction(NULL_STRING,"H",toRegister,toDelete);
		else if(Rpc::nameOrId!=reportedName)
			call=callRemoteFunction(NULL_STRING,"H",Rpc::nameOrId);

		reportedTypes.clear();
		reportedName=NULL_STRING;

		if(!call){
			connected=true;
			Serial.println("Connected to Server");
		}else{
			call.then([](){
				connected=true;
				Serial.println("Connected to Server (using second handshake)");
			},[](const RpcError& e){
				Serial.println("Error connecting to Server");
				e.printStackTrace();
				_disconnect=true;
			});
		}
	}

	void doDisconnect(){
		static const RpcError error("Connection closed");

		for(const auto& item:activeRequests)item.second->reject(error);
		activeRequests.clear();

		for(const auto& item:currentlyExecuting)item.second->cancelSelf();
	}

	void callMeta(const FunctionCallContext& fcc,const RegisteredType& invoker,DataInput input);

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
					if(method!=NULL_STRING){
						fcc.reject(RpcError("Method is not defined on this type"));
						break;
					}else{
						callMeta(fcc,map,data);
						break;
					}
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
				it->second->cancelSelf();
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

	void callMeta(const FunctionCallContext& fcc,const RegisteredType& invoker,DataInput input){
		String arg0;
		if(input.tryGetArgs(arg0)){
			if(arg0=="M"){
				std::vector<String> methods(invoker.size());
				size_t i=0;
				for(const auto& pair:invoker) methods[i++]=pair.first;
				fcc.resolve(methods);
				return;
			}
		}

		fcc.reject("Invalid meta-call");
	}

}