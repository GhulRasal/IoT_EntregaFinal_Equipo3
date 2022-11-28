#include <esp_wpa2.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include "Arduino.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"


//Se declaran los parametros de red
const char* ssid = "Tec";
#define EAP_IDENTITY "A01734977@tec.mx"
#define EAP_PASSWORD "*****"

// Se define el correo y contraseña en caso de que se quiera autentificar de forma no anónima, se comenta ya que en esta ocasión se genera el auth anónimo
//#define USER_EMAIL "martinruizcortinez7@gmail.com"
//#define USER_PASSWORD "Kalilinuxeslamamada"

// Se define el API Key Firebase del proyecto
#define API_KEY "AIzaSyDfFVXvISpNVfK6WumN7NvVeYpEEUMf8oE"//AIzaSyAjjTHMIV0y394tayvijhU-aVVcKdkIZxU

// Se añade el RTDB URL
#define DATABASE_URL "https://hola-587b4-default-rtdb.firebaseio.com"

FirebaseData fbdo; // Se define el FirebaseData como objeto
FirebaseAuth auth; // Se define el FirebaseAuth como autenticacion
FirebaseConfig config; // Se define el FirebaseConfig como configuracion

#define DHTPIN 4     // pin digital conectado al sensor DHT
#define DHTTYPE DHT11   // sensor DHT 11

DHT dht(DHTPIN, DHTTYPE); //se crea variable DHT

unsigned long sendDataPrevMillis = 0; //se crea variable de tiempo de 32 bits (4 bytes)
bool signupOK = false; //se crea variable booleana
int MQ5; //Se define la variable de concentración de gas
int MQ5_PIN = 34; //Se declara el pin del sensor MQ5
int Trigger = 27; //Se define el pin del Trigger del sensor ultrasónico
int Echo = 26; // Se define el pin del Echo del sensor ultrasónico
int LDR; //Se define la variable de intensidad de luz
int LDR_PIN = 35; //Se declara el pin del sensor MQ5
int tiempo; // Variable del tiempo para calcular distancia
int d;  //Se define la variable de distancia
String stringValue; //Valor string para guardar el número recibido de la base de datos
int LEDs[] = {22,23,1,3,19,18,5};    // pines del Display para ESP32 

//se declaran los arreglos que forman los dígitos en el display
int zero[] = {0, 1, 1, 1, 1, 1, 1};   // cero
int one[] = {0, 0, 0, 0, 1, 1, 0};   // uno
int two[] = {1, 0, 1, 1, 0, 1, 1};   // dos
int three[] = {1, 0, 0, 1, 1, 1, 1};   // tres
int four[] = {1, 1, 0, 0, 1, 1, 0};   // cuatro 
int five[] = {1, 1, 0, 1, 1, 0, 1};   // cinco
int six[] = {1, 1, 1, 1, 1, 0, 1};   // seis
int seven[] = {1, 0, 0, 0, 1, 1, 1};   // siete
int eight[] = {1, 1, 1, 1, 1, 1, 1}; // ocho
int nine[] = {1, 1, 0, 1, 1, 1, 1};   // nueve
int mayornueve[] = {1, 0, 0, 0, 0, 0, 0};   // mayor a nueve

void setup() {
    Serial.begin(115200); //Se inicia la comunicación serial 
    dht.begin();  //Inicializamos el sensor DHT11
    pinMode(MQ5, INPUT); // Se definen los pines como entradas o salidas (entrada)
    pinMode(Trigger, OUTPUT); //Salida
    pinMode(Echo, INPUT);   //Entrada
    pinMode(LDR, INPUT);    // Entrada
    for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT); //Pines del display Salidas
    delay(10);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
// Inicia la conexión a la red WPA2
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);   //inicia el modo Wifi
  //esp_wifi_set_mac(ESP_IF_WIFI_STA, &masterCustomMac[0]);
  Serial.print("MAC >> ");
  Serial.println(WiFi.macAddress());
  Serial.printf("Connecting to WiFi: %s ", ssid); //se muestra en el monitor serial que inicia el proceso de conexion
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
    // WPA2 enterprise magic ends here

    WiFi.begin(ssid); //Inicializa la configuración de la biblioteca WiFi y proporciona el estado actual
    //Ciclo que intenta conectar con una red Wifi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); //Imprime la direccion IP del WiFi

  config.api_key = API_KEY; //Se configura la API KEY requerida

  config.database_url = DATABASE_URL; //Se configura la RTDB URL de la base de datos requerida

  //Verifica el registro del usuario anónimo
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Asigna la función de devolución de llamada para la tarea de generación de tokens de ejecución prolongada 
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth); //Inicializa la librería Firebase_ESP_Client con la configuración y autenticación de Firebase
  Firebase.reconnectWiFi(true); //Se reconecta la Base de Datos automáticamente al WiFi
}

