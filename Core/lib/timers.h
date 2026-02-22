//--------------------------------------------------------------------
// ID bit divide
#define ID_1 (0x0040) /* /2 */
#define ID_2 (0x0080) /* /4 */
#define ID_3 (0x00c0) /* /8 */
#define ID__2 (0x0040) /* /2 */
#define ID__4 (0x0080) /* /4 */
#define ID__8 (0x00c0) /* /8 */
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//TAIDEX bit divider
#define TBIDEX_0 (0x0000) /* Divide by 1 */
#define TBIDEX_1 (0x0001) /* Divide by 2 */
#define TBIDEX_2 (0x0002) /* Divide by 3 */
#define TBIDEX_3 (0x0003) /* Divide by 4 */
#define TBIDEX_4 (0x0004) /* Divide by 5 */
#define TBIDEX_5 (0x0005) /* Divide by 6 */
#define TBIDEX_6 (0x0006) /* Divide by 7 */
#define TBIDEX_7 (0x0007) /* Divide by 8 */
#define TBIDEX__1 (0x0000) /* Divide by 1 */
#define TBIDEX__2 (0x0001) /* Divide by 2 */
#define TBIDEX__3 (0x0002) /* Divide by 3 */
#define TBIDEX__4 (0x0003) /* Divide by 4 */
#define TBIDEX__5 (0x0004) /* Divide by 5 */
#define TBIDEX__6 (0x0005) /* Divide by 6 */
#define TBIDEX__7 (0x0006) /* Divide by 7 */
#define TBIDEX__8 (0x0007) /* Divide by 8 */
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//Time sequence macros
#define Time_Sequence_Rate (50) // 50 millisecods
#define S1250 (1250/Time_Sequence_Rate) // 1.25 seconds
#define S1000 (1000/Time_Sequence_Rate) // 1 second
#define S750 (750/Time_Sequence_Rate) // 0.75 seconds
#define S500 (500/Time_Sequence_Rate) // 0.50 seconds
#define S250 (250/Time_Sequence_Rate) // 0.25 seconds
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//INTERVAL = Clock / [ID] / [T?xEX0] / ( 1 / Desired Time in seconds )
#define TB0CCR0_INTERVAL (25000) // 8,000,000/2/8/[1/50msec]
#define FIFTY_MS_COUNT 10
//-------------------------------------------------------------------


void Init_Timer_B0(void); //init the timer
