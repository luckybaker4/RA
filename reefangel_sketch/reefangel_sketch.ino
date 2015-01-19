#include <ReefAngel_Features.h>
#include <Globals.h>
#include <RA_Wifi.h>
#include <Wire.h>
#include <OneWire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <InternalEEPROM.h>
#include <RA_NokiaLCD.h>
#include <RA_ATO.h>
#include <RA_Joystick.h>
#include <LED.h>
#include <RA_TempSensor.h>
#include <Relay.h>
#include <RA_PWM.h>
#include <Timer.h>
#include <Memory.h>
#include <InternalEEPROM.h>
#include <RA_Colors.h>
#include <RA_CustomColors.h>
#include <Salinity.h>
#include <RF.h>
#include <IO.h>
#include <ORP.h>
#include <AI.h>
#include <PH.h>
#include <WaterLevel.h>
#include <Humidity.h>
#include <DCPump.h>
#include <WiFiAlert.h>
#include <ReefAngel.h>

////// Place global variable code below here

unsigned int awcStartLevel=0;  

////// Place global variable code above here

// Define Custom Memory Locations

#define Mem_I_Water_Change_On_Hour       100
#define Mem_I_Water_Change_On_Minute     101
#define Mem_I_Water_Change_Off_Hour      102
#define Mem_I_Water_Change_Off_Minute    103
#define Mem_I_Water_Change_WL_High       104
#define Mem_I_Water_Change_WL_Low        105
#define Mem_B_Water_Change_Enabled       106
#define Mem_B_AutoFeed_Enabled           107
#define Mem_I_AutoFeed_Hour              108
#define Mem_I_AutoFeed_Minute            109
#define Mem_I_Debug                      110
#define Mem_I_ZeoVit_Frequency           111
#define Mem_I_ZeoVit_Duration            112
#define Mem_I_Debug2                      150
#define Mem_I_Debug3                      151
#define Mem_I_Debug4                      152

void setup()
{
    // This must be the first line
    ReefAngel.Init(); //Initialize controller
    ReefAngel.Use2014Screen(); // Let's use 2014 Screen
    ReefAngel.AddSalinityExpansion(); // Salinity Expansion Module
    ReefAngel.AddWaterLevelExpansion(); // Water Level Expansion Module
    // Ports toggled in Feeding Mode
    ReefAngel.FeedingModePorts = Port3Bit | Port4Bit;
    ReefAngel.FeedingModePortsE[0] = Port3Bit | Port4Bit;
    // Ports toggled in Water Change Mode
    ReefAngel.WaterChangePorts = Port7Bit;
    ReefAngel.WaterChangePortsE[0] = 0;
    // Ports toggled when Lights On / Off menu entry selected
    ReefAngel.LightsOnPorts = 0;
    ReefAngel.LightsOnPortsE[0] = 0;
    // Ports turned off when Overheat temperature exceeded
    ReefAngel.OverheatShutoffPorts = 0;
    ReefAngel.OverheatShutoffPortsE[0] = Port1Bit;
    // Use T1 probe as temperature and overheat functions
    ReefAngel.TempProbe = T1_PROBE;
    ReefAngel.OverheatProbe = T1_PROBE;

    // Feeeding and Water Change mode speed
    ReefAngel.DCPump.FeedingSpeed=0;
    ReefAngel.DCPump.WaterChangeSpeed=0;

    //Enable Salinity Temp Compensation
    ReefAngel.Salinity.SetCompensation(0);

    // Ports that are always on
    ReefAngel.Relay.On( Port3 );
    ReefAngel.Relay.On( Port4 );
    ReefAngel.Relay.On( Box1_Port3 );
    ReefAngel.Relay.On( Box1_Port4 );

    ////// Place additional initialization code below here
    
    //Set Auto Water Change to False
    //InternalMemory.write(Mem_B_Water_Change_Enabled,false);
    InternalMemory.write(Mem_B_AutoFeed_Enabled,false);
        
    ////// Place additional initialization code above here
}

