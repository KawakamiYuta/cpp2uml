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
#ifndef MYFRONTENDACTION_HPP
#define MYFRONTENDACTION_HPP #include <cstdio>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "ClassData.hpp"
#include "SrcFile.hpp"

#define VERBOSE 0
#if VERBOSE
#define TRACE() \
	do { \
		::printf("%s\n",__FUNCTION__);\
	} while (0)
#define DEBUGPRINT ::printf
#else
#define TRACE()
#define DEBUGPRINT(...)
#endif
using namespace clang;

extern class SrcList SrcList;

static bool isTargetFile(const std::string& fname)
{
	return SrcList.contain(fname);
}

class LocationFacade/*{{{*/
{
	private:
		std::string mLocStr;
		static const int FILENAME = 0;
		static const int LINE = 1;
		static const int COL = 2;
	public:
		LocationFacade(const SourceManager& aSM,const SourceLocation& aLoc)
			:mLocStr(aLoc.printToString(aSM)){ }
		const std::string getFileName() {
			return split(mLocStr,':')[0];
		}
};/*}}}*/

class QualTypeFacade
{
	private:
		const QualType mType;
	public:
		QualTypeFacade(const QualType& aType)
			:mType(aType){}
		std::string show() {
			const clang::Type* t = mType.getTypePtr();
			return (t->getPointeeCXXRecordDecl())?
				t->getPointeeCXXRecordDecl()->getNameAsString():"";
		}
};

static std::string _asToStr(clang::AccessSpecifier aAs) {
	switch (aAs) {
		case AS_public:
			return "public";
		case AS_protected:
			return "protected";
		case AS_private:
			return "private";
		default:
			return "";
	}
}

class CXXRecordDeclFacade
{
private:
	CXXRecordDecl* mCXXRecordDecl;
	PrintingPolicy mPolicy;
public:

	CXXRecordDeclFacade(CXXRecordDecl* aCXXRecordDecl,PrintingPolicy& aPolicy)
		: mCXXRecordDecl(aCXXRecordDecl),mPolicy(aPolicy) {}

	std::vector<std::string> showAllBaseClasses() {
		std::vector<std::string> ret;
		for (auto&& b : mCXXRecordDecl->bases()) {
			const clang::Type* t = b.getType().getTypePtr();
			std::string typeStr = t->getAsCXXRecordDecl()?
				t->getAsCXXRecordDecl()->getNameAsString():"error";
			ret.push_back(typeStr);
			::printf("base class:%s\n",ret.back().c_str());
		}
		return ret;
	}

	std::vector<FunctionDesc> getAllFunction() {
		std::vector<FunctionDesc> ret;
		for (auto&& m : mCXXRecordDecl->methods()) {
			FunctionDesc desc;
			desc.access = _asToStr(m->getAccess());
			desc.name = m->getNameAsString();
			desc.type = m->getReturnType().getAsString(mPolicy);
			ret.push_back(desc);
		}
		return ret;
	}
};

class FieldDeclFacade
{
private:
		FieldDecl* mFieldDecl;
public:
		FieldDeclFacade(FieldDecl* aFieldDecl)
			: mFieldDecl(aFieldDecl){}
		std::string showParent() {
			return mFieldDecl->getParent()->getNameAsString();
		}
		std::string showName() {
			return mFieldDecl->getNameAsString();
		}
#if 0
		std::string showType() {
			const clang::Type* t = _getType();
			return (t->getPointeeCXXRecordDecl())?
				t->getPointeeCXXRecordDecl()->getNameAsString():
				mFieldDecl->getType().getAsString(Policy);
		}
#endif
		const TemplateSpecializationType* getTemplateSpecializationType() {
			return _getType()->getAs<clang::TemplateSpecializationType>();
		}
private:
		const clang::Type* _getType() {
			return mFieldDecl->getType().getTypePtr();
		}
};

static std::string showDeclContents(clang::Decl* D)
{
	std::string buf;
	llvm::raw_string_ostream os(buf);
	D->print(os);
	return os.str();
}

static std::string showTemplateArgs(const clang::TemplateArgument* aTargs)
{
	std::string buf;
	llvm::raw_string_ostream os(buf);
	aTargs->dump(os);
	return os.str();
}

static std::string showType(const clang::Type* aType)
{
	std::string buf;
	llvm::raw_string_ostream os(buf);
	aType->dump(os);
	return os.str();
}

static bool isClass(clang::FieldDecl* aFieldDecl)
{
	const clang::Type* t = aFieldDecl->getType().getTypePtr();
	return (t != NULL) ?t->isClassType():false;
}

