/**************************************************
***                 DHT22 SENSOR                ***
***************************************************/
#include "DHTSensor.h"

/**************************************************
***                 NOISE SENSOR                ***
***************************************************/
#include "NoiseSensor.h"

/**************************************************
***              PANTALLA ILI9488               ***
***************************************************/
// The SPIFFS (FLASH filing system) is used to hold touch screen
// calibration data
#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h> // librería para manejo de la pantalla y el touchpad

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

void readNavTouches();
void drawNavBar(char keyLabel[][7]);
void goToHome();
void goToMonit();
void touch_calibrate();

/**************************************************
***           CALIBRACION DE TOUCHPAD           ***
***************************************************/
// se emplea SPIFFS (FLASH filing system) para retener la informacion
// en el archivo con el siguiente nombre
#define CALIBRATION_FILE "/TouchCalData2"
#define REPEAT_CAL false // setear en TRUE para repetir la calibracion

// fuentes de texto a utilizar
#define LABEL1_FONT &FreeSansOblique12pt7b // fuente #1 normal
#define LABEL2_FONT &FreeSansBold12pt7b    // fuente #2 negrita


/**************************************************
***           DECLARACIONES DE NAVBAR           ***
***************************************************/
#define NAVBAR_LENGTH 4 // cantidad de botones en la navbar

char navKeyLabel[NAVBAR_LENGTH][7] = { "Home", "Monit.", "Conf.", "About" }; // labels para los botones de la navbar
uint16_t navKeyColor[4] = { TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY };
// crear todas las instancias de botones para la navbar
TFT_eSPI_Button navTouchableKeys[NAVBAR_LENGTH];

#define NAVKEY_SPACING_X 2 // espaciado X
#define NAVKEY_SPACING_Y 0 // espaciado Y
#define NAVKEY_W ((SCREEN_WIDTH / NAVBAR_LENGTH) - NAVKEY_SPACING_X) // ancho
#define NAVKEY_H 40 // alto
#define NAVKEY_X (NAVKEY_W / 2) // posicion en X = ancho / 2
#define NAVKEY_Y (SCREEN_HEIGHT - (NAVKEY_H / 2)) // posicion en Y = 320 - alto
#define NAVKEY_TEXTSIZE 1 // multiplicador de tamaño de fuente

/**************************************************
***       DECLARACIONES DE NAVBAR/MONITOR       ***
***************************************************/
char monitNavKeyLabel[NAVBAR_LENGTH][7] = { "Home", "", "", "" }; // labels para los botones de la navbar

/**************************************************
***         DECLARACIONES DE VARIABLES          ***
***************************************************/
// float temperature = 0;
// float humidity = 0;
float noise = 0;
float noiseAcc = 0;

bool home = true;
bool monit = false;
bool config = false;
bool about = false;

TaskHandle_t update_Data; // TaskHandler Object
void updateData(void* parameter);

/**************************************************
***                    SETUP                    ***
***************************************************/
void setup() {
  // iniciar la comunicacion serial a 9600 baudios
  Serial.begin(115200);

  // inicializar la pantalla TFT
  tft.init();
  // setear la rotacion de pantalla
  tft.setRotation(1);
  // Calibrar pantalla si es necesario y retener en memoria los datos
  touch_calibrate();
  goToHome();

  // inicializar el sensor DHT22
  initDHT();
  // Start task to get temperature
  xTaskCreatePinnedToCore(
    readDHTTask,       /* Función para la tarea */
    "readDHTTask",     /* Nombre de la tarea */
    4000,              /* Tamaño de la pila */
    NULL,              /* Parámetro */
    5,                 /* Prioridad */
    NULL,              /* Manejador */
    1                  /* Núcleo */
  );

  // configuracion para el sensado del sonido
  initSensor();
  // Start task to get sound freq
  xTaskCreatePinnedToCore(
    getFreqTask,       /* Función para la tarea */
    "getFreqTask",     /* Nombre de la tarea */
    4000,              /* Tamaño de la pila */
    NULL,              /* Parámetro */
    4,                 /* Prioridad */
    NULL,              /* Manejador */
    0                  /* Núcleo */
  );

  // Start task to update things
  xTaskCreatePinnedToCore(
    updateData,                        /* Function to implement the task */
    "updateData",                      /* Name of the task */
    1000,                              /* Stack size in words */
    NULL,                              /* Task input parameter */
    5,                                 /* Priority of the task */
    &update_Data,                      /* Task handle. */
    0);                                /* Core where the task should run */
}

