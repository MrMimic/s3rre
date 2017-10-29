/*
Plugs:

D22: LCD (RS)
D24: LCD (E)
D26: LCD (D4)
D28: LCD (D5)
D30: LCD (D6)
D32: LCD (D7)
D34: DHT22
D36: RELAY (IN1) EV1
D38: RELAY (IN2) EV2
D40: RELAY (IN3) Cooler
D42: RELAY (IN4) Ionizer
D50: SD (MISO)
D51: SD (MOSI)
D52: SD (SCK)
D53: SD (CS)

A0: Humidity sensor 1
A1: Humidity sensor 2
A2: 
A3:
A4:
A5:
*/

// Librairies
#include "DHT.h" // librairie DHT22
DHT dht(34, DHT22); // Pin digitale 28, type de capteur DHT22
#include "LiquidCrystal.h" // librairie LCD
LiquidCrystal lcd(22, 24, 26, 28, 30, 32); // lcd(RS, E, D4, D5, D6, D7)
#include "math.h"
#include "SD.h" // librairie SD reader
Sd2Card card; SdVolume volume; SdFile root; 

// Entrees analogiques
float waterSensor1 = A0; // eau 1
float waterSensor2 = A1; // eau 2

// Sorties numeriques
int ev1 = 36; // Electrovanne 1 (Relai)
int ev2 = 38; // Electrovanne 2 (Relai)
int ventil = 40; // Ventilateur
int hygro = 42; // Ionisateur

// Arrosage
boolean evState1 = LOW;
boolean evState2 = LOW;
int waterSensorValue1 = 0;  
int waterSensorValue2 = 0;

// Hygrometrie / Temperature / Ventil / hygro
boolean hygroState = LOW;
boolean ventilState = HIGH;
float hygrometrie = 0;
float temperature = 0;

// Variables globales
int mesureCount = 1;
const int chipSelect = 53;    

void setup(){
  
  Serial.begin(9600); 
  lcd.begin(20, 4); // 20 colonnes, 4 lignes d'affichage LCD
  dht.begin();
  pinMode(ev1, OUTPUT);
  pinMode(ev2, OUTPUT);
  pinMode(hygro, OUTPUT);
  pinMode(ventil, OUTPUT);
  pinMode(53, OUTPUT); // Sur arduino MEGA, le pin 53 doit être défini comme sortie
  
  Serial.print("\nInitialisation de la carte SD ..."); // Test de présence de la carte SD
  if (!card.init(SPI_HALF_SPEED, chipSelect)){
    Serial.println("Echec d'initialisation.");
    return; } 
  else{
    Serial.println("Initialisation éffectuée."); }

  Serial.print("\nType de carte :"); // Type de carte SD
  switch(card.type()){
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown"); }

  if (!volume.init(card)){
    Serial.println("Partition FAT16/32 absente.\nFormatage de la carte requis.");
    return; }

  uint32_t volumesize; // print the type and size of the first FAT-type volume
  Serial.print("\nCarte SD de type FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
 
  volumesize = volume.blocksPerCluster();
  volumesize *= volume.clusterCount();
  volumesize *= 512;
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1048576;
  Serial.println(volumesize);

 
  Serial.println("\nFichiers sur la carte (name, date and size): ");
  root.openRoot(volume);
  root.ls(LS_R | LS_DATE | LS_SIZE);

}  //Fin du setup

void loop(){
  
  // Lecture arrosage 
  waterSensorValue1 = analogRead(waterSensor1);
  waterSensorValue2 = analogRead(waterSensor2);
    
  // Lecture hygrometrie /température
  hygrometrie = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Lecture pH
  //phSensorValue = analogRead(phSensor);
  //ph = map(phSensorValue, 0, 1024, 0, 14);
  
  // Test de valeurs
  if(isnan(waterSensorValue1) || isnan(waterSensorValue2) || 
  isnan(hygrometrie) || isnan(temperature)) {
    lcd.clear();
    lcd.setCursor(2, 0); // Ligne 2
    lcd.print("PROBLEM ENCOUNTED");
    lcd.setCursor(4, 1); // Ligne 2
    lcd.print("- - - - - - -");  
    if(isnan(waterSensorValue1) || isnan(waterSensorValue2)) {  
      lcd.setCursor(1, 2); // Ligne 3
      lcd.print("Failed to read from");
      lcd.setCursor(3, 3); // Ligne 3
      lcd.print("ground sensor!"); }
    if(isnan(hygrometrie) || isnan(temperature)) {
      lcd.setCursor(1, 2); // Ligne 3
      lcd.print("Failed to read from");
      lcd.setCursor(5, 3); // Ligne 3
      lcd.print("DHT sensor!"); }
    delay(10000); } //Temps entre deux mesures
  else{
   
    
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - ARROSAGE - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    
    if((waterSensorValue1 < 500) && (evState1 == LOW)){ // Valeur si inférieure à && électrovanne ouverte --> on ferme
      Serial.print(waterSensorValue1); Serial.print(" "); 
      evState1 = HIGH;   
      digitalWrite(ev1, evState1); }
    else{
      Serial.print(waterSensorValue1); Serial.print(" "); }
    
    if((waterSensorValue2 < 500) && (evState2 == LOW)){ 
      Serial.print(waterSensorValue2); Serial.print(" "); 
      evState2 = HIGH;   
      digitalWrite(ev2, evState2); }
    else{
      Serial.print(waterSensorValue2); Serial.print(" "); }
  
  
    if((waterSensorValue1 > 700) && (evState1 == HIGH)){ // Valeur si supérieure à && électrovanne fermée --> on ouvre
      evState1 = LOW;
      digitalWrite(ev1, evState1); }
      
    if((waterSensorValue2 > 700) && (evState2 == HIGH)){
      evState2 = LOW;
      digitalWrite(ev2, evState2); }
      
    
    if((waterSensorValue1 > 500) && (waterSensorValue1 < 700)){ // Si entre les deux valeurs, on ferme de toute façon
      evState1 = HIGH;
      digitalWrite(ev1, evState1); }
      
    if((waterSensorValue2 > 500) && (waterSensorValue2 < 700)){
      evState2 = HIGH;
      digitalWrite(ev2, evState2); }
    
  
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - HYGROMETRIE - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  
    if((hygrometrie < 70) && (hygroState == LOW)){ // Seuil hygrometrie minimum
      Serial.print(hygrometrie); Serial.print(" ");
      hygroState = HIGH;
      digitalWrite(hygro, hygroState); }
    else if((hygrometrie > 95) && (ventilState == LOW)){ // Seuil hygrometrie max
      Serial.print(hygrometrie); Serial.print(" ");
      ventilState = HIGH;
      digitalWrite(ventil, ventilState); }
    else{
      Serial.print(hygrometrie); Serial.print(" "); }
      
      
    if((hygrometrie > 80) && (hygroState == HIGH)){
      hygroState == LOW;
      digitalWrite(hygro, hygroState); }
    else if((hygrometrie < 87) && (ventilState == HIGH)){
      ventilState == LOW;
      digitalWrite(ventil, ventilState); }
    
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - TEMPERATURE - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Vérif cooler + relais faite, branchement sur bornes normalement fermées, 
  
    if((temperature > 22) && (ventilState == HIGH)){ // Valeur donnee pour un seuil de 27°C
      Serial.print(temperature); Serial.println(" ");
      ventilState = LOW;   
      digitalWrite(ventil, ventilState); }
    else if((temperature < 22) && (ventilState == LOW)){
      ventilState = HIGH;   
      digitalWrite(ventil, ventilState); }
    else{
      Serial.print(temperature); Serial.println(" "); }
      
      
    if((temperature > 25) && (ventilState == LOW)){
      ventilState = HIGH;
      digitalWrite(ventil, ventilState); }

  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// - - - - - - - - - - - - AFFICHAGE LCD - - - - - - - - - - - - - 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
  
    mesureCount ++;
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("RECORD") ;
    lcd.setCursor(1, 2);
    lcd.print(mesureCount);
    lcd.setCursor(9, 0);
    lcd.print("S1: "); lcd.print(waterSensorValue1);
    lcd.setCursor(9, 1);
    lcd.print("S2: "); lcd.print(waterSensorValue2);
    lcd.setCursor(9, 2);
    lcd.print("Hy: "); lcd.print(hygrometrie); lcd.print("%");
    lcd.setCursor(9, 3);
    lcd.print("Tc: "); lcd.print(temperature); lcd.print("C");

    
    delay(1000); // Delai entre deux mesures

  } 
} // Fin du loop


  /*
   - - - - - - - - - Capteurs d'humidite du sol - - - - - - - - - 
   resistance en pont variable : +5v, GND, A0
   Electrovanne : GND, D13
   Relais normalement ouvert, ferme quand +5V
   Ferme un circuit 12V commandant l'EV
   
   Fonctionne avec la librairie LiquidCrystal vous aurez besoin de 6 sorties de l'arduino, vous trouverez facilement le schéma de branchement en faisant une recherche du nom du contrôleur HD44780.
   N'oubliez pas le potentiomètre de contraste, sinon vous risquez de croire que votre écran ne fonctionne pas. 
   
humidity : 0-100%RH;
temperature : -40~125 Celsius
  */