class FindNamedClassVisitor
:public clang::RecursiveASTVisitor<FindNamedClassVisitor>
{
	private:
		PrintingPolicy Policy;
		ASTContext* Context;
		const clang::SourceManager* SM;
	public:
		explicit FindNamedClassVisitor(clang::CompilerInstance* CI)/*{{{*/
			: Policy(PrintingPolicy(CI->getASTContext().getPrintingPolicy()))
			, Context(&CI->getASTContext())
			, SM(&CI->getSourceManager()){
					Policy.SuppressScope = true;
			}/*}}}*/

		void CollectAllClass(DeclContext* aDeclContext) {/*{{{*/
			for (DeclContext::decl_iterator i = aDeclContext->decls_begin(), e = aDeclContext->decls_end(); i != e; i++) {
				Decl* D = *i;
				LocationFacade loc(*SM,D->getLocation());

				if(!isTargetFile(loc.getFileName())) {
					continue;
				}

				::printf("TARGETFILE:%s\n",loc.getFileName().c_str());

				if (NamedDecl *N = dyn_cast<NamedDecl>(D)) {
					std::string kind = D->getDeclKindName();
					if(kind == "CXXRecord") {
						std::stringstream str;
						str << D->getDeclKindName() << " :"<< N->getNameAsString();
						DEBUGPRINT("%s\n",str.str().c_str());
						std::string name = N->getNameAsString();
						Record::maybeAddClass(name);

						CXXRecordDeclFacade CxxRecord =
						   	CXXRecordDeclFacade(dyn_cast<CXXRecordDecl>(D),Policy);

						for (auto&& b : CxxRecord.showAllBaseClasses()) {
							::printf("found base class\n");
							Record::maybeAddClass(b);
							Record::addBase(N->getNameAsString(),b);
						}

						for (auto&& m : CxxRecord.getAllFunction()) {
							Record::addMethod(name,m.name,m.type,m.access);
						}
					}
				}
			}
		}/*}}}*/

		void PrintAllClassLoc(DeclContext* aDeclContext) {/*{{{*/
			TRACE();

			for (DeclContext::decl_iterator i = aDeclContext->decls_begin(), e = aDeclContext->decls_end(); i != e; i++) {
				Decl* D = *i;
				LocationFacade loc(*SM,D->getLocation());

				if(!isTargetFile(loc.getFileName()))
					continue;

				if (NamedDecl *N = dyn_cast<NamedDecl>(D)) {
					std::string kind = D->getDeclKindName();
					if(kind == "CXXRecord") {
						std::stringstream str;
						str << D->getDeclKindName() << " :"<< N->getNameAsString();
						DEBUGPRINT("%s\n",str.str().c_str());
						TraverseDecl(D);
					}
				}
			}

		}/*}}}*/
		bool VisitCallExpr(CallExpr* aCallExpr) {
			::printf("callexpr");
			return true;
		}

		bool VisitCXXRecordDecl(CXXRecordDecl* Declaration) {/*{{{*/
			TRACE();
			//EnumerateDecl(Declaration);
			Record::maybeAddClass(Declaration->getNameAsString());

			return true;
		}/*}}}*/

		bool VisitFieldDecl(FieldDecl* aFieldDecl) {
			TRACE();
			FieldDeclFacade fd(aFieldDecl);

			std::string parent = fd.showParent();
			std::string name = fd.showName();

			std::string type;
			const clang::Type* t = aFieldDecl->getType().getTypePtr();
//			CXXRecordDecl *R = NULL;
//			if(t->getTypeClass() == clang::Type::Elaborated) {
//				R = dyn_cast<ElaboratedType>(t)->getNamedType()->getAsCXXRecordDecl();
//				type = R->getNameAsString();
//			}
			if(t->getPointeeCXXRecordDecl()) {
				type = t->getPointeeCXXRecordDecl()->getNameAsString();
			}
			else {
				type = aFieldDecl->getType().getAsString(Policy);
			}
			//std::string type = showType(aFieldDecl->getType().split().Ty);
			const clang::TemplateSpecializationType* tmpType = t->getAs<clang::TemplateSpecializationType>();

			// template type
			if(tmpType) {
				bool containTargetClass = false;
				if(int num = tmpType->getNumArgs()) {
					DEBUGPRINT("template type found:%s\n",type.c_str());
					const clang::TemplateArgument* args = tmpType->getArgs();
					for (int i = 0; i < num; i++) {
						DEBUGPRINT("template argument[%d]=%s\n",i,showTemplateArgs(&args[i]).c_str());
						if(Record::isTargetClassType(showTemplateArgs(&args[i]))) {
							Record::addField(parent,name,showTemplateArgs(&args[i]),"",
									true);
							containTargetClass = true;
						}
					}
				}
				if(not containTargetClass) {
					Record::addField(parent,name,type,"", false);
				}
			}

			else {
				Record::addField(parent,name,type,"",Record::isTargetClassType(type));
			}

			DEBUGPRINT("(parent,type,name,isClass)=%s,%s,%s,%s\n",
					parent.c_str(),
					type.c_str(),
					name.c_str(),
					(Record::isTargetClassType(type))?"true":"false");



			return true;
		}
};

class FindNamedClassConsumer
:public clang::ASTConsumer
{
	public:
		explicit FindNamedClassConsumer(clang::CompilerInstance* CI)
			: Visitor(CI){}
		virtual void HandleTranslationUnit(clang::ASTContext& Context)
		{
			TRACE();

			Visitor.CollectAllClass(Context.getTranslationUnitDecl());

			Visitor.PrintAllClassLoc(Context.getTranslationUnitDecl());
//			Visitor.TraverseDecl(Context.getTranslationUnitDecl());
//			Visitor.EnumerateDecl(Context.getTranslationUnitDecl());

		}
	private:
		FindNamedClassVisitor Visitor;
};

class FindNamedClassAction
:public clang::ASTFrontendAction
{
	public:
		virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
				clang::CompilerInstance &Compiler,llvm::StringRef InFile) {
			return std::unique_ptr<clang::ASTConsumer>(
					new FindNamedClassConsumer(&Compiler));
		}
};



#endif
