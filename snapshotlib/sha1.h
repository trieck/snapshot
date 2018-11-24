#pragma once

#include <boost/uuid/detail/sha1.hpp>

std::string sha1(const std::string& input);

void sha1(const std::string &input, boost::uuids::detail::sha1::digest_type &digest);
