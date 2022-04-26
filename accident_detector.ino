#include <SoftwareSerial.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

static const int RXPin = 10, TXPin = 11;
static const uint32_t GPSBaud = 9600;
// GSM module
SoftwareSerial myserial(12,13);

//  GPS module
SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;
char msg;
char call;

int x_pin = A0, y_pin = A1, z_pin = A2;
int x_val, y_val, z_val;
float x_out, y_out, z_out;

int inputMin = 0;
int inputMax = 1023;
int sampleRate = 10;

bool sent = false; // Trang thai gui tin nhan, false = chua gui, true = da gui
int yes = 5;
int no = 7;
int sos = 6;
int sos_stat = 0;
int yesButton_stat;
int noButton_stat;
int sosButton_stat;
int yes_stat = 0;
int no_stat = 0;
int count = 0;
String Message = "";
String Speed = "";
float lattitude, longtitude, speedd; //create variable for lattitude and longtitude object

void setup() 
{
  // put your setup code here, to run once:
  analogReference(EXTERNAL);
  myserial.begin(9600);
  Serial.begin(9600);
  ss.begin(GPSBaud);
  
  /*Serial.println("h: to disconnect a call");
  Serial.println("i: to receive a call");
  Serial.println("s: to send messages");
  Serial.println("r: to receive message");
  Serial.println("c: to make a call");
  Serial.println("e: to redial");*/

  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.backlight();
  lcd.print("Please Wait....");
  delay(1000);
  lcd.clear();
  lcd.print("  Auto Crashed");
  lcd.setCursor(0,1);
  lcd.print("    Detector   ");
  delay(1500);
  lcd.clear();
  
  lcd.print("  System Ready  ");
  lcd.setCursor(0,1);
  lcd.print("      :)))    ");
  delay(1500);
  lcd.clear();
 
    // set for LED
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(yes, INPUT);
  pinMode(no, INPUT);
  pinMode(sos, INPUT);

}

void loop() 
{
    // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();          // print info to LCD
  if(sent == false )
    show_coordinate();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detedted: Check wires.");
    while(true);
  }
  
  // SOS
  sosButton_stat = digitalRead(sos);
  if(sosButton_stat == HIGH)
  {
    SOS();
  }
  
  //Accelerometer
  
  x_val = axisSample(x_pin);
  y_val = axisSample(y_pin);
  z_val = axisSample(z_pin);

  long xScaled = map(x_val, inputMin, inputMax, -3000, 3000);
  long yScaled = map(y_val, inputMin, inputMax, -3000, 3000);
  long zScaled = map(z_val, inputMin, inputMax, -3000, 3000);

  x_out = xScaled/1000.0;
  y_out = yScaled/1000.0;
  z_out = zScaled/1000.0;

  Serial.print("X / Y / Z:" );
  Serial.print(x_out);
  Serial.print("G / ");
  Serial.print(y_out);
  Serial.print("G / ");
  Serial.print(z_out);
  Serial.println("G");

  if(x_out <= (-0.2) || x_out >= 0.1 || y_out >=0.02 || z_out <= (-0.15))
  {
    if(sent == false)
    {
      accident();
    }
    lcd.clear();
    led_buzzer();
    if(no_stat != 0)
    {
      Serial.println("No message sent !");
      lcd.print("No message sent !");
      no_stat = 0;
      delay(500);
      lcd.clear();
      delay(200);
    }
    else if(yes_stat != 0 && sent == false)
    {
      lcd.clear();
      lcd.print("Message sent !");
      SendMessage();
      Serial.println("Message sent !");
      yes_stat = 0;
      sent = true;
      delay(500);
      lcd.clear();
      delay(200);
    }
    else if(count >= 20 && sent == false)
    {
      lcd.clear();
      lcd.print("Message sent !");
      SendMessage();
      Serial.println("Message sent ");
      sent = true;
      delay(500);
      lcd.clear();
      delay(200);
    }
    if(sent == true)
    {
      lcd.setCursor(1, 0);
      lcd.print("Message sent !");
      lcd.setCursor(4, 1);
      lcd.print("HELP !!!");
      led_buzzer();
    }
    lcd.clear();
  }

  //GSM
  
  /*if(Serial.available()>0)
    switch(Serial.read())
   {
      case's':
        SendMessage();
        break;
      case 'r':
        ReceiveMessage();
        break;
      case 'c':
        MakeCall();
        break;
   }
   if(myserial.available()>0)
   Serial.write(myserial.read());*/
}

