#include "Debug/SUDebug.h"
#include "HAL/IConsoleManager.h"

#if ENABLE_DRAW_DEBUG
static TAutoConsoleVariable<int32> CVarDebugAI(
	TEXT("su.DebugAI"), 0,
	TEXT("1 = draw horde grid cells (green to red by crowding), separation (yellow: circles touch when a pair starts pushing; orange: current push), ")
	TEXT("spawn area (cyan), recycle distance (magenta), spawn points (green accepted, red rejected), and recycles (yellow lines)"),
	ECVF_Cheat);
#endif

bool SUDebug::IsAIDebugEnabled()
{
#if ENABLE_DRAW_DEBUG
	return CVarDebugAI.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}
