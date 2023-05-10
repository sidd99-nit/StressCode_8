#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;
SoftwareSerial BTSerial(10, 11);
const int BT_BAUD_RATE = 9600;

// Define the maximum window size
#define MAX_WINDOW_SIZE 80

// Define the time interval between readings in milliseconds
#define READ_INTERVAL 2000

// Initialize variables
double readings[MAX_WINDOW_SIZE];
double weights[MAX_WINDOW_SIZE];
int window_size = 1;

bool first_time = true;

int16_t value;
int16_t prevValue;

double stress = 10.00;
int stress_value;
double prevStress ;
double prevAvg;

double prev_percentage_change = 0.00;
double percentage_change = 0.00;

#define WINDOW_SIZE5 9 // Define the window size

int readings5[WINDOW_SIZE5]; // Declare an array to store the readings
int index5 = 0; // Initialize the index to 0

int counter = 0;

void setup() 
{
  Serial.begin(9600);
  BTSerial.begin(BT_BAUD_RATE);
  ads.begin();
  
}

void loop() 
{
  if (BTSerial.available()) 
  {
    String message = BTSerial.readString();
    if(message == "R") 
    { 
      Serial.println("Recalibrate..."); 
      setup();
    }
  }

  // read the sensor value
   value =  ads.readADC_SingleEnded(0);

  int16_t new_reading = value;

  //new
  readings5[index5] = value; // Add the new reading to the array
  double sum = 0;
  for (int i = 0; i < WINDOW_SIZE5; i++) {
    sum += readings5[i]; // Calculate the sum of all the values in the array
  }
  double moving_average = sum / WINDOW_SIZE5; // Calculate the moving average
  index5 = (index5 + 1) % WINDOW_SIZE5; // Increment the index and wrap around if necessary
  //new
  

  // Add the new reading to the list of readings
  for (int i = window_size - 1; i > 0; i--) {
    readings[i] = readings[i - 1];
  }
  readings[0] = new_reading;

  // Update weights for each reading based on their position in the window
  for (int i = 0; i < window_size; i++) 
  {
    weights[i] = ((window_size - i) / (double)(window_size * (window_size + 1) / 2));
  } 

  // Calculate the weighted average of the readings
  double weighted_average = 0;
  double sum_of_weights = 0;
  for (int i = 0; i < window_size; i++) {
    weighted_average += readings[i] * weights[i];
    sum_of_weights += weights[i];
  }
  weighted_average /= sum_of_weights;

  // Calculate the percentage change from the weighted average to the new reading
  if(counter > 4)
  {
     percentage_change = ((moving_average - weighted_average) / weighted_average) * 100;
    
  }
  percentage_change = percentage_change ;
  double positive_percenatge_change = abs(percentage_change) + 10.00;
  
  if(percentage_change < prev_percentage_change )
  {
     stress += prev_percentage_change - percentage_change;
  }

  else if(percentage_change > prev_percentage_change)
  {
    //  if(value < prevValue)
    //  {
    //    double change = ((prevValue - value) / value) * 100;
    //    stress += change;
    //  }    
    //  if(value > prevValue)
    //  {
    //     stress -= percentage_change - prev_percentage_change;
    //  }
    stress -= percentage_change - prev_percentage_change;

  }


  if(stress - prevStress > 10)
  {
     stress = prevStress + 5;
  }
 
  
  
  stress_value = stress;

  if(stress_value < 10 && stress < 10.00)
  {
    stress_value = 10;
    stress = 10.00;
  }
  else 
  {
    if(stress_value > 100 && stress > 100.00)
    {
        stress = 100.00;
        stress_value = 100;
    }
  }

  

  // Print the results
  
  Serial.print("Weighted average: ");
  Serial.println(weighted_average);
  Serial.print("New Value: ");
  Serial.println(value);
  Serial.print("new average: ");
  Serial.println(moving_average);
  Serial.print("percentage change: ");
  Serial.println(percentage_change);
  Serial.print("stress: ");
  Serial.println(stress);
  Serial.println(".............................");

  BTSerial.println(stress_value);

  prev_percentage_change = percentage_change;
  prevValue = value;
  prevAvg = weighted_average;
  prevStress = stress;

  // Increase the window size by 1 until it reaches the maximum window size
  if (window_size < MAX_WINDOW_SIZE) {
    window_size++;
  } else {
    // If the window size reaches the maximum, delete the oldest value
    for (int i = 0; i < MAX_WINDOW_SIZE - 1; i++) {
      readings[i] = readings[i+1];
    }
  }
  counter++;
  delay(2000);
}