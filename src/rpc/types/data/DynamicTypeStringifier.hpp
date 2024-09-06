namespace RpcInternal{
	namespace DynamicData{
		template<typename Ignored>
		void getTypeNames(std::vector<String>&,bool,String*,int,int){}
		template<typename Ignored,typename Curr,typename... NextTypes>
		void getTypeNames(std::vector<String>& vec,bool ts,String* data,int size,int index){
			const String& typeName=TypeDefinition<Curr>::getTypeName(ts);

			String name=index<size?data[index]:"arg"+String(index);
			if(!ts)vec.push_back(typeName+" "+name);
			else if(typeName.startsWith("..."))vec.push_back(name+":"+typeName.substring(3));
			else vec.push_back(name+":"+typeName);
			
			getTypeNames<Ignored,NextTypes...>(vec,ts,data,size,index+1);
		}
		
		template<int T>
		auto getNameArray()->std::array<String,T>{
			std::array<String,T> arr;
			String arg="arg";
			for(int i=0;i<T;++i) arr[i]=arg+i;
			return arr;
		}
		
		template<typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>&,bool ts,std::array<String,sizeof...(Args)> names,String returns){
			auto parameters=std::vector<String>();
			getTypeNames<void,Args...>(parameters,ts,names.data(),names.size(),0);
			return std::make_tuple(parameters,returns);
		}
		template<typename... Args,typename Return>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>& func,bool ts,std::array<String,sizeof...(Args)> names,Return*){
			return getMethodSignature(func,ts,names,TypeDefinition<Return>::getTypeName(ts));
		}
		template<typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>& func,bool ts,String returns){
			return getMethodSignature(func,ts,getNameArray<sizeof...(Args)>(),returns);
		}
		template<typename... Args,typename Return>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>& func,bool ts,Return*){
			return getMethodSignature(func,ts,getNameArray<sizeof...(Args)>(),TypeDefinition<Return>::getTypeName(ts));
		}

		template<typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(std::function<void(FunctionCallContext,Args...)> func,bool ts){
			return getMethodSignature(func,ts,getNameArray<sizeof...(Args)>(),String(ts?"unknown":"object?"));
		}
	}
}