#include <utility>

#include "Arduino.h"

#if ESP32

#include "WiFi.h"

#elif ESP8266

#include "ESP8266WiFi.h"

#endif

/**
 * Usable via webbrowser or curl: 
 * e.g. use 'curl x.x.x.x/D5=' to toggle pin D5
 * when using watch, use 'watch -n 1 curl -s x.x.x.x/D5'
 */
namespace WebDebugger{
	bool _locked;
	bool _connected;
	uint16_t _port;
	WiFiServer server(0);
	String _lastSerialCommand;
	String _currSerialCommand="";


	bool _tryParse(const String& s,uint8_t& pin){
		if(!s)return false;

		for(char i:s)
			if(!isDigit(i))
				return false;

		pin=s.toInt();
		return true;
	}

	template<size_t N>
	bool _tryParse(const String& s,const uint8_t(& arr)[N],uint8_t& pin){
		if(!_tryParse(s,pin))return false;
		if(pin>=N)return false;
		pin=arr[pin];
		return pin!=255;
	}

	bool tryGetPin(const String& s,uint8_t& pin){//s should be upper case
#define PINS(prefix,pins...) {static const uint8_t pinsArray[]{pins};if(s[0]==prefix&&_tryParse(s.substring(1),pinsArray,pin))return true;}
#define PIN(name) if(s==#name){pin=name;return true;}

#ifdef LED_BUILTIN
		if(s=="L"||s=="LED"||s=="LED_BUILTIN"){
			pin=LED_BUILTIN;
			return true;
		}
#endif

//Pins must be in order, without skipping any number. Skipping can be done using 255
#if ESP32
		PINS('T',T0,T1,T2,T3,T4,T5,T6,T7,T8,T9)
		PINS('A',A0,255,255,A3,A4,A5,A6,A7,255,255,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19)
		PIN(DAC1)
		PIN(DAC2)
#elif ESP8266
		PINS('D',D0,D1,D2,D3,D4,D5,D6,D7,D8,D9,D10)
		PIN(A0)
#endif

#undef PINS
#undef PIN
		return _tryParse(s,pin);
	}


