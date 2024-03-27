// CNC Shield Stepper  Control 
const int StepX = 2; //2
const int DirX = 5; //5
const int StepY = 3;
const int DirY = 6;
//
//
//void setup() {
//  pinMode(StepX,OUTPUT);
//  pinMode(DirX,OUTPUT);
//  pinMode(StepY,OUTPUT);
//  pinMode(DirY,OUTPUT);
//  pinMode(StepZ,OUTPUT);
//  pinMode( DirZ,OUTPUT);
//
//}

#include <AccelStepper.h>
#include <HX711.h>
//

// Load cell pins
//#define LOAD_CELL1_DOUT_PIN 2
//#define LOAD_CELL1_SCK_PIN 3
//#define LOAD_CELL2_DOUT_PIN 7
//#define LOAD_CELL2_SCK_PIN 9

//// Constants for load cell calibration
//const float LOAD_CELL1_CALIBRATION_FACTOR = 1.0;
//const float LOAD_CELL2_CALIBRATION_FACTOR = 1.0;

// Initializing the AccelStepper and HX711 Objects
//HX711 loadCell1;
//HX711 loadCell2;

// Spindle settings
const float stepsPerRevolution = 200.0; // Number of steps per stepper motor revolution
const float lead = 4.0; // Spindle gear ratio (mm per revolution)
const float stepsPerUnit = stepsPerRevolution / lead; // Number of steps per unit (mm)
const float angle_step = 1.8; // Degrees per motor pitch

// Variables to communicate with python
int rep_count = 1; // Initialize with 1 as the default value
boolean automatic_mode = false;
unsigned long Tmeas = 2000; // Default value for Tmeas in milliseconds (2 seconds)
unsigned int Cnum = 4; // Using 'const' to define Cnum as an integer constant
float values[4]; // Vector to store the values
float valuess[4]; // Vector to store the ml values in automatic
unsigned int count = 0; // Collected Values Counter
unsigned long startTime = 0; // Variable to store the beginning of the angular reading

void setup() {
  Serial.begin(9600);

  pinMode(StepX,OUTPUT);
  pinMode(DirX,OUTPUT);
  pinMode(StepY,OUTPUT);
  pinMode(DirY,OUTPUT);
  // Start-up of stepper motors
//  motor1.setMaxSpeed(1000);  // Velocidade máxima em passos por segundo
//  motor1.setAcceleration(500);  // Aceleração em passos por segundo por segundo
//  motor2.setMaxSpeed(1000);
//  motor2.setAcceleration(500);

//  // Initialization of the load cells
  //loadCell1.begin(LOAD_CELL1_DOUT_PIN, LOAD_CELL1_SCK_PIN);
//  loadCell2.begin(LOAD_CELL2_DOUT_PIN, LOAD_CELL2_SCK_PIN);
  //loadCell1.set_scale(LOAD_CELL1_CALIBRATION_FACTOR);
//  loadCell2.set_scale(LOAD_CELL2_CALIBRATION_FACTOR);
  //loadCell1.tare();
//  loadCell2.tare();
}

void moveX(float distanceMM) {
  float steps = 55*abs(tan(distanceMM))*stepsPerUnit;
  if (distanceMM > 0) {
    digitalWrite(DirX, LOW); // set direction, HIGH for clockwise, LOW for anticlockwise
  } else {
    digitalWrite(DirX, HIGH);
  }
  for (int i = 0; i < steps; i++) {
    digitalWrite(StepX, HIGH);
    delayMicroseconds(200);
    digitalWrite(StepX, LOW);
    delayMicroseconds(1000);
  }
}

void moveY(float distanceMY) {
  float steps = distanceMY * stepsPerUnit; // Calculate the number of steps based on distance
//  if (distanceMY > 0) {
//    digitalWrite(DirY, HIGH); // set direction, HIGH for clockwise, LOW for anticlockwise
//  } else {
//    digitalWrite(DirY, LOW);
//  }
  digitalWrite(DirY, LOW);
  for (int i = 0; i < steps; i++) {
    digitalWrite(StepY, HIGH);
    delayMicroseconds(200);
    digitalWrite(StepY, LOW);
    delayMicroseconds(1000);
  }
}

