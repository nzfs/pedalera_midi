//---------------|
// v 0.3         |
// nzfs 2022     |
// nzfs@nzfs.net |
// nzfs.net      |
//---------------|

#include <MIDI.h>
#include "Contador.h"

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

// leds
const int LED_PULSADOR[] = {8, 9, 10, 11, 12, 13};

// canal por el cual estoy transmitiendo
const int LED_CANAL[] = {8, 9, 10, 11, 12, 13};

// led RGB que indica el modo actual
const int RGB_R = A0;
const int RGB_G = A1;
const int RGB_B = A2;

// ESCENA
const int LED_ESCENA[] = {8, 9, 10, 11, 12, 13};
int escenaActual;
int canalActual = 1;
const int ESCENA[6];
bool flagEscena;
bool flagCanal;
bool flagCanalSiguiente;
bool flagCanalAnterior;

// mensajes
int CC = 100;

// modos
String modo;
String modoPrevio;

// contador inicial
Contador contadorInicio;

// contador para cambio de modos y ESCENA
Contador contadorToggle;
Contador contadorMomentary;
Contador contadorCanal;
Contador contadorEscena;
Contador contadorEscenaOff;
Contador contadorCanalOff;

//-------------------------------------------------------

void setup()
{
  // Serial.begin(31250);
  Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(PULSADOR_01, INPUT);
  pinMode(PULSADOR_02, INPUT);
  pinMode(PULSADOR_03, INPUT);
  pinMode(PULSADOR_04, INPUT);
  pinMode(PULSADOR_05, INPUT);
  pinMode(PULSADOR_06, INPUT);

  for (int i = 0; i < sizeof(LED_PULSADOR); ++i)
  {
    pinMode(LED_PULSADOR[i], OUTPUT);
  }

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  for (int i = 0; i < sizeof(estadoPulsadores); ++i)
  {
    estadoPulsadores[i] = 0;
    estado[i] = HIGH;
    previo[i] = LOW;
  }

  contadorInicio.contador();
  contadorInicio.timer = false;
  
  // default
  modo = "toggle";
  digitalWrite(RGB_G, HIGH);
  escenaActual = 0;
  selectorDeEscena(escenaActual);
  canalActual = 3;
  flagCanalSiguiente = false;
  flagCanalAnterior = false;
}

//-------------------------------------------------------