/**************************************************
***                    LOOP                     ***
***************************************************/
void loop(void) {
  readNavTouches();

  if (monit && refresh) {
    goToMonit();
    refresh = false;
  }
}

void updateData(void* parameter) {
  while (true) {
    // Pausa de 1 segundos
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.print("Frequency = ");
    Serial.print(counter);
    Serial.print("    Noise signal is: ");
    Serial.println(isLow);
    noise = counter;

    counter = 0;
  }
}

/**************************************************
***       FUNCIONES DE GESTION DE LA TFT        ***
***************************************************/
void readNavTouches() {
  uint16_t t_x = 0, t_y = 0; // variables para guardar coordenadas

  bool pressed = tft.getTouch(&t_x, &t_y); // TRUE si hay un touch correcto en la pantalla

  // ver si alguna coordenada de boton contiene la coordenada del touch
  for (uint8_t x = 0; x < NAVBAR_LENGTH; x++) {
    if (pressed && navTouchableKeys[x].contains(t_x, t_y)) {
      navTouchableKeys[x].press(true);  // el boton esta presionado
    } else {
      navTouchableKeys[x].press(false);  // el boton no esta presionado
    }
  }

  // ver si algun boton ha cambiado de estado
  for (uint8_t x = 0; x < NAVBAR_LENGTH; x++) {

    tft.setFreeFont(LABEL1_FONT);
    //tft.setFreeFont(LABEL2_FONT);

    if (navTouchableKeys[x].justReleased()) navTouchableKeys[x].drawButton(); // dibujar boton normal

    if (navTouchableKeys[x].justPressed()) {
      navTouchableKeys[x].drawButton(true); // dibujar boton invertido

      if (x == 0 && !home) goToHome();

      else if (x == 1 && !monit) goToMonit();


      delay(10); // antirrebote UI
    }
  }
}

void drawNavBar(char keyLabel[][7]) {
  // dibujar los botones
  for (uint8_t x = 0; x < NAVBAR_LENGTH; x++) {

    tft.setFreeFont(LABEL1_FONT);
    // tft.setFreeFont(LABEL2_FONT);

    navTouchableKeys[x].initButton(&tft, NAVKEY_X + x * (NAVKEY_W + NAVKEY_SPACING_X),
      NAVKEY_Y, NAVKEY_W, NAVKEY_H, TFT_WHITE, navKeyColor[x], TFT_WHITE,
      keyLabel[x], NAVKEY_TEXTSIZE); // x, y, w, h, outline, fill, text
    navTouchableKeys[x].drawButton();
  }
}

void goToMonit() {

  if (!monit) {
    home = false;
    monit = true;
    config = false;
    about = false;

    tft.setFreeFont(LABEL1_FONT);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(15, 65);
    tft.print("Ruido (dB): ");
    tft.setCursor(15, 105);
    tft.print("Temperatura (°C): ");
    tft.setCursor(15, 145);
    tft.print("Humedad (%): ");

    drawNavBar(monitNavKeyLabel);
  }

  // actualizar los datos en la pantalla de monitoreo
  //tft.setFreeFont(LABEL2_FONT);
  tft.setTextColor(TFT_GREEN);
  tft.fillRect(240, 45, 50, 25, TFT_DARKGREY);
  tft.setCursor(240, 65);
  tft.print(noise, 1);
  tft.setCursor(300, 65);
  tft.print("dB");
  tft.fillRect(240, 85, 50, 25, TFT_DARKGREY);
  tft.setCursor(240, 105);
  tft.print(temperature, 1);

  tft.setCursor(300, 105);
  tft.drawCircle(305, 90, 3, TFT_GREEN);
  tft.setCursor(310, 105);
  tft.print("C");

  tft.fillRect(240, 125, 50, 25, TFT_DARKGREY);
  tft.setCursor(240, 145);
  tft.print(humidity, 1);
  tft.setCursor(300, 145);
  tft.print("%");
}

void goToHome() {
  home = true;
  monit = false;
  config = false;
  about = false;

  // limpiar la pantalla
  tft.fillScreen(TFT_BLACK);

  // dibujar el fondo en negro
  tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_DARKGREY);

  // dibujar la navbar
  drawNavBar(navKeyLabel);
}

void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    } else {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char*)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char*)calData, 14);
      f.close();
    }
  }
}
