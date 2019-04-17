/*
This program connects the arduino to the WiFi using the ESP8266WiFi.h library. The arduino then gets the
capability to interact with websites. The program allows the arduino to get our IP address first in the getIP
method. Once we receive our IP address we are then able to interact with another website to get details about our
location (city name, longtitude, latitude and more). We then use the city's name to get weather information.
*/

#include <ESP8266WiFi.h>// provides the ability to connect the arduino with the WiFi
#include <ESP8266HTTPClient.h> //provides the ability to interact with websites
#include <ArduinoJson.h> //provides the ability to parse and construct JSON objects

const char* ssid = "University of Washington"; //name of your WiFi network
const char* pass = ""; //password to the WiFi network
const char* key = ""; // API key for ipstack (https://ipstack.com)
const char* weatherKey = ""; // API key for openweathermap (https://openweathermap.org)
String ipAddress; //creates a new data type definition for storing the ip address

// Here we create a new data type definition, a box to hold other data types for each name:value pair coming in from the service, 
// we will create a slot in our structure to hold our data
typedef struct { 
  String ip;    
  String cc;   
  String cn;    
  String rc;
  String rn;
  String cy;
  String ln;
  String lt;
} GeoData;     //then we give our new data structure a name so we can use it in our code

typedef struct {
  String tp;
  String hd;
  String ws;
  String wd;
  String cl;
} MetData;    //then we give our new data structure a name so we can use it in our code

GeoData location; //we have created a GeoData type, but not an instance of that type, so we create the variable 'location' of type GeoData
MetData conditions; //we have created a MetData type, but not an instance of that type, so we create the variable 'conditions' of type MetData

// Setup routine runs once when you press reset or uploading the code to the arduino
void setup() {
  Serial.begin(115200); // initialize serial communications at 115200 bps
  delay(10); // wait for 10 ms
  // Prints the results to the serial monitor
  Serial.print("This board is running: ");  //Prints that the board is running
  Serial.println(F(__FILE__));
  Serial.print("Compiled: "); //Prints that the program was compiled on this date and time
  Serial.println(F(__DATE__ " " __TIME__));
  
  Serial.print("Connecting to "); Serial.println(ssid); //Prints that the arduino is connected to the WiFi

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  //while arduino is connected to wifi print . to serial monitor
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(); Serial.println("WiFi connected"); Serial.println(); //prints that we are connected to the wifi
  Serial.print("Your ESP has been assigned the internal IP address "); //prints the internal IP address
  Serial.println(WiFi.localIP());

  String ipAddress = getIP(); //calls the getIP function to get our external IP address
  getGeo(ipAddress);

  //prints a series of text describing our location based on the GeoData slots
  Serial.println();
  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");

  getMet(location.cy); //calles the getMet function to get weather data based on our location

  //prints a series of text describing the weather based on the MetData slots
  Serial.println();
  Serial.println("With cloudiness at " + conditions.cl + "%, the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour. ");
}

void loop() {
  //if we put getIP() here, it would ping the endpoint over and over . . . DOS attack?
}

String getIP() {
  HTTPClient theClient;
  Serial.println("Making getIP HTTP request");
  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  //checks wether got an error while trying to access the website/API url
  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received getIP HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing getIP...");
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();//return the ip address as a string

    } else {
      Serial.println("Something went wrong with connecting to the endpoint in getIP().");//prints the statement in case of failure connecting to the end point
      return "error";
    }
  }
  Serial.print("Retrieved IP Address from remote endpoint: ");
  Serial.println(ipAddress);
  return ipAddress; //the ip address string is then returned to be stored and accessed for later use
}

void getGeo(String IP) {
  HTTPClient theClient;
  Serial.println("Making getGeo HTTP request");
  theClient.begin("http://api.ipstack.com/" + IP + "?access_key=" + key); //return IP as .json object
  int httpCode = theClient.GET();

  //checks wether got an error while trying to access the website/API url
  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received getGeo HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing getGeo...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed in getGeo()");
        Serial.println(payload);
        return;
      }

      //collects values from JSON keys and stores them as strings because the slots in MetData are strings
      location.ip = root["ip"].as<String>();            
      location.cc = root["country_code"].as<String>();  
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint in getGeo().");//prints the statement in case of failure connecting to the end point
    }
  }
}

void getMet(String city) { 
  HTTPClient theClient;
  Serial.println("Making getMet HTTP request");
  theClient.begin("http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=imperial&appid=" + weatherKey);//return weather as .json object
  int httpCode = theClient.GET();

  //checks wether got an error while trying to access the website/API url
  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received getMet HTTP payload.");
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      Serial.println("Parsing getMet...");
      JsonObject& root = jsonBuffer.parseObject(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed in getMet().");
        return;
      }
      //collects values from JSON keys and stores them as strings because the slots in MetData are strings
      conditions.tp = root["main"]["temp"].as<String>(); 
      conditions.hd = root["main"]["humidity"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      int deg = root["wind"]["deg"].as<int>();//collects and stores deg value as an int for later conversion to directions
      conditions.wd = getNSEW(deg); //calls getNSEW to convert degress to directions
      conditions.cl = root["clouds"]["all"].as<String>();
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");//prints the statement in case of failure connecting to the end point
  }
}

//Receives weather direction data from getMet and converts the degrees into directions
//such as north, east, west, south and more
String getNSEW(int d) {
  String direct;

  if (d > 348.75 && d < 360 || d >= 0  && d < 11.25) {
    direct = "north";
  };
  if (d > 11.25 && d < 33.75) {
    direct = "north northeast";
  };
  if (d > 33.75 && d < 56.25) {
    direct = "northeast";
  };
  if (d > 56.25 && d < 78.75) {
    direct = "east northeast";
  };
  if (d < 78.75 && d < 101.25) {
    direct = "east";
  };
  if (d < 101.25 && d < 123.75) {
    direct = "east southeast";
  };
  if (d < 123.75 && d < 146.25) {
    direct = "southeast";
  };
  if (d < 146.25 && d < 168.75) {
    direct = "south southeast";
  };
  if (d < 168.75 && d < 191.25) {
    direct = "south";
  };
  if (d < 191.25 && d < 213.75) {
    direct = "south southwest";
  };
  if (d < 213.25 && d < 236.25) {
    direct = "southwest";
  };
  if (d < 236.25 && d < 258.75) {
    direct = "west southwest";
  };
  if (d < 258.75 && d < 281.25) {
    direct = "west";
  };
  if (d < 281.25 && d < 303.75) {
    direct = "west northwest";
  };
  if (d < 303.75 && d < 326.25) {
    direct = "south southeast";
  };
  if (d < 326.25 && d < 348.75) {
    direct = "north northwest";
  };
  return direct; //returns which degree matches witch direction
}
