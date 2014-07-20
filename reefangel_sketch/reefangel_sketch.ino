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
#include <ReefAngel.h>

////// Place global variable code below here


////// Place global variable code above here


void setup()
{
    // This must be the first line
    ReefAngel.Init();  //Initialize controller
    ReefAngel.Use2014Screen();  // Let's use 2014 Screen 
    ReefAngel.AddSalinityExpansion();  // Salinity Expansion Module
    ReefAngel.AddWaterLevelExpansion();  // Water Level Expansion Module
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
    

    ////// Place additional initialization code above here
}

void loop()
{
    ReefAngel.StandardHeater( Port1 );
    ReefAngel.StandardHeater( Port2 );
    ReefAngel.DosingPumpRepeat1( Port5 );
    ReefAngel.DosingPumpRepeat2( Port6 );
    ReefAngel.WaterLevelATO( Port7 );
    ReefAngel.DayLights( Box1_Port1 );
    ReefAngel.ActinicLights( Box1_Port2 );
    ReefAngel.DCPump.UseMemory = true;
    ReefAngel.DCPump.DaylightChannel = Sync;
    ReefAngel.DCPump.ActinicChannel = AntiSync;
    ////// Place your custom code below here
    //Run water change port for 1 hour from 7 to 8
    ReefAngel.StandardLights( Port8,19,0,20,0 )

    ////// Place your custom code above here

    // This should always be the last line
    ReefAngel.Portal( "luckybaker4" );
    ReefAngel.ShowInterface();
}