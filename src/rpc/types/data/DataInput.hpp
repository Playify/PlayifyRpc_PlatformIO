#pragma once
#include <Arduino.h>
#define NULL_STRING "\xc3\x28"


class DataInput{
public:
	uint8_t* _data;
	uint32_t _available;
	
public:
	static DataInput empty(){
		return DataInput(nullptr,0);
	}
	DataInput(uint8_t* data,uint32_t length):
		_data(data),
		_available(length)
	{}
	
	DataInput& operator=(const DataInput&) = delete; // non copyable

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
		return RpcError(readString(),readString(),readString(),readString());
	}
};