#include <MIDI.h>

#define MOMENTARY
//#define TOGGLE

MIDI_CREATE_DEFAULT_INSTANCE();

// pulsadores
const int PULSADOR_01 = 2;
const int PULSADOR_02 = 3;
const int PULSADOR_03 = 4;
const int PULSADOR_04 = 5;
const int PULSADOR_05 = 6;
const int PULSADOR_06 = 7;

int estadoPulsadores[6];

bool flag[6];

int estado[6];
int reading[6];
int previo[6];

// display
const int LED =  13;

//-------------------------------------------------------

void setup()
{
  //Serial.begin(9600);
  Serial.begin(31250);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(LED, OUTPUT);

  pinMode(PULSADOR_01, INPUT);
  pinMode(PULSADOR_02, INPUT);
  pinMode(PULSADOR_03, INPUT);
  pinMode(PULSADOR_04, INPUT);
  pinMode(PULSADOR_05, INPUT);
  pinMode(PULSADOR_06, INPUT);

  for (int i = 0; i < sizeof(estadoPulsadores); ++i)
  {
    estadoPulsadores[i] = 0;
    estado[i] = HIGH;
    previo[i] = LOW;
  }
}

//-------------------------------------------------------

void loop()
{
  leerPulsadores();

  // MOMENTARY //
#ifdef MOMENTARY
  for (int i = 0; i < 6; ++i)
  {
    if (estadoPulsadores[i] == HIGH)
    {
      if (!flag[i])
      {
        flag[i] = true;
        MIDI.sendControlChange(100 + i, 127, 1);
      }
    } else
    {
      if (flag[i])
      {
        MIDI.sendControlChange(100 + i, 0, 1);
        flag[i] = false;
      }
    }
  }
#endif

  // TOGGLE //
#ifdef TOGGLE
  for (int i = 0; i < 6; ++i)
  {
    if (estadoPulsadores[i] == HIGH && previo[i] == LOW)
    {
      if (!flag[i])
      {
        if (estado[i] == HIGH)
        {
          MIDI.sendControlChange(100 + i, 127, 1);
          estado[i] = LOW;
        } else
        {
          MIDI.sendControlChange(100 + i, 0, 1);
          estado[i] = HIGH;
        }
        delay(10); // debounce
        flag[i] = true;
      }
    } else if (estadoPulsadores[i] == LOW)
    {
      if (flag[i])
      {
        flag[i] = false;
      }
      delay(10); // debounce
    }
    previo[i] = estadoPulsadores[i];
  }
#endif
}
//-------------------------------------------------------

void leerPulsadores()
{
  estadoPulsadores[0] = digitalRead(PULSADOR_01);
  estadoPulsadores[1] = digitalRead(PULSADOR_02);
  estadoPulsadores[2] = digitalRead(PULSADOR_03);
  estadoPulsadores[3] = digitalRead(PULSADOR_04);
  estadoPulsadores[4] = digitalRead(PULSADOR_05);
  estadoPulsadores[5] = digitalRead(PULSADOR_06);
}
