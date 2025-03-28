namespace RpcInternal{
	String quoted(const String& s){
		return s==NULL_STRING?"null":"\""+s+"\"";
	}

	String jsonEncode(const String& input){
		String output="\""; // Start with the opening quote
		output.reserve(input.length()*2); // Reserve enough space to avoid frequent reallocations

		for(unsigned int i=0;i<input.length();i++){
			const char c=input.charAt(i);
			switch(c){
				case '\"':
					output+="\\\"";
					break;  // Escape double quote
				case '\\':
					output+="\\\\";
					break;  // Escape backslash
				case '\b':
					output+="\\b";
					break;   // Escape backspace
				case '\f':
					output+="\\f";
					break;   // Escape form feed
				case '\n':
					output+="\\n";
					break;   // Escape newline
				case '\r':
					output+="\\r";
					break;   // Escape carriage return
				case '\t':
					output+="\\t";
					break;   // Escape tab
				default:
					// If the character is a control character (less than 0x20) or outside of ASCII (greater than 0x7F),
					// we should encode it as \uXXXX
					if(c<0x20||c>0x7F){
						char buffer[7];
						snprintf(buffer,sizeof(buffer),"\\u%04x",(unsigned char)c);
						output+=buffer;
					}else{
						output+=c;
					}
			}
		}

		output+="\""; // End with the closing quote
		return output;
	}
}

struct RpcTypeNotFoundError: RpcError{
	explicit RpcTypeNotFoundError(const String& type):RpcError(
			"RpcTypeNotFoundError",
			NULL_STRING,
			"Type "+RpcInternal::quoted(type)+" does not exist",
			"",
			"{\"$type\":\"$type\""
			",\"type\":"+RpcInternal::jsonEncode(type)+"}"){}
};

struct RpcMethodNotFoundError: RpcError{
	explicit RpcMethodNotFoundError(const String& type,const String& method):RpcError(
			"RpcMethodNotFoundError",
			NULL_STRING,
			"Method "+RpcInternal::quoted(method)+" does not exist on type "+RpcInternal::quoted(type),
			"",
			"{\"$type\":\"$method\""
			",\"type\":"+RpcInternal::jsonEncode(type)+
			",\"method\":"+RpcInternal::jsonEncode(method)+"}"){}
};

struct RpcMetaMethodNotFoundError: RpcError{
	explicit RpcMetaMethodNotFoundError(const String& type,const String& meta):RpcError(
			"RpcMetaMethodNotFoundError",
			NULL_STRING,
			"Meta-Method "+RpcInternal::quoted(meta)+" does not exist on type "+RpcInternal::quoted(type),
			"",
			"{\"$type\":\"$method-meta\""
			",\"type\":"+RpcInternal::jsonEncode(type)+
			",\"method\":null"
			",\"meta\":"+RpcInternal::jsonEncode(meta)+"}"){}
};


struct RpcConnectionError: RpcError{
	explicit RpcConnectionError(const char* msg):RpcError(
			"RpcConnectionError",
			NULL_STRING,
			msg,
			"",
			R"({"$type":"$connection"})"){}
};

struct RpcEvalError: RpcError{
	explicit RpcEvalError(const char* msg):RpcError(
			"RpcEvalError",
			NULL_STRING,
			msg,
			"",
			R"({"$type":"$eval"})"){}
};

struct RpcDataError: RpcError{
	explicit RpcDataError(const char* msg):RpcError(
			"RpcEvalError",
			NULL_STRING,
			msg,
			"",
			R"({"$type":"$eval"})"){}
};