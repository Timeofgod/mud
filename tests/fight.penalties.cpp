#include "fight.penalties.hpp"

#include "char.utilities.hpp"

#include <gtest/gtest.h>

class FightPenalties : public ::testing::Test
{
public:
	static const GroupPenalties& penalties()
	{
		if (!m_grouping)
		{
			throw std::runtime_error("Penalty tables was not initialized.");
		}

		return *m_grouping;
	}

protected:
	virtual void SetUp();

private:
	static std::shared_ptr<GroupPenalties> m_grouping;
};

void FightPenalties::SetUp()
{
	m_grouping.reset(new GroupPenalties());
	if (m_grouping->init())
	{
		throw std::runtime_error("Couldn't initialize group penalties table.");
	}
}

std::shared_ptr<GroupPenalties> FightPenalties::m_grouping;

TEST_F(FightPenalties, TheSameLevels)
{
	test_utils::CharacterBuilder killer_builder;
	killer_builder.create_new();

	test_utils::CharacterBuilder leader_builder;
	leader_builder.create_new();
	leader_builder.make_group(killer_builder);

	for (int level = 1; level < LVL_IMMORT; ++level)
	{
		killer_builder.set_level(level);
		leader_builder.set_level(level);

		auto killer = killer_builder.get();
		auto leader = leader_builder.get();

		GroupPenaltyCalculator penalty(killer.get(), leader.get(), level, FightPenalties::penalties());

		EXPECT_EQ(penalty.get(), 0);
	}
}

TEST_F(FightPenalties, UndefinedKillerClass_SameLevels)
{
	test_utils::CharacterBuilder killer_builder;
	killer_builder.create_new_with_class(CLASS_UNDEFINED);

	test_utils::CharacterBuilder leader_builder;
	leader_builder.create_new();

	leader_builder.make_group(killer_builder);

	auto killer = killer_builder.get();
	auto leader = leader_builder.get();
	const auto max_level = std::max(killer->get_level(), leader->get_level());
	GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

	EXPECT_EQ(penalty.get(), 100);
}

TEST_F(FightPenalties, UndefinedLeaderClass_SameLevels)
{
	test_utils::CharacterBuilder killer_builder;
	killer_builder.create_new();

	test_utils::CharacterBuilder leader_builder;
	leader_builder.create_new_with_class(CLASS_UNDEFINED);

	leader_builder.make_group(killer_builder);

	auto killer = killer_builder.get();
	auto leader = leader_builder.get();
	const auto max_level = std::max(killer->get_level(), leader->get_level());
	GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

	EXPECT_EQ(penalty.get(), 100);
}

TEST_F(FightPenalties, MobKillerClass_SameLevels)
{
	test_utils::CharacterBuilder killer_builder;
	killer_builder.create_new_with_class(CLASS_MOB);

	test_utils::CharacterBuilder leader_builder;
	leader_builder.create_new();

	leader_builder.make_group(killer_builder);

	auto killer = killer_builder.get();
	auto leader = leader_builder.get();
	const auto max_level = std::max(killer->get_level(), leader->get_level());
	GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

	EXPECT_EQ(penalty.get(), 100);
}

TEST_F(FightPenalties, MobLeaderClass_SameLevels)
{
	test_utils::CharacterBuilder killer_builder;
	killer_builder.create_new();

	test_utils::CharacterBuilder leader_builder;
	leader_builder.create_new_with_class(CLASS_MOB);

	leader_builder.make_group(killer_builder);

	auto killer = killer_builder.get();
	auto leader = leader_builder.get();
	const auto max_level = std::max(killer->get_level(), leader->get_level());
	GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

	EXPECT_EQ(penalty.get(), 100);
}

