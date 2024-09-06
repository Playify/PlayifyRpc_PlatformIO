#include "Arduino.h"

namespace RpcInternal{
#define MACRO_STR2(x) #x
#define MACRO_STR(x) MACRO_STR2(x)
	String getRpcVersion(){
		return MACRO_STR(RPC_LIB_VERSION);
	}
#undef MACRO_STR
#undef MACRO_STR2
}