/*
* PETSCII SHooter
*
* This game is an attempt to write a c64 petscii graphics shooter
*/

#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <peekpoke.h>


#define SCREEN_RAM ((char*)0x0400)
#define COLOUR_RAM ((char*)0xd800)
#define JOYSTICK1_PORT ((char*)0xdc01)
#define JOYSTICK2_PORT ((char*)0xdc00)

#define JOY_UP     0x1
#define JOY_DOWN   0x2
#define JOY_LEFT   0x4
#define JOY_RIGHT  0x8
#define JOY_BUTTON 0x10

unsigned char i = 0;
unsigned char j = 0;

unsigned char joy1, joy2;

#define MAX_PROJECTILES 5
typedef struct projectiles_type{
    unsigned char x[MAX_PROJECTILES];
    unsigned char y[MAX_PROJECTILES];
    unsigned char prev_x[MAX_PROJECTILES];
    unsigned char prev_y[MAX_PROJECTILES];
    unsigned char x_vel[MAX_PROJECTILES];
    unsigned char y_vel[MAX_PROJECTILES];
    unsigned char projectile_flag[MAX_PROJECTILES];
    unsigned char clear[MAX_PROJECTILES];
} s_projectiles;

typedef struct player_type{
    unsigned char x;
    unsigned char y;
    unsigned char prev_x;
    unsigned char prev_y;
} s_player;

#define MAX_ENEMIES 5
typedef struct enemies{
    unsigned char x[MAX_ENEMIES];
    unsigned char y[MAX_ENEMIES];
    unsigned char prev_x[MAX_ENEMIES];
    unsigned char prev_y[MAX_ENEMIES];
    unsigned char size_x[MAX_ENEMIES];
    unsigned char size_y[MAX_ENEMIES];
    unsigned char x_vel[MAX_ENEMIES];
    unsigned char y_vel[MAX_ENEMIES];
    unsigned char direction[MAX_ENEMIES];
    unsigned char live[MAX_ENEMIES];
    unsigned char explode_seq[MAX_ENEMIES];
    unsigned char explode_frames[MAX_ENEMIES];
    unsigned char hit_points[MAX_ENEMIES];
    unsigned char timeToUpdate[MAX_ENEMIES];
} s_enemies;

s_projectiles projectiles;
s_player player;
s_enemies enemies;

unsigned y_lut[25] = {1024, 1064, 1104, 1144, 1184, 1224, 1264, 1304, 1344, 1384, 1424,
                    1464, 1504 , 1544, 1584, 1624, 1664, 1704 ,1744, 1784 , 1824, 1864 , 1904 ,1944 , 1984 };

unsigned char prev_joy_button;

