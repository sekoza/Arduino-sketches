#include <TVout.h>
#include <video_gen.h>

/**************************************************************************
 *  
 *  Hardware:
 *  
 *  D9 --[ 1 k ohm resistor]--+--> (+)TVout(-) <-+
 *  D7 --[ 470 ohm resistor]--+                  |
 *  GND------------------------------------------+
 *  
 *  +5V --- POTENTIOMETER  ----- GND
 *            100 kohm
 *                |
 *  A0 -----------+
 *  
 *  D12 ------ PUSH BUTTON ----- GND
 *  
 *  D11 -------- SPEAKER ------- GND 
 *  
 **************************************************************************/

#include <TVout.h>
#include "bitmap.h"
#include <TVoutfonts/fontALL.h>

//#define DEBUG_ON

#define BUTTON_START 12   //digital 12 - button 
#define WHEEL_MOVE   A0  //Analog   A0 - potentiometer


#define BAT_Y_POS  78  //THE BAT MUST BE 2 PIXEL HIGH, THE TWO LAST ROWS OF THE BMP

#define TXT_Y_POS  90  //Lives and Score position

#define BRK_Y_POS  4       //Bricks initial Y Position
#define BRK_X_POS  1       //Bricks initial X Position

#define BAL_X_MIN  1         //ball min. X Position
#define BAL_X_MAX  125       //ball max. X Position
#define BAL_Y_MIN  0         //ball min. Y Position
#define BAL_Y_MAX    BAT_Y_POS+8      //ball max. Y Position 

#define BAT_Y_POS  79        //bat fixed Y position

TVout TV;

int gamSt = 0;  //0 = menu, 1 = in game, 2 = game over


int batPosX = 0; //bat position
int batOldX = 0; //bat old position

int balPosX = 0; //ball X position
int balPosY = 0; //ball Y position

//----------------------------------------------------------------------------GAME SETUP---|
#define BAL_SPEED  25      //BALL initial speed higher value, slower ball
#define NUM_LIVES   5      //balls to play

//if you want to change the bat width, change te following part.
//NOTE THAT YOU COULD NEED TO SLIGHTLY MODIFY THE RATIO_WHEEL, DEPENDING ON
//YOUR ARDUINO BOARD AND POTENTIOMETER. 

#define RATIO_WHEEL  8.65  //7.65  //Ratio Video X resolution Vs. potentiometer resolution
                                   //the lower the value, the more the bat will go on the right 
#define BAT_SIZE  8    
#define BAT_SHAPE bat8 //bitmap 8 pixels width (bat)
#define BNN_SHAPE bnn8 //bitmap 8 pixels width (delete bat)

//#define RATIO_WHEEL  8.95   //Ratio Video X resolution Vs. potentiometer resolution
                              //the lower the value, the more the bat will go on the right 
//#define BAT_SIZE  16    
//#define BAT_SHAPE bat16 //bitmap 16 pixels width (bat)
//#define BNN_SHAPE bnn16 //bitmap 16 pixels width (delete bat)
//-----------------------------------------------------------------------------------------|


int balDirX = 1; //ball X direction (1 ; -1)
int balDirY = 1; //ball Y direction (1 ; -1)
int balsped = BAL_SPEED ; //ball initial speed

 
//bricks matrix at level begin -  4...2 full brick, 1 = half brick, 0 = no brick
int bMatrix[4][14] = { {4,4,4,4,4,4,4,4,4,4,4,4,4,4},
                       {4,4,4,4,4,4,4,4,4,4,4,4,4,4},
                       {4,4,4,4,4,4,4,4,4,4,4,4,4,4},
                       {4,4,4,4,4,4,4,4,4,4,4,4,4,4} };
                       

//===================================================================================== RESET_BRICKS
void reset_bricks() {  
  
  bMatrix[0][0]=4; bMatrix[0][1]=4; bMatrix[0][2]=4; bMatrix[0][3]=4; bMatrix[0][4]=4; bMatrix[0][5]=4; bMatrix[0][6]=4; bMatrix[0][7]=4; bMatrix[0][8]=4; bMatrix[0][9]=4; bMatrix[0][10]=4; bMatrix[0][11]=4; bMatrix[0][12]=4; bMatrix[0][13]=4;
  bMatrix[1][0]=4; bMatrix[1][1]=4; bMatrix[1][2]=4; bMatrix[1][3]=4; bMatrix[1][4]=4; bMatrix[1][5]=4; bMatrix[1][6]=4; bMatrix[1][7]=4; bMatrix[1][8]=4; bMatrix[1][9]=4; bMatrix[1][10]=4; bMatrix[1][11]=4; bMatrix[1][12]=4; bMatrix[1][13]=4;
  bMatrix[2][0]=4; bMatrix[2][1]=4; bMatrix[2][2]=4; bMatrix[2][3]=4; bMatrix[2][4]=4; bMatrix[2][5]=4; bMatrix[2][6]=4; bMatrix[2][7]=4; bMatrix[2][8]=4; bMatrix[2][9]=4; bMatrix[2][10]=4; bMatrix[2][11]=4; bMatrix[2][12]=4; bMatrix[2][13]=4;
  bMatrix[3][0]=4; bMatrix[3][1]=4; bMatrix[3][2]=4; bMatrix[3][3]=4; bMatrix[3][4]=4; bMatrix[3][5]=4; bMatrix[3][6]=4; bMatrix[3][7]=4; bMatrix[3][8]=4; bMatrix[3][9]=4; bMatrix[3][10]=4; bMatrix[3][11]=4; bMatrix[3][12]=4; bMatrix[3][13]=4;
}   


