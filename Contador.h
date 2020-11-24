class Contador
{
  public:
    bool timer;
    int tiempoTranscurrido;
    int tiempoActual;

    Contador()
    {
    }

    void contador()
    {
      if (timer)
      {
        tiempoTranscurrido = millis();
        timer = false;
      }
      tiempoActual = millis() - tiempoTranscurrido;
    }
};