//Inicia el ciclo
void loop() 
{
  
  float h = dht.readHumidity(); // Lee la humedad y se le asigna la variable h
  float t = dht.readTemperature(); // Lee la temperatura en Celsius y se le asigna la variable t
  float f = dht.readTemperature(true); // Lee la temperatura en Fahrenheit y se le asigna la variable f (true)

  MQ5 = analogRead(MQ5_PIN); //Lee el valor analógico del sensor MQ5 y lo asigna en la variable MQ5
  LDR = analogRead(LDR_PIN); //Lee el valor analógico de la fotoresistencia y lo asigna en la variable LDR

  // Método que inicia la secuencia del Trigger para comenzar a medir
  digitalWrite(Trigger, LOW); // Ponemos el Trigger en estado bajo
  delayMicroseconds(2); //y esperamos 2 microsegundos para generar un pulso limpio
  digitalWrite(Trigger, HIGH); // Ponemos el pin Trigger a estado alto
  delayMicroseconds(10); //esperamos 10 micro segundos para que general el disparo del Trigger
  digitalWrite(Trigger, LOW); //Volvemos a apagar el Trigger
  tiempo = pulseIn(Echo, HIGH); //La función pulseIn obtiene el tiempo entre pulsos, en microsegundos
  d = tiempo/58; //Obtenemos la distancia en cm
  
  //Se imprimen en el monitor serial las variables medidas por los sensores
  Serial.println(" ");
  Serial.print(F("Humidity: "));
  Serial.println(h);
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.print(F("Farenheit: "));
  Serial.print(f);
  Serial.println(F("F "));
  Serial.print(F("Concentracion de gas: "));
  Serial.print(MQ5);
  Serial.println(F("PPM "));
  Serial.print(F("Distancia: "));
  Serial.print(d);
  Serial.println(F("cm"));
  Serial.print (F("Intensidad de luz: "));
  Serial.print(LDR,DEC); // light intensity
  Serial.print(F("%"));
  Serial.println(" ");
  
  //Se verifica si algunas de las mediciones ha fallado
    if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

   if (isnan (MQ5)) {
    Serial.println(F("Failed to read from MQ5 sensor!"));
    return;
  }
  
  if (isnan (d)) {
    Serial.println(F("Failed to read from HC-sr04 sensor!"));
    return;
  }

 if (isnan (LDR)) {
    Serial.println(F("Failed to read from LDR sensor!"));
    return;
  }

// Se verifica la comunicación con la base de datos para poder mandar los datos medidos
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) { //verifica el tiempo de enviar una nueva lectura de datos
    sendDataPrevMillis = millis();
//Sensores
  if (Firebase.RTDB.setFloat(&fbdo, "app_inventor/Humedad", h)){
      Serial.println("PASSED H"); // Se imprime "PASSED H" en el Monitor serial para saber si se ha mandado a la base de datos
    }
  if (Firebase.RTDB.setFloat(&fbdo, "app_inventor/Celsius", t)){
      Serial.println("PASSED T"); // Se imprime "PASSED T" en el MS para saber si se ha mandado a la BD
    }
  if (Firebase.RTDB.setFloat(&fbdo, "app_inventor/Farenheit", f)){
      Serial.println("PASSED F"); // Se imprime "PASSED F" en el MS para saber si se ha mandado a la BD
    }
  if (Firebase.RTDB.setInt(&fbdo, "app_inventor/Gas", MQ5)){
      Serial.println("PASSED MQ5"); // Se imprime "PASSED MQ5" en el MS para saber si se ha mandado a la BD
    }
  if (Firebase.RTDB.setInt(&fbdo, "app_inventor/Distancia", d)){
      Serial.println("PASSED d"); // Se imprime "PASSED d" en el MS para saber si se ha mandado a la BD
    }
  if (Firebase.RTDB.setInt(&fbdo, "app_inventor/Luz", LDR)){
      Serial.println("PASSED LDR"); // Se imprime "PASSED LDR" en el MS para saber si se ha mandado a la BD 
    }
//Display
  if (Firebase.RTDB.getString(&fbdo, "app_inventor/Numero")){ //Se recibe el dato"app_inventor/Numero" de la BD
        if (fbdo.dataType() == "string") //Si el dato es de tipo string se cumple la condicion
        {
          stringValue = fbdo.stringData(); //Se convierte el dato a entero y se guarda en una variable
          int stringVal = stringValue.toInt();
          //Se comparan los valores para mostrarlos en el display
        if (stringVal==0){ //Si el dato es igual a 0
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], zero[i]);} //Se manda el arreglo que forma el 0 en el display
        if (stringVal==1){ //Si el dato es igual a 1
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], one[i]);} //Se manda el arreglo que forma el 1 en el display
        if (stringVal==2){ //Si el dato es igual a 2
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], two[i]);} //Se manda el arreglo que forma el 2 en el display
        if (stringVal==3){ //Si el dato es igual a 3
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], three[i]);} //Se manda el arreglo que forma el 3 en el display
        if (stringVal==4){ //Si el dato es igual a 4
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], four[i]);} //Se manda el arreglo que forma el 4 en el display
        if (stringVal==5){ //Si el dato es igual a 5
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], five[i]);} //Se manda el arreglo que forma el 5 en el display
        if (stringVal==6){ //Si el dato es igual a 6
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], six[i]);} //Se manda el arreglo que forma el 6 en el display
        if (stringVal==7){ //Si el dato es igual a 7
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], seven[i]);} //Se manda el arreglo que forma el 7 en el display
        if (stringVal==8){ //Si el dato es igual a 8
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], eight[i]);} //Se manda el arreglo que forma el 8 en el display
        if (stringVal==9){ //Si el dato es igual a 9
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], nine[i]);} //Se manda el arreglo que forma el 9 en el display
        if (stringVal>9){ //Si el dato es mayor a 9
            for (int i = 0; i<7; i++) digitalWrite(LEDs[i], mayornueve[i]);} //Se manda el arreglo que forma un "-" en el display
        }
     }
else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  delay(2000); //esperamos 2 segundos antes de que inicie el ciclo nuevamente
}
