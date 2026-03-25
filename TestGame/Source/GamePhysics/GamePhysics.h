#include <vector>
#include <string>


enum CollisionMasks
{
	CM_Static = 1 << 0,
	CM_Floor = 1 << 1,
	CM_Hero = 1 << 2,
	CM_Enemy = 1 << 3,
};

inline std::map<CollisionMasks, std::string> CollisionMasksToString =
{
	{ CM_Static, "Static" },
	{ CM_Floor, "Floor" },
	{ CM_Hero, "Hero" },
	{ CM_Enemy, "Enemy" },
};

inline std::map<std::string, CollisionMasks> StringToCollisionMasks =
{
	{ "Static", CM_Static },
	{ "Floor", CM_Floor },
	{ "Hero", CM_Hero },
	{ "Enemy", CM_Enemy },
};

namespace Physics
{
	std::vector<std::string> GetCollisionMasks();
};
