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
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>&,const String& returns,std::array<String,sizeof...(Args)> names,ProgrammingLanguage lang){
			auto parameters=std::vector<String>();
			if(lang==ProgrammingLanguage::JavaScript) parameters.insert(parameters.end(),names.begin(),names.end());
			else getTypeNames<void,Args...>(parameters,lang!=ProgrammingLanguage::CSharp,names.data(),names.size(),0);
			return std::make_tuple(parameters,returns);
		}
		template<typename... Args,typename Return>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>& func,ReturnType<Return>,std::array<String,sizeof...(Args)> names,ProgrammingLanguage lang){
			return getMethodSignature(func,TypeDefinition<Return>::getTypeName(lang!=ProgrammingLanguage::CSharp),names,lang);
		}
		template<typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<void(FunctionCallContext,Args...)>& func,ProgrammingLanguage lang){
			return getMethodSignature(func,lang!=ProgrammingLanguage::CSharp?"unknown":"dynamic?",getNameArray<sizeof...(Args)>(),lang);
		}

		template<typename... Args,typename Return>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<Return(Args...)>&,std::array<String,sizeof...(Args)> names,ProgrammingLanguage lang){
			auto parameters=std::vector<String>();
			if(lang==ProgrammingLanguage::JavaScript) parameters.insert(parameters.end(),names.begin(),names.end());
			else getTypeNames<void,Args...>(parameters,lang!=ProgrammingLanguage::CSharp,names.data(),names.size(),0);
			return std::make_tuple(parameters,TypeDefinition<Return>::getTypeName(lang!=ProgrammingLanguage::CSharp));
		}
		template<typename... Args,typename Return>
		std::tuple<std::vector<String>,String> getMethodSignature(const std::function<Return(Args...)>& func,ProgrammingLanguage lang){
			return getMethodSignature(func,getNameArray<sizeof...(Args)>(),lang);
		}
	}
}