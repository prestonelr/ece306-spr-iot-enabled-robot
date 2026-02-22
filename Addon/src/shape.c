

#include <msp430.h>
#include <string.h>
#include "Core\lib\display.h"
#include "Core\lib\motor.h"
#include "Core\lib\ports.h"
#include "Core\lib\macros.h"
#include "Addon\lib\shape.h"

void ShapeUI_Init(void)
{
    ChangeDisplay("Shape =",0);
    ChangeDisplay("> Circle     ",1);
    ChangeDisplay("SW1 Next     ",2);
    ChangeDisplay("SW2 Select   ",3);
}

/* Call once per main loop cycle */
void ShapeUI_Task(void)
{
    static int state = 0;   /* 0=MENU, 1=RUN */
    static int opt   = 0;   /* 0=circle,1=fig8,2=tri */

    static unsigned char last1 = 0;
    static unsigned char last2 = 0;

    unsigned char sw1 = ((P4IN & SW1) == 0);
    unsigned char sw2 = ((P2IN & SW2) == 0);

    unsigned char edge1 = (sw1 && !last1);
    unsigned char edge2 = (sw2 && !last2);

    last1 = sw1;
    last2 = sw2;

    /* ---------------- MENU ---------------- */
    if(state == 0)
    {
        // SW1 = Next
        if(edge1)
        {
            opt++;
            if(opt > 2) opt = 0;

            ChangeDisplay("Shape =",0);
            if(opt==0) ChangeDisplay("> Circle     ",1);
            if(opt==1) ChangeDisplay("> Figure-8   ",1);
            if(opt==2) ChangeDisplay("> Triangle   ",1);

            ChangeDisplay("SW1 Next     ",2);
            ChangeDisplay("SW2 Select   ",3);
        }

        // SW2 = Select
        if(edge2)
        {
            if(opt==0) /* CIRCLE */
            {
                // Example: alternates straight + gentle arc
                // (Each char is 100mm with the distance-based coordinator)
                CommandCoordinator("/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>//>>/>>/>>/>>/>>/>>/>>//>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>//>>/>>/>>/>>/>>/>>/>>/");  // tight arc circle />>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>/>>//>>/>>/>>/>>/>>/>>/>>/
                ChangeDisplay("CIRCLE       ",0);
            }
            else if(opt==1) /* FIGURE 8 */
            {
                // Figure-8: arc right then arc left, repeat
                CommandCoordinator(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
                ChangeDisplay("FIGURE-8     ",0);
            }
            else /* TRIANGLE */
            {
                // Triangle: forward legs with hard turns between (no reverse)
                CommandCoordinator("FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF~~~~~~~~~~~~~~~~~~~~>>>>>>>>>>>>>~~~~~~~~~~~~~~~~~~~~FFFFFFF");
                ChangeDisplay("TRIANGLE     ",0);
            }

            ChangeDisplay("Running      ",1);
            ChangeDisplay("             ",2);
            ChangeDisplay("             ",3);

            state = 1;
        }
    }
    /* ---------------- RUN ---------------- */
    else
    {
        // advance one tick of command execution
        if(CommandCoordinator(NULL))
        {
            // Coordinator should already have stopped if script ends,
            // but forcing stop is fine:
            CommandCoordinator("~");      // immediate stop command (safe)
            CommandCoordinator(NULL);     // execute that stop tick

            // back to menu
            ChangeDisplay("Shape =",0);
            if(opt==0) ChangeDisplay("> Circle     ",1);
            if(opt==1) ChangeDisplay("> Figure-8   ",1);
            if(opt==2) ChangeDisplay("> Triangle   ",1);

            ChangeDisplay("SW1 Next     ",2);
            ChangeDisplay("SW2 Select   ",3);

            state = 0;
        }
    }
}

