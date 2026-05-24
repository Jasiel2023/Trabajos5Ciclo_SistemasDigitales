int led = 5;
int estado = 0;
void setup()
{
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available()) { 
    estado = Serial.read();
	}
  
  if (estado == '1') { 
    digitalWrite(led, HIGH);
    delay(1000);
	}
}