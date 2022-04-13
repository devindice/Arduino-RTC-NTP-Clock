/***************\
    Libraries
\***************/
// WiFi
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

// RTC Module (Clock)
#include <DS3231.h>

// NTP
#include <NTPClient.h>

// TimeZone
#include <TimeLib.h>
#include <time.h>
#include <Timezone.h>    // https://github.com/JChristensen/Timezone



/****************\
    Parameters
\****************/

// WiFi
const char *ssid     = "";
const char *password = "";

// RTC Module (Clock)
DS3231 clock1;
RTCDateTime dt;

// NTP
int GTMOffset = 0; // SET TO UTC TIME
const char weekDays[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char months[12][10]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", GTMOffset*60*60, 60*60*1000);

// Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);



/***************\
    Functions
\***************/

// Time Zone Input
static tm getDateTimeByParams(long time){
    struct tm *newtime;
    const time_t tim = time;
    newtime = localtime(&tim);
    return *newtime;
}

// Time Zone Output
static String getDateTimeStringByParams(tm *newtime, char* pattern = (char *)"%Y,%m,%d,%H,%M,%S"){
    char buffer[30];
    strftime(buffer, 30, pattern, newtime);
    return buffer;
}

// Time Zone Function
static String getEpochStringByParams(long time, char* pattern = (char *)"%Y,%m,%d,%H,%M,%S"){
    tm newtime;
    newtime = getDateTimeByParams(time);
    return getDateTimeStringByParams(&newtime, pattern);
}

// RTC Module (Clock)
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

// Set RTC Date
void updateRtcDateTime() {
  // Check if we have a WiFi connection
  if ( WiFi.status() == WL_CONNECTED ) {
    // NTP Update
    if (timeClient.update()){
      
      // Definitions
      char *dateTime[7];
      char *ptr = NULL;
      byte index = 0;
      
      Serial.print ( "Syncing RTC clock with NTP: " );
      // Get NTP Epoch Time
      unsigned long epoch = timeClient.getEpochTime();
      // Convert Epoch to the Local time string
      String dateString = getEpochStringByParams(usET.toLocal(epoch));
      Serial.println (dateString);
  
      // Convert string to char array so we can point to individual values
      int dateLength = dateString.length() + 1;
      char dateChar[dateLength];
      dateString.toCharArray(dateChar, dateLength);
  
      // Array of pointers to the dateChar array, Used to point to the memory values in the string.
      ptr = strtok(dateChar, ",");
      while (ptr != NULL)
      {
        dateTime[index] = ptr;
        index++;
        ptr = strtok(NULL, ",");
      }

     // Update RTC Date and Time
     clock1.setDateTime((int) atoi(dateTime[0]),
                        (int) atoi(dateTime[1]),
                        (int) atoi(dateTime[2]),
                        (int) atoi(dateTime[3]),
                        (int) atoi(dateTime[4]),
                        (int) atoi(dateTime[5])
      );
      
    }else{
      Serial.println ( "NTP Update Failed!" );
    }
  }else{
    Serial.println("No WiFi, cannot update NTP"); 
  }
}



/***********\
    Setup
\***********/

void setup(){
  // Set Serial Baud
  Serial.begin(115200);

  // Setup WiFi Hostname and Connect
  WiFi.hostname("Arduino");
  WiFi.begin(ssid, password);

  // Setup Board LED for control
  pinMode(LED_BUILTIN, OUTPUT);

  // Wait WiFi connection
  int x = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    // Blink board LED while connecting to WiFi
    for (int x = 0; x < 5; x++) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(1);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(99);
    };
    Serial.print ( "." );
    x++;
    if (x == 30) {
      Serial.println("WiFi Failed to connect, Will continue to try connecting");
      break;
    }
  }

  // Test NTP connection
  Serial.println("Initialize NTP connection");
  timeClient.begin();
  
  // RTC Module DS3231 (Clock)
  Serial.println("Initialize RTC module");
  clock1.begin();
  
}



/***************\
    Main Loop
\***************/

void loop() {

  updateRtcDateTime();
  // Cycles before syncing RTC with NTP
  for (int x = 0; x < 500; x++) {

    // RTC Clock
    dt = clock1.getDateTime();

    // Print time
    Serial.print(weekDays[dt.dayOfWeek]); Serial.print(", ");
    Serial.print(dt.day); Serial.print(" ");
    Serial.print(months[dt.month -1]); Serial.print(" ");
    Serial.print(dt.year); Serial.print(", ");
    print2digits(dt.hour);   Serial.print(":");
    print2digits(dt.minute); Serial.print(":");
    print2digits(dt.second); Serial.println("");

    // Sleep 1 second
    delay(1000);
  }
  
}