	String runCommand(String cmd){
		cmd.trim();
		if(!cmd.length()) return "No command provided";
		cmd.toUpperCase();

		//Single command
		uint8_t pin;
		if(tryGetPin(cmd,pin)){
#if SOC_TOUCH_SENSOR_NUM>0
			if(cmd[0]=='T')return cmd+" ("+pin+") is:"+touchRead(pin)+" (touch)";
#endif
			if(cmd[0]=='A')return cmd+" ("+pin+") is:"+analogRead(pin)+" (analog)";
			return cmd+" ("+pin+") is "+digitalRead(pin);
		}

		if(cmd=="LOCK"||cmd=="PAUSE"){
			_locked=true;
			return "Program paused, use 'unpause' to continue";
		}
		if(cmd=="UNLOCK"||cmd=="UNPAUSE"||cmd=="RESUME"){
			_locked=false;
			return "Program unpaused";
		}
		if(cmd=="IP")return "IP is "+WiFi.localIP().toString();
		if(cmd=="GATEWAY")return "Gateway is "+WiFi.gatewayIP().toString();
		if(cmd=="SUBNET")return "Subnet is "+WiFi.subnetMask().toString();
		if(cmd=="MAC")return "MAC is "+WiFi.macAddress();
		if(cmd=="WIFI"){
			auto status=WiFi.status();
			static const char* statuses[]{
					"WL_IDLE_STATUS",
					"WL_NO_SSID_AVAIL",
					"WL_SCAN_COMPLETED",
					"WL_CONNECTED",
					"WL_CONNECT_FAILED",
					"WL_CONNECTION_LOST",
					"WL_WRONG_PASSWORD",
					"WL_DISCONNECTED",
					"???"};
			return "Wifi status: "+String(status)+"="+statuses[status<=7?status:8];
		}


		//setter command
		int i=cmd.indexOf('=');
		if(i==-1)return "Unknown command: "+cmd;
		String pinString=cmd.substring(0,i);
		String value=cmd.substring(i+1);


		if(tryGetPin(pinString,pin)){
#if SOC_DAC_PERIPH_NUM>0
			if(pinString.startsWith("DAC")){
				if(value.length()){
					dacWrite(DAC1,value.toInt());
					return pinString+" ("+pin+") set to "+value.toInt()+" (analog)";
				}else{
					dacDisable(DAC1);
					return pinString+" ("+pin+") set to disabled (analog)";
				}
			}
#endif


			if(value==""||value=="!"||value=="t"||value=="toggle"){
				bool toggle=!digitalRead(pin);
				pinMode(pin,OUTPUT);
				digitalWrite(pin,toggle);
				return pinString+" ("+pin+") toggled to "+toggle;
			}
			if(value=="1"||value=="HIGH"||value=="high"){
				pinMode(pin,OUTPUT);
				digitalWrite(pin,HIGH);
				return pinString+" ("+pin+") set to 1";
			}
			if(value=="0"||value=="LOW"||value=="low"){
				pinMode(pin,OUTPUT);
				digitalWrite(pin,LOW);
				return pinString+" ("+pin+") set to 0";
			}
			if(value=="I"||value=="IN"||value=="INPUT"){
				pinMode(pin,INPUT);
				return pinString+" ("+pin+") configured to INPUT";
			}
			if(value=="O"||value=="OUT"||value=="OUTPUT"){
				pinMode(pin,OUTPUT);
				return pinString+" ("+pin+") configured to OUTPUT";
			}
			if(value=="PULLUP"){
				pinMode(pin,INPUT_PULLUP);
				return pinString+" ("+pin+") configured to INPUT_PULLUP";
			}
#ifdef INPUT_PULLDOWN
			if(value=="PULLDOWN"){
				pinMode(pin,INPUT_PULLDOWN);
				return pinString+" ("+pin+") configured to INPUT_PULLDOWN";
			}
#endif
#ifdef INPUT_PULLDOWN_16
			if(value=="PULLDOWN"){
				pinMode(pin,INPUT_PULLDOWN_16);
				return pinString+" ("+pin+") configured to INPUT_PULLDOWN";
			}
#endif
		}
		return "Unknown command: "+cmd;
	}


	void setup(uint16_t port=80){
		server.begin(_port=port);
	}

	void loop(bool serial=true){
		do{
			while(serial&&Serial.available()){
				auto c=char(Serial.read());
				if(c=='\b')_currSerialCommand=_currSerialCommand.substring(0,_currSerialCommand.length()-1);
				else if(c!='\n')_currSerialCommand+=c;
				else{
					_currSerialCommand.trim();
					if(_currSerialCommand.length())_lastSerialCommand=_currSerialCommand;
					else _currSerialCommand=_lastSerialCommand;
					Serial.print("[WebDebugger] ");
					if(_locked)Serial.println("(Paused) ");
					Serial.println(runCommand(_currSerialCommand));

					_currSerialCommand="";
				}
			}


			if(_connected!=WiFi.isConnected()){
				_connected=!_connected;
				if(_connected){
					Serial.print("[WebDebugger] available at http://");
					Serial.print(WiFi.localIP());
					if(_port==80)Serial.println();
					else{
						Serial.print(":");
						Serial.println(_port);
					}
				}
			}

			if(auto client=server.accept()){
				String request=client.readStringUntil('\n');
				String cmd=request.substring(request.indexOf(' ')+2,
											 request.lastIndexOf(' '));//get url, without starting slash

				while(client.connected()){//Skip headers
					String header=client.readStringUntil('\n');
					header.trim();
					if(!header.length())break;
				}
				if(client.connected()){
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/plain");
					client.println();
					if(_locked)client.print("(Paused) ");
					client.println(runCommand(cmd));
					client.stop();
				}
			}
			yield();
		}while(_locked);
	}
}