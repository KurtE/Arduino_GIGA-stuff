#include <iostream>

void setup()
{
  Serial.begin(9600);
	std::cout << "Hello, C++ world! " << CONFIG_BOARD << std::endl;
}

void loop(){}