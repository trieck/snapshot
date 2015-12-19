// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning(disable:4715)

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <sstream>
#include <iostream>

#include <codecvt>
#include <locale>
#include <string>
#include <cassert>
#include <fstream>
#include <cstring>
#include <unordered_map>

#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif

#include <boost/format.hpp>
#include "json/json.h"

using std::cout;
using std::cerr;
using std::endl;