TEST_F(FightPenalties, NoPenaltyWithing5Levels)
{
	for (int killer_class = CLASS_CLERIC; killer_class < NUM_PLAYER_CLASSES; ++killer_class)
	{
		for (int leader_class = CLASS_CLERIC; leader_class < NUM_PLAYER_CLASSES; ++leader_class)
		{
			test_utils::CharacterBuilder killer_builder;
			killer_builder.create_new_with_class(killer_class);

			test_utils::CharacterBuilder leader_builder;
			leader_builder.create_new_with_class(leader_class);

			leader_builder.make_group(killer_builder);

			for (int killer_level = 1; killer_level < LVL_IMMORT; ++killer_level)
			{
				killer_builder.set_level(killer_level);
				for (int leader_level = 1; leader_level < LVL_IMMORT; ++leader_level)
				{
					leader_builder.set_level(leader_level);

					auto killer = killer_builder.get();
					auto leader = leader_builder.get();

					const auto max_level = std::max(killer->get_level(), leader->get_level());
					GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

					if (5 >= std::abs(killer_level - leader_level))
					{
						EXPECT_EQ(penalty.get(), 0);
					}
				}
			}
		}
	}
}

TEST_F(FightPenalties, HasPenaltyWithingMoreThan5Levels)
{
	for (int killer_class = CLASS_CLERIC; killer_class < NUM_PLAYER_CLASSES; ++killer_class)
	{
		for (int leader_class = CLASS_CLERIC; leader_class < NUM_PLAYER_CLASSES; ++leader_class)
		{
			test_utils::CharacterBuilder killer_builder;
			killer_builder.create_new_with_class(killer_class);

			test_utils::CharacterBuilder leader_builder;
			leader_builder.create_new_with_class(leader_class);

			leader_builder.make_group(killer_builder);

			for (int killer_level = 1; killer_level < LVL_IMMORT; ++killer_level)
			{
				killer_builder.set_level(killer_level);
				for (int leader_level = 1; leader_level < LVL_IMMORT; ++leader_level)
				{
					leader_builder.set_level(leader_level);

					auto killer = killer_builder.get();
					auto leader = leader_builder.get();

					const auto max_level = std::max(killer->get_level(), leader->get_level());
					GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

					if (5 < std::abs(killer_level - leader_level))
					{
						EXPECT_GT(penalty.get(), 0);
					}
				}
			}
		}
	}
}

TEST_F(FightPenalties, DISABLED_PrintTable)
{
	constexpr int PLACEHOLDER_LENGTH = 4;
	for (int killer_class = CLASS_CLERIC; killer_class < NUM_PLAYER_CLASSES; ++killer_class)
	{
		for (int leader_class = CLASS_CLERIC; leader_class < NUM_PLAYER_CLASSES; ++leader_class)
		{
			test_utils::CharacterBuilder killer_builder;
			killer_builder.create_new_with_class(killer_class);

			test_utils::CharacterBuilder leader_builder;
			leader_builder.create_new_with_class(leader_class);

			leader_builder.make_group(killer_builder);

			std::cerr << "Combination: " << killer_class << "/" << leader_class << std::endl
				<< "===================" << std::endl << std::setw(1 + PLACEHOLDER_LENGTH) << "|";
			for (int leader_level = 1; leader_level < LVL_IMMORT; ++leader_level)
			{
				std::cerr << std::setw(PLACEHOLDER_LENGTH) << leader_level;
			}
			std::cerr << std::endl << "----+";
			for (int i = 1; i < LVL_IMMORT; ++i)
			{
				std::cerr << "----";
			}
			std::cerr << std::endl;

			for (int killer_level = 1; killer_level < LVL_IMMORT; ++killer_level)
			{
				std::cerr << std::setw(PLACEHOLDER_LENGTH) << killer_level << "| ";

				killer_builder.set_level(killer_level);
				for (int leader_level = 1; leader_level < LVL_IMMORT; ++leader_level)
				{
					leader_builder.set_level(leader_level);

					auto killer = killer_builder.get();
					auto leader = leader_builder.get();

					const auto max_level = std::max(killer->get_level(), leader->get_level());
					GroupPenaltyCalculator penalty(killer.get(), leader.get(), max_level, FightPenalties::penalties());

					std::cerr << std::setw(PLACEHOLDER_LENGTH) << penalty.get();
				}
				std::cerr << std::endl;
			}
		}
	}
}

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :
