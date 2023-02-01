#include "BluetoothSerial.h"
#include <Arduino.h>

#define PIN_A 5 
#define PIN_B 19
#define PIN_PWM 32
#define PWM_Ch 0
#define PWM_Res 10
#define PWM_Freq 20

class Encoder {
  private:
    int PWM_DutyCycle = 0;
    int temp, counter = 0; //Esta variável aumentará ou diminuirá dependendo da rotação do codificador
    int voltas = 0;
    String text = "RPM = ";
    String text_out = "";
    String text_pwm = "PWM = ";
    String text_out_pwm = "";
    String inString = "";
    int stringToInteger;
    int RPM = 0;
    hw_timer_t * timer = NULL; //Variável para armazenar o timer
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; //Variável para sincronizar o timer
    
    BluetoothSerial SerialBT; //Variável para armazenar a conexão serial com o dispositivo Bluetooth
    static void IRAM_ATTR onTimer(); //Função para calcular a velocidade de rotação a partir do encoder
    static void interrupt_handler(); //Função para ler o encoder e controlar o PWM
    
  public:
    void begin();
    void loop();
};

void Encoder::begin() {
  Serial.begin(115200);

  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_PWM, OUTPUT);
  
  attachInterrupt(PIN_A, interrupt_handler, RISING);
  ledcAttachPin(PIN_PWM, PWM_Ch);
  ledcSetup(PWM_Ch, PWM_Freq, PWM_Res);
  ledcWrite(PWM_Ch, PWM_DutyCycle);
  
  SerialBT.begin("Spin"); //Nome do dispositivo Bluetooth
  Serial.println("O dispositivo iniciou, agora você pode emparelhá-lo com o bluetooth!");

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &Encoder::onTimer, true);
  timerAlarmWrite(timer, 2000000, true);
  timerAlarmEnable(timer);
}
//Função para ler o encoder e controlar o PWM
void Encoder::loop() {
  if (SerialBT.available()) {
    int inChar = SerialBT.read();
    if (isDigit(inChar)) inString += (char) inChar;
    if (inChar == '\n') {
      stringToInteger = inString.toInt();
      inString = "";
    }
    PWM_DutyCycle = stringToInteger * 10.23;
    if (PWM_DutyCycle > 1023 || PWM_DutyCycle < 0) PWM_DutyCycle = 0;
    ledcWrite(PWM_Ch, PWM_DutyCycle); 
  }

  if (counter > 400) {
    voltas ++;
    counter = 0;
  }
  if (counter < -400) {
    voltas --;
    counter = 0;
  }
}
//Função para calcular a velocidade de rotação a partir do encoder
void Encoder::interrupt_handler() {
  temp = digitalRead(PIN_A);
  if (temp == digitalRead(PIN_B)) counter ++;
  else counter --;
}
//Função para ler o encoder e controlar o PWM
void Encoder::onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  RPM = counter * 3;
  counter = 0;
  portEXIT_CRITICAL_ISR(&timerMux);
  text_out = text + RPM;
  text_out_pwm = text_pwm + PWM_DutyCycle;
  SerialBT.println(text_out);
  SerialBT.println(text_out_pwm);
}

Encoder encoder; //Variável para armazenar a classe Encoder

//Função para inicializar a conexão serial
void setup() {
  encoder.begin();
}
//Função para calcular a velocidade de rotação a partir do encoder
void loop() {
  encoder.loop();
}



// #include <iostream>

// int main(){
//     std::cout << "Hello World, Spin Embarcado"; 
// }