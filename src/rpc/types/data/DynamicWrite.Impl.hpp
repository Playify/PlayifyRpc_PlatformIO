
namespace DynamicData{
	template<typename T>
	void writeDynamic(DataOutput& data,T value){
		static_assert(std::is_same<T, void>::value,"DynamicData.writeDynamic is not implemented for this type");
	}
	template<>
	void writeDynamic(DataOutput& data,DataInput input){
		data.write(input);
	}
	template<>
	void writeDynamic(DataOutput& data,nullptr_t){
		data.writeLength('n');
	}

	template<>
	void writeDynamic(DataOutput& data,bool value){
		data.writeLength(value?'t':'f');
	}
	
	template<>
	void writeDynamic(DataOutput& data,int8_t value){
		data.writeLength('i');
		data.writeInt(value);
	}
	template<>
	void writeDynamic(DataOutput& data,uint8_t value){
		data.writeLength('i');
		data.writeInt(value);
	}
	template<>
	void writeDynamic(DataOutput& data,int16_t value){
		data.writeLength('i');
		data.writeInt(value);
	}
	template<>
	void writeDynamic(DataOutput& data,uint16_t value){
		data.writeLength('i');
		data.writeInt(value);
	}
	template<>
	void writeDynamic(DataOutput& data,int32_t value){
		data.writeLength('i');
		data.writeInt(value);
	}

	template<>
	void writeDynamic(DataOutput& data,uint32_t value){
		data.writeLength('l');
		data.writeLong(value);
	}

	template<>
	void writeDynamic(DataOutput& data,int64_t value){
		data.writeLength('l');
		data.writeLong(value);
	}
	template<>
	void writeDynamic(DataOutput& data,uint64_t value){
		data.writeLength('l');
		data.writeLong(value);
	}

	template<>
	void writeDynamic(DataOutput& data,float value){
		data.writeLength('d');
		data.writeDouble(value);
	}
	template<>
	void writeDynamic(DataOutput& data,double value){
		data.writeLength('d');
		data.writeDouble(value);
	}

	template<>
	void writeDynamic(DataOutput& data,std::vector<uint8_t> value){
		data.writeLength('b');
		data.write(value.data(),value.size());
	}

	//No Date, Regex, Exception
	template<>
	void writeDynamic(DataOutput& data,RpcObject value){
		data.writeLength('O');
		data.writeString(value.type);
	}
	template<>
	void writeDynamic(DataOutput& data,RpcFunction value){
		data.writeLength('F');
		data.writeString(value.type);
		data.writeString(value.method);
	}

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

	//No Object
	template<typename T>
	void writeDynamic(DataOutput& data,std::vector<T> value){
		data.writeLength(-(value.size()*4+3));
		for(const auto& item: value)
			writeDynamic(data,item);
	}

	template<typename... Types>
	void writeDynamic(DataOutput& data,std::tuple<Types...> value){
		data.writeLength(-(std::tuple_size<std::tuple<Types...>>::value*4+3));
		for(const auto& item: value)
			writeDynamic(data,item);
	}
}