//===================================================================================== DRAW_BRICKS
int draw_bricks() {
  
  int valret = 0  ;
  
    
    for (int j=0;j<4;j++) {
       for (int i=0;i<14;i++) {                  

           if (bMatrix[j][i] >0) valret++;
         
           if (bMatrix[j][i] == 4) TV.bitmap(BRK_X_POS+(i*9),BRK_Y_POS+(j*9),bk4); 
           if (bMatrix[j][i] == 3) TV.bitmap(BRK_X_POS+(i*9),BRK_Y_POS+(j*9),bk3); 
           if (bMatrix[j][i] == 2) TV.bitmap(BRK_X_POS+(i*9),BRK_Y_POS+(j*9),bk2); 
           if (bMatrix[j][i] == 1) TV.bitmap(BRK_X_POS+(i*9),BRK_Y_POS+(j*9),bk1);  
           if (bMatrix[j][i] == 0) TV.bitmap(BRK_X_POS+(i*9),BRK_Y_POS+(j*9),bk0); //blank (no brick)
           
       }      
    }   
    return valret;  //How many bricks left to break 
}

//===================================================================================== SETUP
void setup() {

    TV.begin(_PAL); //128x96 default
   
    pinMode(WHEEL_MOVE,INPUT);     
    pinMode(BUTTON_START,INPUT_PULLUP);   

}

