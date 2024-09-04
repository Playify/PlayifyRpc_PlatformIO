namespace RpcInternal{
	namespace DynamicData{
		//Base case
		template<typename T>
		bool read(DataInput&,int&,std::vector<DataInput>&,T&){
			static_assert(std::is_same<T,void>::value,"DynamicData.read is not implemented for this type");
			return false;
		}

		template<typename T>
		void writeDynamic(DataOutput&,T){
			static_assert(std::is_same<T,void>::value,"DynamicData.writeDynamic is not implemented for this type");
		}

		template<typename T>
		String getTypeName(bool,T*){
			static_assert(std::is_same<T,void>::value,"DynamicData.getTypeName is not implemented for this type");
			return "";
		}

		//Null
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,nullptr_t&){
			if(argCount==0)return false;
			if(data.readLength()=='n'){
				argCount--;
				return true;
			}
			return false;
		}

		template<>
		void writeDynamic(DataOutput& data,nullptr_t){
			data.writeLength('n');
		}

		template<>
		String getTypeName(bool,nullptr_t*){
			return "null";
		}

		//Bool
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,bool& value){
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

		template<>
		void writeDynamic(DataOutput& data,bool value){
			data.writeLength(value?'t':'f');
		}

		template<>
		String getTypeName(bool ts,bool*){
			return ts?"boolean":"bool";
		}

		//Primitives
#define NUMBER_TYPE(type,letter,func,typescript,csharp)                                \
    template<>                                                                         \
    bool read(DataInput& data,int& argCount,std::vector<DataInput>&,type& value){      \
        if(argCount==0)return false;                                                   \
        switch(data.readLength()){                                                     \
            case 'i':                                                                  \
                value=type(data.readInt());                                            \
                argCount--;                                                            \
                return true;                                                           \
            case 'd':                                                                  \
                value=type(data.readDouble());                                         \
                argCount--;                                                            \
                return true;                                                           \
            case 'l':                                                                  \
                value=type(data.readLong());                                           \
                argCount--;                                                            \
                return true;                                                           \
            default:                                                                   \
                return false;                                                          \
        }                                                                              \
    }                                                                                  \
    template<>                                                                         \
    void writeDynamic(DataOutput& data,type value){                                    \
        data.writeLength(letter);                                                      \
        data.func(value);                                                              \
    }                                                                                  \
	template<>                                                                         \
	String getTypeName(bool ts,type*){                                                 \
		return ts?typescript:csharp;                                                   \
	}
		NUMBER_TYPE(int8_t,'i',writeInt,"number","byte")
		NUMBER_TYPE(uint8_t,'i',writeInt,"number","sbyte")
		NUMBER_TYPE(int16_t,'i',writeInt,"number","short")
		NUMBER_TYPE(uint16_t,'i',writeInt,"number","ushort")
		NUMBER_TYPE(int32_t,'i',writeInt,"number","int")
		NUMBER_TYPE(uint32_t,'d',writeDouble,"number","uint")
		NUMBER_TYPE(int64_t,'l',writeLong,"bigint","long")
		NUMBER_TYPE(uint64_t,'l',writeLong,"bigint","ulong")
		NUMBER_TYPE(float,'d',writeDouble,"number","float")
		NUMBER_TYPE(double,'d',writeDouble,"number","double")
#undef NUMBER_TYPE

		//String
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,String& value){
			if(argCount==0)return false;
			int32_t type=data.readLength();
			if(type=='n'){
				value=NULL_STRING;
				argCount--;
				return true;
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
		//read for 'const char*&' doesn't exist, as who would free up the pointer
		template<>
		void writeDynamic(DataOutput& data,String value){
			data.writeLength(-(value.length()*4+1));
			data.insert(data.end(),value.begin(),value.end());
		}

		template<>
		void writeDynamic(DataOutput& data,const char* value){
			auto len=strlen(value);
			data.writeLength(-(len*4+1));
			data.write((uint8_t*)value,len);
		}
		template<>
		String getTypeName(bool ts,String*){
			return "string";
		}
		template<>
		String getTypeName(bool ts,const char**){
			return "string";
		}
		
		
		//Missing: Date, Regex, Object
		
		//Exception
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,RpcError& value){
			if(argCount==0)return false;
			switch(data.readLength()){
				case 'E':
					value=data.readError();
					argCount--;
					return true;
				default:
					return false;
			}
		}
		template<>
		void writeDynamic(DataOutput& data,RpcError value){
			data.writeLength('E');
			data.writeError(value);
		}
		template<>
		String getTypeName(bool ts,RpcError*){
			return ts?"RpcError":"RpcException";
		}
		
		
		//RpcObject
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,RpcObject& value){
			if(argCount==0)return false;
			switch(data.readLength()){
				case 'O':
					value=RpcObject(data.readString());
					argCount--;
					return true;
				default:
					return false;
			}
		}
		template<>
		void writeDynamic(DataOutput& data,RpcObject value){
			data.writeLength('O');
			data.writeString(value.type);
		}
		template<>
		String getTypeName(bool,RpcObject*){
			return "RpcObject";
		}
		
		//RpcFunction
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>&,RpcFunction& value){
			if(argCount==0)return false;
			switch(data.readLength()){
				case 'F':
					value=RpcFunction(data.readString(),data.readString());
					argCount--;
					return true;
				default:
					return false;
			}
		}
		template<>
		void writeDynamic(DataOutput& data,RpcFunction value){
			data.writeLength('F');
			data.writeString(value.type);
			data.writeString(value.method);
		}
		template<>
		String getTypeName(bool,RpcFunction*){
			return "RpcFunction";
		}

		//Buffer
		template<>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::vector<uint8_t>& value){
			if(argCount==0)return false;
			auto tmp=data;
			int32_t type=data.readLength();
			fromAlready:

			if(type=='b'){
				auto size=data.readLength();
				value.resize(size);
				data.readFully(value.data(),size);
				argCount--;
				return true;
			}

			if(type>=0)return false;

			switch((-type)%4){
				case 0:
					data=already[-type/4];
					type=data.readLength();
					goto fromAlready;
				case 3:
					already.push_back(tmp);
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					value.resize(type);
					for(int i=0;i<type;++i){
						int args=1;
						if(!read(data,args,already,value[i])||(args&-1)!=0)
							return false;
					}
					argCount--;
					return true;
				default:
					return false;
			}
		}

		template<>
		void writeDynamic(DataOutput& data,std::vector<uint8_t> value){
			data.writeLength('b');
			data.writeLength(value.size());
			data.write(value.data(),value.size());
		}

		template<>
		String getTypeName(bool ts,std::vector<uint8_t>*){
			return ts?"Uint8Array":"byte[]";
		}
		


		//Vector
		template<typename T>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::vector<T>& value){
			if(argCount==0)return false;
			auto tmp=data;
			int32_t type=data.readLength();

			if(type>=0)return false;

			switch((-type)%4){
				case 0:
					data=already[-type/4];
					type=data.readLength();
					goto fromAlready;
				case 3:
					already.push_back(tmp);
				fromAlready:
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					value.resize(type);
					for(int i=0;i<type;++i){
						int args=1;
						if(!read(data,args,already,value[i])||(args&-1)!=0)
							return false;
					}
					argCount--;
					return true;
				default:
					return false;
			}
		}

		template<typename T>
		void writeDynamic(DataOutput& data,std::vector<T> value){
			data.writeLength(-(value.size()*4+3));
			for(const auto& item: value)
				writeDynamic(data,item);
		}

		template<typename First>
		String getTypeName(bool ts,std::vector<First>*){
			return getTypeName(ts,(First*)nullptr)+"[]";
		}
		
		
		//Tuples
		template<typename... Types>
		bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::tuple<Types...>& value){
			if(argCount==0)return false;
			auto tmp=data;
			int32_t type=data.readLength();

			if(type>=0)return false;

			switch((-type)%4){
				case 0:
					data=already[-type/4];
					type=data.readLength();
					goto fromAlready;
				case 3:
					already.push_back(tmp);
				fromAlready:
					//type variable is not used anymore, therefore reused as 'count'
					type=-type/4;
					if(std::tuple_size<std::tuple<Types...>>::value!=type)return false;
					
					for(int i=0;i<type;++i){
						int args=1;
						if(!read(data,args,already,value[i])||(args&-1)!=0)
							return false;
					}
					argCount--;
					return true;
				default:
					return false;
			}
		}
		
		namespace Tuples{

// Base case: Stop the recursion when Index equals the tuple size
			template<std::size_t Index = 0, typename... Types>
			auto
			writeDynamicTuple(DataOutput& data, const std::tuple<Types...>& value) ->typename std::enable_if<Index == sizeof...(Types), void>::type{
				// Base case does nothing
			}
// Recursive case: Write each tuple element
			template<std::size_t Index = 0, typename... Types>
			auto
			writeDynamicTuple(DataOutput& data, const std::tuple<Types...>& value) ->typename std::enable_if<Index < sizeof...(Types), void>::type{
				writeDynamic(data, std::get<Index>(value));
				writeDynamicTuple<Index + 1>(data, value);
			}

		}

		template<typename... Types>
		void writeDynamic(DataOutput& data,std::tuple<Types...> value){
			data.writeLength(-(std::tuple_size<std::tuple<Types...>>::value*4+3));
			Tuples::writeDynamicTuple(data,value);
		}

		template<typename... Types>
		String getTypeName(bool ts,std::tuple<Types...>*){
			std::vector<String> subTypes;
			getTypeNames<void,Types...>(subTypes,ts);
			
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
		
		
		
		//Specialized
		template<>
		void writeDynamic(DataOutput& data,DataInput input){
			data.write(input);
		}

		template<typename T>
		String getTypeName(bool ts,MultipleArguments<T>*){
			auto sub=getTypeName<T>(ts,(T*)nullptr);
			return (ts?"...":"params ")+sub+"[]";
		}
	}
}