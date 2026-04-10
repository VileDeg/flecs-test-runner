#include "common.h"

namespace aux {

	struct AllOperatorsType {
		int value;

		friend bool operator==(const AllOperatorsType& lhs, const AllOperatorsType& rhs) {
			return lhs.value == rhs.value;
		}
		friend bool operator<(const AllOperatorsType & lhs, const AllOperatorsType & rhs) {
			return lhs.value < rhs.value;
		}
		DERIVED_OPERATORS(AllOperatorsType);
	};

	struct NoComparisonOperatorType {
		int value;
	};

	struct module {
		module(flecs::world& world) {
			world.component<AllOperatorsType>()
				.on_equals()  // this needed to register hooks
				.on_compare() // this needed to register hooks
				.member<int>("value");
		}
	};
}

template <typename T>
class ComponentMetadataTest : public ::testing::Test
{
protected:
	void SetUp() override {
		tr::setLogLevel(tr::LogLevel::TRACE);

		InitWorld(_ecs);
	}

	void InitWorld(flecs::world& ecs) {
		TestRunner::initialize<movement::module>(ecs);
	}

	flecs::world _ecs;
};

using MyTypes = ::testing::Types<
	Speed
>;
TYPED_TEST_SUITE(ComponentMetadataTest, MyTypes);

TYPED_TEST(ComponentMetadataTest, Exists) {
	flecs::entity comp = _ecs.entity<TypeParam>();
	auto* ops = comp.try_get<SupportedOperators>();

	EXPECT_NE(ops, nullptr);
}

TYPED_TEST(ComponentMetadataTest, HasOps) {
	flecs::entity comp = _ecs.entity<TypeParam>();
	auto& ops = comp.get<SupportedOperators>();
	EXPECT_TRUE(ops.cmp);
	EXPECT_TRUE(ops.equals);
}
