
int a = 0;

void setup()
{
    pinMode(13, OUTPUT);
}

void loop()
{
    digitalWrite(13, a++%2);
    delay(500);
}