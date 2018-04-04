/*
 * MIT License
 *
 * Copyright (c) 2016, 2017 by J. Daly at Michigan State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 #pragma once

#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::string> ParseArgs(int argc, char* argv[]);

template<class K, class V>
const V& GetOrElse(const std::unordered_map<K, V> &m, const K& key, const V& def) {
	if (m.find(key) == m.end()) return def;
	else return m.at(key);
}

template<class K, class V>
V* GetOrNull(const std::unordered_map<K, V*> &m, const K& key) {
	if (m.find(key) == m.end()) return nullptr;
	else return m.at(key);
}

const std::string& GetOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, const std::string& def);
bool GetBoolOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, bool def);
int GetIntOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, int def);
unsigned int GetUIntOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, unsigned int def);
double GetDoubleOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, double def);

void Split(const std::string &s, char delim, std::vector<std::string>& tokens);
