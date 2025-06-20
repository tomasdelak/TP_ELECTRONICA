#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

// OLED
#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);

// Sensor Hall
const int pin_1 = 36;
const int pin_2 = 39;
const int pin_3 = 34;
const int pin_4 = 35;
#define NUM_DEDOS 4
const int pinesDedos[NUM_DEDOS] = {pin_1, pin_2, pin_3, pin_4};
const char* nombresDedos[NUM_DEDOS] = {"Indice", "Mayor", "Anular", "Menique"};
const float UMBRAL_FLEXION = 2.0;


// Medición de tiempo
unsigned long tiemposDedos[NUM_DEDOS];

// Juego (Actividad 2)
int nivelActual = 1;
int velocidadNivel = 300;
int puntaje = 0;
bool jugando = false;

bool enFlexionSalto = false;
unsigned long ultimaFlexionSalto = 0;


struct DatosPaciente {
  String paciente;                
  float tiempo_calentamiento[4];
  int puntaje_juego[4];
};

DatosPaciente registros[10];



void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("No se detectó pantalla OLED"));
    while (true);
  }

  display.clearDisplay();
  display.display();


  Serial.println("Ingrese el nombre del paciente y presione Enter:"); // Escribir el nombre del paciente en el monitor serial. Configurar la linea en Both NL y CR
  
  // Esperar a que haya algo disponible
  while (Serial.available() == 0) {
    delay(100);
  }

  registros[0].paciente = Serial.readStringUntil('\n');  // Leer hasta Enter
  registros[0].paciente.trim(); // Elimina espacios en blanco sobrantes

  Serial.print("Nombre guardado: ");
  Serial.println(registros[0].paciente);
}


void loop() {

   for (int dedo = 0; dedo < NUM_DEDOS; dedo++) {  
    unsigned long tiempoInicioDedo = millis();
    int repeticiones = 0;

    mostrarMensaje("Dedo:", nombresDedos[dedo]);
    unsigned long tiempoMensaje = millis();
    while (millis() - tiempoMensaje < 1000) { 
      delay(10); 
    }

    bool enFlexion = false;
    unsigned long ultimaFlexion = 0;

    while (repeticiones < 5) {
      float volt = leerVoltaje(pinesDedos[dedo]);
/*
      Serial.print("Voltaje ");
      Serial.print(nombresDedos[dedo]);
      Serial.print(": ");
      Serial.println(volt);
*/
      bool flexionado = volt < UMBRAL_FLEXION;

      if (flexionado && !enFlexion) {
        if (millis() - ultimaFlexion > 300) {
          repeticiones++;
          mostrarRepeticiones(nombresDedos[dedo], repeticiones);
          ultimaFlexion = millis();
        }
        enFlexion = true;
      }

      if (!flexionado && enFlexion) {
        enFlexion = false;
      }

      delay(10);
    }
    registros[0].tiempo_calentamiento[dedo] = (millis() - tiempoInicioDedo) / 1000.0;

  }
  
  mostrarMensaje("Sensor de salto:", "...");    
  delay(1500);
  mostrarMensaje("Listo?", "3");
  delay(500);
  mostrarMensaje("Listo?", "2");
  delay(500);
  mostrarMensaje("Listo?", "1");
  delay(500);



  for (int dedo = 0; dedo < NUM_DEDOS; dedo++){
    mostrarMensaje("Dedo:", nombresDedos[dedo]);
    unsigned long tiempoMensaje = millis();
    while (millis() - tiempoMensaje < 4000) {  
      delay(10);
      int nivel = iniciarJuego(dedo);
      registros[0].puntaje_juego[dedo] = nivel;  // Almacena en la estructura
}
}

  imprimirDatosPaciente(registros[0]);


}

// Definimos funciones

float leerVoltaje(int PIN_HALL) {
  int valor = analogRead(PIN_HALL);
  return valor * (3.3 / 4095.0);
}

void mostrarMensaje(const char* linea1, const char* linea2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(linea1);
  display.setCursor(0, 40);
  display.println(linea2);
  display.display();
}

void mostrarRepeticiones(const char* dedo, int rep) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Dedo: ");
  display.println(dedo);
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print("Reps: ");
  display.println(rep - 1);
  display.display();
}


