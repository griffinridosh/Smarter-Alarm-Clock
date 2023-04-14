import processing.serial.*; 
Serial port;
final String serialName = "COM7";
final int BAUD_RATE = 230400;

void setup()
{
  port = new Serial(this, serialName, BAUD_RATE);
}

byte[] frame_header = new byte[] {61,62,63,64};
void draw() {
  //Array time[] => time[0]->hours | time[1]->minutes | time[0]->seconds
  int time[]={hour(), minute(), second()};
  port.write(frame_header);
  //port.write(time);
  for (int n = 0; n <= 2; n++) {
        port.write(time[n]);
   }
  delay(500);
}
