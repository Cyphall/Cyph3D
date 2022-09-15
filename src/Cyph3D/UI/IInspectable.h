#pragma once

class IInspectable
{
public:
	virtual ~IInspectable() = default;

	virtual void onDrawUi() = 0;
};