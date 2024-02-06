#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <Arduino_USBHostMbed5.h>

USBHostMSD msd;
mbed::FATFileSystem usb("usb");

void setup()
{
    Serial.begin(115200);
    
    pinMode(PA_15, OUTPUT); //enable the USB-A port
    digitalWrite(PA_15, HIGH);
    
    while (!Serial)
        ;
    delay(1000);


    Serial.println("Starting USB Dir List example...");

    // if you are using a Max Carrier uncomment the following line
    // start_hub();
    Serial.println("msd trying to connect");
    while (!msd.connect()) {
        //while (!port.connected()) {
        delay(1000);
    }
    Serial.println("msd connected");
    Serial.print("Mounting USB device... ");
    int err = usb.mount(&msd);
    if (err) {
        Serial.print("Error mounting USB device ");
        Serial.println(err);
        while (1);
    }
    Serial.println("done.");
delay(1000);

    char buf[256];
    
    // Display the root directory
    Serial.print("Opening the root directory... ");
    DIR* d = opendir("/usb/");
    Serial.println(!d ? "Fail :(" : "Done");
    if (!d) {
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", strerror(errno), -errno);
        Serial.print(buf);
    }
    Serial.println("done.");

    Serial.println("Root directory:");
    unsigned int count { 0 };
    while (true) {
        struct dirent* e = readdir(d);
        if (!e) {
            break;
        }
        count++;
        snprintf(buf, sizeof(buf), "    %s\r\n", e->d_name);
        Serial.print(buf);
    }
    Serial.print(count);
    Serial.println(" files found!");

    snprintf(buf, sizeof(buf), "Closing the root directory... ");
    Serial.print(buf);
    fflush(stdout);
    err = closedir(d);
    snprintf(buf, sizeof(buf), "%s\r\n", (err < 0 ? "Fail :(" : "OK"));
    Serial.print(buf);
    if (err < 0) {
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", strerror(errno), -errno);
        Serial.print(buf);
    }
}

void loop()
{
}