void loop()
{
  leerPulsadores(); // leo el estado de los pulsadores

  if(contadorInicio.tiempoActual < 3000)
  {
    // contadorInicio.timer = true;
    contadorInicio.contador();
    // Serial.print("inicio");
  }
  else
  {
    // Serial.print("empezo");
    
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
            digitalWrite(LED_PULSADOR[i], HIGH);
            delay(10);
            MIDI.sendControlChange(CC + i, 127, canalActual + 1);
          }
        } else
        {
          if (flag[i])
          {
            digitalWrite(LED_PULSADOR[i], LOW);
            delay(10);
            // MIDI.sendControlChange(CC + i, 0, canalActual + 1);
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
              digitalWrite(LED_PULSADOR[i], HIGH);
              MIDI.sendControlChange(CC + i, 127, canalActual + 1);
              estado[i] = LOW;
            } else
            {
              digitalWrite(LED_PULSADOR[i], LOW);
              MIDI.sendControlChange(CC + i, 0, canalActual + 1);
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
    } else if (modo.equals("escena"))
    {
      //  SELECTOR DE ESCENAS //
      for (int i = 0; i < 5; ++i)
      {
        if (estadoPulsadores[i] == HIGH)
        {
          escenaActual = i;
        }
        if (escenaActual == i)
        {
          digitalWrite(LED_PULSADOR[i], HIGH);
        } else
        {
          digitalWrite(LED_PULSADOR[i], LOW);
        }
      }
      selectorDeEscena(escenaActual);
  
    } else if (modo.equals("canal"))
    {
      if (estadoPulsadores[1] == HIGH)
      {
        if (!flagCanalSiguiente && canalActual <= 15)
        {
          canalActual++;
          flagCanalSiguiente = true;
          delay(10);
        }
      } else if (estadoPulsadores[1] == LOW)
      {
        flagCanalSiguiente = false;
      }
      if (estadoPulsadores[4] == HIGH)
      {
        if (!flagCanalAnterior && canalActual >= 0)
        {
          canalActual--;
          flagCanalAnterior = true;
          delay(10);
        }
      } else if (estadoPulsadores[4] == LOW)
      {
        flagCanalAnterior = false;
      }
      selectorDeCanal(canalActual);
    }
  
    // CAMBIO DE MODO //
    // mantener presionado el pulsador 0 para pasar a modo momentary
    if (estadoPulsadores[0] == HIGH && modo.equals("toggle") ||
        estadoPulsadores[0] == HIGH && modo.equals("escena"))
    {
      contadorToggle.contador();
      contadorToggle.timer = false;
      if (contadorToggle.tiempoActual >= 1000)
      {
        if (modo.equals("toggle"))
        {
          modo = "momentary";
          ledEstado(modo);
          // apago led
          apagarLeds();
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[0] == LOW)
    {
      contadorToggle.timer = true;
    }
  
    // mantener presionado el pulsador 1 para pasar el modo toggle
    if (estadoPulsadores[1] == HIGH && modo.equals("momentary") ||
        estadoPulsadores[1] == HIGH && modo.equals("escena"))
    {
      contadorMomentary.contador();
      contadorMomentary.timer = false;
      if (contadorMomentary.tiempoActual >= 1000)
      {
        if (modo.equals("momentary"))
        {
          modo = "toggle";
          ledEstado(modo);
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[1] == LOW)
    {
      contadorMomentary.timer = true;
    }
  
    // mantener presionado el pulsador 5 para el modo escena
    if (estadoPulsadores[5] == HIGH && modo.equals("toggle") ||
        estadoPulsadores[5] == HIGH && modo.equals("momentary"))
    {
      contadorEscena.contador();
      contadorEscena.timer = false;
      if (contadorEscena.tiempoActual >= 1000)
      {
        if (modo.equals("momentary") || modo.equals("toggle"))
        {
          modoPrevio = modo;
          modo = "escena";
          ledEstado(modo);
          apagarLeds();
          flagEscena = true;
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[5] == LOW)
    {
      contadorEscena.timer = true;
      flagEscena = false;
    }
  
    // salir del modo escena
    if (estadoPulsadores[5] == HIGH && modo.equals("escena") && !flagEscena)
    {
      contadorEscenaOff.contador();
      contadorEscenaOff.timer = false;
      if (contadorEscenaOff.tiempoActual >= 1000)
      {
        if (modo.equals("escena"))
        {
          modo = modoPrevio;
          ledEstado(modo);
          apagarLeds();
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[5] == LOW)
    {
      contadorEscenaOff.timer = true;
    }
  
    // CANAL //
    // mantener presionado el pulsador 2 para el selector de canal
    if (estadoPulsadores[2] == HIGH && modo.equals("toggle") ||
        estadoPulsadores[2] == HIGH && modo.equals("momentary"))
    {
      contadorCanal.contador();
      contadorCanal.timer = false;
      if (contadorCanal.tiempoActual >= 1000)
      {
        if (modo.equals("momentary") || modo.equals("toggle"))
        {
          modoPrevio = modo;
          modo = "canal";
          ledEstado(modo);
          flagCanal = true;
          apagarLeds();
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[2] == LOW)
    {
      contadorCanal.timer = true;
      flagCanal = false;
    }
    // salir del modo canal
    if (estadoPulsadores[0] == HIGH && modo.equals("canal") && !flagCanal)
    {
      contadorCanalOff.contador();
      contadorCanalOff.timer = false;
      if (contadorCanalOff.tiempoActual >= 1000)
      {
        if (modo.equals("canal"))
        {
          for (int i = 0; i < 6; ++i)
          {
            digitalWrite(LED_PULSADOR[i], LOW);
          }
          modo = modoPrevio;
          ledEstado(modo);
          delay(10);
        }
      }
    }
    else if (estadoPulsadores[0] == LOW)
    {
      contadorCanalOff.timer = true;
    }
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

void apagarLeds()
{
  for (int i = 0; i < 6; ++i)
  {
    digitalWrite(LED_PULSADOR[i], LOW);
  }
}

//-------------------------------------------------------

void selectorDeEscena(int _escenaActual)
{
  switch (_escenaActual)
  {
    case 0:
      CC = 100;
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
  }
}

//-------------------------------------------------------

void ledEstado(String _modo)
{
  if (_modo.equals("toggle"))
  {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
  } else if (_modo.equals("momentary"))
  {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
  } else if (_modo.equals("escena"))
  {
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
  } else if (_modo.equals("canal"))
  {
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, HIGH);
  }
}

//-------------------------------------------------------

void selectorDeCanal(int _canalActual)
{
  switch (_canalActual)
  {
    case 0:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], HIGH);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], LOW);
      }
      break;
    case 1:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], HIGH);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], LOW);
      }
      break;
    case 2:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], HIGH);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], LOW);
      }
      break;
    case 3:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], HIGH);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], LOW);
      }
      break;
    case 4:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], LOW);
      }
      break;
    case 5:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 6:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], HIGH);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 7:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], HIGH);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 8:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], HIGH);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 9:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], HIGH);
        digitalWrite(LED_PULSADOR[4], LOW);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 10:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 11:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], HIGH);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 12:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], HIGH);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 13:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], HIGH);
        digitalWrite(LED_PULSADOR[3], LOW);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 14:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], LOW);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], HIGH);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
    case 15:
      if (modo = "canal")
      {
        digitalWrite(LED_PULSADOR[0], HIGH);
        digitalWrite(LED_PULSADOR[1], LOW);
        digitalWrite(LED_PULSADOR[2], LOW);
        digitalWrite(LED_PULSADOR[3], HIGH);
        digitalWrite(LED_PULSADOR[4], HIGH);
        digitalWrite(LED_PULSADOR[5], HIGH);
      }
      break;
  }
}

//-------------------------------------------------------

void recibirEstadoToggle()
{
  
}
