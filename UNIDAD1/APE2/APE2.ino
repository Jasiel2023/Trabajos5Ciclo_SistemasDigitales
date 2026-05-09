const int LED_PINS[]={2,3,4,5,6,7};
const int BOTON_PIN= 8;
int patronActual = 0;
void setup()
{
  for(int i=0; i<= 5; i++){
  	pinMode(LED_PINS[i], OUTPUT);
  }
  pinMode(BOTON_PIN, INPUT);
}

void loop()
{
  leerBoton();
  ejecutarPatrones();
  
}

void leerBoton(){
  if(digitalRead(BOTON_PIN) == HIGH){
    patronActual++;
    if(patronActual > 5){
      patronActual = 0;
    }
    delay(300);
  }else{
    patronActual=0;
    for(int i=0;i <= 5; i++){
    digitalWrite(LED_PINS[i], LOW);
  }
  }
}

void patronSecuencia(){
  for(int i=0;i <= 5; i++){
    digitalWrite(LED_PINS[i], HIGH);
    delay(150);
  }

  for(int i=0;i <= 5; i++){
    digitalWrite(LED_PINS[i], LOW);
    delay(150);
  }

  
}

void patronPersecucion(){
  for(int i=0;i <= 5; i++){
    digitalWrite(LED_PINS[i], HIGH);
    delay(150);
    digitalWrite(LED_PINS[i], LOW);
  }
}

void patronParpadeo(){
  int cont = 0;
   while(cont < 4){
       for(int i=0;i <= 5; i++){
        digitalWrite(LED_PINS[i], HIGH);
      }
    delay(300);
    for(int i=0;i <= 5; i++){
        digitalWrite(LED_PINS[i], LOW);
      }
      delay(300);
      cont++;
   }
}

void patronAleatorio(){
  for(int i=0;i <= 5; i++){
    int aleatorio = random(0, 2);
        if(aleatorio == 0){
          digitalWrite(LED_PINS[i], LOW);
        }else{
          digitalWrite(LED_PINS[i], HIGH);
        }
        
   }
    delay(300);
    for(int i=0;i <= 5; i++){
        digitalWrite(LED_PINS[i], LOW);
      }
}

void patronOnda(){
   for(int i=0;i <= 5; i++){
        digitalWrite(LED_PINS[i], HIGH);
        delay(200);
        digitalWrite(LED_PINS[i], LOW);
      }
    
   for(int i=5;i >= 0; i--){
        digitalWrite(LED_PINS[i], HIGH);
        delay(200);
        digitalWrite(LED_PINS[i], LOW);
      }
}

void ejecutarPatrones(){
	switch (patronActual){
    case 1:
     patronSecuencia();
     break;
    case 2:
     patronPersecucion();
     break;
     case 3:
     patronParpadeo();
     break;
     case 4:
     patronAleatorio();
     break;
     case 5:
     patronOnda();
     break;
  } 
}




