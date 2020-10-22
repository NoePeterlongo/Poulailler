
static bool toggleLEDBUILTIN=false;

void clignottement(float frequence = 1)
{
    pinMode(LED_BUILTIN, OUTPUT);
    cli();
    //set timer1 interrupt at 1Hz
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    OCR1A = (uint16_t)(16*1e6/(2*frequence*1024));
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

ISR(TIMER1_COMPA_vect){
  if (toggleLEDBUILTIN){
    digitalWrite(LED_BUILTIN,HIGH);
    toggleLEDBUILTIN = 0;
  }
  else{
    digitalWrite(LED_BUILTIN,LOW);
    toggleLEDBUILTIN = 1;
  }
}
