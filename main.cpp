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
#include <memory>
#include <iostream>
#include "clang/Tooling/CommonOptionsParser.h"
#include "MyFrontendAction.hpp"
#include "SrcFile.hpp"

std::map<std::string,Record*> Record::recordDB;
class SrcList SrcList;

static llvm::cl::OptionCategory MyToolCategory("My tool options");
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
int main(int argc, char const* argv[])
{
	clang::tooling::CommonOptionsParser op(argc,argv,MyToolCategory);

	SrcList.add(op.getSourcePathList());

	clang::tooling::ClangTool tool(op.getCompilations(),op.getSourcePathList());
	tool.run(clang::tooling::newFrontendActionFactory<FindNamedClassAction>().get());

	Record::printAsDot();

	return 0;
#ifdef TEST
	if(argc > 1) {
		std::cout << "run";
		clang::tooling::runToolOnCode(new FindNamedClassAction,argv[1]);
	}
	Record::printAsDot();
	std::cout << "exit";
	return 0;
#endif
}
