/*
.__                .__                            
|  |  __ __  _____ |__| ____   ____  __ __  ______
|  | |  |  \/     \|  |/    \ /  _ \|  |  \/  ___/
|  |_|  |  /  Y Y  \  |   |  (  <_> )  |  /\___ \ 
|____/____/|__|_|  /__|___|  /\____/|____//____  >
                 \/        \/                  \/ 
                           .__                       
  _____ ___.__. ____  ____ |  |   ____   ____ ___.__.
 /     <   |  |/ ___\/  _ \|  |  /  _ \ / ___<   |  |
|  Y Y  \___  \  \__(  <_> )  |_(  <_> ) /_/  >___  |
|__|_|  / ____|\___  >____/|____/\____/\___  // ____|
      \/\/         \/                 /_____/ \/     
*/
//-------------------------------------------------------------------------------------
// Luminous Mycology - a project by Bob Eells, October-November of 2013, with a lot of help from his meatspace and cyberspace friends.
// Special thanks to Penax, Chainsaw, Mutant Garage, Tanjent and BBQNinja

// use the code for anything you like, but don't trust it further than you can throw it.
// hobbies are only so successful at keeping planes in the sky.

// uses 3 ultrasound distance finders to set R, G, B values for a variety of devices.

#include <NewPing.h>

#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
//#define HOW_SMOOTH 3 // how many readings to average together in ping_median - REMOVED - performance impact was too high.  Ended up simply throwing out zeros below a variable threshold.
#define REVERSE_POLARITY 0 //  0 = less distance is more bright.  1 = more distance is more bright.
#define IGNORE_ZEROS 7 // how many 0 readings we should ignore before trusting them.  Set low to get a faster response to people leaving at the expense of glitchier display.

NewPing sonar_r(4, 7, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar_g(2, 8 , MAX_DISTANCE); 
NewPing sonar_b(13, 12, MAX_DISTANCE); 

//these hold number of cm for each sensor - red, green, blue.
unsigned int cm_r = 0; 
unsigned int cm_g = 0;
unsigned int cm_b = 0;

//zero-counts, tracking the number of consecutive 0s for any given pixel.  
//These sensors are glitchy and sometimes fail to hear the echo, but ping_median was too big a performance impact.  Rolled my own simple outlier discarder.
//for my purposes, I'm happy just throwing out the first n 0s I get.
unsigned int zc_r = 0;
unsigned int zc_g = 0;
unsigned int zc_b = 0;

//define our RGB pins
int r_pin = 10;
int g_pin = 5;
int b_pin = 6;

//preset_ret is used to detect a 999 - the code thrown when we detect a sensor reading while inside a preset program.
//this acts like a global goto and drops everything to go back to rendering sensor data.  It's a poor man's interrupt.
int preset_ret = 0;

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see debug info - so much debug info.
  pinMode(r_pin, OUTPUT); //initialize LED pins
  pinMode(g_pin, OUTPUT);  
  pinMode(b_pin, OUTPUT);    
  check_LEDs(r_pin, g_pin, b_pin);
}

