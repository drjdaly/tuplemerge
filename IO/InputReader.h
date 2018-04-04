/*
 * MIT License
 *
 * Copyright (c) 2016, 2017 by S. Yingchareonthawornchai and J. Daly at Michigan State University
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
#ifndef  INPUTREADER_H
#define  INPUTREADER_H 
#include "../ElementaryClasses.h"

//CREDIT:: REUSE INPUT READER FROM Hypersplit //
class  InputReader {
public:

	static int dim ;
	static int reps ;

	static std::vector<Rule> ReadFilterFile(const std::string& filename);

	static SQLiteData ExtractDatabaseInfo(const std::string&  filename, TestMode& mode, ClassifierTests& classifier);
	static std::vector<std::vector<unsigned int>> ReadPackets(const std::string& filename);
private:
	static unsigned int inline atoui(const std::string& in);
	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	static std::vector<std::string> split(const std::string &s, char delim);

//	static void ReadIPRange(vector<unsigned int>& IPrange, const string& token);
	static void  ReadIPRange(std::array<unsigned int,2>& IPrange, unsigned int& prefix_length, const std::string& token);
	static void ReadPort(std::array<unsigned int,2>& Portrange, unsigned int& prefix_length, const std::string& from, const std::string& to);
	static void ReadProtocol(std::array<unsigned int,2>& Protocol, unsigned int& prefix_length, const std::string& last_token);
	static void ParseRange(std::array<unsigned int, 2>& range, const std::string& text);
	static int ReadFilter(std::vector<std::string>& tokens, std::vector<Rule>& ruleset, unsigned int cost);
	static  void LoadFilters(std::ifstream& fp, std::vector<Rule>& ruleset);
	static std::vector<Rule> ReadFilterFileClassBench(const std::string&  filename);
	static std::vector<Rule> ReadFilterFileMSU(const std::string& filename);

	static const int LOW = 0;
	static const int HIGH = 1;
};

#endif
