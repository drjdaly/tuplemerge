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
#pragma once
#include <map>
#include <string>
#include <vector>
//#include "../SQLite/sqlite3.h"
#include "../ElementaryClasses.h"

class OutputWriter {
public:

	//static bool WriteToSQLiteBucketingResults(const std::string& database_name, struct SQLiteData& sqldata, const std::vector<std::string>& header, const std::vector<std::map<std::string, std::string>>& data);
	//static bool WriteToSQLiteAdaptiveBucketingResults(const std::string& database_name, struct SQLiteData& sqldata, const std::vector<std::string>& header, const std::vector<std::map<std::string, std::string>>& data);

	static bool WriteToSQLite(const std::string& database_name, struct SQLiteData& sqldata, const std::vector<std::string>& header, const std::vector<std::map<std::string, std::string>>& data);
	static bool WriteCsvFile(const std::string& filename, const std::vector<std::string>& header, const std::vector<std::map<std::string, std::string>>& data);

	static bool WritePackets(const std::string& filename, const std::vector<std::vector<Point>>& packets);
private:
	static int Callback(void *NotUsed, int argc, char **argv, char **azColName);
	static int CallBackCount(void* data, int count, char** rows, char**);
	static bool WriteToSQLitePrivate(const std::string& database_name, const struct SQLiteData& sqldata);
};

