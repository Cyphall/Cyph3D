#pragma once

#include <Cyph3D/UI/IInspectable.h>

namespace c3d
{
class UIInspector
{
public:
	static void show();

	static IInspectable* getSelected();
	static void setSelected(IInspectable* selected);

private:
	static IInspectable* _selected;
};
}