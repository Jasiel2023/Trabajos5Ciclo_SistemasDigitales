
const int LED_PIN = 13;
byte estadoLed = 0;
int contador = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  
  byte a =  0b00000011;
  byte b =  0b00000010;

  Serial.println("---Resultados AND, OR, XOR, NOT,Shift");
  Serial.print("AND(a&b):"); Serial.println(byte(a&b), BIN);
  Serial.print("OR(a|b):"); Serial.println(byte(a|b), BIN);
  Serial.print("XOR(a^b):"); Serial.println(byte(a ^ b), BIN);
  Serial.print("NOT(~a):"); Serial.println(byte(~a), BIN);
  Serial.print("Shift(1 << 2):"); Serial.println(byte(1 << 2), BIN);

  estadoLed = estadoLed | (1<<0);
}

void loop() {
  // put your main code here, to run repeatedly:
  estadoLed = estadoLed ^ 0b00000001;

  if((estadoLed & 1) == 1){
    digitalWrite(LED_PIN, HIGH);

  }else{
    digitalWrite(LED_PIN, LOW);
  }

  delay(500);

  contador = (contador + 1) % 8;

  Serial.print("Desplazamineto actual:");
  Serial.println(1 << contador, BIN);

  delay(500);
}


