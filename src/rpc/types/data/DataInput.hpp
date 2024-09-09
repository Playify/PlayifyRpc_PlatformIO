#pragma once
#include <Arduino.h>
#define NULL_STRING "\xc3\x28"

class DataInput{
public:
	uint8_t* _data;
	uint32_t _available;
	uint32_t _initialLength;

public:
	DataInput():
			_data(nullptr),
			_available(0)
	{}
	DataInput(uint8_t* data,uint32_t length,uint32_t initialLength):
			_data(data),
			_available(length),
			_initialLength(initialLength)
	{}

	//DataInput& operator=(const DataInput&) = delete; // non copyable

private:
	template<typename T>
	T readReverse(){
		if(available()<sizeof(T))return T();

		T t;
		size_t i=sizeof(T);
		while(i-->0) *(((uint8_t*)&t)+i)=readByte();
		return t;
	}

public:
	uint32_t available() const{
		return _available;
	}
	uint32_t index() const{
		return _initialLength-_available;
	}
	DataInput goBack(uint32_t offset) const{
		return DataInput(
				_data-offset,
				_available+offset,
			_initialLength
			);
	}
	/*

	uint16_t read(uint8_t* b, uint16_t len);

	uint16_t read(uint8_t* b, uint16_t off, uint16_t len);*/

	void readFully(uint8_t* b, uint16_t len){
		while(len--)*(b++)=readByte();
	}

	void readFully(uint8_t* b, uint16_t off, uint16_t len){
		readFully(b+off,len);
	}

	uint8_t readByte(){
		_available--;
		return *(_data++);
	}

	bool readBoolean(){return readByte()!=0;}

	int16_t readShort(){return readReverse<int16_t>();}

	uint16_t readUShort(){return readReverse<uint16_t>();}

	int32_t readInt(){return readReverse<int32_t>();}

	uint32_t readUInt(){return readReverse<uint32_t>();}

	int64_t readLong(){return readReverse<int64_t>();}

	uint64_t readULong(){return readReverse<uint64_t>();}

	float readFloat(){return readReverse<float>();}

	double readDouble(){return readReverse<double>();}

	int32_t readLength(){
		int32_t result=0;
		uint8_t push=0;
		while(true){
			uint8_t read=readByte();
			if(read==0) return push==0?0:~result;
			if((read&0x80)==0){
				result|=read<<push;
				return result;
			}
			result|=(read&0x7f)<<push;
			push+=7;
		}
	}

	String readString(){
		int32_t len=readLength();
		if(len<0)return NULL_STRING;

		uint8_t arr[len+1];
		readFully(arr,len);
		arr[len]=0;

		return {(char*)arr};
	}

	RpcError readError(){
		String type=readString();
		String from=readString();
		if(from==NULL_STRING)from="???";
		String message=readString();
		String stackTrace=readString();
		if(stackTrace==NULL_STRING)stackTrace="";
		String jsonData=_available?readString():R"json({"$info":"JsonData was not included, due to an old PlayifyRpc version"})json";
		return RpcError(type,from,message,stackTrace,jsonData);
	}


	template<typename Func>
	bool tryCall(Func func);
	template<typename Func>
	bool tryCallSingle(Func func);
	template<typename T>
	bool tryGetResult(T& value);
	template<typename... Args>
	bool tryGetArgs(Args&... args);
};