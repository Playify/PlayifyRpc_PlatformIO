namespace RpcInternal{
	namespace DynamicData{
		
		template<typename T=void>
		struct TypeDefinition{
			static bool read(DataInput&,int&,T&){
				static_assert(std::is_same<T,void>::value,"Not implemented");
				return false;
			}
			static void writeDynamic(DataOutput&,T){
				static_assert(std::is_same<T,void>::value,"Not implemented");
			}
			static String getTypeName(bool){
				static_assert(!std::is_same<T,void>::value,"Not implemented");
				return "unknown";
			}
		};
		template<>
		struct TypeDefinition<void>{
			static void writeDynamic(DataOutput& data){
				data.writeLength('n');
			}
			static String getTypeName(bool){
				return "void";
			}
		};
		template<>
		struct TypeDefinition<nullptr_t>{
			static bool read(DataInput& data,int& argCount,nullptr_t&){
				if(argCount==0)return false;
				if(data.readLength()=='n'){
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,nullptr_t){
				data.writeLength('n');
			}
			static String getTypeName(bool){
				return "null";
			}
		};

		template<>
		struct TypeDefinition<bool>{
			static bool read(DataInput& data,int& argCount,bool& value){
				if(argCount==0)return false;
				switch(data.readLength()){
					case 't':
						value=true;
						argCount--;
						return true;
					case 'f':
						value=false;
						argCount--;
						return true;
					default:
						return false;
				}
			}
			static void writeDynamic(DataOutput& data,bool value){
				data.writeLength(value?'t':'f');
			}
			static String getTypeName(bool ts){
				return ts?"boolean":"bool";
			}
		};

		//region Number Types
#define NUMBER_TYPE(type,letter,func,typescript,csharp)															\
		template<>																								\
		struct TypeDefinition<type>{																			\
			static bool read(DataInput& data,int& argCount,type& value){		\
				if(argCount==0)return false;                                             						\
				switch(data.readLength()){                                               						\
					case 'i':                                                            						\
						value=type(data.readInt());                                      						\
						argCount--;                                                      						\
						return true;                                                     						\
					case 'd':                                                            						\
						value=type(data.readDouble());                                   						\
						argCount--;                                                      						\
						return true;                                                     						\
					case 'l':                                                            						\
						value=type(data.readLong());                                     						\
						argCount--;                                                      						\
						return true;                                                     						\
					default:                                                             						\
						return false;                                                    						\
				}																								\
			}																									\
			static void writeDynamic(DataOutput& data,type value){                       						\
				data.writeLength(letter);                                                						\
				data.func(value);																				\
			}																									\
			static String getTypeName(bool ts){                                          						\
				return ts?typescript:csharp;																	\
			}																									\
		};

		NUMBER_TYPE(int8_t,'i',writeInt,"number","sbyte")
		NUMBER_TYPE(uint8_t,'i',writeInt,"number","byte")
		NUMBER_TYPE(int16_t,'i',writeInt,"number","short")
		NUMBER_TYPE(uint16_t,'i',writeInt,"number","ushort")
		NUMBER_TYPE(int32_t,'i',writeInt,"number","int")
		NUMBER_TYPE(uint32_t,'d',writeDouble,"number","uint")
		NUMBER_TYPE(int64_t,'l',writeLong,"bigint","long")
		NUMBER_TYPE(uint64_t,'l',writeLong,"bigint","ulong")
		NUMBER_TYPE(float,'d',writeDouble,"number","float")
		NUMBER_TYPE(double,'d',writeDouble,"number","double")
#undef NUMBER_TYPE
//endregion

		template<>
		struct TypeDefinition<String>{
			static bool read(DataInput& data,int& argCount,String& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();
				if(type=='n'){
					value=NULL_STRING;
					argCount--;
					return true;
				}
				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}
				if(type<0&&((-type)%4)==1){
					int32_t count=-type/4;
					uint8_t chars[count+1];
					data.readFully(chars,count);
					chars[count]=0;
					value=String((char*)chars);
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,String value){
				data.writeLength(-(value.length()*4+1));
				data.insert(data.end(),value.begin(),value.end());
			}
			static String getTypeName(bool){
				return "string";
			}
		};
		template<>
		struct TypeDefinition<const char*>{
			template<typename T=void>
			static bool read(DataInput&,int&,const char*&){
				static_assert(std::is_same<T,void>::value,"reading 'const char*' is not supported");
				return false;
			}
			static void writeDynamic(DataOutput& data,const char* value){
				auto len=strlen(value);
				data.writeLength(-(len*4+1));
				data.write((uint8_t*)value,len);
			}
			static String getTypeName(bool){
				return "string";
			}
		};

		//Missing: Date, Regex, Object

		template<>
		struct TypeDefinition<RpcError>{
			static bool read(DataInput& data,int& argCount,RpcError& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();
				
				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type=='E'){
					value=data.readError();
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,RpcError value){
				data.writeLength('E');
				data.writeError(value);
			}
			static String getTypeName(bool ts){
				return ts?"RpcError":"RpcException";
			}
		};

		template<>
		struct TypeDefinition<RpcObject>{
			static bool read(DataInput& data,int& argCount,RpcObject& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();

				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type=='O'){
					value=RpcObject(data.readString());
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,RpcObject value){
				data.writeLength('O');
				data.writeString(value.type);
			}
			static String getTypeName(bool){
				return "RpcObject";
			}
		};

		template<>
		struct TypeDefinition<RpcFunction>{
			static bool read(DataInput& data,int& argCount,RpcFunction& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();

				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type=='F'){
					value=RpcFunction(data.readString(),data.readString());
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,RpcFunction value){
				data.writeLength('F');
				data.writeString(value.type);
				data.writeString(value.method);
			}
			static String getTypeName(bool){
				return "RpcFunction";
			}
		};

		template<>
		struct TypeDefinition<std::vector<uint8_t>>{
			static bool read(DataInput& data,int& argCount,std::vector<uint8_t>& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();
				
				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type=='b'){
					auto size=data.readLength();
					value.resize(size);
					data.readFully(value.data(),size);
					argCount--;
					return true;
				}

				if(type<0&&(-type)%4==3){
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					value.resize(type);
					for(int i=0;i<type;++i){
						int args=1;
						if(!TypeDefinition<uint8_t>::read(data,args,value[i])||(args&-1)!=0)
							return false;
					}
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,std::vector<uint8_t> value){
				data.writeLength('b');
				data.writeLength(value.size());
				data.write(value.data(),value.size());
			}
			static String getTypeName(bool ts){
				return ts?"Uint8Array":"byte[]";
			}
		};

		template<typename T,typename... Rest>
		struct TypeDefinition<std::vector<T,Rest...>>{
			static bool read(DataInput& data,int& argCount,std::vector<T,Rest...>& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();

				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type<0&&(-type)%4==3){
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					value.resize(type);
					for(int i=0;i<type;++i){
						int args=1;
						if(!TypeDefinition<T>::read(data,args,value[i])||(args&-1)!=0)
							return false;
					}
					argCount--;
					return true;
				}
				return false;
			}
			static void writeDynamic(DataOutput& data,std::vector<T,Rest...> value){
				data.writeLength(-(value.size()*4+3));
				for(const auto& item: value)
					TypeDefinition<T>::writeDynamic(data,item);
			}
			static String getTypeName(bool ts){
				return TypeDefinition<T>::getTypeName(ts)+"[]";
			}
		};

		template<typename... Types>
		struct TypeDefinition<std::tuple<Types...>>{
			static bool read(DataInput& data,int& argCount,std::tuple<Types...>& value){
				if(argCount==0)return false;
				int32_t type=data.readLength();

				if(type<0&&(-type)%4==0){
					DataInput data2=data.goBack(-type/4);
					return read(data2,argCount,value);
				}

				if(type<0&&(-type)%4==3){
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					if(std::tuple_size<std::tuple<Types...>>::value!=type)return false;
					argCount--;
					return readTupleElements<0>(data, value);
				}
				return false;
			}
			template<std::size_t Index = 0>
			static auto readTupleElements(DataInput&, std::tuple<Types...>&)-> typename std::enable_if<Index == sizeof...(Types), bool>::type {
				return true;
			}

			template<std::size_t Index = 0>
			static auto readTupleElements(DataInput& data, std::tuple<Types...>& value)-> typename std::enable_if<Index < sizeof...(Types), bool>::type {
				int args = 1;
				if (!TypeDefinition<decltype(std::get<Index>(value))>::read(data, args, std::get<Index>(value)) || args!= 0) {
					return false;
				}
				return readTupleElements<Index + 1>(data, value);
			}

// Base case: Stop the recursion when Index equals the tuple size
			template<std::size_t Index = 0>
			auto static writeDynamicTuple(DataOutput&, const std::tuple<Types...>&) ->typename std::enable_if<Index == sizeof...(Types), void>::type{}
// Recursive case: Write each tuple element
			template<std::size_t Index = 0>
			auto static writeDynamicTuple(DataOutput& data, const std::tuple<Types...>& value) ->typename std::enable_if<Index < sizeof...(Types), void>::type{
				TypeDefinition<typename Helpers::ConstRef::remove_const_ref<decltype(std::get<Index>(value))>::type>::writeDynamic(data, std::get<Index>(value));
				writeDynamicTuple<Index + 1>(data, value);
			}
			template<typename Ignored>
			static void getTupleTypeNames(std::vector<String>&,bool){}
			template<typename Ignored,typename Curr,typename... NextTypes>
			static void getTupleTypeNames(std::vector<String>& vec,bool ts){
				vec.push_back(TypeDefinition<Curr>::getTypeName(ts));
				getTupleTypeNames<Ignored,NextTypes...>(vec,ts);
			}

			static void writeDynamic(DataOutput& data,std::tuple<Types...> value){
				data.writeLength(-(std::tuple_size<std::tuple<Types...>>::value*4+3));
				writeDynamicTuple(data,value);
			}
			static String getTypeName(bool ts){
				std::vector<String> subTypes;
				getTupleTypeNames<void,Types...>(subTypes,ts);

				String sum=ts?"[":"(";
				bool first=true;
				for(const auto& item: subTypes){
					if(first)first=false;
					else sum+=',';
					sum+=item;
				}
				sum+=ts?']':')';

				return sum;
			}
		};

		template<>
		struct TypeDefinition<DataInput>{
			//static bool read(DataInput& data,int& argCount,DataInput& value);
			static void writeDynamic(DataOutput& data,DataInput value){
				data.write(value);
			}
			static String getTypeName(bool ts){
				return ts?"...any[]":"params object[]";
			}
		};

		template<typename T>
		struct TypeDefinition<MultipleArguments<T>>{
			//static bool read(DataInput& data,int& argCount,MultipleArguments<T>& value);
			//static void writeDynamic(DataOutput& data,MultipleArguments<T> value);
			static String getTypeName(bool ts){
				auto sub=TypeDefinition<T>::getTypeName(ts);
				return (ts?"...":"params ")+sub+"[]";
			}
		};
	}
}