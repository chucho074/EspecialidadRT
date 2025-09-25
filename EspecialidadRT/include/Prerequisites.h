#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>


//FileSystem
namespace fsys = std::filesystem;

//Path
using Path = std::filesystem::path;
using std::fstream;
using std::ios;

template<typename T>
using Vector = std::vector<T>;

using String = std::string;

template<class Key,
  class T>
using UMap = std::unordered_map<Key, T>;


static String
trim_and_reduce_spaces(const String& str) {
  String result;
  bool in_space = false;

  size_t start = str.find_first_not_of(" \t\n\r\f\v");
  if(start == String::npos) return "";

  size_t end = str.find_last_not_of(" \t\n\r\f\v");

  for(size_t i = start; i <= end; ++i) {
    if(isspace(str[i])) {
      if(!in_space) {
        result += ' ';
        in_space = true;
      }
    }
    else {
      result += str[i];
      in_space = false;
    }
  }

  return result;
}

static Vector<String>
split(const String& str, char delim) {
  Vector <String> tokens;
  size_t start = 0;
  size_t end = str.find(delim);
  while(end != String::npos) {
    tokens.push_back(trim_and_reduce_spaces(str.substr(start, end - start)));
    start = end + 1;
    end = str.find(delim, start);
  }
  //tokens.push_back(str.substr(start, end));
  tokens.push_back(trim_and_reduce_spaces(str.substr(start)));
  return tokens;
}