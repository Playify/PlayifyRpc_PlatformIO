#include <initializer_list>
#include "vector"

template<typename T>
struct MultipleArguments:std::vector<T>{
	MultipleArguments()= default;

	MultipleArguments(std::initializer_list<T> args):std::vector<T>(args){}

	explicit MultipleArguments(std::vector<T> args):std::vector<T>(args){}
};


namespace DynamicData{
	template<typename T>
	void writeDynamic(DataOutput& data,T value);


	namespace WriteAll{
		void writeAll(DataOutput&){}

		template<typename T,typename... Args>
		void writeAll(DataOutput& data,T curr,Args... rest){
			writeDynamic(data,curr);
			writeAll(data,rest...);
		}

		template<typename T,typename... Args>
		void writeAll(DataOutput& data,MultipleArguments<T> multiple,Args... rest){
			for(const auto& curr: multiple)
				writeDynamic(data,curr);
			writeAll(data,rest...);
		}
	}
	namespace Count{
		size_t argCount(){return 0;}

		template<typename T,typename... Args>
		size_t argCount(T,Args... rest){
			return 1+argCount(rest...);
		}

		template<typename T,typename... Args>
		size_t argCount(MultipleArguments<T> multi,Args... rest){
			return multi.size()+argCount(rest...);
		}
	}

	template<typename... Args>
	void writeDynamicArray(DataOutput& data,Args... args){
		data.writeLength(Count::argCount(args...));
		WriteAll::writeAll(data,args...);
	}
}


namespace DynamicData{
	template<typename T>
	void writeDynamic(DataOutput& data,T value){
		static_assert(std::is_same<T, void>::value,"DynamicData.writeDynamic is not implemented for this type");
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
	void writeDynamic(DataOutput& data,int32_t value){
		data.writeLength('i');
		data.writeInt(value);
	}

	template<>
	void writeDynamic(DataOutput& data,double value){
		data.writeLength('d');
		data.writeDouble(value);
	}

	template<>
	void writeDynamic(DataOutput& data,uint64_t value){
		data.writeLength('l');
		data.writeLong(value);
	}

	template<>
	void writeDynamic(DataOutput& data,int64_t value){
		data.writeLength('l');
		data.writeLong(value);
	}

	template<>
	void writeDynamic(DataOutput& data,std::vector<uint8_t> value){
		data.writeLength('b');
		data.write(value.data(),value.size());
	}

	//No Date, Regex, Exception
	//TODO RpcObject,RpcFunction

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