void loop() {
  randomSeed(analogRead(0));      // There's a fair bit of randomness in here...may as well seed it with something randomish.
  delay(50);                      // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int uS_r = sonar_r.ping();//ping_median(HOW_SMOOTH); // Send ping, get ping time in microseconds (uS).
  unsigned int uS_g = sonar_g.ping();//_median(HOW_SMOOTH); // Send ping, get ping time in microseconds (uS).
  unsigned int uS_b = sonar_b.ping();//_median(HOW_SMOOTH); // Send ping, get ping time in microseconds (uS).
  
  //REVERSE_POLARITY controls whether the lights intensify as you get closer or as you get further from the sensors.
  if(REVERSE_POLARITY)
  {
    cm_r = (uS_r / US_ROUNDTRIP_CM);
    cm_g = (uS_g / US_ROUNDTRIP_CM);
    cm_b = (uS_b / US_ROUNDTRIP_CM);  
  }else{
    cm_r = MAX_DISTANCE-(uS_r / US_ROUNDTRIP_CM);
    cm_g = MAX_DISTANCE-(uS_g / US_ROUNDTRIP_CM);
    cm_b = MAX_DISTANCE-(uS_b / US_ROUNDTRIP_CM);
  
    if(cm_r == MAX_DISTANCE)
      cm_r = 0;
    if(cm_g == MAX_DISTANCE)
      cm_g = 0;
    if(cm_b == MAX_DISTANCE)
      cm_b = 0;  
  }
  
  //if we hit the case where there's no one within range of the sensors, then default down to pre-programmed patterns.
  if(zc_r == IGNORE_ZEROS && zc_g == IGNORE_ZEROS && zc_b == IGNORE_ZEROS)
  {
    //Run patterns - patterns available are defined below:
    //int blinks(int brightness, int delay_time, int fadeAmount, int rp, int gp, int bp) 
    //int randomized(int loops, int ms_del, int rp, int gp, int bp)
    //int omgp(int rp,int gp,int bp, int ms_del)
    //int alternator(int color_1[], int color_2[], int rp, int gp, int bp)
    //int fader(int color_1[], int color_2[], int steps, int rp, int gp, int bp, int ms_del)
    int color_1[] = {160,20,0};
    int color_2[] = {0,0,255};
    preset_ret = 1;
    
    //preset_ret = nice_fader(r_pin,g_pin,b_pin,20);
    if(preset_ret != 999 && (random(10) > 5))
    {  
      preset_ret = omgp_fader(r_pin,g_pin,b_pin,20);
    }
    if(preset_ret != 999 && (random(10) > 5))
    {            
      preset_ret = fader(color_1,color_2,500,r_pin,g_pin,b_pin,20);      
    }      
    if(preset_ret != 999 && (random(10) > 5))
    {        
      preset_ret = alternator(color_1,color_2,r_pin,g_pin,b_pin);
    }
    if(preset_ret != 999 && (random(10) > 5))
    {        
      preset_ret = randomized(20,20,r_pin,g_pin,b_pin);
    }
    if(preset_ret != 999 && (random(10) > 5))
    {        
      preset_ret = omgp(r_pin,g_pin,b_pin,500);
    }
    if(preset_ret != 999 && (random(10) > 5))
    {    
      preset_ret = randomized(200,1,r_pin,g_pin,b_pin);
    }
    if(preset_ret != 999 && (random(10) > 5))
    {
      preset_ret = blinks(0,50,20,r_pin,g_pin,b_pin);
    }    
  }
  
  Serial.println(String("Zeros: ")+zc_r+","+zc_g+","+zc_b+" - "+"Pings: "+cm_r+", "+cm_g+", "+cm_b);
  
  checkZerosAndWrite(r_pin,cm_r,zc_r);
  checkZerosAndWrite(g_pin,cm_g,zc_g);
  checkZerosAndWrite(b_pin,cm_b,zc_b);
}  

/*
              __    __                              
___________ _/  |__/  |_  ___________  ____   ______
\____ \__  \\   __\   __\/ __ \_  __ \/    \ /  ___/
|  |_> > __ \|  |  |  | \  ___/|  | \/   |  \\___ \ 
|   __(____  /__|  |__|  \___  >__|  |___|  /____  >
|__|       \/                \/           \/     \/ 

*/

//All patterns return 1 if completed successfully.  They return 999 if kicked out by a sensor detection.

int check_LEDs(int rp, int gp, int bp)
{
  lmWrite(rp,255);
  lmWrite(gp, 0);      
  lmWrite(bp, 0);       
  Serial.println(String("Checking red"));
  delay(1000);
  lmWrite(gp,255);
  lmWrite(rp, 0);      
  lmWrite(bp, 0);        
  Serial.println(String("Checking green"));
  delay(1000);
  lmWrite(bp,255);
  lmWrite(rp, 0);      
  lmWrite(gp, 0);        
  Serial.println(String("Checking blue"));
  delay(1000);
}

int blinks(int brightness, int delay_time, int fadeAmount, int rp, int gp, int bp)  { 
  for(int x=0;x<127;x++)
  {
    //set the modulo below to get at least a few pings a second.
    if((x % 2) == 0)
    {
      if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
      {
        return 999;
      }        
    }
    // set the brightness 
    lmWrite(rp, brightness);   
    lmWrite(gp, 0);      
    lmWrite(bp, 0);     
   
    delay(delay_time);  
  
    lmWrite(rp, 0);   
    lmWrite(gp, brightness);      
    lmWrite(bp, 0);      
    
    delay(delay_time);        
    
    lmWrite(rp, 0);   
    lmWrite(gp, 0);      
    lmWrite(bp, brightness);      
    
    delay(delay_time);  
    brightness = brightness + fadeAmount;

    Serial.println(String("blinks: ")+x);    
 }
 return 1; 
}


