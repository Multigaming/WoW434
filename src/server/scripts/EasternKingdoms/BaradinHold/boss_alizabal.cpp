/*
 * Copyright (C) 2008-2012 Holystone Productions>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PoolMgr.h"
#include "Group.h"
#include "SpellInfo.h"
#include "baradin_hold.h"

enum Spells
{
	spell_Skewer        = 104936,
	spell_Seething_Hate = 105067,
	spell_Blade_Dance   = 104994,
	spell_berserk       = 47008,
};

enum Events
{
	Event_Skewer              = 1,
	Event_Seething_Hate       = 2,
	Event_Blade_Dance_Begin   = 3,
	Event_Blade_Dance_Move    = 4,
	Event_Blade_Dance_End     = 5,
	Event_berserk             = 6,
};

enum MovementPoints
{
    Point_Target_Blade_Dance_Player   = 36612631,
};

class boss_alizabal : public CreatureScript
{
    public:
        boss_alizabal() : CreatureScript("boss_alizabal") { }

        struct boss_alizabalAI : public BossAI
        {
            boss_alizabalAI(Creature* creature) : BossAI(creature, DATA_ALIZABAL)
            {
			}

			void reset()
			{
				me->RemoveAurasDueToSpell(spell_berserk);
				events.ScheduleEvent(Event_Skewer, 10000);
				events.ScheduleEvent(Event_Seething_Hate, 3000);
				events.ScheduleEvent(Event_Blade_Dance_Begin, 60000);
			}

			void EnterCombat(Unit* /*who*/)
            {
				events.ScheduleEvent(Event_Skewer, 10000);
				events.ScheduleEvent(Event_Seething_Hate, 3000);
				events.ScheduleEvent(Event_Blade_Dance_Begin, 60000);
			}

			void UpdateAI(uint32 const diff)
            {
				if (!UpdateVictim() || !CheckInRoom())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

				while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
					case Event_Skewer:
						DoCastVictim(spell_Skewer);
						events.ScheduleEvent(Event_Skewer, 10000);
						break;
					case Event_Seething_Hate:
						if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
							DoCast(target, spell_Seething_Hate);
						events.ScheduleEvent(Event_Seething_Hate, 20000);
							break;
					case Event_Blade_Dance_Begin:
						if (Aura* pBlade = me->GetAura(spell_Blade_Dance))
							pBlade->SetDuration(60000);
							me->GetAura(spell_Blade_Dance); // debug
							me->SetSpeed(MOVE_RUN, 10*3.0f, true);
							events.CancelEvent(Event_Skewer);
							events.CancelEvent(Event_Seething_Hate);
							events.ScheduleEvent(Event_Blade_Dance_Move, 20000);
							events.ScheduleEvent(Event_Blade_Dance_End, 60000+1);
							break;
					case Event_Blade_Dance_Move:
						{
						events.ScheduleEvent(Event_Blade_Dance_Move, 60000/3);
						Unit* unit = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me));
					       if (!unit)
                                unit = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
						   if (unit)
                                me->GetMotionMaster()->MovePoint(Point_Target_Blade_Dance_Player, unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
                            break;
						}
					case Event_Blade_Dance_End:
						if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                                me->GetMotionMaster()->MovementExpired();
                            DoStartMovement(me->getVictim());
                            me->SetSpeed(MOVE_RUN, 10, true);
                            events.CancelEvent(Event_Blade_Dance_Move);
                            events.ScheduleEvent(Event_Blade_Dance_Begin, 10000);
                            break;
					case Event_berserk:
						DoCast(spell_berserk);
						break;
					}
			}


		DoMeleeAttackIfReady();
	}
};
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_alizabalAI(creature);
    }
};

void AddSC_boss_alizabal()
{
    new boss_alizabal();
}