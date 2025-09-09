// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once
#include "Arduino.h"

struct Duration{
	uint32_t microseconds;

	constexpr Duration():microseconds(){}
private:
	constexpr explicit Duration(const uint32_t micros):microseconds(micros){}
public:
	uint32_t seconds() const{return microseconds/1000000;}
	uint32_t millis() const{return microseconds/1000;}
	uint32_t micros() const{return microseconds;}

	static constexpr Duration seconds(const uint32_t t){return Duration(t*1000000);}
	static constexpr Duration millis(const uint32_t t){return Duration(t*1000);}
	static constexpr Duration micros(const uint32_t t){return Duration(t);}
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
	constexpr Duration operator*(const uint32_t value) const { return Duration(microseconds * value); }
	Duration& operator*=(const uint32_t value) { return *this = *this * value; }
	constexpr Duration operator/(const uint32_t value) const { return Duration(microseconds / value); }
	Duration& operator/=(const uint32_t value) { return *this = *this / value; }
};

constexpr Duration operator ""_s(const unsigned long long t){return Duration::seconds(t);}
constexpr Duration operator ""_ms(const unsigned long long t){return Duration::millis(t);}
constexpr Duration operator ""_us(const unsigned long long t){return Duration::micros(t);}



namespace TimeHandler{
	uint32_t lastTime;
	Duration deltaTime;


	//counts down to zero, then return true every time
	inline bool waitZero(Duration& timer){
		if(timer>deltaTime){
			timer-=deltaTime;
			return false;
		}
		timer=Duration::zero();
		return true;
	}
	//counts down to zero, then return true once
	inline bool waitZeroOnce(Duration& timer){
		if(!timer)return false;
		return waitZero(timer);
	}

	//counts down to zero, then return true and resets the timer
	inline bool everyZero(Duration& timer,const Duration duration){
		if(timer>deltaTime){
			timer-=deltaTime;
			return false;
		}
		if(duration>deltaTime) timer+=duration-deltaTime;//Reset timer with overshoot protection
		else timer=duration;//Overflow protection for fast timers
		return true;
	}

	inline bool everyZero(Duration& timer,const Duration duration,const bool resetNow,const bool triggerOnReset=true){
		if(resetNow){
			timer=duration;
			return triggerOnReset;
		}
		return everyZero(timer,duration);
	}

	//counts up to target, then return true every time
	inline bool waitReach(Duration& timer,const Duration target){
		auto sum=timer+deltaTime;
		if(sum<timer) sum=Duration::maximum();//Integer Overflow
		
		if(sum<target){
			timer=sum;
			return false;
		}
		timer=target;
		return true;
	}
	//counts up to target, then return true once
	inline bool waitReachOnce(Duration& timer,const Duration target){
		if(timer>=target)return false;
		return waitReach(timer,target);
	}

	//counts up to target, then return true and resets the timer
	inline bool everyReach(Duration& timer,const Duration target){
		auto sum=timer+deltaTime;
		if(sum<timer) sum=Duration::maximum();//Integer Overflow
		
		if(sum<target){
			timer=sum;
			return false;
		}
		timer=sum-target;
		return true;
	}

	inline void setup(){
		lastTime=micros();
	}

	inline void loop(){
		const auto delta=micros()-lastTime;
		lastTime+=delta;
		deltaTime=Duration::micros(delta);
	}
}

#define DBG_VAR(x) do{Serial.print(#x);Serial.print(':');Serial.println(x);}while(0)
