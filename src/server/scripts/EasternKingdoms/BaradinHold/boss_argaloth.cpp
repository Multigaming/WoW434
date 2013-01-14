/*
* Copyright (C) 2011-2012 Holystone Productions>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
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

#include"ScriptPCH.h"
#include"baradin_hold.h"

enum Spells
{
    spell_berserk              = 47008,
    spell_consuming_darkness   = 88954,
    spell_meteor_slash         = 88942,
    spell_fel_firestorm        = 88972,
};

enum Events
{
    EVENT_BERSERK = 1,
    EVENT_CONSUMING_DARKNESS,
    EVENT_METEOR_SLASH,
};

class boss_argaloth: public CreatureScript
{
    public:
        boss_argaloth() : CreatureScript("boss_argaloth") { }

    struct boss_argalothAI: public BossAI
    {
        boss_argalothAI(Creature* creature) : BossAI(creature, DATA_ARGALOTH) { }

        uint32 fel_firestorm_casted;

        void Reset()
        {
            _Reset();
            me->RemoveAurasDueToSpell(spell_berserk);
            events.ScheduleEvent(EVENT_BERSERK, 300 *IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CONSUMING_DARKNESS, 14 *IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_METEOR_SLASH, 10 *IN_MILLISECONDS);
            fel_firestorm_casted = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->GetHealthPct() < 66 && fel_firestorm_casted == 0)
            {
                DoCast(spell_fel_firestorm);
                events.DelayEvents(3 *IN_MILLISECONDS);
                fel_firestorm_casted = 1;
            }
            if (me->GetHealthPct() < 33 && fel_firestorm_casted == 1)
            {
                DoCast(spell_fel_firestorm);
                events.DelayEvents(3 *IN_MILLISECONDS);
                fel_firestorm_casted = 2;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CONSUMING_DARKNESS:
                    DoCast(spell_consuming_darkness);
                    events.RescheduleEvent(EVENT_CONSUMING_DARKNESS, 22 *IN_MILLISECONDS);
                    break;
                case EVENT_METEOR_SLASH:
                    DoCast(spell_meteor_slash);
                    events.RescheduleEvent(EVENT_METEOR_SLASH, 15 *IN_MILLISECONDS);
                    break;
                case EVENT_BERSERK:
                    DoCast(me, spell_berserk);
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
     };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_argalothAI(creature);
    }
};

void AddSC_boss_argaloth()
{
    new boss_argaloth();
}