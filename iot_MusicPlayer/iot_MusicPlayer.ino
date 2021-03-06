#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>
#include <DHT11.h>
//#include<LiquidCrystal.h>

//LiquidCrystal lcd(4,6,10,11,12,13);

int button = 9;
SdFat sd;
SFEMP3Shield MP3player;

int temp_humid_pin = 2;
DHT11 dht11(temp_humid_pin); 
float temp, humi;
int err;
String now1 = "";
String now = "";

int cds = A0;
int light;

byte command;

int startbtn=0;
void setup() {


  pinMode(button,INPUT);
  
  uint8_t result; //result code from some function as to be tested at later time.

  Serial.begin(9600);


  
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");

  //Initialize the MP3 Player Shield
  result = MP3player.begin();

  
 
  Serial.print(result);
  Serial.println(F(" 개의 음악이 있습니다."));
  Serial.println(F("===Track List==="));
  Serial.println(F("[1] Twice, cheerup"));
  Serial.println(F("[2] 10cm, 사랑은 은하수 다방에서"));
  Serial.println(F("[3] 10cm, 스토커"));
  Serial.println(F("[4] Sam Smith                     , I`m Not The Only One          "));
  Serial.println(F("[5] Dean, D"));
  Serial.println(F("[6] JustinBieber, Love Yourself"));
  Serial.println(F("[7] 헤이즈, AndJuly"));
  Serial.println(F("[8] 헤이즈, 돌아오지마"));
  Serial.println(F("[9] 태연, why"));
  Serial.println(F("========"));
  help();
}


void loop() {
  int buttonInput = digitalRead(button);
  
  if(buttonInput == LOW){
    if(startbtn == 0){
      parse_menu('w');
      delay(100);
      startbtn=1;
    }
    else{
      parse_menu('R');
      delay(100);
      startbtn=0;

    }
    
  }

  if(Serial.available()) {
    
    parse_menu(Serial.read()); // get command from serial input
    
  }
  
  
  delay(100);
}


void parse_menu(byte key_command) {

  uint8_t result; // result code from some function as to be tested at later time.

  // but do take much space if only needed temporarily, hence they are here.
  char title[30]; 
  char artist[30]; 
  char album[30]; 

  Serial.print(F("명령어를 입력해주세요 : "));
  Serial.write(key_command);
  Serial.println(F(" "));

  //if s, stop the current track
  if(key_command == 's') {
    Serial.println(F("Stopping"));
    MP3player.stopTrack();
    


  //if 1-9, play corresponding track
  }else if((key_command == '-') || (key_command == '+')) {
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t

    if(key_command == '-') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(mp3_vol.byte[1] >= 254) { // range check
        mp3_vol.byte[1] = 254;
      } else {
        mp3_vol.byte[1] += 2; // keep it simpler with whole dB's
      }
    } else {
      if(mp3_vol.byte[1] <= 2) { // range check
        mp3_vol.byte[1] = 2;
      } else {
        mp3_vol.byte[1] -= 2;
      }
    }
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    Serial.print(F("Volume changed to -"));
    Serial.print(mp3_vol.byte[1]>>1, 1);
    Serial.println(F("[dB]"));

 
  } else if(key_command >= '1' && key_command <= '9') {
    //convert ascii numbers to real numbers
    key_command = key_command - 48;

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif

    //tell the MP3 Shield to play a track
    result = MP3player.playTrack(key_command);
    
    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    } else {

      Serial.println(F("Playing:"));

      MP3player.trackTitle((char*)&title);
      MP3player.trackArtist((char*)&artist);
      MP3player.trackAlbum((char*)&album);

      //print out the arrays of track information
      Serial.write((byte*)&title, 30);
      Serial.println();
      Serial.print(F("by:  "));
      Serial.write((byte*)&artist, 30);
      Serial.println();
      Serial.print(F("Album:  "));
      Serial.write((byte*)&album, 30);
      Serial.println();

    }


  } 
   else if(key_command == 'p') {
    if( MP3player.getState() == playback) {
      MP3player.pauseMusic();


      Serial.println(F("paused"));
      
    } else if( MP3player.getState() == paused_playback) {
      MP3player.resumeMusic();

      Serial.println(F("resume"));
    } else {
      Serial.println(F("Not playing"));
    }

  } else if(key_command == 'R') {
    MP3player.stopTrack();
    MP3player.vs_init();
    Serial.println(F("Reset"));



  }  else if(key_command == 'h') {
    help();
  } else if(key_command == 'w'){
    checking();
    weather();
    parse_menu(command);
    delay(100);
  } else if(key_command == 'W'){
    checking();
    weather();
    delay(100);

  } else if(key_command == 'L'){
    lcdPrint();
  } else if(key_command == 'r'){
    random_play();
    parse_menu(random_play());
  }

}


void help() {

  Serial.println(F("Command:"));
  Serial.println(F(" [1-9] Play track"));
  Serial.println(F(" [s] Stop"));
  Serial.println(F(" [+ or -] Volume"));
  Serial.println(F(" [p] Pause/Resume"));
  Serial.println(F(" [R] Reset"));
  Serial.println(F(" [L] LCD show"));
  Serial.println(F(" [W] Weather Checking"));
  Serial.println(F(" [w] Recommend weather music"));
  Serial.println(F(" [r] RandomPlay"));
  Serial.println(F(" [h] Help"));
  
}

void checking(){
  light = analogRead(cds);
  Serial.print("Light:");
  Serial.println(light);
  if((err=dht11.read(humi,temp))==0){
    Serial.print("Temperature:");
    Serial.print(temp);
    now1+=temp;
    now1+="C";
    
    Serial.print(" Humid:");
    Serial.print(humi);
    now1+=" ";
    now1+=humi;
    now1+="%";
    Serial.println();
 
  }
  else{
    Serial.println();
    Serial.print("Error No :");
    Serial.print(err);
    Serial.println();
  }
}

void weather(){
  
   if(light < 500){
    if(temp > 25 && humi < 65 ){
      
      command=random_Create(1);
      now = "Day, Clear";
    }
    else{
      command=random_Create(3);
      now = "Day, Rainy";
    }
  }
  else{
    if(temp > 25 && humi < 65 ){
      command=random_Create(5);
      now = "Night, Clear";
    }
    else{
      command=random_Create(7);
      now = "Night, Rainy";
    }
  }
  Serial.println(now+"");
  
}

char random_Create(int a){
  
  byte rand1 = random(a,a+2);

  char result = rand1 + 48;

  return result;
}

char random_play(){
  byte rand1 = random(1,10);

  char result = rand1 + 48;

  return result;
}

void lcdPrint(){
   //checking();
   //lcd.clear();
   //lcd.setCursor(0,0);
   //lcd.print(now1);
   //lcd.setCursor(0,1);
   //lcd.print(now);
}