//===================================================================================== LOOP
void loop() {
 
  unsigned long mill = millis() ;  //delay ball 
   
  int batMove = 0;
  int butStart = 1; 
  int Score = 0;
  int lives = NUM_LIVES;
  int shpMove = 0;
  int shpPosX = 0;

  //-----------------------------------------------------------------------------------------------------MENU  
              
  if (gamSt == 0){   //MENU

        TV.clear_screen();           
        TV.bitmap(0,  0, logo);
        TV.bitmap(0, 32, start);   
        delay(500);     

        butStart = 1; 
        while ( butStart == 1){                                     
           butStart = digitalRead(BUTTON_START);      
         }

         Score = 0; 
         lives = NUM_LIVES; 
         gamSt = 1;           
   }
    
 
 //-----------------------------------------------------------------------------------------------------IN GAME
 
  while (gamSt == 1) {  // IN GAME
   
    
       //Score = 0; NOT HERE!
       //lives = NUM_LIVES; NOT HERE!
         
         batMove = analogRead(WHEEL_MOVE); //bat  move
         batPosX = batMove / RATIO_WHEEL ; 
         batOldX = batPosX ;
               
         TV.clear_screen();
         reset_bricks() ;
         int bricksleft = draw_bricks();
         TV.bitmap(batPosX, BAT_Y_POS, BAT_SHAPE); 
         TV.select_font(font4x6);
         TV.draw_line(0,TXT_Y_POS-2,127,TXT_Y_POS-2,WHITE);
         TV.print(122,TXT_Y_POS,lives);           
         TV.print(  0,TXT_Y_POS,Score);
         delay(1000);
         
         balPosX = batPosX + BAT_SIZE/2 ; //ball X position
         balPosY = BAT_Y_POS-1; //ball Y position
         balDirX =  1; //ball X direction (1 ; -1)
         balDirY = -1; //ball Y direction (1 ; -1)
         balsped = BAL_SPEED ; //ball initial speed
         mill = millis() ;
                   
         //-----------------------------------------------------------------------------------------------------Main Cycle-begin
         
         while (gamSt == 1 && bricksleft >0) { 
              
            
            //-------------------------------------------------------------Bat-begin-
    
            batMove = analogRead(WHEEL_MOVE); //bat move                
            
            batPosX = batMove / RATIO_WHEEL ; 
    
            if (batPosX -1 == batOldX || batPosX + 1 == batOldX) batPosX = batOldX; //Avoiding flickering 
            
            if (batOldX != batPosX) {
                TV.bitmap(batOldX, BAT_Y_POS, BNN_SHAPE); 
                TV.bitmap(batPosX, BAT_Y_POS, BAT_SHAPE);         
                batOldX = batPosX;
            }
            
            //-------------------------------------------------------------Bat-end--- 
            
                    
            //-------------------------------------------------------------Ball-move-                
    
            if ( mill + balsped  < millis() ) {
    
               TV.select_font(font4x6);
               TV.draw_line(0,TXT_Y_POS-2,127,TXT_Y_POS-2,WHITE);
               TV.print(122,TXT_Y_POS,lives);           
               TV.print(  0,TXT_Y_POS,Score);
               
               TV.set_pixel(balPosX,balPosY,BLACK);  //before to redraw the bricks
               
               if ( TV.get_pixel( balPosX+balDirX , balPosY+balDirY ) == WHITE ) {
                                    
                  if (balPosY < 36+BRK_Y_POS){  //hit a brick
        
                      int brickX = (int)( (balPosX+balDirX-BRK_X_POS  ) / 9 );   
                      int brickY = (int)( (balPosY+balDirY-BRK_Y_POS  ) / 9 );                     
        
                      bMatrix[brickY][brickX] -= 1;   
                      if (bMatrix[brickY][brickX] > 0) TV.tone(2000,50); else TV.tone(3000,50); 
                      
                      bricksleft = draw_bricks(); 
    
                      //like in the original game, after 4 bricks broken the ball goes faster
                      if (bricksleft +3  < 14*4 && balsped == BAL_SPEED) balsped -= 10;   //14*4 = total bricks                                  
                      
                      Score++;
                      
                      TV.print(  0,TXT_Y_POS,Score);
                      
                      //if the balPosX starting from 1 -instead of 0- can be divided by 9, means the ball is in the empty column between two bricks. 
                      //so it will change the X direction instead of Y
                      if ( (balPosX-BRK_X_POS+1) % 9 == 0) {
                        balDirX *= -1;
                      }
                      else {
                        balDirY *= -1;
                      }                 
                                         
                  }      
                  else if (balPosY  < BAL_Y_MAX) { //the ball hit the bat                 
                      
                     //if the bat hit the ball with the lateral side, the ball get a bigger angle and come back from the incoming direction                     
                     if (balPosX+balDirX == batPosX) balDirX =-2; 
                        else if (balPosX+balDirX == batPosX + BAT_SIZE -1) balDirX = 2;                      
                             else if (balDirX ==2 || balDirX == -2) balDirX = balDirX /2;
                                  
                                  
                      balDirY =-1;
                      
                      TV.tone(1000,50);
                  }
                  
               }           
               else {  //the ball hit nothing            
                 if (balPosX + balDirX > BAL_X_MAX || balPosX + balDirX < BAL_X_MIN) {balDirX *= -1;} 
                 if (balPosY + balDirY < BAL_Y_MIN) {balDirY *= -1;} 
    
                 if (balPosY  >= BAL_Y_MAX) {  //missed ball
                  
                   TV.set_pixel(balPosX,balPosY,BLACK); //remove this line if you don't like to see the black dot on the white line where the ball gone out.
    
            #ifdef DEBUG_ON               
                   balDirY = -1; 
            #else
                   lives--;
                   
                   TV.tone(200,500);
                   delay(2000);
               
                   TV.print(122,TXT_Y_POS,lives);
    
                   balPosX = batPosX + BAT_SIZE/2 ; //ball X position
                   balPosY = BAT_Y_POS-1; //ball Y position
                   balDirX =  1; //ball X direction (1 ; -1)
                   balDirY = -1; //ball Y direction (1 ; -1)
             #endif       
    
                   if (lives == 0) {
                        gamSt = 2; 
                        TV.tone(500,50); delay(60);    
                        TV.tone(400,50); delay(60);
                        TV.tone(300,50); delay(60);
                        TV.tone(200,50); delay(60);
                        TV.tone(100,50); delay(60);
                   }
                   
                 }    
                
               }
               
               if (balPosY == 45+BRK_Y_POS) draw_bricks();  //needed to refresh the bricks from garbage that sometimes appears
    
               //TV.set_pixel(balPosX,balPosY,BLACK); //must to be done before to redraw the bricks to avoid the garbage
               balPosX += balDirX;
               balPosY += balDirY;           
               TV.set_pixel(balPosX,balPosY,WHITE);
                        
               mill = millis() ;           
            }
            
            
         //-----------------------------------------------------------------------------------------------------Main Cycle-end--- /
          
         } //while gamest == 1 and bricksleft >0
         
   }  //while (gamSt == 1)

      //-----------------------------------------------------------------------------------------------------GAME OVER 
      if (gamSt == 2){  //GAME OVER
        gamSt = 0;     
        TV.clear_screen();           
        TV.bitmap(0,  0, logo);
        TV.bitmap(0, 32, over); 
        TV.select_font(font6x8);
        TV.print(40,TXT_Y_POS,"SCORE:"); 
        TV.print(76,TXT_Y_POS,Score);  

        delay(1000);

        butStart = 1;
        while ( butStart == 1){                                     
           butStart = digitalRead(BUTTON_START); //cannon shot    
         }
      } //if (gamSt == 2)
 } //loop   