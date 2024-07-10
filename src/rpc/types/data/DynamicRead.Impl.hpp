
namespace DynamicData{
	template<typename T>
	bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,T& value){
		static_assert(std::is_same<T,void>::value,"DynamicData.read is not implemented for this type");
		return false;
	}

	template<typename... T>
	bool read(DataInput& data,int& argCount,std::vector<DataInput>& already,std::vector<T...>& value){
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

	#define READ_NUMBER(type)                                                          \
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
    }
	READ_NUMBER(int8_t)
	READ_NUMBER(uint8_t)
	READ_NUMBER(int16_t)
	READ_NUMBER(uint16_t)
	READ_NUMBER(int32_t)
	READ_NUMBER(uint32_t)
	READ_NUMBER(uint64_t)
	READ_NUMBER(int64_t)
	READ_NUMBER(float)
	READ_NUMBER(double)
	#undef READ_NUMBER

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
}