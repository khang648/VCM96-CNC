#include <SPI.h>
#include <Wire.h> 
#include <RA8875.h> 
//#include <TimerOne.h>
#include <avr/interrupt.h>
  
#define ENABLE_PIN    53 
#define Z_DIR_PIN     51
#define Y_DIR_PIN     49
#define X_DIR_PIN     47
#define Z_STEP_PIN    45
#define Y_STEP_PIN    43
#define X_STEP_PIN    41
#define X_SW_PIN      39 
#define Y_SW_PIN      37
#define PUMP1_PIN     35
#define PUMP2_PIN     33
#define PUMP3_PIN     31
#define PUMP4_PIN     29
#define PRESSURE_PIN  27
#define RA8875_INT    4
#define RA8875_CS     10 
#define RA8875_RESET  9
#define time_pump 3000
#define time_after_pump 2000

uint16_t tx, ty;
char u[20], u1[20], u2[20];
int key_old=0, key=96, key1=0, key2=0, key_old_t=0, key_t=2, key1_t=0, key2_t=0;
int pause_click=0;
int run_value=0, home_value=0;
int poker = 0;
int x_limit=0,y_limit=0;
int column_top=0, column_bottom=0;
int n1=0, n2=0;
uint32_t count=0;
uint32_t firstmove_step;

RA8875 tft = RA8875(RA8875_CS,RA8875_RESET);

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  tft.begin(RA8875_480x272);
  tft.touchBegin(RA8875_INT); 
  tft.fillScreen(RA8875_WHITE);
  tft.setFontScale(2);
  tft.setTextColor(RA8875_RED,RA8875_WHITE); 
  tft.setCursor(83,60);  tft.println("PHUSA BIOCHEM");
  tft.setFontScale(1);
  tft.setTextColor(RA8875_BLUE,RA8875_WHITE); 
  tft.setCursor(162,180);  tft.println("LOADING...");
  home_page();

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN,OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(X_SW_PIN,INPUT_PULLUP);
  pinMode(Y_SW_PIN,INPUT_PULLUP);
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  pinMode(PUMP3_PIN, OUTPUT);
  pinMode(PUMP4_PIN, OUTPUT);
  pinMode(PRESSURE_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, HIGH);

  //Timer1.initialize(100);
  //Timer1.attachInterrupt(Touch);

  cli();     
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;  
  TCCR1B |= (1 << CS11) | (1 << CS10) ;   
  TCNT1 = 40536;
  TIMSK1 = (1 << TOIE1);
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;      
  TCCR2B |= (1 << CS21) | (1 << CS20) | (1 << CS22) ;   
  TCNT2 = 100;
  TIMSK2 = (1 << TOIE2);    
  sei();                                  
}

ISR (TIMER1_OVF_vect) 
{
  n1++;
  if(n1==10)
  {
    count++;
    n1=0;
  }    
  TCNT1 = 40536;
}

ISR (TIMER2_OVF_vect) 
{
  n2++;
  if(n2==10)
  {
    n2=0;
    Touch(); 
  }
  TCNT2 = 100;
}

void(* resetFunc) (void) = 0;

