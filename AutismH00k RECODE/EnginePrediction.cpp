#include "EnginePrediction.h"
#include "NetVars.h"
#include "XorStr.hpp"
#include "MathFunctions.h"
#include "Hacks.h"
BYTE bMoveData[200];

void start_prediction(CUserCmd* pCmd)
{
	auto m_local = hackManager.pLocal();
	if (Interfaces::MoveHelper && m_local->IsAlive())
	{
		float curtime = Interfaces::Globals->curtime;
		float frametime = Interfaces::Globals->frametime;
		int iFlags = m_local->GetFlags();
		Interfaces::Globals->curtime = (float)m_local->GetTickBase() * Interfaces::Globals->interval_per_tick;
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;
		Interfaces::MoveHelper->SetHost(m_local);
		Interfaces::Prediction1->SetupMove(m_local, pCmd, nullptr, bMoveData);
		Interfaces::GameMovement->ProcessMovement(m_local, bMoveData);
		Interfaces::Prediction1->FinishMove(m_local, pCmd, bMoveData);
		Interfaces::MoveHelper->SetHost(0);
		Interfaces::Globals->curtime = curtime;
		Interfaces::Globals->frametime = frametime;
		*m_local->GetPointerFlags() = iFlags;
	}
}