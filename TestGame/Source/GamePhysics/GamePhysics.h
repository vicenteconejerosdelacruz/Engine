#include <vector>
#include <string>

enum CollisionFlags
{
	CF_WallGrabArea = 1 << 0,
	CF_WallBlockVerticalUp = 1 << 1,
};

inline std::unordered_map<CollisionFlags, std::string> CollisionFlagsToString =
{
	{ CF_WallGrabArea, "WallGrabArea" },
	{ CF_WallBlockVerticalUp, "WallBlockVerticalUp" },
};

inline std::unordered_map<std::string, CollisionFlags> StringToCollisionFlags =
{
	{ "WallGrabArea", CF_WallGrabArea },
	{ "WallBlockVerticalUp", CF_WallBlockVerticalUp },
};

namespace Physics
{
	std::vector<std::string> GetCollisionMasks();
};
