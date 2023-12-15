#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Pyxis/Log.h"

//platform specifics
#ifdef PX_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