int randomized(int loops, int ms_del, int rp, int gp, int bp){
  float r,g,b;
  float r2,g2,b2;
  float temp_r,temp_g,temp_b;
  float dr,dg,db;
  int steps;  
  r=0;
  g=0;
  b=0;
  for(int y=0;y<loops;y++)
  {
    steps = random(256,1024);
    r2 = r;
    g2 = g;
    b2 = b;
  
    r = random(255);
    g = random(255);
    b = random(255);  
  
    dr = getDelta(r,r2,steps);
    dg = getDelta(g,g2,steps);
    db = getDelta(b,b2,steps);

    temp_r = r2;
    temp_g = g2;
    temp_b = b2;
        
    for(int x = 0;x<steps;x++)
    {
      if((x % 40) == 0)
      {
        Serial.println(String("Randomized: ")+y+", "+x);              
        if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
        {
          return 999;
        }        
      }
      temp_r += dr;
      temp_g += dg;
      temp_b += db;
      
      lmWrite(rp,temp_r);
      lmWrite(gp,temp_g);
      lmWrite(bp,temp_b);        
      
      delay(ms_del); 
    }  
  }
  return 1;
}

int omgp(int rp,int gp,int bp, int ms_del)
{
                //red,ora,yel,gre,blu,ind,vio
  int reds[] =   {255,160,255,  0,  0, 80,255};
  int greens[] = {  0, 20,255,255,  0, 20,  0};
  int blues [] = {  0,  0,  0,  0,255, 80,255};  
  
  for(int x = 0;x<7;x++)
  {
    Serial.println(String("Oh My God, Ponies!: ")+x);       
    if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
    {
      return 999;
    }     
    lmWrite(rp,reds[x]);
    lmWrite(gp,greens[x]);
    lmWrite(bp,blues[x]);        
    delay(ms_del); 
  }    
  return 1;
}

//here there be dragons - call at your own risk - this pattern is buggy and doesn't seem worth saving.
int nice_fader(int rp,int gp, int bp, int ms_del)
{
  int last_r,last_g,last_b = 0;
  int color_1[3],color_2[3];
  
  for(int r = 0;r < 261;r = r+20)
  {
    if(r == 260)
      r = 255;
    for(int g = 0;g < 261;g = g+20)
    {
      if(g == 260)
        g = 255;
      for(int b = 0;b < 261; b = b+20)
      {
        if(b == 260)
          b = 255;
              
        //repackage for fader()
        color_1[0] = last_r;
        color_1[1] = last_g;
        color_1[2] = last_b;
        
        color_2[0] = r;
        color_2[1] = g;
        color_2[2] = b;
        Serial.println(String("Nice Fader!: ")+r);     
        fader(color_2,color_1,20,rp,gp,bp,ms_del);        
        last_r = r;
        last_g = g;
        last_b = b;
      }
    }
  }
}

int omgp_fader(int rp,int gp,int bp, int ms_del)
{
                //vio,ind,blu,gre,yel,ora,red,vio
  int reds[]   = {255, 80,0  ,  0,255,160,255,255};
  int greens[] = {  0, 20,0  ,255,255, 20,  0,  0};
  int blues[]  = {255, 80,255,  0,  0,  0,  0,255};
  
  int color_1[3];
  int color_2[3];
  
  for(int x = 0;x<7;x++)
  {
    Serial.println(String("Oh My God, Ponies Fader!: ")+x);       
    if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
    {
      return 999;
    }     
    //repackage for fader()
    color_1[0] = reds[x];
    color_1[1] = greens[x];
    color_1[2] = blues[x];
    
    color_2[0] = reds[x+1];
    color_2[1] = greens[x+1];
    color_2[2] = blues[x+1];

    fader(color_2,color_1,800,rp,gp,bp,ms_del);
  }    
  return 1;
}

