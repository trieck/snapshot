#pragma once
#pragma warning(disable:4715)

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include "targetver.h"
#include <boost/config/compiler/visualc.hpp>
#else
#include <bits/algorithmfwd.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <limits.h>

#include <sstream>
#include <iostream>
#include <locale>
#include <string>
#include <cassert>
#include <fstream>
#include <cstring>
#include <unordered_map>
#include <iterator>
#include <regex>

#include <boost/format.hpp>
#include "json/json.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

using stringvec = std::vector<std::string>;
using OpenMode = std::ios::openmode;

#include "util.h"
