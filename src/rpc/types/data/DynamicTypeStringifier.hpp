namespace RpcInternal{
	namespace DynamicData{
		template<typename T>
		String getTypeName(bool ts,T* dummy=nullptr);
		template<typename... T>
		String getTypeName(bool ts,std::vector<T...>* dummy=nullptr);
		template<typename... T>
		String getTypeName(bool ts,MultipleArguments<T...>* dummy=nullptr);


		template<typename Ignored>
		void getTypeNames(std::vector<String>&,bool){}
		template<typename Ignored,typename Curr,typename...Next>
		void getTypeNames(std::vector<String>& vec,bool ts){
			vec.push_back(getTypeName(ts,(Curr*)nullptr));
			getTypeNames<Ignored,Next...>(vec,ts);
		}

/*

		template<typename Return,typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(std::function<Return(Args...)>,bool ts){//TODO they have a context, and return void => no return info available
			auto returns=getTypeName<Return>(ts);
			auto parameters=std::vector<String>();
			getTypeNames<void,Args...>(parameters,ts);

			return std::make_tuple(parameters,returns);
		}
*/

		template<typename... Args>
		std::tuple<std::vector<String>,String> getMethodSignature(std::function<void(FunctionCallContext,Args...)>,bool ts){
			auto parameters=std::vector<String>();
			getTypeNames<void,Args...>(parameters,ts);
			int i=0;
			if(ts)
				for(auto& item: parameters){
					if(ts&&item.startsWith("..."))
						item="...arg"+String(i++)+":"+item.substring(3);
					else item="arg"+String(i++)+":"+item;
				}
			else
				for(auto& item: parameters)
					item+=" arg"+String(i++);

			return std::make_tuple(parameters,ts?"unknown":"dynamic");
		}
	}
}