int alternator(int color_1[], int color_2[], int rp, int gp, int bp)
{
  int d = 1024;
  while(d > 1)
  {  
    Serial.println(String("Alternator: ")+d);       
    if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
    {
      return 999;
    }         
    lmWrite(rp,color_1[0]);
    lmWrite(gp,color_1[1]);
    lmWrite(bp,color_1[2]);
    delay(d);
    lmWrite(rp,color_2[0]);
    lmWrite(gp,color_2[1]);
    lmWrite(bp,color_2[2]);  
    delay(d);
    d = d/2;
  }
  while(d < 1025)
  {  
    Serial.println(String("Alternator: ")+d);       
    if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
    {
      return 999;
    }         
    lmWrite(rp,color_1[0]);
    lmWrite(gp,color_1[1]);
    lmWrite(bp,color_1[2]);
    delay(d);
    lmWrite(rp,color_2[0]);
    lmWrite(gp,color_2[1]);
    lmWrite(bp,color_2[2]);  
    delay(d);
    d = d*2;
  }  
  return 1;
}

int fader(int color_1[], int color_2[], int steps, int rp, int gp, int bp, int ms_del)
{
  float r,g,b,r2,g2,b2,dr,dg,db, temp_r,temp_g,temp_b;
  r = color_1[0];
  g = color_1[1];
  b = color_1[2];
  r2 = color_2[0];
  g2 = color_2[1];
  b2 = color_2[2];
  dr = getDelta(r,r2,steps);
  dg = getDelta(g,g2,steps);
  db = getDelta(b,b2,steps);
  temp_r = r2;
  temp_g = g2;
  temp_b = b2;
        
  for(int x = 0;x<steps;x++)
  {
    if((x % 40) == 0)
    {        
      if(checkPings(sonar_r,sonar_g,sonar_b) == 999)
      {
        return 999;
      }        
    }
    temp_r += dr;
    temp_g += dg;
    temp_b += db;
    Serial.print("fader: ");
    Serial.print(temp_r);
    Serial.print(",");
    Serial.print(temp_g);
    Serial.print(",");
    Serial.println(temp_b);   
    lmWrite(rp,temp_r);
    lmWrite(gp,temp_g);
    lmWrite(bp,temp_b);        
    
    delay(ms_del); 
  }  
  return 1;  
}

 /*

.__           .__                       
|  |__   ____ |  | ______   ___________ 
|  |  \_/ __ \|  | \____ \_/ __ \_  __ \
|   Y  \  ___/|  |_|  |_> >  ___/|  | \/
|___|  /\___  >____/   __/ \___  >__|   
     \/     \/     |__|        \/       
  _____                    __  .__                      
_/ ____\_ __  ____   _____/  |_|__| ____   ____   ______
\   __\  |  \/    \_/ ___\   __\  |/  _ \ /    \ /  ___/
 |  | |  |  /   |  \  \___|  | |  (  <_> )   |  \\___ \ 
 |__| |____/|___|  /\___  >__| |__|\____/|___|  /____  >
                 \/     \/                    \/     \/ 

*/

//write a pin based on our number of zeros.  Accepts the zero count by refernce so it can update as needed.
void checkZerosAndWrite(unsigned int pin, unsigned int cm, unsigned int &zc)
{
  if(cm == 0 && IGNORE_ZEROS > 0 && zc < IGNORE_ZEROS)
  {
    zc++;
  }else{
    lmWrite(pin, cm);   
    if(cm != 0)
      zc = 0;
  }
}

//heavily used for fades - finds the float value that you need to add to your pixel to move from color to color2 in a given number of steps.
float getDelta(float color, float color2, int steps)
{
  float delta = 0;
  if(color2 > color)
  {
    delta = ((color2 - color) / steps) * -1;
  }else{
    delta = ((color - color2) / steps);
  }
  return delta;  
}

void lmWrite(int pin,float val)
{
	//Programs were written for Common cathode, so values must adjust for common anode, so I added a wrapper around the analogWrite call.
        //Funny thing is, somewhere in the movement to transistors and multiple LEDs, I had to swap it back.  If the values are goofy,
        //swap next two lines to switch back...It's bad enough to add a subtract to every output, I'm not adding a conditional too.
	//analogWrite(255-pin,val);
        analogWrite(pin,(val));
}

int checkPings(NewPing sonar_r, NewPing sonar_g, NewPing sonar_b)
{
  int r = sonar_r.ping();
  int g = sonar_g.ping();
  int b = sonar_b.ping();
  if(r > 0 || g > 0 || b > 0)
  {
    return 999;
  }
  return 1;  
}
