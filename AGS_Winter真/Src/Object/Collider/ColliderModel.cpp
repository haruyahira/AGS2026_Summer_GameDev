#include "ColliderModel.h"
ColliderModel::ColliderModel(TAG tag, const Transform* follow)
	:
	ColliderBase(SHAPE::MODEL, tag, follow)
{
}
ColliderModel::~ColliderModel(void)
{
}