// Wait for raster line
void rasterWait(int cnt) {
    char i;

    for(i=0; i<cnt; i++) {
        while (VIC.rasterline < 250 || VIC.rasterline > 252);
    }
}
void clearPlayer()
{
    unsigned base_address = y_lut[player.prev_y] + player.prev_x;
    POKE(base_address, 0x20);
    POKE(base_address+1, 0x20);
    POKE(base_address+2, 0x20);
    POKE(base_address+40, 0x20);
    POKE(base_address+41, 0x20);
    POKE(base_address+42, 0x20);
    POKE(base_address+43, 0x20);
    POKE(base_address+80, 0x20);
    POKE(base_address+81, 0x20);
    POKE(base_address+82, 0x20);    
}
void drawPlayer()
{
    unsigned base_address = y_lut[player.y] + player.x;
    POKE(base_address, 0x70);
    POKE(base_address+1, 0x43);
    POKE(base_address+2, 0x6E);
    POKE(base_address+40, 0x42);
    POKE(base_address+41, 0x66);
    POKE(base_address+42, 0x6B);
    POKE(base_address+43, 0x40);
    POKE(base_address+80, 0x6D);
    POKE(base_address+81, 0x43);
    POKE(base_address+82, 0x7D);
}
void start_projectile()
{
    // find first unused projectiles
    for (i = 0; i < MAX_PROJECTILES; i++)
    {
        if(projectiles.projectile_flag[i] == 0)
        {
            projectiles.x[i] = player.x + 4;
            projectiles.y[i] = player.y + 1;
            projectiles.x_vel[i] = 1;
            projectiles.y_vel[i] = 0;
            projectiles.projectile_flag[i] = 1;
            break;
        }
    }
}
void clear_explosion(char x, char y)
{
    unsigned  base_address = y*40+ x;
    POKE(SCREEN_RAM + base_address, 0x20);
    POKE(SCREEN_RAM + base_address+1, 0x20);
    POKE(SCREEN_RAM + base_address+2, 0x20);
    POKE(SCREEN_RAM + base_address+40, 0x20);
    POKE(SCREEN_RAM + base_address+41, 0x20);
    POKE(SCREEN_RAM + base_address+42, 0x20);
    POKE(SCREEN_RAM + base_address+80, 0x20);
    POKE(SCREEN_RAM + base_address+81, 0x20);
    POKE(SCREEN_RAM + base_address+82, 0x20);    
}
void draw_explosion(char x, char y)
{
    //unsigned int base_address = y_lut[y]+ x;
    unsigned  base_address = y*40+ x;
    POKE(SCREEN_RAM + base_address, 0x2A);
    POKE(SCREEN_RAM + base_address+1, 0x20);
    POKE(SCREEN_RAM + base_address+2, 0x2A);
    POKE(SCREEN_RAM + base_address+40, 0x20);
    POKE(SCREEN_RAM + base_address+41, 0x2A);
    POKE(SCREEN_RAM + base_address+42, 0x20);
    POKE(SCREEN_RAM + base_address+80, 0x2A);
    POKE(SCREEN_RAM + base_address+81, 0x20);
    POKE(SCREEN_RAM + base_address+82, 0x2A);
}
void updateProjectilePositions()
{
    for (i = 0; i < MAX_PROJECTILES; i++)
    {
        if(projectiles.projectile_flag[i] == 1)
        {
            projectiles.prev_x[i] = projectiles.x[i];
            projectiles.prev_y[i] = projectiles.y[i];
            projectiles.x[i] = projectiles.x[i]+ projectiles.x_vel[i];
        }
    }
}
void drawProjectiles()
{
    for (i = 0; i < MAX_PROJECTILES; i++)
    {
        if(projectiles.projectile_flag[i] == 1)
        {
            unsigned base_address = y_lut[projectiles.prev_y[i]]+ projectiles.prev_x[i];
            // clear old projectile
            POKE(base_address, 0x20);

            if(projectiles.x[i] > 36)
            {
                projectiles.projectile_flag[i] = 0;
                base_address = y_lut[projectiles.y[i]]+ projectiles.x[i];
                POKE(base_address, 0x20);
            }
            else
            {
                base_address = y_lut[projectiles.y[i]]+ projectiles.x[i];
                POKE(base_address, 0x51);
            }
        }
        if(projectiles.clear[i])
        {
            unsigned base_address = y_lut[projectiles.y[i]]+ projectiles.x[i];
            POKE(base_address, 0x20);
            base_address = y_lut[projectiles.prev_y[i]]+ projectiles.prev_x[i];
            POKE(base_address, 0x20);
            projectiles.clear[i] = 0;
        }
    }
}
void clearEnemy()
{
    for(i = 0; i < MAX_ENEMIES; i++)
    {
        unsigned base_address = y_lut[enemies.prev_y[i]] + enemies.prev_x[i];
        POKE(base_address, 0x20);
        POKE(base_address+1, 0x20);
        POKE(base_address+2, 0x20);
        POKE(base_address+3, 0x20);
        POKE(base_address+40, 0x20);
        POKE(base_address+41, 0x20);
        POKE(base_address+42, 0x20);
        POKE(base_address+43, 0x20);
        POKE(base_address+80, 0x20);
        POKE(base_address+81, 0x20);
        POKE(base_address+82, 0x20);
        POKE(base_address+83, 0x20);
        POKE(base_address+120, 0x20);
        POKE(base_address+121, 0x20);
        POKE(base_address+122, 0x20);
        POKE(base_address+123, 0x20);
        POKE(base_address+160, 0x20);
        POKE(base_address+161, 0x20);
        POKE(base_address+162, 0x20);
        POKE(base_address+163, 0x20); 
    }   
}
void drawEnemy()
{
    for(i = 0; i < MAX_ENEMIES; i++)
    {
        
        if(enemies.explode_seq[i] == 1 )
        {
            if(enemies.explode_frames[i] < 20)
            {
                draw_explosion(enemies.x[i], enemies.y[i]); 
                enemies.explode_frames[i]++;
            }
            else
            {
                clear_explosion(enemies.x[i], enemies.y[i]);
                enemies.explode_seq[i] = 0;
                enemies.explode_frames[i] = 0;
            }

        }
        else if (enemies.live[i] == 1 )
        {
            unsigned base_address = y_lut[enemies.y[i]] + enemies.x[i];
            POKE(base_address, 0x20);
            POKE(base_address+1, 0x70);
            POKE(base_address+2, 0x40);
            POKE(base_address+3, 0x6E);
            POKE(base_address+40, 0x20);
            POKE(base_address+41, 0x5D);
            POKE(base_address+42, 0x41);
            POKE(base_address+43, 0x5D);
            POKE(base_address+80, 0x40);
            POKE(base_address+81, 0x73);

            POKE(base_address+82, 0x30+ enemies.hit_points[i]);

            POKE(base_address+83, 0x5D);
            POKE(base_address+120, 0x20);
            POKE(base_address+121, 0x5D);
            POKE(base_address+122, 0x41);
            POKE(base_address+123, 0x5D);
            POKE(base_address+160, 0x20);
            POKE(base_address+161, 0x6D);
            POKE(base_address+162, 0x40);
            POKE(base_address+163, 0x7D);
        }
    }
    

    
}

