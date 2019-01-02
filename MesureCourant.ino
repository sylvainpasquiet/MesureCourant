
//#define SANS_RF 

// Enable debug prints
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_SPLASH_SCREEN_DISABLED


#define MY_NODE_ID 30

#define MY_RF24_CE_PIN  9
#define MY_RF24_CS_PIN  10
#define MY_RF24_CHANNEL  125
#define MY_RF24_PA_LEVEL RF24_PA_HIGH


#ifndef SANS_RF 
#include <MySensors.h>
#endif

#define CHILD_ID_CURRENT_1 0
#define CHILD_ID_CURRENT_2 1
#define CURRENT_1_PIN 0
#define CURRENT_2_PIN 1

#define VOIES_ANA 8

float voMeasuredMax[VOIES_ANA];
float voMeasuredMean[VOIES_ANA];
float voMeasuredMin[VOIES_ANA];
float voMeasured[VOIES_ANA];
float calcVoltage[VOIES_ANA];
float calcCurrent[VOIES_ANA];
float lastCurrent[VOIES_ANA];
float calcValue;
unsigned char EtapeG7=0;
unsigned long OldTime=0;
unsigned long OldTimeSend[VOIES_ANA];

uint32_t SLEEP_TIME = 10; // Sleep time between reads (in milliseconds)
unsigned char i;

unsigned long gOldTimeSend=0;

#ifndef SANS_RF 
MyMessage Msg;

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Mesures de courant", "2.20");

	// Register all sensors to gateway (they will be created as child devices)
  for (i=0;i<VOIES_ANA;i++) 
  {
	  present(CHILD_ID_CURRENT_1+i, S_MULTIMETER);
    sleep(50);
    send(Msg.setSensor(CHILD_ID_CURRENT_1+i).setType(V_CURRENT).set(0.0,3),true);
    sleep(50);
  }
}
#endif

void setup()
{ 
  analogReference(EXTERNAL);
  //Serial.begin(115200);  
  lastCurrent[0]=-1.0;
  lastCurrent[i]=-1.0;
}

void loop()
{
  unsigned long ActualTime = millis();
  switch(EtapeG7)
  {
    case 0:
      OldTime=millis();
      while(millis()-OldTime<1000)
      {
        for (i=0;i<VOIES_ANA;i++) 
        {
          voMeasured[i]=analogRead(i);
          OldTimeSend[i] = 0;
        }
      }
      EtapeG7=10;
      break;
      
    case 10:
      for (i=0;i<VOIES_ANA;i++) 
      {
        voMeasuredMax[i]=0.0;
        voMeasuredMin[i]=1023.0;
      }
      EtapeG7=11;
      break;  
      
    case 11:
      OldTime=millis();
      while(millis()-OldTime<1000)
      {
        for (i=0;i<VOIES_ANA;i++) 
        {
          voMeasured[i] = (float)analogRead(i);
          if (voMeasuredMax[i]<voMeasured[i]) voMeasuredMax[i]=voMeasured[i];
          if (voMeasuredMin[i]>voMeasured[i]) voMeasuredMin[i]=voMeasured[i];
        }
      }
      EtapeG7=12;
      break;
      
    case  12:
      for (i=0;i<VOIES_ANA;i++) 
      {
        voMeasuredMean[i]=(voMeasuredMin[i]+voMeasuredMax[i])/2.0;
        voMeasured[i]=voMeasuredMax[i]-voMeasuredMean[i];
        if (voMeasured[i]<0.0) voMeasured[i]=0.0;
        calcVoltage[i]=voMeasured[i]* 1000.0 * (3.3 / 1023.0);
        calcValue=calcVoltage[i]* 30.0/1.414213562;
        if (fabs(calcValue-calcCurrent[i])>100)
        {
          calcCurrent[i]=calcValue;
        }
        else
        {
          calcCurrent[i]=((calcCurrent[i]*49.0 + calcValue)/50.0);  
          calcCurrent[i]=(float)calcCurrent[i];
        } 
        if (calcCurrent[i]<150) calcCurrent[i]=0.0;       
      }     
      EtapeG7=10;
      break;
  }

  for (i=0;i<2;i++) 
  {
    if (((fabs(calcCurrent[i]-lastCurrent[i])>10)||(ActualTime-OldTimeSend[i]>(10000+1000*i)))&&(((ActualTime-gOldTimeSend)>10000)||((ActualTime-gOldTimeSend)>1500)&&(fabs(calcCurrent[i]-lastCurrent[i])>200))) 
    {
      if(send(Msg.setSensor(CHILD_ID_CURRENT_1+i).setType(V_CURRENT).set(ceil(calcCurrent[i])/1000,3),true)) 
      {
        lastCurrent[i] = calcCurrent[i];
      }
      OldTimeSend[i] = ActualTime;
      gOldTimeSend = ActualTime;
      /*
      Serial.print(i);
      Serial.print(" - Mesure (0-1023): ");
      Serial.print(voMeasured[i]);
      
      Serial.print(" - min (0-1023): ");
      Serial.print(voMeasuredMin[i]);
      
      Serial.print(" - Moyenne (0-1023): ");
      Serial.print(voMeasuredMean[i]);
      
      Serial.print(" - max (0-1023): ");
      Serial.print(voMeasuredMax[i]);   
      
      Serial.print(" - Voltage: ");
      Serial.print(calcVoltage[i]);
      
      Serial.print(" mV - Current: ");
      Serial.print(ceil(calcCurrent[i])/1000,3);
      Serial.println(" A");
      */
    }
  }

#ifndef SANS_RF 
	wait(SLEEP_TIME);
#else
  delay(10);
#endif  
}
