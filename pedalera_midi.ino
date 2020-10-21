//----------------//
// v 0.1          //
// nzfs 2020      //
// nzfs@nzfs.net  //
// nzfs.net       //
//----------------//

#include <MIDI.h>
#include "Contador.h"

MIDI_CREATE_DEFAULT_INSTANCE();
int canal = 1;

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

// escenas
int escenas[4];

// modos
String modo;

// contador para cambio de modos y escenas
Contador contador01;
Contador contador02;

//-------------------------------------------------------

void setup()
{
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

  modo = "toggle";
}

//-------------------------------------------------------

void loop()
{
  leerPulsadores();

  // MOMENTARY //
  if (modo.equals("momentary"))
  {
    for (int i = 0; i < 6; ++i)
    {
      if (estadoPulsadores[i] == HIGH)
      {
        if (!flag[i])
        {
          flag[i] = true;
          MIDI.sendControlChange(100 + i, 127, canal);
        }
      } else
      {
        if (flag[i])
        {
          MIDI.sendControlChange(100 + i, 0, canal);
          flag[i] = false;
        }
      }
    }
  } else if (modo.equals("toggle"))
  {
    // TOGGLE //
    for (int i = 0; i < 6; ++i)
    {
      if (estadoPulsadores[i] == HIGH && previo[i] == LOW)
      {
        if (!flag[i])
        {
          if (estado[i] == HIGH)
          {
            MIDI.sendControlChange(100 + i, 127, canal);
            estado[i] = LOW;
          } else
          {
            MIDI.sendControlChange(100 + i, 0, canal);
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
  }

  // cambio de modo
  // mantener presionado el pulsador 0 para pasar a modo momentary
  if (estadoPulsadores[0] == HIGH && modo.equals("toggle"))
  {
    contador01.contador();
    contador01.timer = false;
    if (contador01.tiempoActual >= 1000)
    {
      if (modo.equals("toggle"))
      {
        modo = "momentary";
        digitalWrite(LED, HIGH);
        delay(10);
      }
    }
  }
  else if (estadoPulsadores[0] == LOW)
  {
    contador01.timer = true;
  }

  // mantener presionado el pulsador 1 para pasar el modo toggle
  if (estadoPulsadores[1] == HIGH && modo.equals("momentary"))
  {
    contador02.contador();
    contador02.timer = false;
    if (contador02.tiempoActual >= 1000)
    {
      if (modo.equals("momentary"))
      {
        modo = "toggle";
        digitalWrite(LED, LOW);
        delay(10);
      }
    }
  }
  else if (estadoPulsadores[1] == LOW)
  {
    contador02.timer = true;
  }
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

//-------------------------------------------------------
