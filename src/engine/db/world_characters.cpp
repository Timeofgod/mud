#include "world_characters.h"
#include "engine/core/handler.h"
#include "gameplay/ai/mobact.h"
#include "global_objects.h"
#include "gameplay/ai/mob_memory.h"

#include <third_party_libs/fmt/include/fmt/format.h>

Characters &character_list = GlobalObjects::characters();    // global container of entities

Characters::CL_RNumChangeObserver::CL_RNumChangeObserver(Characters &cl) : m_parent(cl) {
}

Characters::~Characters() {
	log("~Characters()");
}

void Characters::CL_RNumChangeObserver::notify(ProtectedCharData &character, const MobRnum old_rnum) {
	const auto character_ptr = dynamic_cast<CharData *>(&character);
	if (nullptr == character_ptr) {
		log("LOGIC ERROR: Character object passed to RNUM change observer "
			"is not an instance of CharacterData class. Old RNUM: %d.",
			old_rnum);

		return;
	}
	
	m_parent.m_vnum_to_characters_set[mob_index[old_rnum].vnum].erase(character_ptr);
	if (m_parent.m_vnum_to_characters_set[mob_index[old_rnum].vnum].empty()) {
		m_parent.m_vnum_to_characters_set.erase(mob_index[old_rnum].vnum);
	}

	const auto new_rnum = character.get_rnum();
	if (new_rnum != kNobody) {
		m_parent.m_vnum_to_characters_set[mob_index[new_rnum].vnum].insert(character_ptr);
	}
}

Characters::Characters() {
	m_rnum_change_observer = std::make_shared<CL_RNumChangeObserver>(*this);
}

void Characters::push_front(const CharData::shared_ptr &character) {
	m_list.push_front(character);
	m_character_raw_ptr_to_character_ptr[character.get()] = m_list.begin();

	const auto rnum = character->get_rnum();
	if (kNobody != rnum) {
		m_vnum_to_characters_set[mob_index[rnum].vnum].insert(character.get());
	}
	character->subscribe_for_rnum_changes(m_rnum_change_observer);

	if (character->purged()) {
		/*
		* Anton Gorev (2017/10/29): It is possible is character quit the game without
		* disconnecting and then reenter the game. In this case #character of his descriptor will be reused but
		* flag #purged still be set. Technically now flag #purged means only "character_list is going to reset
		* shared pointer to this object." but it doesn't mean that object was removed (even partially).
		*
		* Thus we can safely put character with flag #purged and just clear this flag.
		*/
		character->set_purged(false);
	}
}

void Characters::get_mobs_by_vnum(const MobVnum vnum, list_t &result) {
	const auto i = m_vnum_to_characters_set.find(vnum);

	result.clear();
	if (i == m_vnum_to_characters_set.end()) {
		return;
	}
	for (const auto &character : i->second) {
		result.push_back(*m_character_raw_ptr_to_character_ptr[character]);
	}
}

void Characters::foreach(const foreach_f &function) const {
	const list_t &list = get_list();
	std::for_each(list.begin(), list.end(), function);
}

void Characters::foreach_on_copy(const foreach_f function) const {
	const list_t list = get_list();
	std::for_each(list.begin(), list.end(), function);
}

void Characters::foreach_on_filtered_copy(const foreach_f function, const predicate_f predicate) const {
	list_t list;
	std::copy_if(get_list().begin(), get_list().end(), std::back_inserter(list), predicate);
	std::for_each(list.begin(), list.end(), function);
}

void Characters::AddToExtractedList(CharData *ch) {
	if (ch->IsNpc()) {
//		ch->script->set_purged(true);
		mobs_by_vnum_remove(ch, mob_index[(ch)->get_rnum()].vnum);
	}
	log("add to extracted list %s %d", GET_NAME(ch), GET_MOB_VNUM(ch));
	m_extracted_list.insert(ch);
}

void Characters::PurgeExtractedList() {
	if (!m_extracted_list.empty()) {
		for (auto &it : m_extracted_list) {
			ExtractCharFromWorld(it, false);
		}
		m_extracted_list.clear();
	}
}

void Characters::mobs_by_vnum_remove(CharData *character, MobVnum vnum) {
	if (!m_vnum_to_characters_set[vnum].empty()) {
		m_vnum_to_characters_set[vnum].erase(character);
		if (m_vnum_to_characters_set[vnum].empty()) {
			m_vnum_to_characters_set.erase(vnum);
		}
	}
}

void Characters::remove(CharData *character) {
	const auto index_i = m_character_raw_ptr_to_character_ptr.find(character);
	if (index_i == m_character_raw_ptr_to_character_ptr.end()) {
		const size_t BUFFER_SIZE = 1024;
		char buffer[BUFFER_SIZE];
		snprintf(buffer, BUFFER_SIZE, "Character at address %p requested to remove not found in the world.", character);
		mudlog(buffer, LGH, kLvlImplementator, SYSLOG, true);

		return;
	}

	m_purge_list.push_back(*index_i->second);
	m_purge_set.insert(index_i->second->get());

	character->unsubscribe_from_rnum_changes(m_rnum_change_observer);
	if (kNobody != character->get_rnum()) {
		const auto vnum = mob_index[character->get_rnum()].vnum;
		mobs_by_vnum_remove(character, vnum);
	}
// креш
//	if (m_extracted_list.contains(character)) {
//		m_extracted_list.erase(character);
//	}
	m_list.erase(index_i->second);
	m_character_raw_ptr_to_character_ptr.erase(index_i);

	character->set_purged();
}

void Characters::purge() {
	m_purge_set.clear();
	for (const auto &character : m_purge_list) {
		if (character->IsNpc()) {
			mob_ai::clearMemory(character.get());
		}

		character->SetFlag(EMobFlag::kMobFreed);
	}

	m_purge_list.clear();
}

bool Characters::has(const CharData *character) const {
	return m_character_raw_ptr_to_character_ptr.find(character) != m_character_raw_ptr_to_character_ptr.end()
		|| m_purge_set.find(character) != m_purge_set.end();
}

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :
