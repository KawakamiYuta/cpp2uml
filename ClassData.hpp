/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Yuta Kawakami.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef CLASSDAT_HPP
#define CLASSDAT_HPP
#include <algorithm>
#include <regex>
#include "DotFile.hpp"

static std::string replaceStr(const std::string& target,const std::string& replace,const std::string& to);

struct FunctionDesc
{
};

struct MemberDataDesc
{
	std::string access;
	std::string type;
	std::string name;
	bool isClassType;
	std::string show()
	{
		std::string mark = (access == "public")? "+":(access == "protected")?"#": "-";
		return mark + name + ":" + replaceStr(replaceStr(type,"<","\\<"),">","\\>");
	}
};

struct Record
{
	static std::map<std::string,Record*> recordDB;
	std::string name;
	std::vector<MemberDataDesc> data;
	std::vector<std::string> method;
	Record(const std::string& prmName):name(prmName){}
	static void maybeAddClass(const std::string& prmName)
	{
		auto itr = recordDB.find(prmName);
		if(itr == recordDB.end()) {
			recordDB[prmName] = new Record(prmName);
		}
	}
	static bool EquRecord(MemberDataDesc a,MemberDataDesc b)
	{
		return a.name == b.name;
	}
	static void printAsDot() {
		static DotFile Df("out.dot");
		for (auto&& elm: recordDB) {
			auto it = std::unique(elm.second->data.begin(),elm.second->data.end(),EquRecord);
			elm.second->data.resize(std::distance(elm.second->data.begin(),it));
			std::stringstream d;
			d << elm.second->name << " [ label = \"{" << elm.second->name << "|";
			for (auto&& data : elm.second->data) {
				if(data.isClassType) {
					Df.append("edge [arrowhead = \"empty\"]\n");
					Df.append(elm.second->name + " -> " + data.type + "\n");
				}
				else {
					d << data.show() << "\\l";
				}
			}
			d << "|";
			for (auto&& method : elm.second->method) {
				d << "+" << method.c_str() << "\\l";
			}
			d << "}\"]" << std::endl;
			Df.append(d.str());
		}
	}

	static bool isTargetClassType(const std::string& belong)
	{
		auto itr = recordDB.find(belong);
		return (itr != recordDB.end())? true:false;
	}

	static void addField(const std::string& belong,const std::string& name,
			const std::string& type = "",const std::string& access = "",bool isClassType = false)
	{
		MemberDataDesc desc = {access,type,name,isClassType};
		recordDB[belong]->data.push_back(desc);
	}
};

static std::string replaceStr(const std::string& target,const std::string& replace,const std::string& to)
{
	std::string str = target;
	std::string::size_type pos = str.find(replace);
	while(pos != std::string::npos) {
		str.replace(pos,replace.size(),to);
		pos = str.find(replace,pos + to.size());
	}
	return str;
}
static std::string basename(const std::string& path)
{
	return path.substr(path.find_last_of('/') + 1);
}

static std::vector<std::string> split(const std::string& str,char delm)
{
	std::vector<std::string> elems;
	std::stringstream ss(str);
	std::string item;
	while (getline(ss,item,delm)) {
		if (!item.empty()) {
			elems.push_back(item);
		}
	}
	return elems;
}

#endif
