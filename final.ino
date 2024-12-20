#define OUTPUT_PIN 31  // Pin care generează curent
#define INPUT_PIN 30   // Pin care primește curent

const int sensorPins[] = {A0, A1, A2}; // Pinii senzorilor
const int numSensors = 3;

int blackValues[numSensors] = {0, 0, 0}; // Valorile pentru fundal negru
int threshold = 2;          // Prag mai mare pentru diferențe semnificative

// Pinii motoarelor
#define ENA 10
#define IN1 6
#define IN2 7
#define ENB 9
#define IN3 8
#define IN4 11

void setup() {
  // Configurăm pinii jumper-ului
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);

  // Pornim OUTPUT_PIN pentru a genera curent
  digitalWrite(OUTPUT_PIN, HIGH);
  
  // Configurăm pinii motoarelor ca ieșiri
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Inițializare serială
  Serial.begin(9600);

  // Stabilirea datelor inițiale dacă jumper-ul este pus
  Serial.println("Incepere calibrare. Pune jumperul pentru calibrare.");
  while (digitalRead(INPUT_PIN) == HIGH) {
    calibrateSensors();
    delay(100); // Calibrare la fiecare 1 secundă
  }

  Serial.println("Calibrare finalizată. Scoate jumperul pentru a continua.");
}

void loop() {
  static bool isCalibrating = false;

  if (digitalRead(INPUT_PIN) == HIGH) {
    if (!isCalibrating) {
      Serial.println("Jumper pus. Oprire motoare și începere calibrare.");
      stopMotors();
      isCalibrating = true;
    }
    calibrateSensors();
  } else {
    if (isCalibrating) {
      Serial.println("Jumper scos. Folosire date curente pentru calibrare.");
      isCalibrating = false;
    }
    followLine(); // Apelăm funcția pentru urmărirea liniei
  }
}

// Funcția de calibrare a senzorilor
void calibrateSensors() {
  for (int i = 0; i < numSensors; i++) {
    blackValues[i] = analogRead(sensorPins[i]); // Setează valoarea actuală ca punct de referință pentru fundal negru
    Serial.print("Senzor ");
    Serial.print(i);
    Serial.print(" - Valoare calibrată (negru): ");
    Serial.println(blackValues[i]);
  }
}

// Funcția pentru urmărirea liniei
void followLine() {
  int sensorValues[numSensors];
  int binaryArray[numSensors] = {0}; // Array binar pentru stări senzori
  int leftCount = 0; // Numără senzori activi pe partea stângă
  int rightCount = 0; // Numără senzori activi pe partea dreaptă

  // Citirea datelor de la senzori și actualizarea array-ului binar
  for (int i = 0; i < numSensors; i++) {
    sensorValues[i] = analogRead(sensorPins[i]);
    if (sensorValues[i] > blackValues[i] + threshold) {
      binaryArray[i] = 1;
      if (i == 0) leftCount++; // Senzor stânga (A0)
      if (i == 2) rightCount++; // Senzor dreapta (A2)
    } else {
      binaryArray[i] = 0;
    }
  }

  // Interpretarea array-ului binar pentru a decide mișcarea
  int baseSpeed = 110; // Viteză de bază
  int leftSpeed = baseSpeed;
  int rightSpeed = baseSpeed;

  if (binaryArray[1] == 1 && leftCount == 0 && rightCount == 0) {
    // Linia este centrată pe senzorul A1
    leftSpeed = baseSpeed;
    rightSpeed = baseSpeed;
  } else if (leftCount > rightCount) {
    // Senzorul din stânga activat (A0)
    leftSpeed = baseSpeed * 0.6;
    rightSpeed = baseSpeed * 1.5;
  } else if (rightCount > leftCount) {
    // Senzorul din dreapta activat (A2)
    leftSpeed = baseSpeed * 1.5;
    rightSpeed = baseSpeed * 0.6;
  } else {
    // Nicio linie detectată, mișcare înapoi și căutare
    leftSpeed = -100;
    rightSpeed = -100;

    adjustMotors(leftSpeed, rightSpeed);
    delay(200); // Mișcare înapoi pentru o perioadă scurtă
    return;
  }

  adjustMotors(leftSpeed, rightSpeed);
}

// Funcție pentru ajustarea vitezelor motoarelor
void adjustMotors(int speedLeft, int speedRight) {
  // Motor stânga
  if (speedLeft >= 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, speedLeft);
  } else {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, abs(speedLeft));
  }

  // Motor dreapta
  if (speedRight >= 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, speedRight);
  } else {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, abs(speedRight));
  }
}

// Funcții pentru controlul motoarelor
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
