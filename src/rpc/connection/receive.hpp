
namespace RpcInternal{
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
			path+="?id="+Rpc::id;
			reportedName=Rpc::name;
			if(reportedName!=NULL_STRING)
				path+="&name="+urlEncode(reportedName);
			reportedTypes.resize(RegisteredTypes::registered.size());
			size_t i=0;
			String dollarId="$"+Rpc::id;
			for(const auto& pair:RegisteredTypes::registered){
				if(pair.first!=dollarId)
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
				if(Rpc::name!=reportedName)
					call=callRemoteFunction(NULL_STRING,"H",Rpc::name,toRegister,toDelete);
				else
					call=callRemoteFunction(NULL_STRING,"H",toRegister,toDelete);
			else if(Rpc::name!=reportedName)
				call=callRemoteFunction(NULL_STRING,"H",Rpc::name);

			reportedTypes.clear();
			reportedName=NULL_STRING;

			if(!call) connected=true;
			else{
				call.then([](){
					connected=true;
				},[](const RpcError& e){
					Serial.print("[Rpc] Error connecting to RPC: ");
					e.printStackTrace();
					_disconnect=true;
				});
			}
		}

		void doDisconnect(){
			static const RpcConnectionError error("Connection closed");

			for(const auto& item:activeRequests)item.second->reject(error);
			activeRequests.clear();

			for(const auto& item:currentlyExecuting)item.second->cancelSelf();
		}

		void callMeta(const FunctionCallContext& fcc,const RegisteredType& invoker,DataInput input,const String& string);

		void receiveRpc(DataInput data){
			auto packetType=PacketType(data.readByte());

			switch(packetType){
				case FunctionCall:{
					int32_t callId=data.readLength();

					String type=data.readString();
					String method=data.readString();

					auto shared=std::make_shared<FunctionCallContext::Shared>(callId,type,method);
					auto fcc=FunctionCallContext(shared);

					const auto& typeIterator=RegisteredTypes::registered.find(type);
					if(typeIterator==RegisteredTypes::registered.end()){
						fcc.reject(RpcTypeNotFoundError(type));
						break;
					}

					const auto& map=*typeIterator->second.first;
					if(method==NULL_STRING){
						callMeta(fcc,map,data,type);
						break;
					}

					const auto& methodIterator=map.find(method);
					if(methodIterator==map.end()){
						fcc.reject(RpcMethodNotFoundError(type,method));
						return;
					}
					auto& callers=methodIterator->second.callers;
					for(auto& caller:callers)
						if(caller(fcc,data))
							return;
					fcc.reject("Error casting arguments");

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

		void callMeta(const FunctionCallContext& fcc,const RegisteredType& invoker,DataInput input,const String& type){
			String meta;
			String method;
			bool ts;
			if(input.tryGetArgs(meta)){
				if(meta=="M"){
					std::vector<String> methods(invoker.size());
					size_t i=0;
					for(const auto& pair:invoker) methods[i++]=pair.first;
					fcc.resolve(methods);
					return;
				}
			}else if(input.tryGetArgs(meta,method,ts)){
				if(meta=="S"){
					getMethodSignatures(fcc,invoker,type,method,ts);
					return;
				}
			}else if(input.tryGetArgs(meta,method)){
				if(meta=="S"){
					getMethodSignatures(fcc,invoker,type,method,false);
					return;
				}
			}else meta=NULL_STRING;

			fcc.reject(RpcMetaMethodNotFoundError(type,meta));
		}
	}
}