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
    unsigned char live[MAX_ENEMIES];
    unsigned char explode_seq[MAX_ENEMIES];
} s_enemies;

s_projectiles projectiles;
s_player player;
s_enemies enemies;

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
    unsigned int base_address = player.prev_y * 40 + player.prev_x;
    POKE(SCREEN_RAM + base_address, 0x20);
    POKE(SCREEN_RAM + base_address+1, 0x20);
    POKE(SCREEN_RAM + base_address+2, 0x20);
    POKE(SCREEN_RAM + base_address+40, 0x20);
    POKE(SCREEN_RAM + base_address+41, 0x20);
    POKE(SCREEN_RAM + base_address+42, 0x20);
    POKE(SCREEN_RAM + base_address+43, 0x20);
    POKE(SCREEN_RAM + base_address+80, 0x20);
    POKE(SCREEN_RAM + base_address+81, 0x20);
    POKE(SCREEN_RAM + base_address+82, 0x20);    
}
void drawPlayer()
{
    unsigned int base_address = player.y * 40 + player.x;
    POKE(SCREEN_RAM + base_address, 0x70);
    POKE(SCREEN_RAM + base_address+1, 0x43);
    POKE(SCREEN_RAM + base_address+2, 0x6E);
    POKE(SCREEN_RAM + base_address+40, 0x42);
    POKE(SCREEN_RAM + base_address+41, 0x66);
    POKE(SCREEN_RAM + base_address+42, 0x6B);
    POKE(SCREEN_RAM + base_address+43, 0x40);
    POKE(SCREEN_RAM + base_address+80, 0x6D);
    POKE(SCREEN_RAM + base_address+81, 0x43);
    POKE(SCREEN_RAM + base_address+82, 0x7D);
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
void draw_explosion(char x, char y)
{
    unsigned int base_address = y * 40 + x;
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
void drawProjectiles()
{
    
    //POKE(SCREEN_RAM + base_address+44, 0x51);
    for (i = 0; i < MAX_PROJECTILES; i++)
    {
        if(projectiles.projectile_flag[i] == 1)
        {
            unsigned int base_address = projectiles.y[i]* 40 + projectiles.x[i];
            // clear old projectile
            POKE(SCREEN_RAM + base_address, 0x20);
            // update new position
            projectiles.x[i] = projectiles.x[i]+ projectiles.x_vel[i];
            if(projectiles.x[i] > 36)
            {
                projectiles.projectile_flag[i] = 0;
                base_address = projectiles.y[i]* 40 + projectiles.x[i];
                POKE(SCREEN_RAM + base_address, 0x20);
                draw_explosion(projectiles.x[i], projectiles.y[i]-1);
            }
            else
            {
                base_address = projectiles.y[i]* 40 + projectiles.x[i];
                POKE(SCREEN_RAM + base_address, 0x51);
            }
        }
    }
}
void clearEnemy()
{
    unsigned int base_address = enemies.prev_y[0] * 40 + enemies.prev_x[0];
    POKE(SCREEN_RAM + base_address, 0x20);
    POKE(SCREEN_RAM + base_address+1, 0x20);
    POKE(SCREEN_RAM + base_address+2, 0x20);
    POKE(SCREEN_RAM + base_address+3, 0x20);
    POKE(SCREEN_RAM + base_address+40, 0x20);
    POKE(SCREEN_RAM + base_address+41, 0x20);
    POKE(SCREEN_RAM + base_address+42, 0x20);
    POKE(SCREEN_RAM + base_address+43, 0x20);
    POKE(SCREEN_RAM + base_address+80, 0x20);
    POKE(SCREEN_RAM + base_address+81, 0x20);
    POKE(SCREEN_RAM + base_address+82, 0x20);
    POKE(SCREEN_RAM + base_address+83, 0x20);
    POKE(SCREEN_RAM + base_address+120, 0x20);
    POKE(SCREEN_RAM + base_address+121, 0x20);
    POKE(SCREEN_RAM + base_address+122, 0x20);
    POKE(SCREEN_RAM + base_address+123, 0x20);
    POKE(SCREEN_RAM + base_address+160, 0x20);
    POKE(SCREEN_RAM + base_address+161, 0x20);
    POKE(SCREEN_RAM + base_address+162, 0x20);
    POKE(SCREEN_RAM + base_address+163, 0x20);    
}
void drawEnemy()
{
    unsigned int base_address = enemies.y[0] * 40 + enemies.x[0];
    POKE(SCREEN_RAM + base_address, 0x20);
    POKE(SCREEN_RAM + base_address+1, 0x70);
    POKE(SCREEN_RAM + base_address+2, 0x40);
    POKE(SCREEN_RAM + base_address+3, 0x6E);
    POKE(SCREEN_RAM + base_address+40, 0x20);
    POKE(SCREEN_RAM + base_address+41, 0x5D);
    POKE(SCREEN_RAM + base_address+42, 0x41);
    POKE(SCREEN_RAM + base_address+43, 0x5D);
    POKE(SCREEN_RAM + base_address+80, 0x40);
    POKE(SCREEN_RAM + base_address+81, 0x73);
    POKE(SCREEN_RAM + base_address+82, 0x53);
    POKE(SCREEN_RAM + base_address+83, 0x5D);
    POKE(SCREEN_RAM + base_address+120, 0x20);
    POKE(SCREEN_RAM + base_address+121, 0x5D);
    POKE(SCREEN_RAM + base_address+122, 0x41);
    POKE(SCREEN_RAM + base_address+123, 0x5D);
    POKE(SCREEN_RAM + base_address+160, 0x20);
    POKE(SCREEN_RAM + base_address+161, 0x6D);
    POKE(SCREEN_RAM + base_address+162, 0x40);
    POKE(SCREEN_RAM + base_address+163, 0x7D);
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
unsigned char timeToUpdate = 0;
void updateEnemyPositions()
{
    timeToUpdate = timeToUpdate + 1;
    if ( timeToUpdate > 5)
    {
        if(enemies.y[0] >19)
            enemy_direction = 1;
        if(enemies.y[0] <3)
            enemy_direction = 0;
        
        if(enemy_direction)
            enemies.y[0] = enemies.y[0] -1;
        else
            enemies.y[0] = enemies.y[0] + 1;
        
        timeToUpdate = 0;
    }
}
void updateProjectilePositions()
{

}
void checkForCollisions()
{

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
    enemies.prev_x[0] = enemies.x[0];
    enemies.prev_y[0] = enemies.y[0];
}
int main(void)
{
    /* Set screen colors */
    textcolor (COLOR_GREEN);
    bordercolor (COLOR_GREEN);
    bgcolor (COLOR_BLACK);
    player.x =5; 
    player.y =5;
    enemies.x[0] = 30;
    enemies.y[0] = 10;
    player.prev_x = player.x;
    player.prev_y = player.y;

    /* Clear the screen, put cursor in upper left corner */
    clrscr ();

    MainMenu();

    clrscr ();
    
    //cgetc();
    POKE(53272,21); // use Uppercase/Petscii mode
    while(1)
    {
        rasterWait(3);
        readJoysticks();
        updateGameLogic();
        drawScreen();
    }
    return 0;
}