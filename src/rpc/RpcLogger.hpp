
namespace RpcLogger{
	enum LogLevel{
		Log,
		Special,
		Debug,
		Info,
		Warning,
		Error,
		Critical,
	};

	template<typename... Args>
	void logLevel(const LogLevel level,Args... args){
		auto call=RpcInternal::callRemoteFunction(NULL_STRING,"L",int(level),args...);
		call.onError([](const RpcError& e){
			Serial.print("Error printing to RpcLogger: ");
			e.printStackTrace();
		});
	}

	template<typename... Args>
	void log(Args...args){ logLevel(LogLevel::Log,args...); }

	template<typename... Args>
	void special(Args...args){ logLevel(LogLevel::Special,args...); }

	template<typename... Args>
	void debug(Args...args){ logLevel(LogLevel::Debug,args...); }

	template<typename... Args>
	void info(Args...args){ logLevel(LogLevel::Info,args...); }

	template<typename... Args>
	void warn(Args...args){ logLevel(LogLevel::Warning,args...); }

	template<typename... Args>
	void warning(Args...args){ logLevel(LogLevel::Warning,args...); }

	template<typename... Args>
	void error(Args...args){ logLevel(LogLevel::Error,args...); }

	template<typename... Args>
	void critical(Args...args){ logLevel(LogLevel::Critical,args...); }
}