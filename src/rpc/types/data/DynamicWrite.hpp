#include <initializer_list>
#include "vector"

template<typename T>
struct MultipleArguments: std::vector<T>{
	MultipleArguments() = default;

	MultipleArguments(std::initializer_list<T> args):std::vector<T>(args){}

	explicit MultipleArguments(std::vector<T> args):std::vector<T>(args){}
};


namespace RpcInternal{
	namespace DynamicData{
		template<typename T>
		void writeDynamic(DataOutput& data,T value);

		template<typename T>
		void writeDynamic(DataOutput& data,std::vector<T> value);
		template<typename... Types>
		void writeDynamic(DataOutput& data,std::tuple<Types...> value);


		namespace WriteAll{
			void writeAll(DataOutput&){}

			template<typename T,typename... Args>
			void writeAll(DataOutput& data,T curr,Args... rest){
				writeDynamic(data,curr);
				writeAll(data,rest...);
			}

			template<typename T,typename... Args>
			void writeAll(DataOutput& data,MultipleArguments <T> multiple,Args... rest){
				for(const auto& curr:multiple)
					writeDynamic(data,curr);
				writeAll(data,rest...);
			}
		}
		namespace Count{
			size_t argCount(){ return 0; }
			template<typename T,typename... Args>
			size_t argCount(T,Args... rest){return 1+argCount(rest...);}
			template<typename T,typename... Args>
			size_t argCount(MultipleArguments <T> multi,Args... rest){return multi.size()+argCount(rest...);}
		}

		template<typename... Args>
		void writeDynamicArray(DataOutput& data,Args... args){
			data.writeLength(Count::argCount(args...));
			WriteAll::writeAll(data,args...);
		}

		template<>
		void writeDynamicArray(DataOutput& data,DataInput input){
			data.write(input);
		}
	}
}