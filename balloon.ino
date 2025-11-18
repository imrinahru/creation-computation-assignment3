/*
Multi-Servo Control via Serial with Attach/Detach Support
---------------------------------------------------------
This sketch reads comma-separated values over Serial and applies them as angles to servos.
It also supports attaching and detaching servos dynamically.

Features:
---------
- Angle control: Send numeric CSV values (0-180 degrees)
- Attach/Detach control: Send true/false CSV values
- Intelligent filtering: Arduino only changes servo state when needed
- All servos attached by default for backward compatibility

Hardware Setup:
--------------
1. Connect servos to the following Arduino pins:
   - Servo 1: Pin 2
   - Servo 2: Pin 3
   - Servo 3: Pin 4
   - Servo 4: Pin 5
2. Power your servos appropriately (most servos need external power)
   - DO NOT power multiple servos directly from Arduino's 5V!
   - Use an external power supply or battery pack

Angle Data Examples:
-------------------
Send these strings over Serial (end with newline):
  "90"          -> Sets first servo to 90 degrees
  "90,180"      -> Sets first servo to 90, second to 180
  "90,180,45"   -> Sets first three servos
  "90,180,45,0" -> Sets all four servos

Attach State Examples:
---------------------
Send these strings over Serial (end with newline):
  "true,true,true,true"   -> All servos attached (default)
  "true,false,true,false" -> Servos 1&3 attached, 2&4 detached
  "false,false,false,false" -> All servos detached

How It Works:
------------
- Arduino detects data type automatically (numeric vs true/false)
- Attach state changes are filtered (only applied when different)
- Detached servos ignore angle commands (no jitter or power draw)
- You can send both data types in any order

Serial Configuration:
-------------------
- Baud Rate: 57600
- Line Ending: Newline (\n)

Note: You can modify maxServos constant to support fewer/more servos
(remember to update servoPins and servoAttached arrays accordingly)
*/
#include <Servo.h>

// Define maximum number of servos
const int maxServos = 1;
// Create servo objects
Servo servos[maxServos];  // Array to hold servo objects

// Define constants for servo pin numbers
const int servoPins[maxServos] = {2};  // Pins for servos 1-4

// Track current attach state for each servo
bool servoAttached[maxServos] = {true}; // All attached by default

void setup() {
  // Initialize serial communication at 57600 baud rate
  Serial.begin(57600);
  
  // Attach all servos to their pins
  for(int i = 0; i < maxServos; i++) {
    servos[i].attach(servoPins[i]);
  }
}

void loop() {
  // Check if data is available on the serial port
  if (Serial.available() > 0) {
    // Read the incoming data until a newline character
    String input = Serial.readStringUntil('\n');
    // If the input is not empty, process it
    if (input.length() > 0) {
      // Detect if this is attach state data or angle data
      if (input.indexOf("true") != -1 || input.indexOf("false") != -1) {
        // Contains "true" or "false" = attach state data
        processAttachState(input);
      } else {
        // Regular numeric data = angle data
        processAngleData(input);
      }
    }
  }
}

void processAngleData(String input) {
  int lastIndex = 0;
  int servoIndex = 0;
  
  // Keep finding commas until we run out or hit our servo limit
  while(servoIndex < maxServos) {
    int nextComma = input.indexOf(',', lastIndex);
    String valueString;
    
    // Extract the value string
    if(nextComma == -1) {
      // Last value
      valueString = input.substring(lastIndex);
      // Only write to servo if it's attached
      if(servoAttached[servoIndex]) {
        // Convert to int, constrain, and apply to final servo
        servos[servoIndex].write(constrain(valueString.toInt(), 0, 180));
      }
      break;  // No more values to process
    } else {
      // Not the last value
      valueString = input.substring(lastIndex, nextComma);
      // Only write to servo if it's attached
      if(servoAttached[servoIndex]) {
        // Convert to int, constrain, and apply to current servo
        servos[servoIndex].write(constrain(valueString.toInt(), 0, 180));
      }
      // Move to next value
      lastIndex = nextComma + 1;
      servoIndex++;
    }
  }
}

void processAttachState(String input) {
  int lastIndex = 0;
  int servoIndex = 0;
  
  // Parse comma-separated true/false values
  while(servoIndex < maxServos) {
    int nextComma = input.indexOf(',', lastIndex);
    String stateString;
    
    if(nextComma == -1) {
      // Last value
      stateString = input.substring(lastIndex);
      updateServoAttachState(servoIndex, stateString);
      break;
    } else {
      // Not the last value
      stateString = input.substring(lastIndex, nextComma);
      updateServoAttachState(servoIndex, stateString);
      lastIndex = nextComma + 1;
      servoIndex++;
    }
  }
}

void updateServoAttachState(int servoIndex, String stateString) {
  // Trim whitespace
  stateString.trim();
  
  // Determine desired state (support both "true" and "1")
  bool shouldAttach = (stateString == "true" || stateString == "1");
  
  // INTELLIGENCE: Only make changes if state is different
  if (shouldAttach && !servoAttached[servoIndex]) {
    // Need to attach
    servos[servoIndex].attach(servoPins[servoIndex]);
    servoAttached[servoIndex] = true;
  } 
  else if (!shouldAttach && servoAttached[servoIndex]) {
    // Need to detach
    servos[servoIndex].detach();
    servoAttached[servoIndex] = false;
  }
  // If state is already correct, do nothing (filters redundant calls)
}