void loop()
{
    ReefAngel.StandardHeater( Port1 );
    //Cycle zeovit reactor every 3 hours
    ReefAngel.Relay.Set(Port2,now()%21600<10800);
    ReefAngel.DosingPumpRepeat1( Port5 );
    ReefAngel.DosingPumpRepeat2( Port6 );
    ReefAngel.WaterLevelATO( Port7 );
    ReefAngel.DayLights( Box1_Port1 );
    ReefAngel.ActinicLights( Box1_Port2 );
    ReefAngel.DCPump.UseMemory = true;
    ReefAngel.DCPump.DaylightChannel = Sync;
    ReefAngel.DCPump.ActinicChannel = AntiSync;
    ////// Place your custom code below here
    //Enable wifi alert
    static WiFiAlert awcAlert;
    static WiFiAlert awcDeviateAlert;

    //check to see if the water levels are out of whack, if so, disable water change
    if ( ReefAngel.WaterLevel.GetLevel(0) >= InternalMemory.read(Mem_I_Water_Change_WL_High) && ReefAngel.WaterLevel.GetLevel(0)<= InternalMemory.read(Mem_I_Water_Change_WL_Low)) {
        awcAlert.Send("Sump Levels out of range.  AWC disabled!");
        ReefAngel.Relay.Override(Port8,0);
    }
        //set it back to auto if we are within range
    else {
        ReefAngel.Relay.Override(Port8,2);
    }

    //check to see if water changes are enabled, if they are, start the routine
    if (InternalMemory.read(Mem_B_Water_Change_Enabled)) {
        // Use Standard Light port to determine if we are scheduled for a water change
        ReefAngel.StandardLights(Port8,InternalMemory.read(Mem_I_Water_Change_On_Hour),InternalMemory.read(Mem_I_Water_Change_On_Minute),InternalMemory.read(Mem_I_Water_Change_Off_Hour),InternalMemory.read(Mem_I_Water_Change_Off_Minute));
    }
    
    //grab the water level at the begging of the awc to make sure it doesn't +/- too much water
    //if ( hour()==InternalMemory.read(Mem_I_Water_Change_On_Hour) && minute()==InternalMemory.read(Mem_I_Water_Change_On_Minute) && second()==0 ){
    //        awcStartLevel=ReefAngel.WaterLevel.GetLevel(0);            
    //}
    
    //if the water change port is on, we need to override the ATO port and set it to off
    if ( ReefAngel.Relay.Status(Port8) ) {
        ReefAngel.Relay.Override(Port7,0);
    //    // if we deviate more than 3% turn off the water change and alert
    //   if ( awcStartLevel-ReefAngel.WaterLevel.GetLevel(0) > 6 || awcStartLevel-ReefAngel.WaterLevel.GetLevel(0) < -6 ){
          
    //        awcDeviateAlert.Send("Sump level deviated greater than range during AWC!");
   //         InternalMemory.write(Mem_I_Debug,awcStartLevel);
   //         InternalMemory.write(Mem_I_Debug2,awcStartLevel-ReefAngel.WaterLevel.GetLevel(0));
   //         ReefAngel.Relay.Override(Port8,0);
        }
    //set it back to auto if the port is not on
    else {
        ReefAngel.Relay.Override(Port7,2);
    }

    //Feeding mode timer
    // http://forum.reefangel.com/viewtopic.php?f=12&t=3390&hilit=clear+override
    //http://forum.reefangel.com/viewtopic.php?f=12&t=1915&p=15149&hilit=schedule+feed+mode#p15149
    if (InternalMemory.read(Mem_B_AutoFeed_Enabled)) {
        if ( hour()==InternalMemory.read(Mem_I_AutoFeed_Hour) && minute()==InternalMemory.read(Mem_I_AutoFeed_Minute) && second()==0 ){
            ReefAngel.FeedingModeStart();
        }
    }
    //End Feeding mode timer


    //Random mode for Jabeo pumps
    //http://forum.reefangel.com/viewtopic.php?f=11&t=3873&p=32519&hilit=kalk+stir#p32519
    
    // Add random mode if we set to Mode to Custom in portal
    static int rmode;
    static boolean changeMode=true;


    // These are the modes we can cycle through. You can add more and even repeat...
    byte modes[] = { ReefCrest, Lagoon, Constant, TidalSwell, ShortPulse, LongPulse, Else, Gyre };

    if (now()%SECS_PER_DAY==0 || changeMode==true) { // Change at midnight or if controller rebooted
    rmode=random(100)%sizeof(modes); // Change the mode once per day to pick from our array
    changeMode=false;
    }

    // Set timer when in feeding mode
    static unsigned long feeding;
    if (ReefAngel.DisplayedMenu==FEEDING_MODE) feeding=now();

    if (now()-feeding<7200) {
      // Continue NTM for the 120 minutes
      ReefAngel.DCPump.UseMemory=false;
      ReefAngel.DCPump.Duration=InternalMemory.DCPumpDuration_read();
      ReefAngel.DCPump.Mode=NutrientTransport;
    } else if (now()%SECS_PER_DAY<43200 || now()%SECS_PER_DAY>=79200) { // 12pm / 10pm
      // Night mode (go to 30%)
      ReefAngel.DCPump.UseMemory=false;
      ReefAngel.DCPump.Duration=InternalMemory.DCPumpDuration_read();
      ReefAngel.DCPump.Mode=Constant;
      ReefAngel.DCPump.Speed=30;
    } else if (InternalMemory.DCPumpMode_read()==11) {
      // Custom Mode and nothing else going on
      ReefAngel.DCPump.UseMemory=false;
      ReefAngel.DCPump.Duration=InternalMemory.DCPumpDuration_read();
      ReefAngel.DCPump.Mode=modes[rmode];  // Put the mode to the random mode :)
      ReefAngel.DCPump.Speed=InternalMemory.DCPumpSpeed_read(); // Set speed from portal
    } else {
      ReefAngel.DCPump.UseMemory=true; // Will reset all values from memory
    }
    //End Random mode for pumps
    
    //Setup Web Portal Authentication
    //ReefAngel.Network.WifiAuthentication("replace:replace");
    
    //Register DDNS
    ReefAngel.DDNS("reeftank");
    
    ////// Place your custom code above here

    // This should always be the last line
    ReefAngel.Portal( "luckybaker4" );
    ReefAngel.ShowInterface();
}

