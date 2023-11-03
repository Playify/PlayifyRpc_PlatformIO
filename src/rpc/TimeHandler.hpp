#pragma once

struct Duration{
	uint32_t microseconds;

	constexpr Duration():microseconds(){}
private:
	constexpr explicit Duration(uint32_t micros):microseconds(micros){}
public:

	static constexpr Duration seconds(uint32_t t){return Duration(t*1000000);}
	static constexpr Duration millis(uint32_t t){return Duration(t*1000);}
	static constexpr Duration micros(uint32_t t){return Duration(t);}
	static constexpr Duration zero(){return Duration(0);}
	static constexpr Duration maximum(){return Duration(UINT32_MAX);}

	//Conversion
	constexpr uint32_t operator+() const { return microseconds; }
	explicit constexpr operator bool() const { return microseconds;}
	//Comparison
	constexpr bool operator<(const Duration& rhs) const { return microseconds < rhs.microseconds; }
	constexpr bool operator>(const Duration& rhs) const { return rhs < *this; }
	constexpr bool operator<=(const Duration& rhs) const { return !(*this > rhs); }
	constexpr bool operator>=(const Duration& rhs) const { return !(*this < rhs); }
	constexpr bool operator==(const Duration& rhs) const { return microseconds == rhs.microseconds; }
	constexpr bool operator!=(const Duration& rhs) const { return !(*this == rhs); }
	//Math
	constexpr Duration operator+(const Duration& rhs) const { return Duration(microseconds + rhs.microseconds); }
	Duration& operator+=(const Duration& rhs) { return *this = *this + rhs; }
	constexpr Duration operator-(const Duration& rhs) const { return Duration(microseconds - rhs.microseconds); }
	Duration& operator-=(const Duration& rhs) { return *this = *this - rhs; }
	constexpr Duration operator*(uint32_t value) const { return Duration(microseconds * value); }
	Duration& operator*=(const uint32_t value) { return *this = *this * value; }
	constexpr Duration operator/(uint32_t value) const { return Duration(microseconds / value); }
	Duration& operator/=(const uint32_t value) { return *this = *this / value; }
};

constexpr Duration operator "" _s(unsigned long long t){return Duration::seconds(t);}
constexpr Duration operator "" _ms(unsigned long long t){return Duration::millis(t);}
constexpr Duration operator "" _us(unsigned long long t){return Duration::micros(t);}

#undef min
template<typename T>
constexpr T min(T a,T b){
	return a<b?a:b;
}
#undef max
template<typename T>
constexpr T max(T a,T b){
	return a<b?b:a;
}
#undef abs
template<typename T>
constexpr T abs(T a){
	return a<0?-a:a;
}

void hexdump(const void* data,size_t length){
	const auto* bytes=reinterpret_cast<const uint8_t*>(data);
	char hex[3];
	hex[2]='\0';

	for(size_t i=0;i<length;++i){
		uint8_t byte=bytes[i];

		// Print the hexadecimal representation
		sprintf(hex,"%02X",byte);
		Serial.print(hex);
		Serial.print(' ');

		// Print the ASCII representation
		if(byte>=0x20&&byte<=0x7E) Serial.print(static_cast<char>(byte));
		else Serial.print('.');


		// Add a newline after every 16 bytes
		if((i+1)%16==0) Serial.println();
		else Serial.print(" | ");
	}

	// Add a newline if necessary
	if(length%16!=0) Serial.println();
}

namespace TimeHandler{
	uint32_t lastTime;
	Duration deltaTime;


	//counts down to zero, then return true every time or once
	bool waitZero(Duration& timer,const bool once){
		if(once&&!timer)
			return false;

		if(timer>deltaTime){
			timer-=deltaTime;
			return false;
		}else{
			timer=Duration::zero();
			return true;
		}
	}

	bool cyclicEvent(Duration& timer,Duration duration){
		if(timer>duration)timer=duration;

		if(timer>deltaTime){
			timer-=deltaTime;
			return false;
		}else{
			if(duration>deltaTime) timer+=duration-deltaTime;
			else timer=Duration::zero();//Overflow protection for fast timers

			return true;
		}
	}
	bool cyclicEvent(Duration& timer,Duration duration,bool resetNow){
		if(resetNow){
			timer=duration;
			return true;
		}
		return cyclicEvent(timer,duration);
	}

	void setup(){
		lastTime=micros();
	}

	void loop(){
		auto delta=micros()-lastTime;
		lastTime+=delta;
		deltaTime=Duration::micros(delta);
	}
}/*
extern void* operator new(size_t,void*) noexcept;//Somewhere already declared
bool strEquals(const char* a,const char* b){return strcmp(a,b)==0;}

*/