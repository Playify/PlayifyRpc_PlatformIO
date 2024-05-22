String quoted(String s){
	return s==NULL_STRING?"null":"\""+s+"\"";
}

String jsonEncode(const String& input){
	String output="\""; // Start with the opening quote
	output.reserve(input.length()*2); // Reserve enough space to avoid frequent reallocations

	for(unsigned int i=0;i<input.length();i++){
		char c=input.charAt(i);
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

struct RpcTypeNotFoundError: public RpcError{
	explicit RpcTypeNotFoundError(String type):RpcError(
			"RpcTypeNotFoundError",
			NULL_STRING,
			"Type "+quoted(type)+" does not exist",
			"",
			"{\"$type\":\"$type\""
			",\"type\":"+jsonEncode(type)+"}"){}
};

struct RpcMethodNotFoundError: public RpcError{
	explicit RpcMethodNotFoundError(String type,String method):RpcError(
			"RpcMethodNotFoundError",
			NULL_STRING,
			"Method "+quoted(method)+" does not exist on type "+quoted(type),
			"",
			"{\"$type\":\"$method\""
			",\"type\":"+jsonEncode(type)+
			",\"method\":"+jsonEncode(method)+"}"){}
};

struct RpcMetaMethodNotFoundError: public RpcError{
	explicit RpcMetaMethodNotFoundError(String type,String meta):RpcError(
			"RpcMetaMethodNotFoundError",
			NULL_STRING,
			"Meta-Method "+quoted(meta)+" does not exist on type "+quoted(type),
			"",
			"{\"$type\":\"$method-meta\""
			",\"type\":"+jsonEncode(type)+
			",\"method\":null"
			",\"meta\":"+jsonEncode(meta)+"}"){}
};


struct RpcConnectionError: public RpcError{
	explicit RpcConnectionError(const char* msg):RpcError(
			"RpcConnectionError",
			NULL_STRING,
			msg,
			"",
			"{\"$type\":\"$connection\"}"){}
};

struct RpcEvalError: public RpcError{
	explicit RpcEvalError(const char* msg):RpcError(
			"RpcEvalError",
			NULL_STRING,
			msg,
			"",
			"{\"$type\":\"$eval\"}"){}
};