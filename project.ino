#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
// uart ayarları değikenleri...
long baudRate = 9600; //baudrate
int wordLength = 8; // karakter uzunluğu(5,6,7,8,9)
char parity = 'N'; // parite  (N,E,O)
int stopBit = 1; // stop biti (1,2)

//genel değişkenler
String inputString = "";   
String onVal,offVal;

bool stringComplete = false;  
bool state = false;
bool echoState = false;

char temp;
char inChar;

int detecton,detectoff;
int start = 0,stop=-1;
int prevStart = 0,prevStop=-1;
int ledState = LOW;   

const long interval = 1000; 

long OnTime = 100;          
long OffTime = 100;

unsigned long previousMillis = 0;   
unsigned long previousMillis2 = 0;    

//fonksiyonları tanıt
void task1(); 
void task2();
void led1sn();
void uartConfig();

void setup() {

  uartConfig();        // uart konfigürasyonlarını yap..
  pinMode(2, INPUT);   
  digitalWrite(2, LOW); 
  attachInterrupt(0, serialInterrupt, CHANGE); //rx pini için interrupt ayarlanıyor.
  pinMode(13, OUTPUT);  //arduino hazır Led pinini tanıt
  sei();  // kesmeler aktif.

}
    
void loop() { // sonsuz döngü.
 
 if(start == 0)    // start stringi alınmış ise echo yap ve ledi girilen ledon/ledoff değerleri aralığında çalıştır
 {
  task1();    // 13 pine bağlı ledi  ledon/ledoff sürelerine göre yak.
  task2();    //echoyu aktif et.
  prevStart = start; //start değerini saklı tut 

 }

 if(stop == 0)  // stop stringi alınmış ise echoyu durdur ve ledi 1s aralığında çalıştır.
 {
  led1sn();  //1s blink
  echoState = false; // 
  prevStop = stop; //stop değerini saklı tut 

 }
//Serial.println(echoState);

}



void serialInterrupt()   // uart kesmesi.. gelen her veri için program buraya dallanıyor. 
{                        // Ayrıca "start" komutu alndıysa echo yapılıyor..

   while (Serial.available()) {
    inChar = (char)Serial.read();
       if(echoState)  // eğer echo aktif ise gelen veriyi yansıt.. (tekrar gönder)
      {
         Serial.print(inChar);
   }

    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;  // char tipinde karakterler stringe çevrildi.
    }
  }


if (stringComplete) {     // string alındıysa gelen dizide start, stop, ledon ve ledoff kelimelerini ara
    start = inputString.indexOf("start"); //start
    stop = inputString.indexOf("stop"); //stop
    if (start == -1 && stop == -1) {    // ne start ne de stop alınmadı ise önceki start ve stop değerlerini koru.
      start = prevStart;
      stop = prevStop;

    }

   detecton = inputString.indexOf("ledon=");   // "ledon=" stringi aranıyor..
   detectoff = inputString.indexOf("ledoff="); //"ledoff=" stringi aranıyor..

     if (detecton == 0){          // stringde ledon kelimesi geçiyorsa, eşittir karakterinden sonraki değeri al.
       onVal = inputString.substring(6, 15); // ledon süre değerini onVal değerine aktar.
       OnTime = onVal.toInt();  //onVal stringindeki ledon süresini int olan OnTime değerine eşitle.
     }
     if (detectoff == 0){    //stringde ledoff kelimesi geçiyorsa, eşittir karakterinden sonraki değeri al.
       offVal = inputString.substring(7, 15); // ledoff süre değerini onVal değerine aktar.
       OffTime = offVal.toInt(); //offVal stringindeki ledoff süresini int olan OffTime değerine  eşitle.
     }
 
    inputString = "";  // stringi boşalt
    stringComplete = false;  // prosesi sıfırla..

  //Serial.print(detecton);Serial.print("\t");Serial.println(detectoff);
  //Serial.print(onVal);Serial.print("\t");Serial.println(offVal);
  }
   
}




void task1()   // "OnTime" ve "OffTime"  değerlerine göre milis ile blink taskı.
{

  unsigned long currentMillis = millis();  //Cooperative Multitasking olduğu için bütün beklemeler milis fonksiyonu ile yapılıyor. 
  //Dolayısıyla CPU bir anlamda bütün taskları birlikte çalıştırıyor..
 
  if((ledState == HIGH) && (currentMillis - previousMillis >=OnTime)) //milis ile delay kodları
  {
    ledState = LOW;  
    previousMillis = currentMillis;  
    digitalWrite(13, ledState);  
  }
  else if ((ledState == LOW) && (currentMillis - previousMillis >=OffTime))
  {
    ledState = HIGH; 
    previousMillis = currentMillis;  
    digitalWrite(13, ledState);	  
  }

  }




  void task2()  // echo taskı 
  {

    echoState = true;
    /*
    bu fonksiyon gereksiz gibi duruyor fakat gelen projede echo bir task şeklinde istendiği için
    böyle yaptım..
    */
  
  }




  void led1sn(){   // 1000ms standart blink fonksiyonu

  unsigned long currentMillis2 = millis();  //milis ile delay kodları

  if (currentMillis2 - previousMillis2 >= interval) {
    previousMillis2 = currentMillis2;
    
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(13, ledState);
  }

  }




void uartConfig(){    // uart konfigüre fonksiyonu.
  Serial.begin(baudRate); // baud hızını ayarla

  if (wordLength == 5)    // karakter uzunluğunu ayarla
    {
      UCSR0C = UCSR0C | B00000000;
    }
  else if (wordLength == 6)
    {
      UCSR0C = UCSR0C | B00000010;
     }
  else if (wordLength == 7)
    {
      UCSR0C = UCSR0C | B00000100;
    }
  else if (wordLength == 9)
    {
      UCSR0C = UCSR0C | B00000110;
      UCSR0B = UCSR0B | B00000100;
    }
  else // (wordlength == 8)    // varsayılan
    {
      UCSR0C = UCSR0C | B00000110;
    }


    if ((parity == 'O')|(parity == 'o'))   // pariteyi ayarla

      {
        UCSR0C = UCSR0C | B00110000;
      }
    else if ((parity == 'E')|(parity == 'e'))
      {
        UCSR0C = UCSR0C | B00100000;
      }
    else // ((parity == 'N')|(parity == 'n')))  // varsayılan
      {
        UCSR0C = UCSR0C | B00000000;
      }


    if (stopBit == 1)      //stop bitini ayarla..
      {
        UCSR0C = UCSR0C | B00000100;
      }
    else // (stop == 2)
      {
        UCSR0C = UCSR0C | B00000000;
      }
    
}