void readJoysticks()
{
    joy1 = PEEK(JOYSTICK1_PORT);
    joy2 = PEEK(JOYSTICK2_PORT);
}
unsigned char readJoysticks_mainmenu()
{
    joy2 = PEEK(JOYSTICK2_PORT);
    if(!(joy2 & JOY_BUTTON))
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}
void drawMenu()
{
    gotoxy(10,10);
    cprintf("PETSCII SHOOTER!");
    gotoxy(10,11);
    cprintf("PRESS BUTTON ON JOY2 TO START");
}
void MainMenu()
{
    drawMenu();
    while(1)
    {
        if(readJoysticks_mainmenu())
        {
            break;
        }
    }
}
void updatePlayerPosition()
{
    if(!(joy2 & JOY_UP) )
    {
        if(player.y > 0)
        {
            player.y = player.y - 1;
        }
        
    }
        
    if(!(joy2 & JOY_DOWN) )
    {
        if(player.y < 22)
        {
            player.y = player.y +1;
        }
    }
        
    if(!(joy2 & JOY_LEFT))   
    {
        if(player.x > 1)
        {
            player.x = player.x - 1;
        }
    }  
        
    if(!(joy2 & JOY_RIGHT))
    {
        
        if(player.x < 24)
        {
            player.x = player.x + 1;
        }
    }
    // only fire new projectile when the joystick button is pressed down    
    if(!(joy2 & JOY_BUTTON) && prev_joy_button)
    {
        start_projectile();
    }
    prev_joy_button = (joy2 & JOY_BUTTON);
}
unsigned char enemy_direction = 0;
void updateEnemyPositions()
{
    bordercolor (COLOR_GREEN);
    for(i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies.live[i] == 1)
        {
            if ( enemies.timeToUpdate[i] > 5)
            {
                
                if(enemies.y[i] >19)
                    enemies.direction[i] = 1;
                if(enemies.y[i] <3)
                    enemies.direction[i] = 0;
                
                if(enemies.direction[i])
                    enemies.y[i] = enemies.y[i] - enemies.y_vel[i];
                else
                    enemies.y[i] = enemies.y[i] + enemies.y_vel[i];
                
                 enemies.timeToUpdate[i]= 0;
            }
        }
        enemies.timeToUpdate[i]++;
    }

}
void checkForCollisions()
{
    for(i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies.live[i])
        {
            for( j=0; j < MAX_PROJECTILES; j++ )
            {
                // check that projectile is live
                if(projectiles.projectile_flag[j])
                {
                    if( projectiles.x[j] >= enemies.x[i] && projectiles.x[j] <=  enemies.x[i] + enemies.size_x[i])
                    {
                        if( projectiles.y[j] >= enemies.y[i] && projectiles.y[j] <= enemies.y[i] + enemies.size_y[i])
                        {
                            bordercolor (COLOR_RED);
                            // disable projectile
                            projectiles.projectile_flag[j] = 0;
                            projectiles.clear[j] = 1;
                            if(enemies.hit_points[i] > 0)
                            {
                                enemies.hit_points[i]--;
                                if( enemies.hit_points[i] == 0)
                                {
                                    enemies.live[i] = 0;
                                    enemies.explode_seq[i] = 1;
                                }
                            }
                            
                                
                        }
                    }
                }
            }
        }
    }
} 

void updateScore()
{

}
void updateGameLogic()
{
    updatePlayerPosition();
    updateEnemyPositions();
    updateProjectilePositions();
    checkForCollisions();
    updateScore();
}
void drawScreen()
{
    clearPlayer();
    drawPlayer();
    clearEnemy();
    drawEnemy();
    drawProjectiles();
    player.prev_x = player.x;
    player.prev_y = player.y;
    for(i = 0; i < MAX_ENEMIES; i++)
    {
        enemies.prev_x[i] = enemies.x[i];
        enemies.prev_y[i] = enemies.y[i];
    }
}
// petscii/uppercase mode 
#define PETSCII_UPPERCASE_MODE 53272u
int main(void)
{
    /* Set screen colors */
    textcolor (COLOR_GREEN);
    bordercolor (COLOR_GREEN);
    bgcolor (COLOR_BLACK);
    player.x =5; 
    player.y =5;
    enemies.x[0] = 30;
    enemies.y[0] = 5;
    enemies.live[0] = 1;
    enemies.hit_points[0] = 5;
    enemies.size_x[0] = 4;
    enemies.size_y[0] = 5;
    enemies.x_vel[0] = 1;
    enemies.y_vel[0] = 1;
    enemies.direction[0] = 1;
    enemies.x[1] = 20;
    enemies.y[1] = 15;
    enemies.live[1] = 1;
    enemies.hit_points[1] = 6;
    enemies.size_x[1] = 4;
    enemies.size_y[1] = 5;
    enemies.x_vel[1] = 1;
    enemies.y_vel[1] = 1;
    enemies.direction[1] = 1;
    player.prev_x = player.x;
    player.prev_y = player.y;

    /* Clear the screen, put cursor in upper left corner */
    clrscr ();

    MainMenu();

    clrscr ();
    
    //cgetc();
    POKE(PETSCII_UPPERCASE_MODE,21); // use Uppercase/Petscii mode
    while(1)
    {
        rasterWait(3);
        readJoysticks();
        updateGameLogic();
        drawScreen();
    }
    return 0;
}