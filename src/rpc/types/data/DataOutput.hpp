#pragma once
#include "Arduino.h"
#include "vector"

class DataOutput : public std::vector<uint8_t>{
public:
	DataOutput(){
		reserve(32);
	}
	//explicit DataOutput(const DataOutput&)= default; // non construction-copyable
	//DataOutput& operator=(const DataOutput&) = delete; // non copyable

private:
	void writeReverse(uint8_t* buf, uint16_t len){insert(end(),std::reverse_iterator<uint8_t*>(buf+len),std::reverse_iterator<uint8_t*>(buf));}

public:

	using vector::size;
	using vector::data;

	using vector::begin;
	using vector::end;

	void write(uint8_t* buf, uint16_t off, uint16_t len){insert(end(),buf+off,buf+off+len);}

	void write(uint8_t* buf, uint16_t len){insert(end(),buf,buf+len);}

	void write(const DataOutput& data){insert(end(),data.begin(),data.end());}

	void writeBoolean(const bool v){writeByte(v);}
	void writeByte(const uint8_t v){push_back(v);}
	void writeShort(uint16_t v){writeReverse((uint8_t*)&v,2);}//get address of v, and use as byte array
	void writeInt(uint32_t v){writeReverse((uint8_t*)&v,4);}//get address of v, and use as byte array
	void writeLong(uint64_t v){writeReverse((uint8_t*)&v,8);}//get address of v, and use as byte array
	void writeFloat(float v){writeReverse((uint8_t*)&v,4);}//get address of v, and use as byte array
	void writeDouble(double v){writeReverse((uint8_t*)&v,8);}//get address of v, and use as byte array
	

	void writeLength(int32_t i){
		uint32_t u=i<0?~i:i;
		while(u>=0x80){
			writeByte(u|0x80);
			u>>=7;
		}
		if(i<0){
			writeByte(u|0x80);
			writeByte(0);
		}else{
			writeByte(u);
		}
	}

	void writeString(const String& s){
		if(s==NULL_STRING)writeLength(-1);
		else{
			const uint32_t len=s.length();
			writeLength((int32_t)len);
			insert(end(),s.begin(),s.end());
		}
	}
	
	void writeError(const RpcError& error){
		writeString(error.type);
		writeString(error.from);
		writeString(error.msg);
		writeString(error.stackTrace);
	}
};