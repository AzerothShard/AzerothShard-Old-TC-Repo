#include "Chat.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "Language.h"
#include "MovementGenerator.h"
#include "Opcodes.h"
#include "SpellAuras.h"
#include "TargetedMovementGenerator.h"
#include "WeatherMgr.h"
#include "Player.h"
#include "Pet.h"
#include "LFG.h"
#include "GroupMgr.h"
#include "MMapFactory.h"
#include "DisableMgr.h"
#include "SpellHistory.h"
#include "ReputationMgr.h"
/*
class AzthCommands : public CommandScript
{
public:
    AzthCommands() : CommandScript("AzthCommands") { }

    ChatCommand* GetCommands() const override
    {
        static ChatCommand rateCommandTable[] = {
            { "xp",               SEC_PLAYER,                               false, &HandleRateXpCommand,          "", NULL},
            { NULL,               SEC_PLAYER,                               false, NULL,                          "", NULL}
        };
        
        static ChatCommand commandTable[] =
        {
            { "qc",               rbac::RBAC_PERM_COMMAND_QUESTCOMPLETER,   false, &HandleQuestCompleterCommand,   "", NULL },
            { "rate",             SEC_PLAYER,                               false, NULL,                           "", rateCommandTable},
            { NULL,               0,                                  false, NULL,                           "", NULL }
        };
        
        return commandTable;
    }
    
    static bool HandleRateXpCommand(ChatHandler *handler, const char *args) {
        // take a pointer to the player who uses the command
        Player *me = handler->GetSession() ? handler->GetSession()->GetPlayer() : NULL;
        if (!me)
            return false;

        // already at max level, no point in using the command at all
        if (me->getLevel() == sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)) {
            handler->SendSysMessage("|CFF7BBEF7[Custom Rates]|r: You are already at maximum level.");
            return true;
        }

        // no arguments, show current XP rate
        if (!*args) {
            handler->PSendSysMessage("|CFF7BBEF7[Custom Rates]|r: Your current XP rate is %u.", me->azthPlayer->getCustomQuestRate());
            return true;
        }

        // first, check if I can use the command
        if (me->GetSession()->GetSecurity() < (int) sWorld->getIntConfig(CONFIG_PLAYER_INDIVIDUAL_XP_RATE_SECURITY)) {
            handler->SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // take a pointer to player's selection
        Player *target = handler->getSelectedPlayer();
        if (!target || !target->GetSession()) {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // extract value
        int rate = atoi((char *) args);
        int maxRate = sWorld->getIntConfig(CONFIG_PLAYER_MAXIMUM_INDIVIDUAL_XP_RATE);
        if (rate < 0 || rate > maxRate) {
            handler->PSendSysMessage("|CFF7BBEF7[Custom Rates]|r: Invalid rate specified, must be in interval [0,%i].", maxRate);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // take a pointer to the player we need to set xp rate to
        // can be either player itself, or his selection, if
        // selection security is lower than his security
        Player *player = NULL;
        if (target == me)
            player = me;
        else {
            if (me->GetSession()->GetSecurity() > target->GetSession()->GetSecurity())
                player = target;
            else {
                handler->SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        // set player custom XP rate and save it in DB for later use
        uint32 uRate = static_cast<uint32> (rate);
        player->SetCustomXpRate(uRate);
        int32 rateFromDB = CustomRates::GetXpRateFromDB(player);
        if (rateFromDB == -1)
            CustomRates::SaveXpRateToDB(player, uRate, false);
        else
            CustomRates::SaveXpRateToDB(player, uRate, true);

        // show a message indicating custom XP rate change
        if (player == me)
            handler->PSendSysMessage("|CFF7BBEF7[Custom Rates]|r: You have set your XP rate to %u.", uRate);
        else {
            handler->PSendSysMessage("|CFF7BBEF7[Custom Rates]|r: You have set %s's XP rate to %u.", handler->GetNameLink(player).c_str(), uRate);
            ChatHandler(player->GetSession()).PSendSysMessage("|CFF7BBEF7[Custom Rates]|r: %s has set your XP rate to %u.", handler->GetNameLink().c_str(), uRate);
        }

        return true;
    }

    // Based on HandleQuestComplete method of cs_quest.cpp
    static bool HandleQuestCompleterCompHelper(Player* player, uint32 entry, ChatHandler* handler)
    {
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Add quest items for quests that require items
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 id = quest->RequiredItemId[x];
            uint32 count = quest->RequiredItemCount[x];
            if (!id || !count)
                continue;

            uint32 curItemCount = player->GetItemCount(id, true);

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count-curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, id, true);
                player->SendNewItem(item, count-curItemCount, true, false);
            }
        }

        // All creature/GO slain/cast (not required, but otherwise it will display "Creature slain 0/10")
        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 creature = quest->RequiredNpcOrGo[i];
            uint32 creatureCount = quest->RequiredNpcOrGoCount[i];

            if (creature > 0)
            {
                if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creature))
                    for (uint16 z = 0; z < creatureCount; ++z)
                        player->KilledMonster(creatureInfo, ObjectGuid::Empty);
            }
            else if (creature < 0)
                for (uint16 z = 0; z < creatureCount; ++z)
                    player->KillCreditGO(creature);
        }

        // If the quest requires reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction())
        {
            uint32 repValue = quest->GetRepObjectiveValue();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue);
        }

        // If the quest requires a SECOND reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction2())
        {
            uint32 repValue2 = quest->GetRepObjectiveValue2();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue2)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue2);
        }

        // If the quest requires money
        int32 ReqOrRewMoney = quest->GetRewOrReqMoney();
        if (ReqOrRewMoney < 0)
            player->ModifyMoney(-ReqOrRewMoney);

        if (sWorld->getBoolConfig(CONFIG_QUEST_ENABLE_QUEST_TRACKER)) // check if Quest Tracker is enabled
        {
            // prepare Quest Tracker datas
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_QUEST_TRACK_GM_COMPLETE);
            stmt->setUInt32(0, quest->GetQuestId());
            stmt->setUInt32(1, player->GetGUID().GetCounter());

            // add to Quest Tracker
            CharacterDatabase.Execute(stmt);
        }

        player->CompleteQuest(entry);
        return true;
    }
    
    // Based on TrueWoW code
    static bool HandleQuestCompleterCommand(ChatHandler* handler, char const* args)
    {
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
        {
            handler->PSendSysMessage("Syntax: .qc $quest\n\nControlla se la $quest è buggata.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 entry = atol(cId);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest || quest == 0)
        {
            handler->PSendSysMessage("Please enter a quest link.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            uint32 checked = 0;
            PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_QUESTCOMPLETER);
            stmt->setUInt32(0, entry);
            PreparedQueryResult resultCheck = WorldDatabase.Query(stmt);

            if (!resultCheck)
            {
                handler->PSendSysMessage("Errore: quest non trovata.");
                handler->SetSentErrorMessage(true);
                return false;
            }

            std::string questTitle = quest->GetTitle();

            checked = (*resultCheck)[0].GetBool();

            if (checked == 1)
            {
                std::string name;
                const char* playerName = handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : NULL;

                if (playerName)
                {
                    name = playerName;
                    normalizePlayerName(name);

                    Player* player = ObjectAccessor::FindPlayerByName(name);
                    if (player->GetQuestStatus(entry) != QUEST_STATUS_INCOMPLETE)
                    {
                        handler->PSendSysMessage("[%s] è buggata!", questTitle.c_str());
                        return true;
                    }
                    else
                    {
                        HandleQuestCompleterCompHelper(player, entry, handler);
                        handler->PSendSysMessage("[%s] è buggata ed è stata completata!", questTitle.c_str());
                        return true;
                    }
                }
                else
                {
                    handler->PSendSysMessage("[%s] è buggata!", questTitle.c_str());
                    return true;
                }
            }
            else
            {
                handler->PSendSysMessage("[%s] non risulta essere buggata, se ritieni che lo sia ti preghiamo di segnalarcelo utilizzando il bugtracker.", questTitle.c_str());
                return true;
            }
        }
    }
};

void AddSC_AzthCommands()
{
    new AzthCommands();
}
*/