#pragma once
#include "Arduino.h"

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
	bool cyclicEvent(Duration& timer,Duration duration,bool resetNow,bool triggerOnReset=true){
		if(resetNow){
			timer=duration;
			return triggerOnReset;
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
}

#define DBG_VAR(x) do{Serial.print(#x);Serial.print(':');Serial.println(x);}while(0)