int iniciarJuego(int dedo) {
  // Constantes y estado inicial
  const int ALTURA_DISPLAY = 64;
  const int ANCHO_PERSONAJE = 10;
  const int ALTURA_PERSONAJE = 10;
  const int ANCHO_OBSTACULO = 3;
  const int ALTURA_OBSTACULO = 5;
  const int SUELO_Y = ALTURA_DISPLAY - 1;

  int personajeX = 30;
  int obstaculoX = 128;
  bool salto = false;

  int estadoSalto = 0; // 0 = en suelo, 1 = subiendo, 2 = bajando
  int yActual = SUELO_Y - ALTURA_PERSONAJE;
  unsigned long tiempoSalto = 0;

  jugando = true;
  int nivelActual = 1;
  velocidadNivel = 150;
  puntaje = 0;

  unsigned long tiempoUltimoFrame = millis();

  while (jugando) {
    if (millis() - tiempoUltimoFrame >= velocidadNivel) {
      tiempoUltimoFrame = millis();
      salto = detectarSaltoUnico(dedo);


      // Animación de salto
      if (estadoSalto == 0 && salto) {
        estadoSalto = 1;
        tiempoSalto = millis();
      }

      if (estadoSalto == 1) {
        yActual -= 5;
        if (millis() - tiempoSalto > 350) {
          estadoSalto = 2;
          tiempoSalto = millis();
        }
      } else if (estadoSalto == 2) {
        yActual += 5;
        if (yActual >= SUELO_Y - ALTURA_PERSONAJE) {
          yActual = SUELO_Y - ALTURA_PERSONAJE;
          estadoSalto = 0;
        }
      }

      // Mover obstáculo
      obstaculoX -= 5;
      if (obstaculoX < -ANCHO_OBSTACULO) {
        obstaculoX = 128;
        puntaje++;
        if (puntaje % 3 == 0) {
          nivelActual++;
          velocidadNivel = max(50, velocidadNivel - 20);
        }
      }

      int personajeTop = yActual;
      int personajeBottom = personajeTop + ALTURA_PERSONAJE;
      int obstaculoTop = SUELO_Y - ALTURA_OBSTACULO;
      int obstaculoBottom = obstaculoTop + ALTURA_OBSTACULO;

      bool colisionX = personajeX < obstaculoX + ANCHO_OBSTACULO &&
                       personajeX + ANCHO_PERSONAJE > obstaculoX;
      bool colisionY = personajeBottom > obstaculoTop && personajeTop < obstaculoBottom;

      if (colisionX && colisionY) {
        mostrarGameOver();
        return nivelActual;
      }

      // Mostrar escena
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Nivel: ");
      display.println(nivelActual);
      display.setCursor(0, 10);
      display.print("Puntaje: ");
      display.println(puntaje);

      display.fillRect(personajeX, personajeTop, ANCHO_PERSONAJE, ALTURA_PERSONAJE, SSD1306_WHITE);
      display.fillRect(obstaculoX, obstaculoTop, ANCHO_OBSTACULO, ALTURA_OBSTACULO, SSD1306_WHITE);
      display.drawLine(0, SUELO_Y, ANCHO_PANTALLA - 1, SUELO_Y, SSD1306_WHITE);

      display.display();
    }
  }
}


bool detectarSaltoUnico(int i) {
  float volt = leerVoltaje(pinesDedos[i]);   
  bool flexionado = volt < UMBRAL_FLEXION;

  if (flexionado && !enFlexionSalto && millis() - ultimaFlexionSalto > 300) {
    enFlexionSalto = true;  
    ultimaFlexionSalto = millis();
    return true;  // Se detectó un salto nuevo
  }

  if (!flexionado) {
    enFlexionSalto = false;  // Se resetea para detectar el siguiente salto
  }

  return false;
}


void mostrarGameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.println("Game Over");
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Nivel alcanzado: ");
  display.println(nivelActual);
  display.display();

  Serial.print("Juego terminado. Nivel alcanzado: ");
  Serial.println(nivelActual);

  delay(3000);
}


void imprimirDatosPaciente(const DatosPaciente& paciente) {
  Serial.println("----- RESULTADOS DEL PACIENTE -----");
  Serial.print("Nombre: ");
  Serial.println(paciente.paciente);

  const char* nombresDedos[4] = {"Indice", "Mayor", "Anular", "Meñique"};

  for (int i = 0; i < 4; i++) {
    Serial.print("Dedo ");
    Serial.print(nombresDedos[i]);
    Serial.print(" - Tiempo calentamiento: ");
    Serial.print(paciente.tiempo_calentamiento[i], 2);
    Serial.print(" segundos | Puntaje juego: ");
    Serial.println(paciente.puntaje_juego[i]);
  }

  Serial.println("-----------------------------------");
}


// Despues al final tenemos que dejar printeado en el serialport "usuario, tiempo_calentamiento_dedoindice, tiempo_calentamiento_dedomayor, tiempo_calentamiento_dedoanular, tiempo_calentamiento_dedomenique, puntaje_juego, tiempo_total_juego"