void loop() {
  unsigned long currentMillis = millis(); // Declare the currentMillis variable
  while (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    
    if (command.startsWith("TMEAS:")) {
      // Process Tmeas Command
      Tmeas = command.substring(6).toInt(); // Milliseconds
    }
    else if (command.startsWith("NCOUNT:")) {
      // Process Cnum Command
      Cnum = command.substring(7).toInt();
    } 
    else if (command.startsWith("MA")) {
      int maIndex = command.indexOf("MA ");
      int mlIndex = command.indexOf("ML ");
      if (maIndex != -1 && mlIndex != -1) {
          String maValues = command.substring(maIndex + 4, mlIndex);
          String mlValues = command.substring(mlIndex + 4);
      
          float maValue = maValues.toFloat();
          float mlValue = mlValues.toFloat();
          // Converts the number of steps in mm to the desired position in steps
          moveX(maValue); // Move mm converted to angle torsion in z-axis
          delay(1000); // Delay od 1 second

          moveY(mlValue); // Move mm in x-axis 
          delay(1000); // Delay de 1 segundo
//          motor2.moveTo(targetPositionML);
//          motor2.run();
      }
          // Wait until Tmeas milliseconds have passed
          while (currentMillis - startTime < Tmeas) {
            currentMillis = millis();  // Update the value of currentMillis
          }

          while (count < Cnum) {
                // Simulating Reading Random Values
                // The MA and ML values are available to further calculations
                //float fxx = analogRead(loadCell1.get_units());
//              float mzz = analogRead(loadCell2.get_units()); 
                float fxx = random(0, 1000);  // Generate a random value between 0 and 1000
                float mzz = random(0, 1000); 
              
              // Store value in vector
              values[count] = fxx;
              valuess[count] = mzz;
                           
              // Increments the counter
              count++;
          }
      
          if (count == Cnum) {
              // Average the values in the vector
              float sum = 0;
              float sum2 = 0;
              for (int i = 0; i < Cnum; i++) {
                  sum += values[i];
                  sum2 += valuess[i];
              }
              float fx_medio = sum / Cnum; // Assign the value 
              float mz_medio = sum2 / Cnum; // Assign the value 
              
              // Reset the counter and vector for the next measurement
              count = 0;
              memset(values, 0, sizeof(values));
              memset(valuess, 0, sizeof(valuess));
              startTime = currentMillis;
             

          Serial.print("FX: ");
          Serial.print(fx_medio);
          Serial.print(" MZ: ");
          Serial.println(mz_medio);
      }
    }
    else if (command.startsWith("MOTOR_ANGULAR:")) {
          // Processing Angle Motor Control in Manual Mode
          String angular_value_str = command.substring(14);
          float angular_value = angular_value_str.toFloat();

          // Converts the number of steps in mm to the desired position in steps
//          float targetPosition = angular_value * stepsPerUnit;
//          motor1.moveTo(targetPosition);
//          motor1.run();

          moveX(angular_value); // Move mm in x-axis
          delay(1000); // Delay of 1 second

          // Wait until Tmeas milliseconds have passed
          while (currentMillis - startTime < Tmeas) {
            currentMillis = millis();  // Update the value of currentMillis
          }

          while (count < Cnum) {
              // Simulating Reading Random Values
              //float momentoo = analogRead(loadCell2.get_units());
              float momentoo = random(0, 1000);
              // Store value in vector
              values[count] = momentoo;
              
              // Increments the counter
              count++;
          }
      
          if (count == Cnum) {
              // Average the values in the vector
              float sum = 0;
              for (int i = 0; i < Cnum; i++) {
                  sum += values[i];
              }
              float momento_medio = sum / Cnum; // Assign the value 
              
              // Reset the counter and vector for the next measurement
              count = 0;
              memset(values, 0, sizeof(values));
              startTime = currentMillis;

              Serial.print("MOMENTO:");
              Serial.println(momento_medio);
          }
    }
    else if (command.startsWith("MOTOR_LINEAR:")) {
          // Processing Linear Motor Control in Manual Mode
          String linear_value_str = command.substring(14);
          float linear_value = linear_value_str.toFloat();

//          float targetPosition = linear_value * stepsPerUnit;
//          motor2.moveTo(targetPosition);
//          motor2.run();

        
          moveY(linear_value); // Move mm no Y-axis
          delay(1000); // Delay of 1 second
          
          // Wait until Tmeas milliseconds have passed
          while (currentMillis - startTime < Tmeas) {
            currentMillis = millis();  // Update the value of currentMillis
          }

          while (count < Cnum) {
              // Simulating Reading Random Values
              //float forcaa = analogRead(loadCell1.get_units());
              float forcaa = random(0, 1000);

              // Store value in vector
              values[count] = forcaa;
              
              // Increments the counter
              count++;
          }
      
          if (count == Cnum) {
              // Average the values in the vector
              float sum = 0;
              for (int i = 0; i < Cnum; i++) {
                  sum += values[i];
              }
              float forca_medio = sum / Cnum; // Assign the value
              
              // Reset the counter and vector for the next measurement
              count = 0;
              memset(values, 0, sizeof(values));
              startTime = currentMillis;
              
              Serial.print("FORCA:");
              Serial.println(forca_medio);
          }
   }
  }
}