void home_page()
{
  tft.fillScreen(RA8875_WHITE);
  tft.setFontScale(1);
  //tft.fillRect(0,240,95,35,RA8875_GREEN);
  tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
  tft.setCursor(0,238);  tft.println("CREATE");
  //tft.fillRect(195,240,111,31,RA8875_GREEN);
  tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
  tft.setCursor(195,238);   tft.println("HISTORY");
  //tft.fillRect(400,240,79,31,RA8875_GREEN);
  tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
  tft.setCursor(400,238);   tft.println("ADMIN");
  tft.fillRect(0,0,480,235,RA8875_BLACK);
  tft.fillTriangle(330,65,400,65,365,15,RA8875_BLUE);
  tft.fillTriangle(415,80,415,150,465,115,RA8875_BLUE);
  tft.fillTriangle(330,165,400,165,365,215,RA8875_BLUE);
  tft.fillTriangle(315,80,315,150,265,115,RA8875_BLUE);
  tft.setFontScale(0.5);
  tft.setTextColor(RA8875_GREEN,RA8875_BLUE);
  tft.setCursor(357,38);   tft.println("Y+");
  tft.setCursor(426,106);   tft.println("X+");
  tft.setCursor(357,177);   tft.println("Y-");
  tft.setCursor(289,106);   tft.println("X-");
  tft.fillCircle(48,50,35,RA8875_GREEN);
  tft.drawCircle(48,50,40,RA8875_GREEN);
  tft.setFontScale(2.5);
  
//  tft.fillRect(330,25,50,40,RA8875_GREEN);
//  tft.fillRect(405,90,40,50,RA8875_GREEN);
//  tft.fillRect(330,165,50,40,RA8875_GREEN);
//  tft.fillRect(265,90,40,50,RA8875_GREEN);
  tft.setTextColor(RA8875_WHITE,RA8875_GREEN);
  tft.setCursor(36,25);   tft.println("H");
  tft.setFontScale(0.5);
  tft.setTextColor(RA8875_WHITE,RA8875_BLACK);
  tft.setCursor(10,105);   tft.println("Time(s)");
  tft.fillRect(10,123,78,31,RA8875_BLUE);
  tft.setFontScale(1);
  tft.setTextColor(RA8875_RED,RA8875_WHITE);
  tft.setCursor(10,123);    sprintf(u1,"%5.0d",key_t);    tft.print(u1);
  tft.fillRect(150,22,50,50,RA8875_BLUE);
  tft.fillRect(150,95,50,50,RA8875_BLUE);
  tft.fillRect(150,168,50,50,RA8875_BLUE);
  tft.setTextColor(RA8875_WHITE,RA8875_BLUE);
  tft.setCursor(160,30);   tft.println("P1");
  tft.setCursor(160,103);   tft.println("P2");
  tft.setCursor(160,176);   tft.println("P3");
}

