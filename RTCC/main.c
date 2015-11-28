#include <p32xxxx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <plib.h>
#include <xc.h>

#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1
#pragma config POSCMOD  = HS, FNOSC = PRIPLL, FPBDIV = DIV_1, FSOSCEN = ON  

#define nop4() _nop();_nop();_nop();_nop();
#define nop16() nop4(); nop4(); nop4(); nop4();
#define nop64() nop16(); nop16(); nop16(); nop16();
#define nop256() nop64(); nop64(); nop64(); nop64();
#define nop1024() nop256(); nop256(); nop256(); nop256();

void main (void) 
{ 
    INTDisableInterrupts();

    rtccTime tm,tm1,tAlrm,starttm; // time structure
    rtccDate dt,dt1,dAlrm; // date structure
    
    mPORTASetPinsDigitalOut(BIT_0);
    
    while (1) {
        mPORTAToggleBits(BIT_0);
        nop1024();
    }

    RtccInit();

    while(RtccGetClkStat() != RTCC_CLK_ON); //espera que el SOSC este corriendo
    
    tm.l=0x00;
    tm.sec=0x30;                                                                 
    tm.min=0x55;
    tm.hour=0x15;

    dt.wday=0x02;
    dt.mday=0x15;
    dt.mon=0x09;
    dt.year=0x09;

    RtccSetTimeDate(tm.l, dt.l);
    
    tm1.l = RtccGetTime();
    dt1.l = RtccGetDate();
    
    RtccOpen(tm.l, dt.l, 0);  //fijo hora, fecha y calibracion

    // another way to see the RTCC is running: check the SYNC bit
    while(mRtccGetSync());    // wait sync to be low
    while(!mRtccGetSync());    // wait to be high
    while(mRtccGetSync());    // wait sync to be low again

    do
    {
           RtccGetTimeDate(&tm, &dt);// get current time and date
    }while((tm.sec&0xf)>0x7);// don't want to have minute or BCD rollover

    // get current time
    starttm.l=RtccGetTime();

    while(1) {
        do{
            tm.l=RtccGetTime();
        } while(tm.sec==starttm.sec);
     
        // reset starting time
        starttm.sec = tm.sec;
      
        // toggle leds
        mPORTAToggleBits(BIT_0);
    }
    
    ///// if you want to program the alarm in order to interrupt you can do this:

    tAlrm.l=tm.l;
    dAlrm.l=dt.l;
    tAlrm.sec+=2;// alarm due in 2 secs

    RtccChimeEnable();// repetir indefinidamente
    RtccSetAlarmRptCount(255);// contador
    RtccSetAlarmRpt(RTCC_RPT_SEC);// cada 1 seg
     RtccSetAlarmTimeDate(tAlrm.l, dAlrm.l);// setear la hora de alarma
    RtccAlarmEnable();// habilitar la alarma

    //RtccSetCalibration(200);// value to calibrate with at each minute
    
    mRtccSetIntPriority(3,1);
    mRtccEnableInt();// enable the RTCC event interrupts in the INT controller.
}



