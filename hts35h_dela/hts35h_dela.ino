#include <Arduino.h>
#include <lx16a-servo.h>
LX16ABus servoBus;
LX16AServo servo(&servoBus, 1);

void setup() {
	Serial1.begin(115200);
	Serial1.println("Beginning Servo Example");
	servoBus.beginOnePinMode(&Serial1,1);
	servoBus.debug(true);
	servoBus.retry=0;
}

void loop() {
	int divisor =4;

	for (int x = 2500; x <15000; x+=1000) {
		long start = millis();
		int angle = x;
		int16_t pos = 0;
		pos = servo.pos_read();
//Serial1.print("\n\nPosition at %d -> %s\n", pos,
		//	servo.isCommandOk() ? "OK" : "\n\nERR!!\n\n");

		//do {
			servo.move_time(angle, 10*divisor);
		//} while (!servo.isCommandOk());
//		Serial.printf("Move to %d -> %s\n", angle,
//				servo.isCommandOk() ? "OK" : "\n\nERR!!\n\n");
//		Serial.println("Voltage = " + String(servo.vin()));
//		Serial.println("Temp = " + String(servo.temp()));
//		Serial.println("ID  = " + String(servo.id_read()));
//		Serial.println("Motor Mode  = " + String(servo.readIsMotorMode()));
		long took = millis()-start;
		long time = (10*divisor)-took;
		if(time>0)
			delay(time);
		else{
			Serial1.println("Real Time broken, took: "+String(took));
		}
	}

servo.move_time(2500, 3000);
delay(4000);
}

