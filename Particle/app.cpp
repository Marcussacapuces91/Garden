#include "app.h"


volatile byte App::p;
volatile unsigned App::humid;
volatile int App::temp;
volatile byte App::cs;

/*
void _interrupt() {
    ++App::p;
}

*/
