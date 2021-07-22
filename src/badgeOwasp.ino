#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

//NeoPixel
#define PIN 15      //Puerto IO15
#define NUMPIXELS 2 //Numero de pixeles
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//Definición de pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET  4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String nombre = "";
String cmd;
int flag = -1;        // Solicitud de datos: -1(No se ha introducido ningun comando
                      //                     1(Se escribio el comando change, owasp twitter o facebook
                      //                     2(Se ingreso nombre)
                      //                     3(Se ingreso twitter)
                      //                     4(Se ingreso facebook)
                      
String twitter = "";  // Guarda el twitter del asistente
String facebook = ""; // Guarda el Facebook del asistente
String msg ="";
String txt;
String redesS[] = {"","",""};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int ind = 0;

void IRAM_ATTR redesSociales(void);

void setup() {
  // put your setup code here, to run once:
  // Inicializamos pixeles
  pixels.begin();
  pixels.clear();

  // Inicializamos serial
  Serial.begin(115200);
  cmd.reserve(10);
  twitter.reserve(10);
  facebook.reserve(10);
  nombre.reserve(20);
  msg.reserve(10);

  // Inicializando pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("Se requieere pantalla OLED"));
  }else {
   timer = timerBegin(0, 80, true);
   timerAttachInterrupt(timer, &redesSociales, true);

    timerAlarmWrite(timer, 3000000, true);
    timerAlarmEnable(timer);
    //pantallaOLED("", 2);
    //delay(1000);
  }
  owasp();

}

void loop() {
  char c;
  int i = 0;

  if(Serial.available() > 0) {
    c = (char)Serial.read();
    if(c != 8){
      cmd += c; 
    }

    if(c == '\n' || c == '\r'){
      cmd.trim();
      if(cmd.equals("change") && flag == 1){
        Serial.println(F("\nIngrese el nombre a visualizar en la pantalla OLED"));
        parpadeaPixel(true);
        flag = 2;
        cmd = "";
      }else if(cmd.equals("help") && flag == 1){
        help();
        parpadeaPixel(true);
        cmd = "";
      }else if(cmd.equals("owasp") && flag == 1) {
        owasp();
        parpadeaPixel(true);
        cmd = "";
      }else if(cmd.equals("twitter") && flag == 1) {
        Serial.println(F("\nIngrese su twitter:"));
        flag = 3;
        parpadeaPixel(true);
        cmd = "";
      }else if(cmd.equals("facebook") && flag == 1){ 
        Serial.println(F("\nIngrese su facebook:"));
        flag = 4;
        parpadeaPixel(true);
        cmd = "";
      }else if(cmd.length() == 0 and flag == -1){
        owasp();
        cmd = "";
        flag = 1;
      }else {
        switch (flag) {
          case 2:
            msg = String(cmd);
            redesS[0] = String("Asistente:\n"+msg);
            flag = 1;
            cmd ="";
          break;

          case 3:
            twitter = String(cmd);
            redesS[1] = String("Twitter:\n"+twitter);
            flag = 1;
            cmd = "";
          break;

          case 4:
            facebook = String(cmd);
            redesS[2] = String("Facebook:\n"+facebook);
            flag = 1;
            cmd = "";
          break;

          default:
            Serial.println("Comando no encontrado");
            parpadeaPixel(false);
            flag = 1;
            cmd = "";
          break;
        } 
      }
    }else if(c == 8 && cmd.length() > 0){
      i = cmd.length() - 1;
      cmd.setCharAt(i,' ');
      cmd.trim();
    }
  }
  pantallaOLED(2);
}

void owasp(){
  Serial.println(F("  ______   __       __   ______    ______   _______         __         ______   ________  ______   __       __ "));
  Serial.println(F(" /      \\ /  |  _  /  | /      \\  /      \\ /       \\       /  |       /      \\ /        |/      \\ /  \\     /  |"));
  Serial.println(F("/$$$$$$  |$$ | / \\ $$ |/$$$$$$  |/$$$$$$  |$$$$$$$  |      $$ |      /$$$$$$  |$$$$$$$$//$$$$$$  |$$  \\   /$$ |"));
  Serial.println(F("$$ |  $$ |$$ |/$  \\$$ |$$ |__$$ |$$ \\__$$/ $$ |__$$ |      $$ |      $$ |__$$ |   $$ |  $$ |__$$ |$$$  \\ /$$$ |"));
  Serial.println(F("$$ |  $$ |$$ /$$$  $$ |$$    $$ |$$      \\ $$    $$/       $$ |      $$    $$ |   $$ |  $$    $$ |$$$$  /$$$$ |"));
  Serial.println(F("$$ |  $$ |$$ $$/$$ $$ |$$$$$$$$ | $$$$$$  |$$$$$$$/        $$ |      $$$$$$$$ |   $$ |  $$$$$$$$ |$$ $$ $$/$$ |"));
  Serial.println(F("$$ \\__$$ |$$$$/  $$$$ |$$ |  $$ |/  \\__$$ |$$ |            $$ |_____ $$ |  $$ |   $$ |  $$ |  $$ |$$ |$$$/ $$ |"));
  Serial.println(F("$$    $$/ $$$/    $$$ |$$ |  $$ |$$    $$/ $$ |            $$       |$$ |  $$ |   $$ |  $$ |  $$ |$$ | $/  $$ |"));
  Serial.println(F(" $$$$$$/  $$/      $$/ $$/   $$/  $$$$$$/  $$/             $$$$$$$$/ $$/   $$/    $$/   $$/   $$/ $$/      $$/ "));

  Serial.println(F("\nhelp - Muestra la lista de comandos"));
                                                                                                               
}

void pantallaOLED(int sizef){
  display.clearDisplay();
  display.setTextSize(sizef);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  if(txt.length() == 0){
    display.println("Asistente:\n0xFF");
  }else{
    display.println(txt); 
  }
  display.display();
  delay(10);
}

void IRAM_ATTR redesSociales(){
  if(redesS[ind].length() > 0 && ind < 3){
    txt = String(redesS[ind]);
  }

  if(ind > 2){
    ind = 0;
  }else{
    ind++;
  }
}
 
void help(){
  Serial.println("\nhelp - Muestra los comandos a usuar en el badge");
  Serial.println("\tchange\t\t-\tCambiar nombre en el badge");
  Serial.println("\ttwitter\t\t-\tAgrega twitter a mostrar en el badge");
  Serial.println("\tfacebook\t-\tAgrega facebook a mostrar en el badge");
  Serial.println("\towasp\t\t-\tMuestra el nombre del evento");
}

void parpadeaPixel(boolean adv){
  for(int i=0; i < 10; i++){
    pixels.clear();
    if (adv){
      pixels.setPixelColor(0, pixels.Color(0,255,0));
      pixels.show();
      delay(200);
      pixels.clear();
      pixels.setPixelColor(1, pixels.Color(0,255,0));
      pixels.show();
      delay(200); 
    }else {
      pixels.setPixelColor(0, pixels.Color(255,0,0));
      pixels.show();
      delay(200);
      pixels.clear();
      pixels.setPixelColor(1, pixels.Color(255,0,0));
      pixels.show();
      delay(200);
    }
  }
}

void modificarFecha(int hora, int minuto, int dia, int mes, int anio){
  
}