void home_return()
{ 
  digitalWrite(X_DIR_PIN,LOW);
  digitalWrite(Y_DIR_PIN,LOW);
  home_value=1;
  digitalWrite(ENABLE_PIN,LOW);
  Serial.println("I'm comming HOME !!!");
  
  while(x_limit==0 || y_limit==0)
  {
    if(x_limit==0)  digitalWrite(X_STEP_PIN,HIGH); 
    if(y_limit==0)  digitalWrite(Y_STEP_PIN,HIGH);
    delayMicroseconds(40);
    if(x_limit==0)  digitalWrite(X_STEP_PIN,LOW); 
    if(y_limit==0)  digitalWrite(Y_STEP_PIN,LOW);
    delayMicroseconds(10);
    
    if(digitalRead(Y_SW_PIN)==LOW)
    {
      delay(500);
      digitalWrite(Y_DIR_PIN,HIGH);
      for(int k=0;k<=3600;k++)
      {
        digitalWrite(Y_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(Y_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      y_limit = 1;
    }

    if(digitalRead(X_SW_PIN)==LOW)
    { 
      delay(500);
      digitalWrite(X_DIR_PIN,HIGH);
      for(int f=0;f<=3600;f++)
      {
        digitalWrite(X_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      x_limit = 1; 
    }
  }
  digitalWrite(ENABLE_PIN, HIGH);
  home_value = 0;
  x_limit = 0;
  y_limit = 0;
}

void loop()
{
  if(tft.touchDetect())
  {  
    delay(500);
    tft.touchReadPixel(&tx, &ty);   
    while(tft.touchDetect()); 
    tx=tx;
    ty=ty;
//  tft.fillRect(330,25,50,40,RA8875_GREEN);
//  tft.fillRect(405,90,40,50,RA8875_GREEN);
//  tft.fillRect(330,165,50,40,RA8875_GREEN);
//  tft.fillRect(265,90,40,50,RA8875_GREEN);
    
    //Ấn HOME
    if(ty>=15 && ty<=85 && tx>=13 && tx<=83)
    {
      home_return();       
    }

    //Ấn Time
    if(ty>=123 && ty<=154 && tx>=10 && tx<=88)
    {
      drawButtons1();
      while(key1_t!=-2&&key1_t!=-1)
      {
        value1();
      }
      if(key1_t==-1)
      {
        key1_t = 54 ;
        if(key_t>180) key_t = 180;
        key_old_t = key_t;                              
      }
      if(key1_t==-2)
      {
        key1_t = 54;
        key_t = key_old_t;
      }
      home_page();
    }

    //Ấn P1
    if(tx>=150 && tx<=200 && ty>=22 && ty<=72)
    {
      digitalWrite(PUMP1_PIN, HIGH);
      //delay(key_t*1000);
      my_delay(key_t);   
      digitalWrite(PUMP1_PIN,LOW);
      tx=0;ty=0;
    }

    //Ấn P2
    if(tx>=150 && tx<=200 && ty>=95 && ty<=145)
    {
      digitalWrite(PUMP2_PIN, HIGH);
      //delay(key_t*1000);
      my_delay(key_t);
      digitalWrite(PUMP2_PIN,LOW);
      tx=0;ty=0;
    }

    //Ấn P3
    if(tx>=150 && tx<=200 && ty>=168 && ty<=218)
    {
      digitalWrite(PUMP3_PIN, HIGH);
      //delay(key_t*1000);
      my_delay(key_t);
      digitalWrite(PUMP3_PIN,LOW);
      tx=0;ty=0;
    }

    //Ấn Y+
    if(tx>=340 && tx<=390 && ty>=25 && ty<=65)
    {
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(Y_DIR_PIN,HIGH);
      for(int i=0;i<=7200;i++)
      {
        digitalWrite(Y_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(Y_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      tx=0;
      ty=0;
      digitalWrite(ENABLE_PIN, HIGH);
    }

    //Ấn X+
    if(tx>=415 && tx<=455 && ty>=90 && ty<=140)
    {
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(X_DIR_PIN,HIGH);
      for(int i=0;i<=7200;i++)
      {
        digitalWrite(X_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      tx=0;
      ty=0;
      digitalWrite(ENABLE_PIN, HIGH);
    }

    //Ấn Y-
    if(tx>=340 && tx<=390 && ty>=165 && ty<=205)
    {
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(Y_DIR_PIN,LOW);
      for(int i=0;i<=7200;i++)
      {
        digitalWrite(Y_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(Y_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      tx=0;
      ty=0;
      digitalWrite(ENABLE_PIN, HIGH);
    }

    //Ấn X-
    if(tx>=275 && tx<=315 && ty>=90 && ty<=140)
    {
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(X_DIR_PIN,LOW);
      for(int i=0;i<=7200;i++)
      {
        digitalWrite(X_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      tx=0;
      ty=0;
      digitalWrite(ENABLE_PIN, HIGH);
    }

    if(ty>=240 && ty<=270)
    {
      /*********** Ấn CREATE ***********/
      if(tx>=0 && tx<=95)
      {
        while(1)
        {        
          tft.fillScreen(RA8875_CYAN);
          tft.setTextColor(RA8875_BLACK,RA8875_CYAN);
          tft.setCursor(105,50);   tft.println("SAMPLES:");
          //tt.fillRect(255,52,111,31,RA8875_WHITE);
          tft.setTextColor(RA8875_BLACK,RA8875_WHITE);
          tft.setCursor(255,52);    sprintf(u,"%5.0d",key);    tft.print(u);      
          tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
          tft.setCursor(2,238);  tft.println("BACK");
          tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
          tft.setCursor(430,238);   tft.println("RUN");       
          
          while(1)
          {
            if(tft.touchDetect())
            {
              delay(100);
              tft.touchReadPixel(&tx, &ty);
              while(tft.touchDetect());
              tx=tx;
              ty=ty;
              //Nếu ấn nhập giá trị số mẫu
              if((ty>=52 && ty<=83) && (tx>=155 && tx<=366))
              {
                drawButtons();
                while(key1!=-2&&key1!=-1)
                {
                  value();
                }
                if(key1==-1)
                {
                  key1 = 54 ;
                  if(key>96) key = 96;
                  if(key<1) key = 1;
                  key_old = key;                
                  break;               
                }
                if(key1==-2)
                {
                  key1 = 54;
                  key = key_old;
                  break;
                }
              } 
              
              if(ty>=240 && ty<=270)
              {
                // Nếu ấn BACK
                if(tx>=2 && tx<=65)
                {
                  resetFunc();         
                }
                // Nếu ấn RUN
                if(tx>=430 && tx<=495)
                {
                  run_value = 1;
                  tft.fillScreen(RA8875_WHITE);                
                  tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
                  tft.setCursor(2,238);  tft.println("PAUSE");   
                  //tft.fillRect(2,210,79,31,RA8875_RED);      
                  tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
                  tft.setCursor(414,238);   tft.println("STOP");
                  //tft.fillRect(414,210,63,31,RA8875_RED);

                  if(key_old%4 != 0)  poker =  int(key_old/4)+1;
                  else                poker=key_old/4;
                    
                  home_return();  
                  //Activation buffer
                  firstmove_step = 57600;
                  run(PUMP1_PIN);                 
                  //Ủ 30s, thấp 30s, cao 30s
                  digitalWrite(PUMP4_PIN,LOW);
                  my_delay(30);
                  digitalWrite(PUMP4_PIN,HIGH);
                  digitalWrite(PRESSURE_PIN,HIGH);
                  my_delay(30);
                  digitalWrite(PRESSURE_PIN,LOW);
                  my_delay(30);
                  digitalWrite(PUMP4_PIN,LOW);
                  //Washing 2 buffer
                  firstmove_step += 28800;
                  run(PUMP2_PIN);
                  //Ủ 15s, thấp 30s, cao 60s
                  digitalWrite(PUMP4_PIN,LOW);
                  my_delay(15);
                  digitalWrite(PUMP4_PIN,HIGH);
                  digitalWrite(PRESSURE_PIN,HIGH);
                  my_delay(30);
                  digitalWrite(PRESSURE_PIN,LOW);
                  my_delay(60);
                  digitalWrite(PUMP4_PIN,LOW);
                  //Mẫu 
                  tft.fillRect(77,36,320,160,RA8875_BLUE);
                  tft.setFontScale(1);
                  tft.setTextColor(RA8875_WHITE,RA8875_BLUE);
                  tft.setCursor(117,42);   tft.println("CHO MAU VAO COT");
                  tft.setFontScale(0.8);
                  tft.setCursor(125,100);   tft.println("AN \"CONTINUE\" KHI HOAN THANH");
                  tft.setTextColor(RA8875_BLACK,RA8875_YELLOW);
                  tft.setFontScale(1);
                  tft.setCursor(175,165);   tft.println("CONTINUE");
                  //tft.fillRect(175,164,127,32,RA8875_GREEN);
                  while(1)
                  {
                    if(tft.touchDetect())
                    {
                      delay(100);
                      tft.touchReadPixel(&tx, &ty);
                      while(tft.touchDetect());
                      tx=tx;ty=ty;
                      if(ty>=164 && ty<=196 && tx>=175 && tx<=302)
                      {  
                        tft.fillScreen(RA8875_WHITE);                
                        tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
                        tft.setCursor(2,238);  tft.println("PAUSE");   
                        tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
                        tft.setCursor(414,238);   tft.println("STOP");
                        break;
                      }
                    }    
                  }

                  //Ủ 60s, thấp 15s, cao 30s
                  digitalWrite(PUMP4_PIN,LOW);
                  my_delay(60);
                  digitalWrite(PUMP4_PIN,HIGH);
                  digitalWrite(PRESSURE_PIN,HIGH);
                  my_delay(15);
                  digitalWrite(PRESSURE_PIN,LOW);
                  my_delay(30);
                  digitalWrite(PUMP4_PIN,LOW);
                  //Washing 2 buffer 
                  run(PUMP2_PIN);
                  //thấp 30s, cao 45s
                  digitalWrite(PUMP4_PIN,HIGH);
                  digitalWrite(PRESSURE_PIN,HIGH);
                  my_delay(30);
                  digitalWrite(PRESSURE_PIN,LOW);
                  my_delay(45);
                  digitalWrite(PUMP4_PIN,LOW);
                  //Elution buffer
                  firstmove_step += 28800;
                  run(PUMP3_PIN);
                  firstmove_step = 0;
                  //Ủ 90s, thấp 15s, cao 60s
                  digitalWrite(PUMP4_PIN,LOW);
                  my_delay(90);
                  digitalWrite(PUMP4_PIN,HIGH);
                  digitalWrite(PRESSURE_PIN,HIGH);
                  my_delay(15);
                  digitalWrite(PRESSURE_PIN,LOW);
                  my_delay(60);
                  digitalWrite(PUMP4_PIN,LOW);
                  
                  
                  tft.fillRect(77,36,320,160,RA8875_BLUE);
                  tft.setFontScale(1);
                  tft.setTextColor(RA8875_WHITE,RA8875_BLUE);
                  tft.setCursor(135,42);   tft.println("DA HOAN THANH");
                  tft.setFontScale(0.8);
                  tft.setCursor(160,100);   tft.println("AN \"OK\" DE QUAY LAI");
                  tft.setFontScale(1);
                  tft.fillRect(172,164,127,32,RA8875_YELLOW);
                  tft.setTextColor(RA8875_BLACK,RA8875_YELLOW);
                  tft.setCursor(222,165);   tft.println("OK");
                  while(1)
                  {
                    if(tft.touchDetect())
                    {
                      delay(100);
                      tft.touchReadPixel(&tx, &ty);
                      while(tft.touchDetect());
                      tx=tx;ty=ty;
                      if(ty>=164 && ty<=196 && tx>=175 && tx<=302)
                      {  
                        resetFunc();    
                      }
                    }    
                  } 
              
                }
              }
            }
          }    
        }
      }
      /********** Ấn HISTORY **********/
      if(tx>=195 && tx<=306)
      {
        tft.clearMemory(true);
      }
      /*********** Ấn ADMIN ***********/
      if(tx>=400 && tx<=479)
      {
        while(1)
        {
          tft.fillScreen(RA8875_GREEN);
        }
      }
    }
  }
}


void run(int PUMP_PIN)
{
  tft.fillRect(120,65,250,100,RA8875_BLUE);
  tft.setFontScale(1);
  tft.setTextColor(RA8875_WHITE,RA8875_BLUE);
  tft.setCursor(158,95);   tft.println("DANG BOM...");

  if(poker%2!=0)
  {
    Serial.println(key_old%2);
    Serial.println("poker la so le");
    column_top =  round((poker/2)+0.5); 
    column_bottom = column_top - 1;
    Serial.print("column_top: ");
    Serial.println(column_top);
    Serial.print("column_bottom: ");
    Serial.println(column_bottom);

    digitalWrite(ENABLE_PIN,LOW);
    digitalWrite(X_DIR_PIN,HIGH);
    digitalWrite(Y_DIR_PIN,HIGH);

    //Di chuyển đến cột đầu tiên    
    
    for(uint32_t i=0;i<firstmove_step;i++)
    {
      if(i<25600)
      {
        digitalWrite(X_STEP_PIN,HIGH);
        digitalWrite(Y_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        digitalWrite(Y_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      else
      {
        digitalWrite(X_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
    }
    delay(1000);
    digitalWrite(PUMP_PIN,HIGH);
    delay(time_pump);
    digitalWrite(PUMP_PIN,LOW);
    delay(time_after_pump);

    if(column_top-1 > 0)
    {
      for(int i=0;i<(column_top-1);i++)
      {
        for(uint32_t j=0;j<14400;j++)
          {
            digitalWrite(X_STEP_PIN,HIGH);
            delayMicroseconds(40);
            digitalWrite(X_STEP_PIN,LOW);
            delayMicroseconds(40);
          }
        delay(1000);
        digitalWrite(PUMP_PIN,HIGH);
        delay(time_pump);
        digitalWrite(PUMP_PIN,LOW);
        delay(time_after_pump);
      }
      
      digitalWrite(X_DIR_PIN,LOW);
      digitalWrite(Y_DIR_PIN,HIGH);
      for(uint32_t j=0;j<30000;j++)
      {                      
        if(j<14400)
        {                                              
          digitalWrite(X_STEP_PIN,HIGH);
          digitalWrite(Y_STEP_PIN,HIGH);
          delayMicroseconds(40);
          digitalWrite(X_STEP_PIN,LOW);
          digitalWrite(Y_STEP_PIN,LOW);
          delayMicroseconds(40);
        }
        else
        {
          digitalWrite(Y_STEP_PIN,HIGH);
          delayMicroseconds(40);
          digitalWrite(Y_STEP_PIN,LOW);
          delayMicroseconds(40);
        }
      }
      delay(1000);
      digitalWrite(PUMP_PIN,HIGH);
      delay(time_pump);
      digitalWrite(PUMP_PIN,LOW);
      delay(time_after_pump);

      for(int i=0;i<(column_bottom-1);i++)
      {
        for(uint32_t j=0;j<14400;j++)
          {
            digitalWrite(X_STEP_PIN,HIGH);
            delayMicroseconds(40);
            digitalWrite(X_STEP_PIN,LOW);
            delayMicroseconds(40);
          }
        delay(1000);
        digitalWrite(PUMP_PIN,HIGH);
        delay(time_pump);
        digitalWrite(PUMP_PIN,LOW);
        delay(time_after_pump);
      } 
    }
    home_return();
  }

  else
  {
    Serial.println("poker la so chan");
    column_top =  poker/2; 
    column_bottom = column_top;
    Serial.print("column_top: ");
    Serial.println(column_top);
    Serial.print("column_bottom: ");
    Serial.println(column_bottom);

    digitalWrite(ENABLE_PIN,LOW);
    digitalWrite(X_DIR_PIN,HIGH);
    digitalWrite(Y_DIR_PIN,HIGH);

    //Di chuyển đến cột đầu tiên
    for(uint32_t i=0;i<firstmove_step;i++)
    {
      if(i<25600)
      {
        digitalWrite(X_STEP_PIN,HIGH);
        digitalWrite(Y_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        digitalWrite(Y_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
      else
      {
        digitalWrite(X_STEP_PIN,HIGH);
        delayMicroseconds(40);
        digitalWrite(X_STEP_PIN,LOW);
        delayMicroseconds(40);
      }
    }

    delay(1000);
    digitalWrite(PUMP_PIN,HIGH);
    delay(time_pump);
    digitalWrite(PUMP_PIN,LOW);
    delay(time_after_pump);
  
    for(int i=0;i<(column_top-1);i++)
    {
      for(uint32_t j=0;j<14400;j++)
        {
          digitalWrite(X_STEP_PIN,HIGH);
          delayMicroseconds(40);
          digitalWrite(X_STEP_PIN,LOW);
          delayMicroseconds(40);
        }
      delay(1000);
      digitalWrite(PUMP_PIN,HIGH);
      delay(time_pump);
      digitalWrite(PUMP_PIN,LOW);
      delay(time_after_pump);
    }
    
    digitalWrite(X_DIR_PIN,LOW);
    digitalWrite(Y_DIR_PIN,HIGH);
    
    for(uint32_t j=0;j<30000;j++)
    {                              
      digitalWrite(Y_STEP_PIN,HIGH);
      delayMicroseconds(40);
      digitalWrite(Y_STEP_PIN,LOW);
      delayMicroseconds(40);
    }
    delay(1000);
    digitalWrite(PUMP_PIN,HIGH);
    delay(time_pump);
    digitalWrite(PUMP_PIN,LOW);
    delay(time_after_pump);

    for(int i=0;i<(column_bottom-1);i++)
    {
      for(uint32_t j=0;j<14400;j++)
        {
          digitalWrite(X_STEP_PIN,HIGH);
          delayMicroseconds(40);
          digitalWrite(X_STEP_PIN,LOW);
          delayMicroseconds(40);
        }
      delay(1000);
      digitalWrite(PUMP_PIN,HIGH);
      delay(time_pump);
      digitalWrite(PUMP_PIN,LOW);
      delay(time_after_pump);
    }  
    home_return();
  } 

  tft.fillScreen(RA8875_WHITE);                
  tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
  tft.setCursor(2,238);  tft.println("PAUSE");   
  tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
  tft.setCursor(414,238);   tft.println("STOP");
}

void drawButtons()
{
  tft.fillRect(80,50,300,36,RA8875_GREEN);
  tft.fillRect(80,85,300,140,RA8875_YELLOW);
  for (int i=0; i<5; i++)
  {
    tft.fillRoundRect(90+i*60,90,40,40,8,RA8875_CYAN);
    tft.setTextColor(RA8875_RED,RA8875_CYAN);
    tft.setCursor(101+i*60,91);
    tft.print(i,DEC);
  }
  for (int i=0; i<5; i++)
  {
    tft.fillRoundRect(90+i*60,140,40,40,8,RA8875_CYAN);
    tft.setTextColor(RA8875_RED,RA8875_CYAN);
    tft.setCursor(101+i*60,141);
    tft.print(i+5,DEC);
  }

  tft.setTextColor(RA8875_RED,RA8875_CYAN);
  tft.setCursor(87,190);      tft.print("Clear");
  tft.setCursor(182,190);     tft.print("Enter");
  tft.setCursor(280,190);     tft.print("Cancel");
  tft.setTextColor(RA8875_RED,RA8875_WHITE);
  tft.setCursor(290,51);    sprintf(u,"%5.0d",key);    tft.print(u);
}

void value(){
  if (tft.touchDetect()){ 
    delay(100);
    tft.touchReadPixel(&tx, &ty);
    while(tft.touchDetect());
    tx=tx;ty=ty;
    
    if ((ty>=90) && (ty<=120))     for(int i=0; i<5;i++)     if ((tx>=90+i*60) && (tx<=120+i*60))      key=key*10+i;
    if ((ty>=150) && (ty<=180))    for(int i=0; i<5;i++)     if ((tx>=90+i*60) && (tx<=120+i*60))      key=key*10+i+5;
    
    if ((ty>=190) && (ty<=220)) {
      if ((tx>=89) && (tx<=150))     key=0;
      if ((tx>=182) && (tx<=230))    key1=-1;
      if ((tx>=280) && (tx<=330))    key1=-2;
    }
    tft.setTextColor(RA8875_RED,RA8875_WHITE);
    tft.setCursor(290,51);    sprintf(u,"%5.0d",key);    tft.print(u);
  } 
}

void drawButtons1()
{
  tft.fillRect(80,50,300,36,RA8875_GREEN);
  tft.fillRect(80,85,300,140,RA8875_YELLOW);
  for (int i=0; i<5; i++)
  {
    tft.fillRoundRect(90+i*60,90,40,40,8,RA8875_CYAN);
    tft.setTextColor(RA8875_RED,RA8875_CYAN);
    tft.setCursor(101+i*60,91);
    tft.print(i,DEC);
  }
  for (int i=0; i<5; i++)
  {
    tft.fillRoundRect(90+i*60,140,40,40,8,RA8875_CYAN);
    tft.setTextColor(RA8875_RED,RA8875_CYAN);
    tft.setCursor(101+i*60,141);
    tft.print(i+5,DEC);
  }

  tft.setTextColor(RA8875_RED,RA8875_CYAN);
  tft.setCursor(87,190);      tft.print("Clear");
  tft.setCursor(182,190);     tft.print("Enter");
  tft.setCursor(280,190);     tft.print("Cancel");
  tft.setTextColor(RA8875_RED,RA8875_WHITE);
  tft.setCursor(290,51);    sprintf(u,"%5.0d",key_t);    tft.print(u1);
}

void value1(){
  if (tft.touchDetect()){ 
    delay(100);
    tft.touchReadPixel(&tx, &ty);
    while(tft.touchDetect());
    tx=tx;ty=ty;
    
    if ((ty>=90) && (ty<=120))     for(int i=0; i<5;i++)     if ((tx>=90+i*60) && (tx<=120+i*60))      key_t=key_t*10+i;
    if ((ty>=150) && (ty<=180))    for(int i=0; i<5;i++)     if ((tx>=90+i*60) && (tx<=120+i*60))      key_t=key_t*10+i+5;
    
    if ((ty>=190) && (ty<=220)) {
      if ((tx>=89) && (tx<=150))     key_t=0;
      if ((tx>=182) && (tx<=230))    key1_t=-1;
      if ((tx>=280) && (tx<=330))    key1_t=-2;
    }
    tft.setTextColor(RA8875_RED,RA8875_WHITE);
    tft.setCursor(290,51);    sprintf(u1,"%5.0d",key_t);    tft.print(u1);
  } 
}

void Touch()
{ 
  if(run_value==1 && home_value!=1)
  {
    if(digitalRead(X_SW_PIN)==LOW || digitalRead(Y_SW_PIN)==LOW) 
    {
      digitalWrite(ENABLE_PIN,HIGH);
      tft.fillRect(39,70,400,120,RA8875_YELLOW);
      tft.setTextColor(RA8875_RED,RA8875_YELLOW);
      tft.setCursor(58,80);   tft.println("Limit Switch Triggered!");
      tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
      tft.setCursor(207,155);   tft.println("EXIT");
      //tft.fillRect(207,155,63,31,RA8875_BLACK);
      while(1)
      {
         if(tft.touchDetect())
         {
           delay(100);
           tft.touchReadPixel(&tx, &ty);
           while(tft.touchDetect());
           tx=tx;ty=ty;
           if(ty>=140 && ty<=186 && tx>=207 && tx<=270)
           {  
             resetFunc();    
           }
         }    
      }     
    }
    
    if(tft.touchDetect())
    {
      delay(100);
      tft.touchReadPixel(&tx, &ty);
      while(tft.touchDetect());
      tx=tx;ty=ty;
      //Nếu ấn PAUSE
      if(ty>=238 && ty<=269 && tx>=2 && tx<=81)
      {
        if(pause_click==0) 
        {
          pause_click = 1;
          digitalWrite(ENABLE_PIN,HIGH);
          tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
          tft.setCursor(2,238);  tft.println("RESUME");
          while(pause_click == 1)
          {
            if(tft.touchDetect())
            {
              delay(100);
              tft.touchReadPixel(&tx, &ty);
              while(tft.touchDetect());
              tx=tx;ty=ty;
              if(ty>=238 && ty<=269 && tx>=2 && tx<=81)
              {
                digitalWrite(ENABLE_PIN,LOW);
                tft.fillRect(2,238,110,31,RA8875_WHITE);
                tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
                tft.setCursor(2,238);  tft.println("PAUSE");
                pause_click = 0;
              }
            }      
          }    
        }
      }
      
      //Nếu ấn STOP
      if(ty>=238 && ty<=269 && tx>=414 && tx<=477)
      {
        digitalWrite(ENABLE_PIN,HIGH);
        tft.fillRect(100,70,300,120,RA8875_BLUE);
        tft.setTextColor(RA8875_BLACK,RA8875_WHITE);
        tft.setCursor(150,80);   tft.println("Are you sure ?");
        tft.setCursor(150,150);   tft.println("NO");
        tft.setCursor(280,150);   tft.println("YES");
        while(1)
        {
          if(tft.touchDetect())
          {
            delay(100);
            tft.touchReadPixel(&tx, &ty);
            while(tft.touchDetect());
            tx=tx;ty=ty;

            //Ấn Yes
            if((ty >= 150 && ty <= 180&&tx > 280 && tx < 320))
            {        
              resetFunc();
            }

            //Ấn No
            if((ty >= 150 && ty <= 180 && tx > 150 && tx < 200))
            { 
              digitalWrite(ENABLE_PIN,LOW);
              tft.fillScreen(RA8875_WHITE);                
              tft.setTextColor(RA8875_BLACK,RA8875_BLUE); 
              tft.setCursor(2,238);  tft.println("PAUSE");   
              //tft.fillRect(2,210,79,31,RA8875_RED);      
              tft.setTextColor(RA8875_BLACK,RA8875_BLUE);
              tft.setCursor(414,238);   tft.println("STOP");
              //tft.fillRect(414,210,63,31,RA8875_RED); 
              break;
            }
          }
        }       
      }
    }
  }
}

void my_delay(int time_delay)
{
  count = 0;
  uint32_t old_time;
  old_time = count;

  while(count - old_time <= time_delay)
  {
    //delay
    Serial.println(count-old_time);
  }    
}