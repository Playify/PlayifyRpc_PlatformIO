#include <initializer_list>
#include "vector"

namespace RpcInternal{
	namespace DynamicData{

		namespace WriteAll{
			void writeAll(DataOutput&){}

			template<typename T,typename... Args>
			void writeAll(DataOutput& data,T curr,Args... rest){
				TypeDefinition<T>::writeDynamic(data,curr);
				writeAll(data,rest...);
			}

			template<typename T,typename... Args>
			void writeAll(DataOutput& data,MultipleArguments <T> multiple,Args... rest){
				for(const auto& curr:multiple)
					TypeDefinition<T>::writeDynamic(data,curr);
				writeAll(data,rest...);
			}
			
			size_t argCount(){ return 0; }
			template<typename T,typename... Args>
			size_t argCount(T,Args... rest){return 1+argCount(rest...);}
			template<typename T,typename... Args>
			size_t argCount(MultipleArguments <T> multi,Args... rest){return multi.size()+argCount(rest...);}
		}
		template<typename... Args>
		void writeDynamicArray(DataOutput& data,Args... args){
			data.writeLength(WriteAll::argCount(args...));
			WriteAll::writeAll(data,args...);
		}

		template<>
		void writeDynamicArray(DataOutput& data,DataInput input){
			data.write(input);
		}
	}
}