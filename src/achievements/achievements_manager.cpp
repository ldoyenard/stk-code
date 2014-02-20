//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "achievements/achievements_manager.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_slot.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "online/current_user.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

AchievementsManager* AchievementsManager::m_achievements_manager = NULL;

// ----------------------------------------------------------------------------
/** Constructor, which reads data/achievements.xml.
 */
AchievementsManager::AchievementsManager()
{
    parseAssetFile();
}   // AchievementsManager

// ----------------------------------------------------------------------------
AchievementsManager::~AchievementsManager()
{
    std::map<uint32_t, AchievementInfo *>::iterator it;
    for ( it = m_achievements_info.begin(); it != m_achievements_info.end(); ++it ) {
        delete it->second;
    }
    m_achievements_info.clear();
}   // ~AchievementsManager

// ----------------------------------------------------------------------------
/** Parses the data/achievements.xml file and stores the information about
 *  all achievements in its internal map.
 */
void AchievementsManager::parseAssetFile()
{
    const std::string file_name = file_manager->getAsset("achievements.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    unsigned int num_nodes = root->getNumNodes();
    for(unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        std::string type("");
        node->get("type", &type);
        AchievementInfo * achievement_info;
        if(type == "single")
        {
            achievement_info = new SingleAchievementInfo(node);
        }
        else if(type == "map")
        {
            achievement_info = new MapAchievementInfo(node);
        }
        else
        {
            Log::error("AchievementsManager",
                       "Non-existent achievement type '%s'. Skipping - "
                       "definitely results in unwanted behaviour.",
                       type.c_str());
            continue;
        }
        m_achievements_info[achievement_info->getID()] = achievement_info;
    }
    if(num_nodes != m_achievements_info.size())
        Log::error("AchievementsManager::parseAchievements",
                   "Multiple achievements with the same id!");

    delete root;
}   // parseAssetFile

// ----------------------------------------------------------------------------
/** Create a new AchievementStatus object that stores all achievement status
 *  information for a single player.
 *  \param node The XML of saved data, or NULL if no saved data exists.
 */
AchievementsStatus* 
             AchievementsManager::createAchievementsStatus(const XMLNode *node)
{
    AchievementsStatus *status = new AchievementsStatus();

    // First add all achievements, before restoring the saved data.
    std::map<uint32_t, AchievementInfo *>::const_iterator it;
    for (it  = m_achievements_info.begin(); 
         it != m_achievements_info.end(); ++it)
    {
        Achievement::AchievementType achievement_type = it->second->getType();
        Achievement * achievement;
        if (achievement_type == Achievement::AT_SINGLE)
        {
            achievement = new SingleAchievement(it->second);
        }
        else if (achievement_type == Achievement::AT_MAP)
        {
            achievement = new MapAchievement(it->second);
        }
        status->add(achievement);
    }

    if (node)
        status->load(node);

    return status;
}   // createAchievementStatus

// ----------------------------------------------------------------------------
void AchievementsManager::onRaceEnd()
{
    //reset all values that need to be reset
    PlayerManager::get()->getCurrentPlayer()
        ->getAchievementsStatus()->onRaceEnd();
}  // onRaceEnd

// ----------------------------------------------------------------------------
AchievementInfo * AchievementsManager::getAchievementInfo(uint32_t id) const
{
    std::map<uint32_t, AchievementInfo*>::const_iterator info = 
        m_achievements_info.find(id);
    if (info != m_achievements_info.end())
        return info->second;
    return NULL;
}
