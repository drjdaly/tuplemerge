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
#include "OutputWriter.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

using namespace std;

string Join(const string& separator, const vector<string>& vec) {
	stringstream ss;
	for (const string& s : vec) {
		ss << s << separator;
	}
	string s = ss.str();
	s.erase(s.length() - 1);
	return s;
}

int OutputWriter::Callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

bool OutputWriter::WriteToSQLite(const std::string& database_name, struct SQLiteData& sqldata, const vector<string>& header, const vector<map<string, string>>& data) {
	for (auto& m : data) {
		vector<string> line;
		for (auto& f : header) {
			if (f == "Classifier") {
				sqldata.classifier = m.at(f);
				continue;
			}
			sqldata.result = stod(m.at(f));
			sqldata.mode = f;
			WriteToSQLitePrivate(database_name, sqldata);
		}

	}
	return true;
}
int OutputWriter::CallBackCount(void* data, int count, char** rows, char**)
{
	if (count == 1 && rows) {
		*static_cast<int*>(data) = atoi(rows[0]);
		return 0;
	}
	return 1;
}
bool OutputWriter::WriteToSQLitePrivate(const std::string& database_name, const struct SQLiteData& sqldata) {
/*
	sqlite3 *db;
	std::string table_name = sqldata.classifier;
	char *zErrMsg = 0;
	int rc;
	rc = sqlite3_open(database_name.c_str(), &db);
	if (rc){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	//first check if exists
	std::string wheresql = " where  mode = "  //classifier=" + std::string("\"") + sqldata.classifier + std::string("\" and ") +
		 + std::string("\"") + sqldata.mode + std::string("\" and ") +
		" dim= " + std::to_string(sqldata.dim) + " and " +
		" rule_seed= \"" + sqldata.rule_seed + std::string("\" and ") +
		" rule_size= " + std::to_string(sqldata.rule_size);
	
	std::string count_sql = "select count(*) from " + table_name + wheresql;
	int count = 0;
	rc = sqlite3_exec(db, count_sql.c_str(), CallBackCount, &count, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	std::string sql;
	if (count != 0) {
		//update instead
		sql = std::string("update  ") + table_name +
			" set result = " + std::to_string(sqldata.result) + wheresql;
			
	}
	else {
		 sql = std::string("insert into ") + table_name + " values(\""
			//+ sqldata.classifier + std::string("\", \"")
			+ sqldata.mode + std::string("\", ")
			+ std::to_string(sqldata.dim) + std::string(", \"")
			+ sqldata.rule_seed + std::string("\", ")
			+ std::to_string(sqldata.rule_size) + std::string(", ")
			+ std::to_string(sqldata.result) + std::string(")");
	
	}
	cout << sql << endl;
	rc = sqlite3_exec(db, sql.c_str(), Callback, 0, &zErrMsg);
	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
*/
	return true;
}
bool OutputWriter::WriteCsvFile(const string& filename, const vector<string>& header, const vector<map<string, string>>& data) {
	ofstream out(filename);
	if (out.good()) {
	//	printf("Writing to file %s\n", filename.c_str());
	} else {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}

	out << Join(",", header) << endl;

	for (auto& m : data) {
		vector<string> line;
		for (auto& f : header) {
			line.push_back(m.at(f));
		}
		out << Join(",", line) << endl;
	}
	out.close();

	if (out.good()) {
		//printf("Done writing\n");
	} else {
		printf("Problem writing\n");
	}
	
	return out.good();
}

bool OutputWriter::WritePackets(const string& filename, const vector<vector<Point>>& packets) {
	ofstream out(filename);
	if (!out.good()) {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}
	unsigned int i = 0;
	for (auto& p : packets) {
		vector<string> line;
		for (Point x : p) {
			line.push_back(to_string(x));
		}
		line.push_back(to_string(i));
		line.push_back(to_string(i));
		out << Join("\t", line) << endl;
		i++;
	}
	out.close();
	if (!out.good()) {
		printf("Problem writing\n");
	}
	return out.good();
}
