#include <EEPROM.h>
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

#define EEPROM_MEM 34

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

// String nombre = "";
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
String redesS[] = {"","",""}; //Inicialización del arreglo de redes sociales

// Temporizador por hardware
hw_timer_t *timer = NULL;
// Asegurar la variable que se este compartiendo entre el
// loop principal y el ISR (Interrupt Service Routine)
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int ind = 0;

// Función a llamar cuando ocurra una interrupción de tiempo.
// ISR
void IRAM_ATTR redesSociales(void);

void setup() {
  char a, t, f; // a = Asistente, t = Twitter y f = Facebook
  // Inicializamos pixeles
  pixels.begin();
  pixels.clear();

  // Inicializamos serial
  Serial.begin(115200);
  cmd.reserve(10);
  twitter.reserve(10);
  facebook.reserve(10);
  msg.reserve(10);

  // Inicializando pantalla OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("Se requieere pantalla OLED"));
  }else {
    // Se inicia el temporizador
    timer = timerBegin(0, 80, true);
    // Se asigna la función ISR a llamar cuando se genere
    // una interrupción por el timer
    timerAttachInterrupt(timer, &redesSociales, true);
    
    // Cada 3seg. aproximadamente se escribe en el
    // timer.
    timerAlarmWrite(timer, 3000000, true);
    timerAlarmEnable(timer);

  }

  if(!EEPROM.begin(EEPROM_MEM)){
    txt="Error!!";
  }else {
    a = byte(EEPROM.read(0));
    t = byte(EEPROM.read(10));
    f = byte(EEPROM.read(20));
    if ((isAscii(a) && a != ' ') || (isAscii(t) && t != ' ') || (isAscii(f) && f != ' ')) { //Valida si hay datos en la memoria
      leerDatos();
      Serial.println("Se obtuvieron los datos satisfactorialmente");
    }else {
      Serial.println("No hay datos almacenados.\nReiniciando la memoria");
      inicializarMemoria();
      Serial.println("Memoria limpia");
    }
  }
  
  // Carga la imagen de inicio.
  owasp();

}

void loop() {
  char c;
  int i = 0;

  if(Serial.available() > 0) {
    c = (char)Serial.read();
    // Si no se ha presionado la tecla backspace, se sigue concateneando
    // los caracteres.
    if(c != 8 ){
      cmd += c; 
    }

    // Compatibilidad entre sistemas de comunicación serial
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
            
            // Guarda el nombre en el nombre del asistente en la dirección 0x00
            salvarDatos(msg, 0);
            
            redesS[0] = String("Asistente:\n"+msg);
            flag = 1;
            cmd ="";
          break;

          case 3:
            twitter = String(cmd);

            //Guarda twitter en la dirección 0x0A
            salvarDatos(twitter, 10);
            
            redesS[1] = String("Twitter:\n"+twitter);
            flag = 1;
            cmd = "";
          break;

          case 4:
            facebook = String(cmd);

            //Guarda facebook en la dirección 0x14
            salvarDatos(facebook, 20);
            
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

void salvarDatos(String datos, int direccion){
  char tmp;
  int indx = direccion;
  
  for(int i = 0; i < datos.length(); i++){
    tmp = datos[i];
    EEPROM.write((indx + i), (int)tmp);
    delay(10);
  }
  
  indx += datos.length();
  tmp = '\n';
  EEPROM.write(indx, (int)tmp);
  EEPROM.commit();
  
  Serial.println("Datos guardados");
}

void leerDatos(){
  char c = ' ';
  int pos, direccion;
  String tmp;

  pos = direccion = 0;
  for(int i = 0; i < EEPROM_MEM; i++){
    c = byte(EEPROM.read(i));
    if(c != '\n' && c != ' '){
      tmp += c;
    }else {
      switch(pos){
        // Asistente
        case 0:
          tmp.trim();
          if (tmp.length() > 0){
            redesS[pos] = String("Asistente:\n"+tmp);  
          }
        break;

        //Twitter
        case 1:
          tmp.trim();
          if(tmp.length() > 0){
            redesS[pos] = String("Twitter:\n"+tmp); 
          }
        break;

        //Facebook
        case 2:
          tmp.trim();
          if(tmp.length() > 0) {
            redesS[pos] = String("Facebook:\n"+tmp); 
          }
        break;
      }
      tmp = "";
      pos++;
      direccion += 10;
      i = direccion-1;
    }
  }
}

void inicializarMemoria(){
  char c = ' ';
  
  for(int i = 0; i < EEPROM_MEM; i++){
    EEPROM.write(i,(int)c);
    delay(10); 
  }
  EEPROM.commit(); 
}
