#pragma once
#pragma warning(disable:4715)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN  

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
#include <iterator>

#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif

#include <boost/format.hpp>
#include "json/json.h"

using std::cout;
using std::cerr;
using std::endl;

using stringvec = std::vector<std::string>;
using OpenMode = std::ios::openmode;

#include "util.h"
