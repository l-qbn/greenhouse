 /* ====== ESP8266 Demo ======
 * (Updated Dec 14, 2014)
 * Ray Wang @ Rayshobby LLC
 * http://rayshobby.net/?p=9734
 * ==========================
 *
 * Modified by R. Wozniak
 * Compiled with Arduino 1.60 and Teensyduino 1.21b6
 * ESP8266 Firmware: AT21SDK95-2015-01-24.bin
 *
 * Change SSID and PASS to match your WiFi settings.
 * The IP address is displayed serial upon successful connection.
 */

#define BUFFER_SIZE 1024

#define SSID  "??????????"      // change this to match your WiFi SSID
#define PASS  "??????????"  // change this to match your WiFi password
#define PORT  "80"           // using port 8080 by default

char buffer[BUFFER_SIZE];

// By default we are looking for OK\r\n
char OKrn[] = "OK\r\n";
byte wait_for_esp_response(int timeout, char* term=OKrn) {
  unsigned long t=millis();
  bool found=false;
  int i=0;
  int len=strlen(term);
  // wait for at most timeout milliseconds
  // or if OK\r\n is found
  while(millis()<t+timeout) {
    if(Serial1.available()) {
      buffer[i++]=Serial1.read();
      if(i>=len) {
        if(strncmp(buffer+i-len, term, len)==0) {
          found=true;
          break;
        }
      }
    }
  }
  buffer[i]=0;
  Serial.print(buffer);
  return found;
}

void setup() {

  // assume esp8266 operates at 115200 baud rate
  // change if necessary to match your modules' baud rate
  Serial1.begin(115200);  // Teensy Hardware Serial port 1   (pins 0 and 1)
  Serial.begin(115200);   // Teensy USB Serial Port
  
  delay(5000);
  Serial.println("begin.");  
  setupWiFi();

  // print device IP address
  Serial.print("device ip addr: ");
  Serial1.println("AT+CIFSR");
  wait_for_esp_response(1000);
}

bool read_till_eol() {
  static int i=0;
  if(Serial1.available()) {
    buffer[i++]=Serial1.read();
    if(i==BUFFER_SIZE)  i=0;
    if(i>1 && buffer[i-2]==13 && buffer[i-1]==10) {
      buffer[i]=0;
      i=0;
      Serial.print(buffer);
      return true;
    }
  }
  return false;
}

void loop() {
  int ch_id, packet_len;
  char *pb;  
  if(read_till_eol()) {
    if(strncmp(buffer, "+IPD,", 5)==0) {
      // request: +IPD,ch,len:data
      sscanf(buffer+5, "%d,%d", &ch_id, &packet_len);
      if (packet_len > 0) {
        // read serial until packet_len character received
        // start from :
        pb = buffer+5;
        while(*pb!=':') pb++;
        pb++;
        if (strncmp(pb, "GET /", 5) == 0) {
          wait_for_esp_response(1000);
          Serial.println("-> serve homepage");
          serve_homepage(ch_id);
        }
      }
    }
  }
}

void serve_homepage(int ch_id) {
  
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nRefresh: 300\r\n";

  String content="";
    content += "<!DOCTYPE html>";
    content += "<html>";
    content += "<body>";
    
    content += " <h1> Abraham Lincoln's Gettysburg Address </h1> <br/>  ";
       
    content += "<p> <strong> Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal. <br/>";
    content += " <br/>";
    content += "Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. <br/>";
    content += " <br/>";
    content += "But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, under God, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not perish from the earth. <br/>";
    content += " <br/>";
    content += "Abraham Lincoln <br/>";
    content += "November 19, 1863 </strong> </p> <br/>";
    
    content += " <br/>";
    content += " <p> <font color=#880088> This message brought to you by - TEENSY 3.1 & ESP8266 </font> </p>";
    content += " <p> Teensy server uptime ";
    Serial.print("***************************************** UPTIME = ");
    Serial.println(millis()/1000);
    content += "<font color=#0000FF> ";
    content += String(millis()/1000);
    content += " seconds </font> </p>";
    
      
    content += "</body>";
    content += "</html>";
    content += "<br />\n";       
    content += "\r\n";       

  header += "Content-Length:";
  header += (int)(content.length());
  header += "\r\n\r\n";
  Serial1.print("AT+CIPSEND=");
  Serial1.print(ch_id);
  Serial1.print(",");
  Serial1.println(header.length()+content.length());
  if(wait_for_esp_response(2000)) {
   //delay(100);
   Serial1.print(header);
   Serial1.print(content);
  } 
  else {
  Serial1.print("AT+CIPCLOSE=");
  Serial1.println(ch_id);
 }
}

void setupWiFi() {

  // turn on echo
  Serial1.println("ATE1");
  wait_for_esp_response(1000);
  
  // try empty AT command
  //Serial1.println("AT");
  //wait_for_esp_response(1000);

  // set mode 1 (client)
  Serial1.println("AT+CWMODE=1");
  wait_for_esp_response(1000); 
 
  // reset WiFi module
  Serial1.print("AT+RST\r\n");
  wait_for_esp_response(1500);

   //join AP
  Serial1.print("AT+CWJAP=\"");
  Serial1.print(SSID);
  Serial1.print("\",\"");
  Serial1.print(PASS);
  Serial1.println("\"");
  wait_for_esp_response(5000);

  // start server
  Serial1.println("AT+CIPMUX=1");
   wait_for_esp_response(1000);
  
  //Create TCP Server in 
  Serial1.print("AT+CIPSERVER=1,"); // turn on TCP service
  Serial1.println(PORT);
   wait_for_esp_response(1000);
  
  Serial1.println("AT+CIPSTO=30");  
  wait_for_esp_response(1000);

  Serial1.println("AT+GMR");
  wait_for_esp_response(1000);
  
  Serial1.println("AT+CWJAP?");
  wait_for_esp_response(1000);
  
  Serial1.println("AT+CIPSTA?");
  wait_for_esp_response(1000);
  
  Serial1.println("AT+CWMODE?");
  wait_for_esp_response(1000);
  
  Serial1.println("AT+CIFSR");
  wait_for_esp_response(5000);
  
  Serial1.println("AT+CWLAP");
  wait_for_esp_response(5000);
  
  Serial.println("---------------*****##### READY TO SERVE #####*****---------------");
  
}