void SOS()
{
  lcd.clear();
  sos_stat++;
  Serial.print("SOS: ");
  Serial.println(sos_stat);
  lcd.print("SOS ? ");
  lcd.print(sos_stat);
  led_buzzer();
  if(sos_stat >= 3)
  {
    lcd.clear();
    lcd.print("SOS sent !");
    SendSos();
    lcd.clear();
    count = 0;
    while(count < 5)
    {
      Serial.println("SOS sent !");
      lcd.print("SOS sent !");
      led_buzzer();
      lcd.clear();
      delay(500);
      count++;
    }
    sos_stat = 0;
    count = 0;
  }
  delay(1000);
  lcd.clear();
}

void SendMessage()
{
  myserial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  myserial.println("AT+CMGS=\"+84962389960\"\r"); // Replace x with mobile number
  delay(500);
  myserial.println("Vehicle crashed !");// The SMS text you want to send
  myserial.print("Latitude:");
  myserial.println(lattitude, 6);
  myserial.print("Longtitude:");
  myserial.println(longtitude, 6);
  delay(500);
  myserial.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
  myserial.print(lattitude, 6);
  myserial.print("+");              
  myserial.print(longtitude, 6);
  myserial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

void SendSos()
{
  myserial.println("AT+CMGF=1");
  delay(500);
  myserial.println("AT+CMGS=\"+84962389960\"\r");
  delay(500);
  myserial.println("SOS!!! Send help!!! SOS!!!");
  myserial.print("Latitude:");
  myserial.println(lattitude, 6);
  myserial.print("Longtitude:");
  myserial.println(longtitude, 6);
  delay(500);
  myserial.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
  myserial.print(lattitude, 6);
  myserial.print("+");              
  myserial.print(longtitude, 6);
  myserial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}


 void ReceiveMessage()
{
  myserial.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
  if(myserial.available()>0)
  {
    msg=myserial.read();
    Serial.print(msg);
  }
}


 int axisSample(int inputPin)          //Lấy trung bình giá trị đọc
{
  long value = 0;
  int output;
  analogRead(inputPin);
  delay(1);
  for(int i = 0; i < sampleRate; i++)
    value += analogRead(inputPin);

  return value/sampleRate;
}

void led_buzzer()
{
  digitalWrite(2, 1);
  digitalWrite(3, 1);
  delay(500);
  digitalWrite(2, 0);
  digitalWrite(3, 0);
  delay(500);
}

void accident()
{
  yesButton_stat = digitalRead(yes);
  noButton_stat = digitalRead(no);
  count = 0;
  while(count <= 20 && yesButton_stat == LOW && noButton_stat == LOW)
  {
    yesButton_stat = digitalRead(yes);
    noButton_stat = digitalRead(no);
    Serial.println("Call rescue ?");
    lcd.clear();
    lcd.print("Call rescue ? ");
    Serial.print("count = ");
    Serial.println(count);
    lcd.print(count);
    led_buzzer();
    count++;
    delay(500);
    lcd.clear();
    if(noButton_stat == HIGH)
    {
      no_stat++;
      break;
    }
    else if(yesButton_stat == HIGH)
    {
      yes_stat++;
      break;
    }
  }
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.println(gps.location.lng(), 6);
      lattitude = gps.location.lat();
      longtitude = gps.location.lng();
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  
  if(gps.speed.isValid())
  {
    Serial.print(gps.speed.kmph());
      speedd = gps.speed.kmph();
  }
  Serial.println();
}

void show_coordinate()
{
  lcd.setCursor(0, 0);
  lcd.print("Detecting GPS...");
  if(gps.location.isValid())
  {
    lcd.print("Lat:  ");
    lcd.print(lattitude, 6);
    lcd.setCursor(0, 1);
    lcd.print("Long: ");
    lcd.print(longtitude, 6);
  }
}
