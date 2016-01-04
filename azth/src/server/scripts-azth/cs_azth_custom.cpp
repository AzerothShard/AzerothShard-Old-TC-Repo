/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "Chat.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "Language.h"
#include "Opcodes.h"
#include "Player.h"
#include "Pet.h"

class azth_commandscript : public CommandScript
{
public:
    azth_commandscript() : CommandScript("azth_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> lookupAzthCommands =
        {
            { "maxskill", rbac::RBAC_PERM_COMMAND_AZTH_MAXSKILL, true, &handleAzthMaxSkill, "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "azth", rbac::RBAC_PERM_COMMAND_AZTH, true, NULL, "", lookupAzthCommands },
        };
        return commandTable;
    }

    static bool handleAzthMaxSkill(ChatHandler* handler, const char* args)
    {

        Player* target = handler->getSelectedPlayerOrSelf();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }
        if (target->getLevel() < 80)
        {
            handler->PSendSysMessage(LANG_REQUIRED_LEVEL, 80);
            handler->SetSentErrorMessage(true);
            return false;
        }

        enum SkillSpells
        {
            ONE_HAND_AXES = 196,
            TWO_HAND_AXES = 197,
            ONE_HAND_MACES = 198,
            TWO_HAND_MACES = 199,
            POLEARMS = 200,
            ONE_HAND_SWORDS = 201,
            TWO_HAND_SWORDS = 202,
            STAVES = 227,
            BOWS = 264,
            GUNS = 266,
            DAGGERS = 1180,
            WANDS = 5009,
            CROSSBOWS = 5011,
            FIST_WEAPONS = 15590
        };
        static const SkillSpells spells[] = { ONE_HAND_AXES, TWO_HAND_AXES, ONE_HAND_MACES,
            TWO_HAND_MACES, POLEARMS, ONE_HAND_SWORDS, TWO_HAND_SWORDS, STAVES, BOWS,
            GUNS, DAGGERS, WANDS, CROSSBOWS, FIST_WEAPONS };

        std::list<SkillSpells> learnList;
        for (SkillSpells spell : spells)
        {
            switch (target->getClass())
            {
            case CLASS_WARRIOR:
                if (spell != WANDS)
                    learnList.push_back(spell);
                break;
            case CLASS_DEATH_KNIGHT:
            case CLASS_PALADIN:
                if (spell != STAVES  && spell != BOWS && spell != GUNS && spell != DAGGERS &&
                    spell != WANDS && spell != CROSSBOWS && spell != FIST_WEAPONS)
                    learnList.push_back(spell);
                break;
            case CLASS_HUNTER:
                if (spell != ONE_HAND_MACES  && spell != TWO_HAND_MACES && spell != WANDS)
                    learnList.push_back(spell);
                break;
            case CLASS_ROGUE:
                if (spell != TWO_HAND_AXES && spell != TWO_HAND_MACES && spell != POLEARMS &&
                    spell != TWO_HAND_SWORDS && spell != STAVES && spell != WANDS)
                    learnList.push_back(spell);
                break;
            case CLASS_PRIEST:
                if (spell == WANDS || spell == ONE_HAND_MACES || spell == STAVES ||
                    spell == DAGGERS)
                    learnList.push_back(spell);
                break;
            case CLASS_SHAMAN:
                if (spell != ONE_HAND_SWORDS && spell != TWO_HAND_SWORDS && spell != POLEARMS &&
                    spell != BOWS && spell != GUNS && spell != WANDS && spell != CROSSBOWS)
                    learnList.push_back(spell);
                break;
            case CLASS_WARLOCK:
            case CLASS_MAGE:
                if (spell == WANDS || spell == ONE_HAND_SWORDS || spell == STAVES ||
                    spell == DAGGERS)
                    learnList.push_back(spell);
                break;
            case CLASS_DRUID:
                if (spell != ONE_HAND_SWORDS && spell != TWO_HAND_SWORDS &&
                    spell != BOWS && spell != GUNS && spell != WANDS && spell != CROSSBOWS &&
                    spell != ONE_HAND_AXES && spell != TWO_HAND_AXES)
                    learnList.push_back(spell);
                break;
            default:
                break;
            }
        }

        for (SkillSpells spell : learnList)
        {
            if (!target->HasSpell(spell))
                target->LearnSpell(spell, false);
        }

        target->UpdateSkillsToMaxSkillsForLevel();
        return true;
    }
};

void AddSC_azth_commandscript()
{
    new azth_commandscript();
}
