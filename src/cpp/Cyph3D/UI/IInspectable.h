#pragma once

namespace c3d
{
class IInspectable
{
public:
	virtual ~IInspectable() = default;

	virtual void onDrawUi() = 0;
};
}