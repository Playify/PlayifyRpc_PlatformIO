#include <map>

using CallReceiver=std::function<void(FunctionCallContext ctx,DataInput args)>;


namespace RpcInternal{
	struct IgnoreCase{
		bool operator()(const String& cs1,const String& cs2) const{
			String s1=cs1;
			String s2=cs2;
			s1.toLowerCase();
			s2.toLowerCase();
			return s1<s2;
		}
	};
}
using RegisteredType=std::map<String,CallReceiver,RpcInternal::IgnoreCase>;

namespace RpcInternal{
	namespace RegisteredTypes{
		std::map<String,std::pair<RegisteredType*,bool>> registered;
#if ESP32
		uint32_t nextFunctionId=esp_random();
#elif ESP8266
		uint32_t nextFunctionId=os_random();
#endif
		RegisteredType registeredFunctions;

		void registerType(const String& type,RegisteredType* map,bool deletePointer=false){
			if(registered.count(type)){
				auto& pair=registered[type];
				if(pair.first!=map&&pair.second)delete pair.first;
				pair=std::make_pair(map,deletePointer);
			}else registered[type]=std::make_pair(map,deletePointer);

			if(RpcConnection::connected)
				callRemoteFunction(NULL_STRING,"+",type);
		}

		RegisteredType* registerType(const String& type){
			auto map=new RegisteredType();
			registerType(type,map,true);
			return map;
		}

		void unregisterType(const String& type){
			auto it=registered.find(type);
			if(it==registered.end()) return;

			if(RpcConnection::connected)
				callRemoteFunction(NULL_STRING,"-",type);

			if(it->second.second)delete it->second.first;
			registered.erase(it);
		}


		void setup(){
			registerType("$"+Rpc::id,&registeredFunctions);
		}
	}
}